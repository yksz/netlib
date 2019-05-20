#include "netlib/udp.h"
#include <cassert>
#include <cerrno>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "netlib/internal/init.h"
#include "netlib/resolver.h"

namespace net {

static void toTimeval(int64_t milliseconds, struct timeval* dest) {
    milliseconds = (milliseconds > 0) ? milliseconds : 0;
    dest->tv_sec = milliseconds / 1000;
    dest->tv_usec = milliseconds % 1000 * 1000;
}

error ConnectUDP(const std::string& host, uint16_t port, std::shared_ptr<UDPSocket>* clientSock) {
    if (clientSock == nullptr) {
        assert(0 && "clientSock must not be nullptr");
        return error::illegal_argument;
    }

    internal::init();

    std::string remoteAddr;
    error err = LookupAddress(host, &remoteAddr);
    if (err != error::nil) {
        return err;
    }

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        return error::wrap(etype::os, errno);
    }

    *clientSock = std::make_shared<UDPSocket>(fd, std::move(remoteAddr), port);
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
        return error::wrap(etype::os, errno);
    }

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
        int err = errno;
        close(fd);
        return error::wrap(etype::os, err);
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
        return error::wrap(etype::os, errno);
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
        return error::wrap(etype::os, errno);
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
    int size = recvfrom(m_fd, buf, len, 0, (struct sockaddr*) &from, &fromlen);
    if (size == -1) {
        return error::wrap(etype::os, errno);
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
        return error::wrap(etype::os, errno);
    }
    if (nbytes != nullptr) {
        *nbytes = size;
    }
    return error::nil;
}

error UDPSocket::SetTimeout(int64_t timeoutMilliseconds) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    struct timeval soTimeout;
    toTimeval(timeoutMilliseconds, &soTimeout);
    if (setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &soTimeout, sizeof(soTimeout)) == -1) {
        return error::wrap(etype::os, errno);
    }
    if (setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, &soTimeout, sizeof(soTimeout)) == -1) {
        return error::wrap(etype::os, errno);
    }
    return error::nil;
}

} // namespace net
