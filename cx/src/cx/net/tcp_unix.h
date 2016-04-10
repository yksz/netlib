#pragma once

#include "cx/net/tcp.h"
#include <atomic>

namespace cx {

class UnixTCPSocket final : public TCPSocket {
public:
    UnixTCPSocket(int sockfd, const std::string& host)
        : m_sockfd(sockfd), m_host(host), m_closed(false) {};
    ~UnixTCPSocket();
    UnixTCPSocket(const UnixTCPSocket&) = delete;
    UnixTCPSocket& operator=(const UnixTCPSocket&) = delete;

    error Close();
    bool IsClosed();
    error Read(char* buf, size_t len, int* nbytes);
    error Write(const char* buf, size_t len, int* nbytes);
    error SetSocketTimeout(int timeout);
    std::string GetHost() { return m_host; };

private:
    const int m_sockfd;
    const std::string m_host;
    std::atomic<bool> m_closed;
};

class UnixTCPListener final : public TCPListener {
public:
    explicit UnixTCPListener(int sockfd) : m_sockfd(sockfd), m_closed(false) {};
    ~UnixTCPListener();
    UnixTCPListener(const UnixTCPListener&) = delete;
    UnixTCPListener& operator=(const UnixTCPListener&) = delete;

    error Close();
    bool IsClosed();
    error Accept(std::shared_ptr<TCPSocket>* clientsock);

private:
    const int m_sockfd;
    std::atomic<bool> m_closed;
};

} // namespace cx
