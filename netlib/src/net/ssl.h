#pragma once

#include <cstdint>
#include <atomic>
#include <memory>
#include <string>
#include <openssl/ssl.h>
#include "net/error.h"
#include "net/stream.h"
#include "net/tcp.h"

namespace net {

struct SSLConfig {
    const SSL_METHOD* Method;
    std::string CertFile;
    std::string KeyFile;
    bool InsecureSkipVerify;
};

class SSLSocket final : public ReadWriteCloser {
public:
    SSLSocket(const std::shared_ptr<TCPSocket>& tcp, SSL* ssl)
            : m_tcp(tcp), m_ssl(ssl), m_closed(false) {}
    ~SSLSocket();
    SSLSocket(const SSLSocket&) = delete;
    SSLSocket& operator=(const SSLSocket&) = delete;

    bool IsClosed() { return m_closed; }
    error Close();
    error Read(char* buf, size_t len, int* nbytes);
    error Write(const char* buf, size_t len, int* nbytes);
    /**
     * @param[in] timeoutMilliseconds Set the timeout in milliseconds. Block if 0 or a negative integer is specified.
     */
    error SetTimeout(int64_t timeoutMilliseconds) { return m_tcp->SetTimeout(timeoutMilliseconds); }
    SocketFD FD() { return m_tcp->FD(); }
    std::string RemoteAddress() { return m_tcp->RemoteAddress(); }
    uint16_t RemotePort() { return m_tcp->RemotePort(); }

private:
    std::shared_ptr<TCPSocket> m_tcp;
    SSL* m_ssl;
    std::atomic<bool> m_closed;
};

class SSLListener final : public Closer {
public:
    SSLListener(const std::shared_ptr<TCPListener>& tcp, SSL_CTX* ctx)
            : m_tcp(tcp), m_ctx(ctx) {}
    ~SSLListener();
    SSLListener(const SSLListener&) = delete;
    SSLListener& operator=(const SSLListener&) = delete;

    bool IsClosed();
    error Close();
    /**
     * @param[out] clientSock
     */
    error Accept(std::shared_ptr<SSLSocket>* clientSock);
    /**
     * @param[in] timeoutMilliseconds Set the timeout in milliseconds. Block if 0 or a negative integer is specified.
     */
    error SetTimeout(int64_t timeoutMilliseconds) { return m_tcp->SetTimeout(timeoutMilliseconds); }
    SocketFD FD() { return m_tcp->FD(); }

private:
    std::shared_ptr<TCPListener> m_tcp;
    SSL_CTX* m_ctx;
};

/**
 * @param[in] host A hostname or IPv4
 * @param[in] port
 * @param[in] timeoutMilliseconds Set the timeout in milliseconds. Block if 0 or a negative integer is specified.
 * @param[in] config
 * @param[out] clientSock
 */
error ConnectSSL(const std::string& host, uint16_t port, int64_t timeoutMilliseconds,
        const SSLConfig& config,
        std::shared_ptr<SSLSocket>* clientSock);

/**
 * @param[in] port
 * @param[in] config
 * @param[out] serverSock
 */
error ListenSSL(uint16_t port,
        const SSLConfig& config,
        std::shared_ptr<SSLListener>* serverSock);

} // namespace net
