#include "net/udp.h"
#include <cassert>
#include <cerrno>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "net/internal/init.h"
#include "net/resolver.h"

namespace net {

error ConnectUDP(const std::string& host, uint16_t port, std::shared_ptr<UDPSocket>* clientSock) {
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

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        return toError(errno);
    }

    if (clientSock != nullptr) {
        *clientSock = std::make_shared<UDPSocket>(fd, std::move(ipAddr), port);
    }
    return error::nil;
}

error ListenUDP(uint16_t port, std::shared_ptr<UDPSocket>* serverSock) {
    if (serverSock == nullptr) {
        assert(0 && "serverSock must not be nullptr");
        return error::illegal_argument;
    }

    internal::init();

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        return toError(errno);
    }

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
        int err = errno;
        close(fd);
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

    if (close(m_fd) == -1) {
        return toError(errno);
    }
    m_closed = true;
    return error::nil;
}

error UDPSocket::Read(char* buf, size_t len, int* nbytes) {
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

error UDPSocket::ReadFrom(char* buf, size_t len, int* nbytes,
            std::string* addr, uint16_t* port) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    struct sockaddr_in from = {0};
    socklen_t fromlen = sizeof(from);
    int size = recvfrom(m_fd, buf, len, 0, (struct sockaddr *) &from, &fromlen);
    if (size == -1) {
        return toError(errno);
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
        const std::string& addr, uint16_t port, int* nbytes) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    struct sockaddr_in to = {0};
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = inet_addr(addr.c_str());
    to.sin_port = htons(port);
    int size = sendto(m_fd, buf, len, 0, (struct sockaddr*) &to, sizeof(to));
    if (size == -1) {
        return toError(errno);
    }
    if (nbytes != nullptr) {
        *nbytes = size;
    }
    return error::nil;
}

error UDPSocket::SetSocketTimeout(int64_t timeoutMilliseconds) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    struct timeval soTimeout;
    timeoutMilliseconds = (timeoutMilliseconds > 0) ? timeoutMilliseconds : 0;
    soTimeout.tv_sec = timeoutMilliseconds / 1000;
    soTimeout.tv_usec = timeoutMilliseconds % 1000 * 1000;

    if (setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &soTimeout, sizeof(soTimeout)) == -1) {
        return toError(errno);
    }
    if (setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, &soTimeout, sizeof(soTimeout)) == -1) {
        return toError(errno);
    }
    return error::nil;
}

} // namespace net
