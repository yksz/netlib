#include "net/resolver.h"
#include <cassert>
#if defined(_WIN32) || defined(_WIN64)
 #include <winsock2.h>
 #include <ws2tcpip.h>
#else
 #include <cerrno>
 #include <arpa/inet.h>
 #include <netdb.h>
 #include <netinet/in.h>
 #include <sys/socket.h>
 #include <sys/types.h>
 #include <unistd.h>
#endif // defined(_WIN32) || defined(_WIN64)
#include "net/internal/init.h"

namespace net {

error LookupAddress(const std::string& host, std::string* addr) {
    if (addr == nullptr) {
        assert(0 && "addr must not be nullptr");
        return error::illegal_argument;
    }

    internal::init();

    struct addrinfo* result;
    struct addrinfo hints = {0};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int err = getaddrinfo(host.c_str(), nullptr, &hints, &result);
    if (err != 0) {
        return error::wrap(etype::netdb, err);
    }
    *addr = inet_ntoa(((struct sockaddr_in*) (result->ai_addr))->sin_addr);
    freeaddrinfo(result);
    return error::nil;
}

error LookupPort(const SocketFD& fd, uint16_t* port) {
    if (port == nullptr) {
        assert(0 && "port must not be nullptr");
        return error::illegal_argument;
    }

    internal::init();

    struct sockaddr_in addr = {0};
    socklen_t addrlen = sizeof(addr);
    if (getsockname(fd, (struct sockaddr*) &addr, &addrlen) != 0) {
#if defined(_WIN32) || defined(_WIN64)
        return error::wrap(etype::os, WSAGetLastError());
#else
        return error::wrap(etype::os, errno);
#endif // defined(_WIN32) || defined(_WIN64)
    }
    *port = ntohs(addr.sin_port);
    return error::nil;
}

error LookupLocalHostName(std::string* host) {
    if (host == nullptr) {
        assert(0 && "host must not be nullptr");
        return error::illegal_argument;
    }

    internal::init();

    char name[256] = {0};
    if (gethostname(name, sizeof(name)) != 0) {
#if defined(_WIN32) || defined(_WIN64)
        return error::wrap(etype::os, WSAGetLastError());
#else
        return error::wrap(etype::os, errno);
#endif // defined(_WIN32) || defined(_WIN64)
    }
    *host = name;
    return error::nil;
}

error LookupLocalHostAddress(std::string* addr) {
    if (addr == nullptr) {
        assert(0 && "addr must not be nullptr");
        return error::illegal_argument;
    }

    internal::init();

    std::string host;
    error err = LookupLocalHostName(&host);
    if (err != error::nil) {
        return err;
    }
    return LookupAddress(host, addr);
}

} // namespace net
