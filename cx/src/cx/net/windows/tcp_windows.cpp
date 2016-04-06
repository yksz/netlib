#include "cx/net/windows/tcp_windows.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
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

std::shared_ptr<TCPSocket> ConnectWithTCP(const char* host, int port, int timeout) {
    initOnce();

    SOCKET sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "ERROR: socket: %d\n", WSAGetLastError());
        return nullptr;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.S_un.S_addr = inet_addr(host);

    ioctlsocket(sock, FIONBIO, &kNonBlockingMode);
    if (connect(sock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "ERROR: connect: %d\n", WSAGetLastError());
        goto fail;
    }

    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sock, &writefds);

    struct timeval connTimeout;
    connTimeout.tv_sec = timeout / 1000;
    connTimeout.tv_usec = timeout % 1000 * 1000;

    int result = select(0, NULL, &writefds, NULL, &connTimeout);
    if (result == SOCKET_ERROR) {
        fprintf(stderr, "ERROR: select: %d\n", WSAGetLastError());
        goto fail;
    } else if (FD_ISSET(sock, &writefds)) {
        ioctlsocket(sock, FIONBIO, &kBlockingMode);
        return std::make_shared<WindowsTCPSocket>(sock);
    } else {
        fprintf(stderr, "connect: timeout\n");
        goto fail;
    }

fail:
    closesocket(sock);
    return nullptr;
}

std::unique_ptr<TCPListener> ListenWithTCP(int port) {
    initOnce();

    SOCKET sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "ERROR: socket: %d\n", WSAGetLastError());
        return nullptr;
    }

    BOOL soval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*) &soval, sizeof(soval));

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "ERROR: bind: %d\n", WSAGetLastError());
        return nullptr;
    }

    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "ERROR: listen: %d\n", WSAGetLastError());
        return nullptr;
    }

    printf("Listening on port %d\n", port);
    return std::unique_ptr<WindowsTCPListener>(new WindowsTCPListener(sock));
}


WindowsTCPSocket::~WindowsTCPSocket() {
    Close();
}

void WindowsTCPSocket::Close() {
    if (m_closed) {
        return;
    }

    int result = closesocket(m_sock);
    if (result == -1) {
        fprintf(stderr, "ERROR: close: %d\n", WSAGetLastError());
    }
    m_closed = true;
}

bool WindowsTCPSocket::IsClosed() {
    return m_closed;
}

int WindowsTCPSocket::Read(char* buf, size_t len) {
    if (m_closed) {
        assert(0 && "Already closed");
        return -1;
    }

    int result = recv(m_sock, buf, len, 0);
    if (result == -1) {
        fprintf(stderr, "ERROR: recv: %d\n", WSAGetLastError());
    }
    return result;
}

int WindowsTCPSocket::Write(const char* buf, size_t len) {
    if (m_closed) {
        assert(0 && "Already closed");
        return -1;
    }

    int result = send(m_sock, buf, len, 0);
    if (result == -1) {
        fprintf(stderr, "ERROR: send: %d\n", WSAGetLastError());
    }
    return result;
}

bool WindowsTCPSocket::SetSocketTimeout(int timeout) {
    if (m_closed) {
        assert(0 && "Already closed");
        return false;
    }

    int rcvResult = setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(timeout));
    if (rcvResult == -1) {
        fprintf(stderr, "ERROR: setsockopt: %d\n", WSAGetLastError());
        return false;
    }
    int sndResult = setsockopt(m_sock, SOL_SOCKET, SO_SNDTIMEO, (const char*) &timeout, sizeof(timeout));
    if (sndResult == -1) {
        fprintf(stderr, "ERROR: setsockopt: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}


WindowsTCPListener::~WindowsTCPListener() {
    Close();
}

void WindowsTCPListener::Close() {
    if (m_closed) {
        return;
    }

    int result = closesocket(m_sock);
    if (result == -1) {
        fprintf(stderr, "ERROR: close: %d\n", WSAGetLastError());
    }
    m_closed = true;
}

bool WindowsTCPListener::IsClosed() {
    return m_closed;
}

std::shared_ptr<TCPSocket> WindowsTCPListener::Accept() {
    struct sockaddr_in clientAddr;
    int len;
    SOCKET clientsock;

    len = sizeof(clientAddr);
    clientsock = accept(m_sock, (struct sockaddr*) &clientAddr, &len);
    if (clientsock == INVALID_SOCKET) {
        fprintf(stderr, "ERROR: accept: %d\n", WSAGetLastError());
        return nullptr;
    }

    printf("%s connected\n", inet_ntoa(clientAddr.sin_addr));
    return std::make_shared<WindowsTCPSocket>(clientsock);
}

} // namespace cx
