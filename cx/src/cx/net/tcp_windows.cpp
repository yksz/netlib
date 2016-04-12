#include "cx/net/tcp_windows.h"
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

static void initOnce() {
    static bool s_initialized = false;
    if (s_initialized) {
        return;
    }

    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 0), &data) != 0) {
        fprintf(stderr, "ERROR: WSAStartup: %d\n", WSAGetLastError());
    } else {
        atexit(cleanup);
    }

    s_initialized = true;
}

error ConnectWithTCP(const char* host, int port, int timeout,
        std::shared_ptr<TCPSocket>* clientsock) {
    initOnce();

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        return GetOSError(WSAGetLastError());
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.S_un.S_addr = inet_addr(host);

    int err = 0;

    ioctlsocket(sock, FIONBIO, &kNonBlockingMode);
    if (connect(sock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            goto fail;
        }
    }

    fd_set writefds;
    fd_set exceptfds;
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    FD_SET(sock, &writefds);
    FD_SET(sock, &exceptfds);

    struct timeval connTimeout;
    connTimeout.tv_sec = timeout / 1000;
    connTimeout.tv_usec = timeout % 1000 * 1000;

    int result = select(0, NULL, &writefds, &exceptfds, &connTimeout);
    if (result == SOCKET_ERROR) {
        err = WSAGetLastError();
        goto fail;
    } else if (result == 0) {
        err = WSAETIMEDOUT;
        goto fail;
    } else if (!FD_ISSET(sock, &exceptfds)) {
        err = 0;
    } else {
        int soerr = 0;
        int optlen = sizeof(soerr);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*) &soerr, &optlen);
        if (soerr != 0) {
            err = soerr;
            goto fail;
        }
    }
    if (err == 0) {
        ioctlsocket(sock, FIONBIO, &kBlockingMode);
        *clientsock = std::make_shared<WindowsTCPSocket>(sock, std::string(host));
        return error::nil;
    }

fail:
    closesocket(sock);
    return GetOSError(err);
}

error ListenWithTCP(int port, std::unique_ptr<TCPListener>* serversock) {
    initOnce();

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        return GetOSError(WSAGetLastError());
    }

    BOOL soval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*) &soval, sizeof(soval)) == SOCKET_ERROR) {
        goto fail;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        goto fail;
    }

    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
        goto fail;
    } else {
        *serversock = std::unique_ptr<WindowsTCPListener>(new WindowsTCPListener(sock));
        return error::nil;
    }

fail:
    int err = WSAGetLastError();
    closesocket(sock);
    return GetOSError(err);
}

WindowsTCPSocket::~WindowsTCPSocket() {
    Close();
}

error WindowsTCPSocket::Close() {
    if (m_closed) {
        return error::nil;
    }

    if (closesocket(m_sock) == SOCKET_ERROR) {
        return GetOSError(WSAGetLastError());
    }
    m_closed = true;
    return error::nil;
}

bool WindowsTCPSocket::IsClosed() {
    return m_closed;
}

error WindowsTCPSocket::Read(char* buf, size_t len, int* nbytes) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    *nbytes = recv(m_sock, buf, len, 0);
    if (*nbytes == 0) {
        return error::eof;
    } else if (*nbytes == SOCKET_ERROR) {
        return GetOSError(WSAGetLastError());
    } else {
        return error::nil;
    }
}

error WindowsTCPSocket::Write(const char* buf, size_t len, int* nbytes) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    *nbytes = send(m_sock, buf, len, 0);
    if (*nbytes == SOCKET_ERROR) {
        return GetOSError(WSAGetLastError());
    }
    return error::nil;
}

error WindowsTCPSocket::SetSocketTimeout(int timeout) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    int rcvResult = setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(timeout));
    if (rcvResult == SOCKET_ERROR) {
        return GetOSError(WSAGetLastError());
    }
    int sndResult = setsockopt(m_sock, SOL_SOCKET, SO_SNDTIMEO, (const char*) &timeout, sizeof(timeout));
    if (sndResult == SOCKET_ERROR) {
        return GetOSError(WSAGetLastError());
    }
    return error::nil;
}

WindowsTCPListener::~WindowsTCPListener() {
    Close();
}

error WindowsTCPListener::Close() {
    if (m_closed) {
        return error::nil;
    }

    if (closesocket(m_sock) == SOCKET_ERROR) {
        return GetOSError(WSAGetLastError());
    }
    m_closed = true;
    return error::nil;
}

bool WindowsTCPListener::IsClosed() {
    return m_closed;
}

error WindowsTCPListener::Accept(std::shared_ptr<TCPSocket>* clientsock) {
    struct sockaddr_in clientAddr;
    int len = sizeof(clientAddr);

    SOCKET sock = accept(m_sock, (struct sockaddr*) &clientAddr, &len);
    if (sock == INVALID_SOCKET) {
        return GetOSError(WSAGetLastError());
    }

    std::string host = inet_ntoa(clientAddr.sin_addr);
    *clientsock = std::make_shared<WindowsTCPSocket>(sock, std::move(host));
    return error::nil;
}

} // namespace cx
