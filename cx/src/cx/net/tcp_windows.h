#pragma once

#include "cx/net/tcp.h"
#include <atomic>
#include <winsock2.h>

namespace cx {

class WindowsTCPSocket final : public TCPSocket {
public:
    explicit WindowsTCPSocket(SOCKET sock) : m_sock(sock), m_closed(false) {};
    ~WindowsTCPSocket();
    WindowsTCPSocket(const WindowsTCPSocket&) = delete;
    WindowsTCPSocket& operator=(const WindowsTCPSocket&) = delete;

    void Close();
    bool IsClosed();
    int Read(char* buf, size_t len);
    int Write(const char* buf, size_t len);
    bool SetSocketTimeout(int timeout);

private:
    const SOCKET m_sock;
    std::atomic<bool> m_closed;
};


class WindowsTCPListener final : public TCPListener {
public:
    explicit WindowsTCPListener(SOCKET sock) : m_sock(sock), m_closed(false) {};
    ~WindowsTCPListener();
    WindowsTCPListener(const WindowsTCPListener&) = delete;
    WindowsTCPListener& operator=(const WindowsTCPListener&) = delete;

    void Close();
    bool IsClosed();
    std::shared_ptr<TCPSocket> Accept();

private:
    const SOCKET m_sock;
    std::atomic<bool> m_closed;
};

} // namespace cx
