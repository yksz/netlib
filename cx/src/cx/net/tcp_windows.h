#pragma once

#include "cx/net/tcp.h"
#include <atomic>
#include <winsock2.h>

namespace cx {

class WindowsTCPSocket final : public TCPSocket {
public:
    WindowsTCPSocket(SOCKET sock, const std::string& host)
        : m_sock(sock), m_host(host), m_closed(false) {};
    ~WindowsTCPSocket();
    WindowsTCPSocket(const WindowsTCPSocket&) = delete;
    WindowsTCPSocket& operator=(const WindowsTCPSocket&) = delete;

    error Close();
    bool IsClosed();
    error Read(char* buf, size_t len, int* nbytes);
    error Write(const char* buf, size_t len, int* nbytes);
    error SetSocketTimeout(int timeout);
    std::string GetHost() { return m_host; };

private:
    const SOCKET m_sock;
    const std::string m_host;
    std::atomic<bool> m_closed;
};


class WindowsTCPListener final : public TCPListener {
public:
    explicit WindowsTCPListener(SOCKET sock) : m_sock(sock), m_closed(false) {};
    ~WindowsTCPListener();
    WindowsTCPListener(const WindowsTCPListener&) = delete;
    WindowsTCPListener& operator=(const WindowsTCPListener&) = delete;

    error Close();
    bool IsClosed();
    error Accept(std::shared_ptr<TCPSocket>* clientsock);

private:
    const SOCKET m_sock;
    std::atomic<bool> m_closed;
};

} // namespace cx
