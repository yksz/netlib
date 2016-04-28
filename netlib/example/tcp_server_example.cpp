#include <cstdio>
#include <iostream>
#include <memory>
#include <thread>
#include "net/tcp.h"

using namespace net;

static const uint16_t kPort = 8080;

static void handle(const std::shared_ptr<TCPSocket>& clientsock) {
    std::cout << "ThreadID = " << std::this_thread::get_id() << std::endl;

    char buf[8];
    while (true) {
        error err = clientsock->ReadLine(buf, sizeof(buf));
        if (err == error::eof) {
            break;
        }
        if (err != error::nil) {
            printf("%s\n", ErrorMessage(err));
            break;
        }
        printf("%s", buf);
    }
    clientsock->Close();
}

int main(void) {
    setvbuf(stdout, nullptr, _IONBF, 0);

    std::unique_ptr<TCPListener> listener;
    error err = ListenTCP(kPort, &listener);
    if (err != error::nil) {
        printf("%s\n", ErrorMessage(err));
        return 1;
    }
    printf("Listening on port %d\n", kPort);
    for (;;) {
        std::shared_ptr<TCPSocket> socket;
        error err = listener->Accept(&socket);
        if (err != error::nil) {
            printf("%s\n", ErrorMessage(err));
            continue;
        }
        printf("%s connected\n", socket->GetRemoteAddress().c_str());
        std::thread th([=]() {
            handle(socket);
        });
        th.detach();
    }
    listener->Close();
}
