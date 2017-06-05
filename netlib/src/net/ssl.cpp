#include "net/ssl.h"
#include <cassert>
#include <mutex>
#include <openssl/crypto.h>
#include <openssl/err.h>

namespace net {

static void initOnce() {
    SSL_load_error_strings();
    SSL_library_init();
}

static void init() {
    static std::once_flag flag;
    std::call_once(flag, initOnce);
}

static error newClientCTX(const SSLConfig& config, SSL_CTX** ctx) {
    const SSL_METHOD* method = config.Method;
    if (method == nullptr) {
        method = SSLv23_client_method(); // default
    }

    *ctx = SSL_CTX_new(method);
    if (*ctx == nullptr) {
        return error::wrap(etype::ssl, ERR_get_error());
    }

    int err = 0;
    if (config.CertFile.empty()) {
        if (SSL_CTX_set_default_verify_paths(*ctx) != 1) {
            err = ERR_get_error();
            goto fail;
        }
    } else {
        if (SSL_CTX_load_verify_locations(*ctx, config.CertFile.c_str(), nullptr) != 1) {
            err = ERR_get_error();
            goto fail;
        }
    }
    goto exit;

fail:
    SSL_CTX_free(*ctx);
exit:
    return error::wrap(etype::ssl, err);
}

static error newSSLAndConnect(SSL_CTX* ctx, const std::shared_ptr<TCPSocket>& tcp, SSL** ssl) {
    *ssl = SSL_new(ctx);
    if (*ssl == nullptr) {
        return error::wrap(etype::ssl, ERR_get_error());
    }

    int err = 0;
    int rc = 1;
    rc = SSL_set_fd(*ssl, tcp->FD());
    if (rc != 1) {
        err = SSL_get_error(*ssl, rc);
        goto fail;
    }
    rc = SSL_connect(*ssl);
    if (rc != 1) {
        err = SSL_get_error(*ssl, rc);
        goto fail;
    }
    goto exit;

fail:
    SSL_free(*ssl);
exit:
    return error::wrap(etype::ssl, err);
}

error ConnectSSL(const std::string& host, uint16_t port, int64_t timeoutMilliseconds,
        const SSLConfig& config,
        std::shared_ptr<SSLSocket>* clientSock) {
    if (clientSock == nullptr) {
        assert(0 && "clientSock must not be nullptr");
        return error::illegal_argument;
    }

    init();

    SSL_CTX* ctx;
    error ctxErr = newClientCTX(config, &ctx);
    if (ctxErr != error::nil) {
        return ctxErr;
    }

    std::shared_ptr<TCPSocket> tcp;
    error tcpErr = ConnectTCP(host, port, timeoutMilliseconds, &tcp);
    if (tcpErr != error::nil) {
        return tcpErr;
    }

    SSL* ssl;
    error sslErr = newSSLAndConnect(ctx, tcp, &ssl);
    if (sslErr != error::nil) {
        return sslErr;
    }

    error err = error::nil;
    if (!config.InsecureSkipVerify) {
        // Step 1: verify a server certificate was presented during the negotiation
        X509* cert = SSL_get_peer_certificate(ssl);
        if (cert == nullptr) {
            err = error::ssl_cert;
            goto fail;
        }
        X509_free(cert);
        // Step 2: verify the result of chain verification
        if (SSL_get_verify_result(ssl) != X509_V_OK) {
            err = error::ssl_cert;
            goto fail;
        }
    }

    *clientSock = std::make_shared<SSLSocket>(tcp, ssl);
    goto exit;

fail:
    SSL_CTX_free(ctx);
    SSL_free(ssl);
exit:
    return err;
}

static error newServerCTX(const SSLConfig& config, SSL_CTX** ctx) {
    const SSL_METHOD* method = config.Method;
    if (method == nullptr) {
        method = SSLv23_server_method(); // default
    }

    *ctx = SSL_CTX_new(method);
    if (*ctx == nullptr) {
        return error::wrap(etype::ssl, ERR_get_error());
    }

    int err = 0;
    if (SSL_CTX_use_certificate_file(*ctx, config.CertFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
        err = ERR_get_error();
        goto fail;
    }
    if (SSL_CTX_use_PrivateKey_file(*ctx, config.KeyFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
        err = ERR_get_error();
        goto fail;
    }
    if (!SSL_CTX_check_private_key(*ctx)) {
        err = ERR_get_error();
        goto fail;
    }
    goto exit;

fail:
    SSL_CTX_free(*ctx);
exit:
    return error::wrap(etype::ssl, err);
}

error ListenSSL(uint16_t port,
        const SSLConfig& config,
        std::shared_ptr<SSLListener>* serverSock) {
    if (serverSock == nullptr) {
        assert(0 && "serverSock must not be nullptr");
        return error::illegal_argument;
    }

    init();

    SSL_CTX* ctx;
    error ctxErr = newServerCTX(config, &ctx);
    if (ctxErr != error::nil) {
        return ctxErr;
    }

    std::shared_ptr<TCPListener> tcp;
    error tcpErr = ListenTCP(port, &tcp);
    if (tcpErr != error::nil) {
        return tcpErr;
    }

    *serverSock = std::make_shared<SSLListener>(tcp, ctx);
    return error::nil;
}

SSLSocket::~SSLSocket() {
    Close();
}

error SSLSocket::Close() {
    if (m_closed) {
        return error::nil;
    }

    SSL_shutdown(m_ssl);
    m_tcp->Close();
    SSL_free(m_ssl);
    m_closed = true;
    return error::nil;
}

error SSLSocket::Read(char* buf, size_t len, int* nbytes) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    int size = SSL_read(m_ssl, buf, len);
    if (size == -1) {
        return error::wrap(etype::os, errno);
    }
    if (size == 0) {
        return error::eof;
    }
    if (nbytes != nullptr) {
        *nbytes = size;
    }
    return error::nil;
}

error SSLSocket::Write(const char* buf, size_t len, int* nbytes) {
    if (m_closed) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    int size = SSL_write(m_ssl, buf, len);
    if (size == -1) {
        return error::wrap(etype::os, errno);
    }
    if (nbytes != nullptr) {
        *nbytes = size;
    }
    return error::nil;
}

SSLListener::~SSLListener() {
    Close();
}

bool SSLListener::IsClosed() {
    return m_tcp->IsClosed();
}

error SSLListener::Close() {
    SSL_CTX_free(m_ctx);
    return m_tcp->Close();
}

static error newSSLAndAccept(SSL_CTX* ctx, const std::shared_ptr<TCPSocket>& tcp, SSL** ssl) {
    *ssl = SSL_new(ctx);
    if (*ssl == nullptr) {
        return error::wrap(etype::ssl, ERR_get_error());
    }

    int err = 0;
    int rc = 1;
    rc = SSL_set_fd(*ssl, tcp->FD());
    if (rc != 1) {
        err = SSL_get_error(*ssl, rc);
        goto fail;
    }
    rc = SSL_accept(*ssl);
    if (rc != 1) {
        err = SSL_get_error(*ssl, rc);
        goto fail;
    }
    goto exit;

fail:
    SSL_free(*ssl);
exit:
    return error::wrap(etype::ssl, err);
}

error SSLListener::Accept(std::shared_ptr<SSLSocket>* clientSock) {
    if (clientSock == nullptr) {
        assert(0 && "clientSock must not be nullptr");
        return error::illegal_argument;
    }
    if (IsClosed()) {
        assert(0 && "Already closed");
        return error::illegal_state;
    }

    std::shared_ptr<TCPSocket> clientTCP;
    error tcpErr = m_tcp->Accept(&clientTCP);
    if (tcpErr != error::nil) {
        return tcpErr;
    }

    SSL* ssl;
    error sslErr = newSSLAndAccept(m_ctx, clientTCP, &ssl);
    if (sslErr != error::nil) {
        return sslErr;
    }

    *clientSock = std::make_shared<SSLSocket>(clientTCP, ssl);
    return error::nil;
}

} // namespace net
