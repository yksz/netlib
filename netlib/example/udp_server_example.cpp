#include <cstdio>
#include <cstdlib>
#include <memory>
#include "net/resolver.h"
#include "net/udp.h"

using namespace net;

int main(int argc, char** argv) {
    setvbuf(stdout, nullptr, _IONBF, 0);

    uint16_t port = 0; // random
    if (argc > 1) {
        port = (uint16_t)atoi(argv[1]);
    }

    std::shared_ptr<UDPSocket> socket;
    error err = ListenUDP(port, &socket);
    if (err != error::nil) {
        printf("%s\n", error::Message(err));
        return 1;
    }

    uint16_t localPort = 0;
    LookupPort(socket->FD(), &localPort);
    printf("Listening on port %d\n", localPort);
    for (;;) {
        char buf[256] = {0};
        int nbytes;
        std::string addr;
        uint16_t port;
        socket->ReadFrom(buf, sizeof(buf), &nbytes, &addr, &port);
        printf("%s:%d: %s\n", addr.c_str(), port, buf);
    }
    socket->Close();
    return 0;
}
