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

private:
    std::shared_ptr<TCPSocket> m_tcp;
    SSL* m_ssl;
    std::atomic<bool> m_closed;
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

} // namespace net
