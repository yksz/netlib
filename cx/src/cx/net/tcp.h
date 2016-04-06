#pragma once

#include <memory>
#include "cx/io/stream.h"

namespace cx {

struct TCPSocket : Closer, Reader, Writer {
    virtual ~TCPSocket() {};
    virtual bool IsClosed() = 0;
    virtual bool SetSocketTimeout(int timeout) = 0;
};


struct TCPListener : Closer {
    virtual ~TCPListener() {};
    virtual bool IsClosed() = 0;
    virtual std::shared_ptr<TCPSocket> Accept() = 0;
};


std::shared_ptr<TCPSocket> ConnectWithTCP(const char* host, int port, int timeout = 30000); // 30 sec
std::unique_ptr<TCPListener> ListenWithTCP(int port = 8080);

} // namespace cx
