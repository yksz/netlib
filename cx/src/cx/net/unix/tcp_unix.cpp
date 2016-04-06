#include "cx/net/unix/tcp_unix.h"
#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstdio>
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

std::shared_ptr<TCPSocket> ConnectWithTCP(const char* host, int port, int timeout) {
    initOnce();

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return nullptr;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(host);

    ioctl(sockfd, FIONBIO, &kNonBlockingMode);
    if (connect(sockfd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
        if (errno == EINPROGRESS || errno == EALREADY || errno == EINTR) {
        } else {
            perror("connect");
            goto fail;
        }
    }

    while (true) {
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(sockfd, &writefds);

        struct timeval connTimeout;
        connTimeout.tv_sec = timeout / 1000;
        connTimeout.tv_usec = timeout % 1000 * 1000;

        errno = 0;
        int result = select(sockfd + 1, NULL, &writefds, NULL, &connTimeout);
        if (result == -1) {
            perror("select");
            goto fail;
        } else if (FD_ISSET(sockfd, &writefds)) {
            int soerr;
            socklen_t len = sizeof(soerr);
            getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &soerr, &len);
            if (soerr != 0) {
                fprintf(stderr, "getsockopt: %s\n", strerror(soerr));
                goto fail;
            }
            if (errno == EINPROGRESS || errno == EALREADY || errno == EINTR) {
                continue;
            } else if (errno == 0 || errno == EISCONN) {
                ioctl(sockfd, FIONBIO, &kBlockingMode);
                return std::make_shared<UnixTCPSocket>(sockfd);
            } else {
                goto fail;
            }
        } else {
            fprintf(stderr, "connect: timeout\n");
            goto fail;
        }
    }

fail:
    close(sockfd);
    return nullptr;
}

std::unique_ptr<TCPListener> ListenWithTCP(int port) {
    initOnce();

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return nullptr;
    }

    int soval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &soval, sizeof(soval)) == -1) {
        perror("setsockopt");
        return nullptr;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
        perror("bind");
        return nullptr;
    }

    if (listen(sockfd, SOMAXCONN)) {
        perror("listen");
        return nullptr;
    }

    printf("Listening on port %d\n", port);
    return std::unique_ptr<UnixTCPListener>(new UnixTCPListener(sockfd));
}


UnixTCPSocket::~UnixTCPSocket() {
    Close();
}

void UnixTCPSocket::Close() {
    if (m_closed) {
        return;
    }

    int result = close(m_sockfd);
    if (result == -1) {
        perror("close");
    }
    m_closed = true;
}

bool UnixTCPSocket::IsClosed() {
    return m_closed;
}

int UnixTCPSocket::Read(char* buf, size_t len) {
    if (m_closed) {
        assert(0 && "Already closed");
        return -1;
    }

    int result = recv(m_sockfd, buf, len, 0);
    if (result == -1) {
        perror("recv");
    }
    return result;
}

int UnixTCPSocket::Write(const char* buf, size_t len) {
    if (m_closed) {
        assert(0 && "Already closed");
        return -1;
    }

    int result = send(m_sockfd, buf, len, 0);
    if (result == -1) {
        perror("send");
    }
    return result;
}

bool UnixTCPSocket::SetSocketTimeout(int timeout) {
    if (m_closed) {
        assert(0 && "Already closed");
        return false;
    }

    struct timeval soTimeout;
    soTimeout.tv_sec = timeout / 1000;
    soTimeout.tv_usec = timeout % 1000 * 1000;

    int rcvResult = setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, &soTimeout, sizeof(soTimeout));
    if (rcvResult == -1) {
        perror("setsockopt");
        return false;
    }
    int sndResult = setsockopt(m_sockfd, SOL_SOCKET, SO_SNDTIMEO, &soTimeout, sizeof(soTimeout));
    if (sndResult == -1) {
        perror("setsockopt");
        return false;
    }
    return true;
}


UnixTCPListener::~UnixTCPListener() {
    Close();
}

void UnixTCPListener::Close() {
    if (m_closed) {
        return;
    }

    int result = close(m_sockfd);
    if (result == -1) {
        perror("close");
    }
    m_closed = true;
}

bool UnixTCPListener::IsClosed() {
    return m_closed;
}

std::shared_ptr<TCPSocket> UnixTCPListener::Accept() {
    struct sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);

    int clientfd = accept(m_sockfd, (struct sockaddr*) &clientAddr, &len);
    if (clientfd == -1) {
        perror("accept");
        return nullptr;
    }

    printf("%s connected\n", inet_ntoa(clientAddr.sin_addr));
    return std::make_shared<UnixTCPSocket>(clientfd);
}

} // namespace cx
