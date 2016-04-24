#include "net/resolver.h"
#include <cassert>
#if defined(_WIN32) || defined(_WIN64)
 #include <winsock2.h>
 #include <ws2tcpip.h>
#else
 #include <arpa/inet.h>
 #include <netdb.h>
 #include <netinet/in.h>
 #include <sys/socket.h>
 #include <sys/types.h>
#endif // defined(_WIN32) || defined(_WIN64)
#include "net/internal/init.h"

namespace net {

error LookupAddress(const std::string& host, std::string* addr) {
    if (addr == nullptr) {
        assert(0 && "addr must not be nullptr");
        return error::illegal_argument;
    }

    internal::init();

    struct addrinfo hints = {0};
    struct addrinfo* result;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int err = getaddrinfo(host.c_str(), nullptr, &hints, &result);
    if (err != 0) {
        return toError(err);
    }
    *addr = inet_ntoa(((struct sockaddr_in*) (result->ai_addr))->sin_addr);
    freeaddrinfo(result);
    return error::nil;
}

} // namespace net
