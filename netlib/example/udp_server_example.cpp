#include <cstdio>
#include <memory>
#include "net/udp.h"

using namespace net;

static const int kPort = 8080;

int main(void) {
    setvbuf(stdout, nullptr, _IONBF, 0);

    std::shared_ptr<UDPSocket> socket;
    error err = ListenUDP(kPort, &socket);
    if (err != error::nil) {
        printf("%s\n", ErrorMessage(err));
        return 1;
    }
    printf("Listening on port %d\n", kPort);
    for (;;) {
        char buf[256] = {0};
        int nbytes;
        std::string addr;
        unsigned int port;
        socket->ReadFrom(buf, sizeof(buf), &nbytes, &addr, &port);
        printf("%s:%d: %s\n", addr.c_str(), port, buf);
    }
    socket->Close();
}
