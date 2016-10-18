#include "net/tcp.h"
#include <cassert>
#include <cerrno>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "net/internal/init.h"
#include "net/resolver.h"

namespace net {

static const int kBlockingMode = 0;
static const int kNonBlockingMode = 1;

error ConnectTCP(const std::string& host, uint16_t port, int64_t timeout,
        std::shared_ptr<TCPSocket>* clientSock) {
    if (clientSock == nullptr) {
        assert(0 && "clientSock must not be nullptr");
        return error::illegal_argument;
    }

    internal::init();

    std::string ipAddr;
    error luErr = LookupAddress(host, &ipAddr);
    if (luErr != error::nil) {
        return luErr;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        return toError(errno);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ipAddr.c_str());

    if (timeout <= 0) { // connect in blocking mode
        if (connect(fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
            int err = errno;
            close(fd);
            return toError(err);
        }
        *clientSock = std::make_shared<TCPSocket>(fd, ipAddr);
        return error::nil;
    }

    ioctl(fd, FIONBIO, &kNonBlockingMode);
    if (connect(fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
        if (errno != EINPROGRESS && errno != EALREADY && errno != EINTR) {
            int err = errno;
            close(fd);
            return toError(err);
        }
    }

    struct timeval prev;
    gettimeofday(&prev, nullptr);

    int connErr = 0;
    while (true) {
        fd_set writefds, exceptfds;
        FD_ZERO(&writefds);
        FD_ZERO(&exceptfds);
        FD_SET(fd, &writefds);
        FD_SET(fd, &exceptfds);

        struct timeval connTimeout;
        timeout = (timeout > 0) ? timeout : 0;
        connTimeout.tv_sec = timeout / 1000;
        connTimeout.tv_usec = timeout % 1000 * 1000;

        errno = 0;
        int result = select(fd + 1, nullptr, &writefds, &exceptfds, &connTimeout);
        if (result == -1) {
            connErr = errno;
            goto fail;
        } else if (result == 0) {
            connErr = ETIMEDOUT;
            goto fail;
        } else {
            int soErr;
            socklen_t optlen = sizeof(soErr);
            getsockopt(fd, SOL_SOCKET, SO_ERROR, &soErr, &optlen);
            if (soErr == EINPROGRESS || soErr == EALREADY || soErr == EINTR) {
                continue;
            } else if (soErr == 0 || soErr == EISCONN) {
                ioctl(fd, FIONBIO, &kBlockingMode);
                *clientSock = std::make_shared<TCPSocket>(fd, ipAddr);
                return error::nil;
            } else {
                connErr = soErr;
                goto fail;
            }
        }

        struct timeval now;
        gettimeofday(&now, nullptr);
        timeout -= (now.tv_sec - prev.tv_sec) * 1000 + (now.tv_usec - prev.tv_usec) / 1000;
        prev = now;
    }

fail:
    close(fd);
    return toError(connErr);
}

error ListenTCP(uint16_t port, std::unique_ptr<TCPListener>* serverSock) {
    if (serverSock == nullptr) {
        assert(0 && "serverSock must not be nullptr");
        return error::illegal_argument;
    }

    internal::init();

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        return toError(errno);
    }

    int enabled = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)) == -1) {
        goto fail;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
        goto fail;
    }

    if (listen(fd, SOMAXCONN) == -1) {
        goto fail;
    } else {
        *serverSock = std::unique_ptr<TCPListener>(new TCPListener(fd));
        return error::nil;
    }

fail:
    int err = errno;
    close(fd);
    return toError(err);
}

TCPSocket::~TCPSocket() {
    Close();
}

error TCPSocket::Close() {
    if (m_closed) {
        return error::nil;
    }

    if (close(m_fd) == -1) {
        return toError(errno);
    }
    m_closed = true;
    return error::nil;
}

bool TCPSocket::IsClosed() {
    return m_closed;
}

error TCPSocket::Read(char* buf, size_t len, int* nbytes) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    int size = recv(m_fd, buf, len, 0);
    if (size == -1) {
        return toError(errno);
    }
    if (size == 0) {
        return error::eof;
    }
    if (nbytes != nullptr) {
        *nbytes = size;
    }
    return error::nil;
}

error TCPSocket::Write(const char* buf, size_t len, int* nbytes) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    int size = send(m_fd, buf, len, 0);
    if (size == -1) {
        return toError(errno);
    }
    if (nbytes != nullptr) {
        *nbytes = size;
    }
    return error::nil;
}

error TCPSocket::SetSocketTimeout(int64_t timeout) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    struct timeval soTimeout;
    timeout = (timeout > 0) ? timeout : 0;
    soTimeout.tv_sec = timeout / 1000;
    soTimeout.tv_usec = timeout % 1000 * 1000;

    if (setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &soTimeout, sizeof(soTimeout)) == -1) {
        return toError(errno);
    }
    if (setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, &soTimeout, sizeof(soTimeout)) == -1) {
        return toError(errno);
    }
    return error::nil;
}

TCPListener::~TCPListener() {
    Close();
}

error TCPListener::Close() {
    if (m_closed) {
        return error::nil;
    }

    if (close(m_fd) == -1) {
        return toError(errno);
    }
    m_closed = true;
    return error::nil;
}

bool TCPListener::IsClosed() {
    return m_closed;
}

error TCPListener::Accept(std::shared_ptr<TCPSocket>* clientSock) {
    if (clientSock == nullptr) {
        assert(0 && "clientSock must not be nullptr");
        return error::illegal_argument;
    }

    struct sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);

    int clientFD = accept(m_fd, (struct sockaddr*) &clientAddr, &len);
    if (clientFD == -1) {
        return toError(errno);
    }

    *clientSock = std::make_shared<TCPSocket>(clientFD, inet_ntoa(clientAddr.sin_addr));
    return error::nil;
}

} // namespace net
