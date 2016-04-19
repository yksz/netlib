#include "cx/net/tcp.h"
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
#include "cx/net/init.h"
#include "cx/net/lookup.h"

namespace cx {

static const int kBlockingMode = 0;
static const int kNonBlockingMode = 1;

error ConnectWithTCP(const std::string& host, unsigned int port, int timeout,
        std::shared_ptr<TCPSocket>* clientSock) {
    internal::init();

    std::string ipAddr;
    error luErr = LookupAddress(host, &ipAddr);
    if (luErr != error::nil) {
        return luErr;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        return GetOSError(errno);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ipAddr.c_str());

    ioctl(fd, FIONBIO, &kNonBlockingMode);
    if (connect(fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
        if (errno != EINPROGRESS && errno != EALREADY && errno != EINTR) {
            int err = errno;
            close(fd);
            return GetOSError(err);
        }
    }

    struct timeval prev;
    gettimeofday(&prev, NULL);

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
        int result = select(fd + 1, NULL, &writefds, &exceptfds, &connTimeout);
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
        gettimeofday(&now, NULL);
        timeout -= (now.tv_sec - prev.tv_sec) * 1000 + (now.tv_usec - prev.tv_usec) / 1000;
        prev = now;
    }

fail:
    close(fd);
    return GetOSError(connErr);
}

error ListenWithTCP(unsigned int port, std::unique_ptr<TCPListener>* serverSock) {
    internal::init();

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        return GetOSError(errno);
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

    if (listen(fd, SOMAXCONN)) {
        goto fail;
    } else {
        *serverSock = std::unique_ptr<TCPListener>(new TCPListener(fd));
        return error::nil;
    }

fail:
    int err = errno;
    close(fd);
    return GetOSError(err);
}

TCPSocket::~TCPSocket() {
    Close();
}

error TCPSocket::Close() {
    if (m_closed) {
        return error::nil;
    }

    if (close(m_fd) == -1) {
        return GetOSError(errno);
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

    *nbytes = recv(m_fd, buf, len, 0);
    if (*nbytes == -1) {
        return GetOSError(errno);
    }
    if (*nbytes == 0) {
        return error::eof;
    } else {
        return error::nil;
    }
}

error TCPSocket::Write(const char* buf, size_t len, int* nbytes) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    *nbytes = send(m_fd, buf, len, 0);
    if (*nbytes == -1) {
        return GetOSError(errno);
    }
    return error::nil;
}

error TCPSocket::SetSocketTimeout(int timeout) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    struct timeval soTimeout;
    timeout = (timeout > 0) ? timeout : 0;
    soTimeout.tv_sec = timeout / 1000;
    soTimeout.tv_usec = timeout % 1000 * 1000;

    if (setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &soTimeout, sizeof(soTimeout)) == -1) {
        return GetOSError(errno);
    }
    if (setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, &soTimeout, sizeof(soTimeout)) == -1) {
        return GetOSError(errno);
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
        GetOSError(errno);
    }
    m_closed = true;
    return error::nil;
}

bool TCPListener::IsClosed() {
    return m_closed;
}

error TCPListener::Accept(std::shared_ptr<TCPSocket>* clientSock) {
    struct sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);

    int clientFD = accept(m_fd, (struct sockaddr*) &clientAddr, &len);
    if (clientFD == -1) {
        return GetOSError(errno);
    }

    *clientSock = std::make_shared<TCPSocket>(clientFD, inet_ntoa(clientAddr.sin_addr));
    return error::nil;
}

} // namespace cx
