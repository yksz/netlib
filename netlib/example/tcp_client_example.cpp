#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <memory>
#include <thread>
#include "net/tcp.h"

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

    std::shared_ptr<TCPSocket> socket;
    error err = ConnectWithTCP(host, port, 5000, &socket);
    if (err != error::nil) {
        printf("%s\n", GetErrorMessage(err));
        return 1;
    }
    socket->SetSocketTimeout(10000);
    for (int i = 0; i < kSendCount; i++) {
        socket->WriteFully(msg, strlen(msg));
        socket->WriteFully("\n", 1);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    socket->Close();
}
