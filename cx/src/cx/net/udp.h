#pragma once

#include <atomic>
#include <memory>
#include <string>
#include "cx/error.h"
#include "cx/io/stream.h"
#if defined(_WIN32) || defined(_WIN64)
 #include <winsock2.h>
#endif // defined(_WIN32) || defined(_WIN64)

namespace cx {

#if defined(_WIN32) || defined(_WIN64)
using SocketFD = SOCKET;
#else
using SocketFD = int;
#endif // defined(_WIN32) || defined(_WIN64)

class UDPSocket final : public Closer, public Reader, public Writer {
public:
    UDPSocket(const SocketFD& fd)
            : m_fd(fd), m_remoteAddr(""), m_remotePort(0), m_closed(false) {};
    UDPSocket(const SocketFD& fd, const std::string& addr, unsigned int port)
            : m_fd(fd), m_remoteAddr(addr), m_remotePort(port), m_closed(false) {};
    ~UDPSocket();
    UDPSocket(const UDPSocket&) = delete;
    UDPSocket& operator=(const UDPSocket&) = delete;

    error Close();
    bool IsClosed();
    error Read(char* buf, size_t len, int* nbytes);
    error ReadFrom(char* buf, size_t len, int* nbytes,
            std::string* addr, unsigned int* port);
    error Write(const char* buf, size_t len, int* nbytes);
    error WriteTo(const char* buf, size_t len,
           const std::string& addr, unsigned int port, int* nbytes);
    /**
     * @param[in] timeout Set the timeout in milliseconds. Block if 0 or a negative integer is specified.
     */
    error SetSocketTimeout(int timeout);

private:
    const SocketFD m_fd;
    const std::string m_remoteAddr;
    const unsigned int m_remotePort;
    std::atomic<bool> m_closed;
};

/**
 * @param[in] host A hostname or IPv4
 * @param[in] port
 * @param[out] clientSock
 */
error ConnectUDP(const std::string& host, unsigned int port,
        std::shared_ptr<UDPSocket>* clientSock);

/**
 * @param[in] port
 * @param[out] serverSock
 */
error ListenUDP(unsigned int port, std::shared_ptr<UDPSocket>* serverSock);

} // namespace cx
