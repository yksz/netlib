#include "cx/net/tcp.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

namespace cx {

static unsigned long kBlockingMode = 0;
static unsigned long kNonBlockingMode = 1;

static void cleanup() {
    WSACleanup();
}

static error initializeOnce() {
    static bool s_initialized = false;
    if (s_initialized) {
        return error::nil;
    }

    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 0), &data) != 0) {
        fprintf(stderr, "ERROR: WSAStartup: %d\n", WSAGetLastError());
        return GetOSError(WSAGetLastError());
    } else {
        atexit(cleanup);
    }

    s_initialized = true;
    return error::nil;
}

error ConnectWithTCP(const char* host, int port, int timeout,
        std::shared_ptr<TCPSocket>* clientsock) {
    error initErr = initializeOnce();
    if (initErr != error::nil) {
        return initErr;
    }

    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET) {
        return GetOSError(WSAGetLastError());
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.S_un.S_addr = inet_addr(host);

    ioctlsocket(fd, FIONBIO, &kNonBlockingMode);
    if (connect(fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            closesocket(fd);
            return GetOSError(err);
        }
    }

    fd_set writefds;
    fd_set exceptfds;
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    FD_SET(fd, &writefds);
    FD_SET(fd, &exceptfds);

    struct timeval connTimeout;
    connTimeout.tv_sec = timeout / 1000;
    connTimeout.tv_usec = timeout % 1000 * 1000;

    int err = 0;
    int result = select(0, NULL, &writefds, &exceptfds, &connTimeout);
    if (result == SOCKET_ERROR) {
        err = WSAGetLastError();
        goto fail;
    } else if (result == 0) {
        err = WSAETIMEDOUT;
        goto fail;
    } else if (!FD_ISSET(fd, &exceptfds)) {
        err = 0;
    } else {
        int soErr = 0;
        int optlen = sizeof(soErr);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*) &soErr, &optlen);
        if (soErr != 0) {
            err = soErr;
            goto fail;
        }
    }
    if (err == 0) {
        ioctlsocket(fd, FIONBIO, &kBlockingMode);
        *clientsock = std::make_shared<TCPSocket>(fd, std::string(host));
        return error::nil;
    }

fail:
    closesocket(fd);
    return GetOSError(err);
}

error ListenWithTCP(int port, std::unique_ptr<TCPListener>* serversock) {
    initializeOnce();

    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET) {
        return GetOSError(WSAGetLastError());
    }

    BOOL soval = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*) &soval, sizeof(soval)) == SOCKET_ERROR) {
        goto fail;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        goto fail;
    }

    if (listen(fd, SOMAXCONN) == SOCKET_ERROR) {
        goto fail;
    } else {
        *serversock = std::unique_ptr<TCPListener>(new TCPListener(fd));
        return error::nil;
    }

fail:
    int err = WSAGetLastError();
    closesocket(fd);
    return GetOSError(err);
}

TCPSocket::~TCPSocket() {
    Close();
}

error TCPSocket::Close() {
    if (m_closed) {
        return error::nil;
    }

    if (closesocket(m_fd) == SOCKET_ERROR) {
        return GetOSError(WSAGetLastError());
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
    if (*nbytes == 0) {
        return error::eof;
    } else if (*nbytes == SOCKET_ERROR) {
        return GetOSError(WSAGetLastError());
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
    if (*nbytes == SOCKET_ERROR) {
        return GetOSError(WSAGetLastError());
    }
    return error::nil;
}

error TCPSocket::SetSocketTimeout(int timeout) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    int rcvResult = setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(timeout));
    if (rcvResult == SOCKET_ERROR) {
        return GetOSError(WSAGetLastError());
    }
    int sndResult = setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, (const char*) &timeout, sizeof(timeout));
    if (sndResult == SOCKET_ERROR) {
        return GetOSError(WSAGetLastError());
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

    if (closesocket(m_fd) == SOCKET_ERROR) {
        return GetOSError(WSAGetLastError());
    }
    m_closed = true;
    return error::nil;
}

bool TCPListener::IsClosed() {
    return m_closed;
}

error TCPListener::Accept(std::shared_ptr<TCPSocket>* clientsock) {
    struct sockaddr_in clientAddr;
    int len = sizeof(clientAddr);

    SOCKET sock = accept(m_fd, (struct sockaddr*) &clientAddr, &len);
    if (sock == INVALID_SOCKET) {
        return GetOSError(WSAGetLastError());
    }

    std::string host = inet_ntoa(clientAddr.sin_addr);
    *clientsock = std::make_shared<TCPSocket>(sock, std::move(host));
    return error::nil;
}

} // namespace cx
