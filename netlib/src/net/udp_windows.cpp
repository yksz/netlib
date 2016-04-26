#include "net/udp.h"
#include <cassert>
#include <winsock2.h>
#include "net/internal/init.h"
#include "net/resolver.h"

namespace net {

error ConnectUDP(const std::string& host, unsigned int port, std::shared_ptr<UDPSocket>* clientSock) {
    if (clientSock == nullptr) {
        assert(0 && "clientSock must not be nullptr");
        return error::illegal_argument;
    }

    internal::init();

    std::string ipAddr;
    error err = LookupAddress(host, &ipAddr);
    if (err != error::nil) {
        return err;
    }

    SOCKET fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == INVALID_SOCKET) {
        return toError(WSAGetLastError());
    }

    *clientSock = std::make_shared<UDPSocket>(fd, std::move(ipAddr), port);
    return error::nil;
}

error ListenUDP(unsigned int port, std::shared_ptr<UDPSocket>* serverSock) {
    if (serverSock == nullptr) {
        assert(0 && "serverSock must not be nullptr");
        return error::illegal_argument;
    }

    internal::init();

    SOCKET fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == INVALID_SOCKET) {
        return toError(WSAGetLastError());
    }

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
        int err = WSAGetLastError();
        closesocket(fd);
        return toError(err);
    }

    *serverSock = std::make_shared<UDPSocket>(fd);
    return error::nil;
}

UDPSocket::~UDPSocket() {
    Close();
}

error UDPSocket::Close() {
    if (m_closed) {
        return error::nil;
    }

    if (closesocket(m_fd) == -1) {
        return toError(WSAGetLastError());
    }
    m_closed = true;
    return error::nil;
}

bool UDPSocket::IsClosed() {
    return m_closed;
}

error UDPSocket::Read(char* buf, size_t len, int* nbytes) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    int size = recv(m_fd, buf, len, 0);
    if (size == -1) {
        return toError(WSAGetLastError());
    }
    if (size == 0) {
        return error::eof;
    }
    if (nbytes != nullptr) {
        *nbytes = size;
    }
    return error::nil;
}

error UDPSocket::ReadFrom(char* buf, size_t len, int* nbytes,
            std::string* addr, unsigned int* port) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    struct sockaddr_in from = {0};
    int fromlen = sizeof(from);
    int size = recvfrom(m_fd, buf, len, 0, (struct sockaddr*) &from, &fromlen);
    if (size == -1) {
        return toError(WSAGetLastError());
    }
    *addr = inet_ntoa(from.sin_addr);
    *port = ntohs(from.sin_port);
    if (size == 0) {
        return error::eof;
    }
    if (nbytes != nullptr) {
        *nbytes = size;
    }
    return error::nil;
}

error UDPSocket::Write(const char* buf, size_t len, int* nbytes) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }
    if (m_remoteAddr.empty() && m_remotePort == 0) {
        return error::illegal_state;
    }

    return WriteTo(buf, len, m_remoteAddr, m_remotePort, nbytes);
}

error UDPSocket::WriteTo(const char* buf, size_t len,
        const std::string& addr, unsigned int port, int* nbytes) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    struct sockaddr_in to = {0};
    to.sin_family = AF_INET;
    to.sin_addr.S_un.S_addr = inet_addr(addr.c_str());
    to.sin_port = htons(port);
    int size = sendto(m_fd, buf, len, 0, (struct sockaddr*) &to, sizeof(to));
    if (size == -1) {
        return toError(WSAGetLastError());
    }
    if (nbytes != nullptr) {
        *nbytes = size;
    }
    return error::nil;
}

error UDPSocket::SetSocketTimeout(int timeout) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    timeout = (timeout > 0) ? timeout : 0;

    if (setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(timeout)) == SOCKET_ERROR) {
        return toError(WSAGetLastError());
    }
    if (setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, (const char*) &timeout, sizeof(timeout)) == SOCKET_ERROR) {
        return toError(WSAGetLastError());
    }
    return error::nil;
}

} // namespace net
