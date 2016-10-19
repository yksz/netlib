#pragma once

#include <cstdint>
#include <atomic>
#include <memory>
#include <string>
#include "net/error.h"
#include "net/stream.h"
#if defined(_WIN32) || defined(_WIN64)
 #include <winsock2.h>
#endif // defined(_WIN32) || defined(_WIN64)

namespace net {

#if defined(_WIN32) || defined(_WIN64)
using SocketFD = SOCKET;
#else
using SocketFD = int;
#endif // defined(_WIN32) || defined(_WIN64)

class TCPSocket final : public Closer, public Reader, public Writer {
public:
    TCPSocket(const SocketFD& fd, const std::string& addr)
            : m_fd(fd), m_remoteAddr(addr), m_closed(false) {};
    ~TCPSocket();
    TCPSocket(const TCPSocket&) = delete;
    TCPSocket& operator=(const TCPSocket&) = delete;

    error Close();
    bool IsClosed();
    error Read(char* buf, size_t len, int* nbytes);
    error Write(const char* buf, size_t len, int* nbytes);
    /**
     * @param[in] timeout Set the timeout in milliseconds. Block if 0 or a negative integer is specified.
     */
    error SetSocketTimeout(int64_t timeout);
    std::string RemoteAddress() { return m_remoteAddr; };

private:
    const SocketFD m_fd;
    const std::string m_remoteAddr;
    std::atomic<bool> m_closed;
};

class TCPListener final : public Closer {
public:
    explicit TCPListener(const SocketFD& fd)
            : m_fd(fd), m_closed(false) {};
    ~TCPListener();
    TCPListener(const TCPListener&) = delete;
    TCPListener& operator=(const TCPListener&) = delete;

    error Close();
    bool IsClosed();
    /**
     * @param[out] clientSock
     */
    error Accept(std::shared_ptr<TCPSocket>* clientSock);

private:
    const SocketFD m_fd;
    std::atomic<bool> m_closed;
};

/**
 * @param[in] host A hostname or IPv4
 * @param[in] port
 * @param[in] timeout Set the timeout in milliseconds.
 * @param[out] clientSock
 */
error ConnectTCP(const std::string& host, uint16_t port, int64_t timeout,
        std::shared_ptr<TCPSocket>* clientSock);

/**
 * @param[in] port
 * @param[out] serverSock
 */
error ListenTCP(uint16_t port, std::unique_ptr<TCPListener>* serverSock);

} // namespace net
