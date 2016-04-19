#include "cx/net/udp.h"
#include <cassert>
#include <cerrno>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "cx/net/init.h"
#include "cx/net/lookup.h"

namespace cx {

error ConnectUDP(const std::string& host, unsigned int port,
        std::shared_ptr<UDPSocket>* clientSock) {
    internal::init();

    std::string ipAddr;
    error err = LookupAddress(host, &ipAddr);
    if (err != error::nil) {
        return err;
    }

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        return GetOSError(errno);
    }

    *clientSock = std::make_shared<UDPSocket>(fd, std::move(ipAddr), port);
    return error::nil;
}

error ListenUDP(unsigned int port, std::shared_ptr<UDPSocket>* serverSock) {
    internal::init();

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        return GetOSError(errno);
    }

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
        int err = errno;
        close(fd);
        return GetOSError(err);
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
        return GetOSError(errno);
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

error UDPSocket::ReadFrom(char* buf, size_t len, int* nbytes,
            std::string* addr, unsigned int* port) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    struct sockaddr_in from = {0};
    socklen_t fromlen = sizeof(from);
    *nbytes = recvfrom(m_fd, buf, len, 0, (struct sockaddr *) &from, &fromlen);
    if (*nbytes == -1) {
        return GetOSError(errno);
    }
    *addr = inet_ntoa(from.sin_addr);
    *port = ntohs(from.sin_port);
    if (*nbytes == 0) {
        return error::eof;
    } else {
        return error::nil;
    }
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
    to.sin_addr.s_addr = inet_addr(addr.c_str());
    to.sin_port = htons(port);
    *nbytes = sendto(m_fd, buf, len, 0, (struct sockaddr*) &to, sizeof(to));
    if (*nbytes == -1) {
        return GetOSError(errno);
    }
    return error::nil;
}

error UDPSocket::SetSocketTimeout(int timeout) {
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

} // namespace cx
