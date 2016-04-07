#include <cstdio>
#include <iostream>
#include <memory>
#include <thread>
#include "cx/net/tcp.h"

using namespace cx;

static void handle(const std::shared_ptr<TCPSocket>& clientsock) {
    std::cout << "ThreadID = " << std::this_thread::get_id() << std::endl;

    char buf[8];
    while (clientsock->ReadLine(buf, sizeof(buf))) {
        printf("%s", buf);
    }
    clientsock->Close();
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);

    std::unique_ptr<TCPListener> listener = ListenWithTCP(8080);
    if (listener != nullptr) {
        for (;;) {
            std::shared_ptr<TCPSocket> socket = listener->Accept();
            std::thread th([=]() {
                handle(socket);
            });
            th.detach();
        }
        listener->Close();
    }
}
