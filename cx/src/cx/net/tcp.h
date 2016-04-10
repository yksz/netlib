#pragma once

#include <memory>
#include <string>
#include "cx/error.h"
#include "cx/io/stream.h"

namespace cx {

struct TCPSocket : Closer, Reader, Writer {
    virtual ~TCPSocket() {};
    virtual bool IsClosed() = 0;
    /**
     * @param[in] timeout milliseconds
     */
    virtual error SetSocketTimeout(int timeout) = 0;
    virtual std::string GetHost() = 0;
};

struct TCPListener : Closer {
    virtual ~TCPListener() {};
    virtual bool IsClosed() = 0;
    /**
     * @param[out] clientsock
     */
    virtual error Accept(std::shared_ptr<TCPSocket>* clientsock) = 0;
};

/**
 * @param[in] host IPv4 only
 * @param[in] port
 * @param[in] timeout milliseconds
 * @param[out] clientsock
 */
error ConnectWithTCP(const char* host, int port, int timeout,
        std::shared_ptr<TCPSocket>* clientsock);
/**
 * @param[in] port
 * @param[out] serversock
 */
error ListenWithTCP(int port, std::unique_ptr<TCPListener>* serversock);

} // namespace cx
