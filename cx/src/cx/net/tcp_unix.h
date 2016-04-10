#pragma once

#include "cx/net/tcp.h"
#include <atomic>

namespace cx {

class UnixTCPSocket final : public TCPSocket {
public:
    explicit UnixTCPSocket(int sockfd) : m_sockfd(sockfd), m_closed(false) {};
    ~UnixTCPSocket();
    UnixTCPSocket(const UnixTCPSocket&) = delete;
    UnixTCPSocket& operator=(const UnixTCPSocket&) = delete;

    void Close();
    bool IsClosed();
    int Read(char* buf, size_t len);
    int Write(const char* buf, size_t len);
    bool SetSocketTimeout(int timeout);

private:
    const int m_sockfd;
    std::atomic<bool> m_closed;
};


class UnixTCPListener final : public TCPListener {
public:
    explicit UnixTCPListener(int sockfd) : m_sockfd(sockfd), m_closed(false) {};
    ~UnixTCPListener();
    UnixTCPListener(const UnixTCPListener&) = delete;
    UnixTCPListener& operator=(const UnixTCPListener&) = delete;

    void Close();
    bool IsClosed();
    std::shared_ptr<TCPSocket> Accept();

private:
    const int m_sockfd;
    std::atomic<bool> m_closed;
};

} // namespace cx
