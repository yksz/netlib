#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include "net/resolver.h"
#include "net/tcp.h"

using namespace net;

static void handle(const std::shared_ptr<TCPSocket>& socket) {
    std::cout << "ThreadID = " << std::this_thread::get_id() << std::endl;

    char buf[8];
    while (true) {
        error err = socket->ReadLine(buf, sizeof(buf));
        if (err == error::eof) {
            break;
        }
        if (err != error::nil) {
            printf("%s\n", error::Message(err));
            break;
        }
        printf("%s", buf);
    }
    socket->Close();
}

int main(int argc, char** argv) {
    setvbuf(stdout, nullptr, _IONBF, 0);

    uint16_t port = 0; // random
    if (argc > 1) {
        port = (uint16_t) atoi(argv[1]);
    }

    std::shared_ptr<TCPListener> listener;
    error err = ListenTCP(port, &listener);
    if (err != error::nil) {
        printf("%s\n", error::Message(err));
        return 1;
    }

    uint16_t localPort = 0;
    LookupPort(listener->FD(), &localPort);
    printf("Listening on port %d\n", localPort);
    for (;;) {
        std::shared_ptr<TCPSocket> socket;
        error err = listener->Accept(&socket);
        if (err != error::nil) {
            printf("%s\n", error::Message(err));
            continue;
        }
        printf("%s:%d connected\n", socket->RemoteAddress().c_str(), socket->RemotePort());
        std::thread th([=]() {
            handle(socket);
        });
        th.detach();
    }
    listener->Close();
    return 0;
}
