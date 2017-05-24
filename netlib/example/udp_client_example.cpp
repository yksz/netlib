#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <memory>
#include <thread>
#include "net/udp.h"

using namespace net;

static const int kSendCount = 3;

int main(int argc, char** argv) {
    if (argc <= 3) {
        printf("usage: %s <host> <port> <message>\n", argv[0]);
        exit(1);
    }
    char* host = argv[1];
    int port = atoi(argv[2]);
    char* msg = argv[3];

    std::shared_ptr<UDPSocket> socket;
    error err = ConnectUDP(host, port, &socket);
    if (err != error::nil) {
        printf("%s\n", ErrorMessage(err));
        return 1;
    }
    socket->SetTimeout(10000);
    for (int i = 0; i < kSendCount; i++) {
        int nbytes;
        socket->Write(msg, strlen(msg), &nbytes);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    socket->Close();
    return 0;
}
