#pragma once

#include <cstdint>
#include <atomic>
#include <memory>
#include <string>
#include "net/error.h"
#include "net/fd.h"
#include "net/stream.h"

namespace net {

class TCPSocket final : public ReadWriteCloser {
public:
    TCPSocket(const SocketFD& fd, const std::string& addr)
            : m_fd(fd), m_remoteAddr(addr), m_closed(false), m_timeoutMilliseconds(0) {}
    ~TCPSocket();
    TCPSocket(const TCPSocket&) = delete;
    TCPSocket& operator=(const TCPSocket&) = delete;

    bool IsClosed() { return m_closed; }
    error Close();
    error Read(char* buf, size_t len, int* nbytes);
    error Write(const char* buf, size_t len, int* nbytes);
    /**
     * @param[in] timeoutMilliseconds Set the timeout in milliseconds. Block if 0 or a negative integer is specified.
     */
    error SetTimeout(int64_t timeoutMilliseconds);
    SocketFD FD() { return m_fd; }
    std::string RemoteAddress() { return m_remoteAddr; }

private:
    const SocketFD m_fd;
    const std::string m_remoteAddr;
    std::atomic<bool> m_closed;
    int64_t m_timeoutMilliseconds;
};

class TCPListener final : public Closer {
public:
    explicit TCPListener(const SocketFD& fd)
            : m_fd(fd), m_closed(false), m_timeoutMilliseconds(0) {}
    ~TCPListener();
    TCPListener(const TCPListener&) = delete;
    TCPListener& operator=(const TCPListener&) = delete;

    bool IsClosed() { return m_closed; }
    error Close();
    /**
     * @param[out] clientSock
     */
    error Accept(std::shared_ptr<TCPSocket>* clientSock);
    /**
     * @param[in] timeoutMilliseconds Set the timeout in milliseconds. Block if 0 or a negative integer is specified.
     */
    error SetTimeout(int64_t timeoutMilliseconds);
    SocketFD FD() { return m_fd; }

private:
    const SocketFD m_fd;
    std::atomic<bool> m_closed;
    int64_t m_timeoutMilliseconds;
};

/**
 * @param[in] host A hostname or IPv4
 * @param[in] port
 * @param[in] timeoutMilliseconds Set the timeout in milliseconds. Block if 0 or a negative integer is specified.
 * @param[out] clientSock
 */
error ConnectTCP(const std::string& host, uint16_t port, int64_t timeoutMilliseconds,
        std::shared_ptr<TCPSocket>* clientSock);

/**
 * @param[in] port
 * @param[out] serverSock
 */
error ListenTCP(uint16_t port, std::shared_ptr<TCPListener>* serverSock);

} // namespace net
