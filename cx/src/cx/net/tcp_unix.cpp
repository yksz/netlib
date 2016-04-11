#include "cx/net/tcp_unix.h"
#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace cx {

static const int kBlockingMode = 0;
static const int kNonBlockingMode = 1;

static void initOnce() {
    static bool s_initialized = false;
    if (s_initialized) {
        return;
    }

    signal(SIGPIPE, SIG_IGN);

    s_initialized = true;
}

error ConnectWithTCP(const char* host, int port, int timeout,
        std::shared_ptr<TCPSocket>* clientsock) {
    initOnce();

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        return GetOSError(errno);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(host);

    int err = 0;

    ioctl(sockfd, FIONBIO, &kNonBlockingMode);
    if (connect(sockfd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
        if (errno == EINPROGRESS || errno == EALREADY || errno == EINTR) {
        } else {
            err = errno;
            goto fail;
        }
    }

    while (true) {
        fd_set writefds, exceptfds;
        FD_ZERO(&writefds);
        FD_ZERO(&exceptfds);
        FD_SET(sockfd, &writefds);
        FD_SET(sockfd, &exceptfds);

        struct timeval connTimeout;
        connTimeout.tv_sec = timeout / 1000;
        connTimeout.tv_usec = timeout % 1000 * 1000;

        errno = 0;
        int result = select(sockfd + 1, NULL, &writefds, &exceptfds, &connTimeout);
        if (result == -1) {
            err = errno;
            goto fail;
        } else if (result == 0) {
            err = ETIMEDOUT;
            goto fail;
        } else {
            int soerr;
            socklen_t optlen = sizeof(soerr);
            getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &soerr, &optlen);
            if (soerr == EINPROGRESS || soerr == EALREADY || soerr == EINTR) {
                continue;
            } else if (soerr == 0 || soerr == EISCONN) {
                ioctl(sockfd, FIONBIO, &kBlockingMode);
                *clientsock = std::make_shared<UnixTCPSocket>(sockfd, std::string(host));
                return error::nil;
            } else {
                err = soerr;
                goto fail;
            }
        }
    }

fail:
    close(sockfd);
    return GetOSError(err);
}

error ListenWithTCP(int port, std::unique_ptr<TCPListener>* serversock) {
    initOnce();

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        return GetOSError(errno);
    }

    int soval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &soval, sizeof(soval)) == -1) {
        goto fail;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
        goto fail;
    }

    if (listen(sockfd, SOMAXCONN)) {
        goto fail;
    } else {
        *serversock = std::unique_ptr<UnixTCPListener>(new UnixTCPListener(sockfd));
        return error::nil;
    }

fail:
    int err = errno;
    close(sockfd);
    return GetOSError(err);
}

UnixTCPSocket::~UnixTCPSocket() {
    Close();
}

error UnixTCPSocket::Close() {
    if (m_closed) {
        return error::nil;
    }

    if (close(m_sockfd) == -1) {
        return GetOSError(errno);
    }
    m_closed = true;
    return error::nil;
}

bool UnixTCPSocket::IsClosed() {
    return m_closed;
}

error UnixTCPSocket::Read(char* buf, size_t len, int* nbytes) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    *nbytes = recv(m_sockfd, buf, len, 0);
    if (*nbytes == 0) {
        return error::eof;
    } else if (*nbytes == -1) {
        return GetOSError(errno);
    } else {
        return error::nil;
    }
}

error UnixTCPSocket::Write(const char* buf, size_t len, int* nbytes) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    *nbytes = send(m_sockfd, buf, len, 0);
    if (*nbytes == -1) {
        return GetOSError(errno);
    }
    return error::nil;
}

error UnixTCPSocket::SetSocketTimeout(int timeout) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    struct timeval soTimeout;
    soTimeout.tv_sec = timeout / 1000;
    soTimeout.tv_usec = timeout % 1000 * 1000;

    int rcvResult = setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, &soTimeout, sizeof(soTimeout));
    if (rcvResult == -1) {
        return GetOSError(errno);
    }
    int sndResult = setsockopt(m_sockfd, SOL_SOCKET, SO_SNDTIMEO, &soTimeout, sizeof(soTimeout));
    if (sndResult == -1) {
        return GetOSError(errno);
    }
    return error::nil;
}

UnixTCPListener::~UnixTCPListener() {
    Close();
}

error UnixTCPListener::Close() {
    if (m_closed) {
        return error::nil;
    }

    if (close(m_sockfd) == -1) {
        GetOSError(errno);
    }
    m_closed = true;
    return error::nil;
}

bool UnixTCPListener::IsClosed() {
    return m_closed;
}

error UnixTCPListener::Accept(std::shared_ptr<TCPSocket>* clientsock) {
    struct sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);

    int clientfd = accept(m_sockfd, (struct sockaddr*) &clientAddr, &len);
    if (clientfd == -1) {
        return GetOSError(errno);
    }

    std::string host = inet_ntoa(clientAddr.sin_addr);
    *clientsock = std::make_shared<UnixTCPSocket>(clientfd, std::move(host));
    return error::nil;
}

} // namespace cx
