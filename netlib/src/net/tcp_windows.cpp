#include "net/tcp.h"
#include <cassert>
#include <mstcpip.h>
#include <winsock2.h>
#include "net/internal/init.h"
#include "net/resolver.h"

namespace net {

static unsigned long kBlockingMode = 0;
static unsigned long kNonBlockingMode = 1;

static void toTimeval(int64_t milliseconds, struct timeval* dest) {
    milliseconds = (milliseconds > 0) ? milliseconds : 0;
    dest->tv_sec = milliseconds / 1000;
    dest->tv_usec = milliseconds % 1000 * 1000;
}

static error waitUntilReady(const SocketFD& fd,
        fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
        int64_t timeoutMilliseconds) {
    if (readfds != nullptr) {
        FD_ZERO(readfds);
        FD_SET(fd, readfds);
    }
    if (writefds != nullptr) {
        FD_ZERO(writefds);
        FD_SET(fd, writefds);
    }
    if (exceptfds != nullptr) {
        FD_ZERO(exceptfds);
        FD_SET(fd, exceptfds);
    }
    struct timeval timeout;
    toTimeval(timeoutMilliseconds, &timeout);

    int result = select(0, readfds, writefds, exceptfds, &timeout);
    if (result == SOCKET_ERROR) {
        return error::wrap(etype::os, WSAGetLastError());
    } else if (result == 0) {
        return error::timedout;
    } else {
        int soErr = 0;
        int optlen = sizeof(soErr);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*) &soErr, &optlen);
        if (soErr != 0) {
            return error::wrap(etype::os, soErr);
        }
        return error::nil;
    }
}

error ConnectTCP(const std::string& host, uint16_t port, int64_t timeoutMilliseconds,
        std::shared_ptr<TCPSocket>* clientSock) {
    if (clientSock == nullptr) {
        assert(0 && "clientSock must not be nullptr");
        return error::illegal_argument;
    }

    internal::init();

    std::string remoteAddr;
    error addrErr = LookupAddress(host, &remoteAddr);
    if (addrErr != error::nil) {
        return addrErr;
    }

    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET) {
        return error::wrap(etype::os, WSAGetLastError());
    }

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.S_un.S_addr = inet_addr(remoteAddr.c_str());

    if (timeoutMilliseconds <= 0) { // connect in blocking mode
        if (connect(fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            int connErr = WSAGetLastError();
            closesocket(fd);
            return error::wrap(etype::os, connErr);
        }
    } else { // connect in non blocking mode
        ioctlsocket(fd, FIONBIO, &kNonBlockingMode);
        if (connect(fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            int connErr = WSAGetLastError();
            if (connErr != WSAEWOULDBLOCK) {
                closesocket(fd);
                return error::wrap(etype::os, connErr);
            }
        }
        fd_set writefds, exceptfds;
        error err = waitUntilReady(fd, nullptr, &writefds, &exceptfds, timeoutMilliseconds);
        if (err != error::nil) {
            closesocket(fd);
            return err;
        }
        ioctlsocket(fd, FIONBIO, &kBlockingMode);
    }

    *clientSock = std::make_shared<TCPSocket>(fd, std::move(remoteAddr), port);
    return error::nil;
}

error ListenTCP(uint16_t port, std::shared_ptr<TCPListener>* serverSock) {
    if (serverSock == nullptr) {
        assert(0 && "serverSock must not be nullptr");
        return error::illegal_argument;
    }

    internal::init();

    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET) {
        return error::wrap(etype::os, WSAGetLastError());
    }

    BOOL enabled = TRUE;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*) &enabled, sizeof(enabled)) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        closesocket(fd);
        return error::wrap(etype::os, err);
    }

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        closesocket(fd);
        return error::wrap(etype::os, err);
    }

    if (listen(fd, SOMAXCONN) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        closesocket(fd);
        return error::wrap(etype::os, err);
    }

    *serverSock = std::make_shared<TCPListener>(fd);
    return error::nil;
}

TCPSocket::~TCPSocket() {
    Close();
}

error TCPSocket::Close() {
    if (m_closed) {
        return error::nil;
    }

    if (closesocket(m_fd) == SOCKET_ERROR) {
        return error::wrap(etype::os, WSAGetLastError());
    }
    m_closed = true;
    return error::nil;
}

static error read(const SocketFD& fd, char* buf, size_t len, int* nbytes) {
    int size = recv(fd, buf, len, 0);
    if (size == SOCKET_ERROR) {
        return error::wrap(etype::os, WSAGetLastError());
    }
    if (size == 0) {
        return error::eof;
    }
    if (nbytes != nullptr) {
        *nbytes = size;
    }
    return error::nil;
}

error TCPSocket::Read(char* buf, size_t len, int* nbytes) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    if (m_timeoutMilliseconds > 0) {
        fd_set readfds;
        error err = waitUntilReady(m_fd, &readfds, nullptr, nullptr, m_timeoutMilliseconds);
        if (err != error::nil) {
            return err;
        }
    }
    return read(m_fd, buf, len, nbytes);
}

static error write(const SocketFD& fd, const char* buf, size_t len, int* nbytes) {
    int size = send(fd, buf, len, 0);
    if (size == SOCKET_ERROR) {
        return error::wrap(etype::os, WSAGetLastError());
    }
    if (nbytes != nullptr) {
        *nbytes = size;
    }
    return error::nil;
}

error TCPSocket::Write(const char* buf, size_t len, int* nbytes) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    if (m_timeoutMilliseconds > 0) {
        fd_set writefds;
        error err = waitUntilReady(m_fd, nullptr, &writefds, nullptr, m_timeoutMilliseconds);
        if (err != error::nil) {
            return err;
        }
    }
    return write(m_fd, buf, len, nbytes);
}

error TCPSocket::SetTimeout(int64_t timeoutMilliseconds) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    if (timeoutMilliseconds > 0) {
        ioctlsocket(m_fd, FIONBIO, &kNonBlockingMode);
    } else {
        ioctlsocket(m_fd, FIONBIO, &kBlockingMode);
    }
    m_timeoutMilliseconds = timeoutMilliseconds;
    return error::nil;
}

error TCPSocket::SetKeepAlive(bool on) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    BOOL enabled = on;
    if (setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, (const char*) &enabled, sizeof(enabled)) == SOCKET_ERROR) {
        return error::wrap(etype::os, errno);
    }
    return error::nil;
}

error TCPSocket::SetKeepAlivePeriod(int periodSeconds) {
    if (periodSeconds < 1) {
        assert(0 && "periodSeconds must not be less than 1");
        return error::illegal_argument;
    }
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    struct tcp_keepalive keepalive;
    keepalive.onoff = 1;
    keepalive.keepalivetime = periodSeconds * 1000;
    keepalive.keepaliveinterval = keepalive.keepalivetime;
    DWORD ret;
    if (WSAIoctl(m_fd, SIO_KEEPALIVE_VALS, &keepalive, sizeof(keepalive), nullptr, 0, &ret, nullptr, nullptr) == SOCKET_ERROR) {
        return error::wrap(etype::os, WSAGetLastError());
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
        return error::wrap(etype::os, WSAGetLastError());
    }
    m_closed = true;
    return error::nil;
}

error TCPListener::Accept(std::shared_ptr<TCPSocket>* clientSock) {
    if (clientSock == nullptr) {
        assert(0 && "clientSock must not be nullptr");
        return error::illegal_argument;
    }
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    if (m_timeoutMilliseconds > 0) {
        fd_set readfds;
        error err = waitUntilReady(m_fd, &readfds, nullptr, nullptr, m_timeoutMilliseconds);
        if (err != error::nil) {
            return err;
        }
    }

    struct sockaddr_in clientAddr = {0};
    int addrlen = sizeof(clientAddr);
    SOCKET clientFD = accept(m_fd, (struct sockaddr*) &clientAddr, &addrlen);
    if (clientFD == INVALID_SOCKET) {
        return error::wrap(etype::os, WSAGetLastError());
    }

    std::string remoteAddr = inet_ntoa(clientAddr.sin_addr);
    uint16_t remotePort = ntohs(clientAddr.sin_port);
    *clientSock = std::make_shared<TCPSocket>(clientFD, std::move(remoteAddr), remotePort);
    return error::nil;
}

error TCPListener::SetTimeout(int64_t timeoutMilliseconds) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    m_timeoutMilliseconds = timeoutMilliseconds;
    return error::nil;
}

} // namespace net
