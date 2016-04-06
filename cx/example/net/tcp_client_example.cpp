#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <memory>
#include <thread>
#include "cx/net/tcp.h"

using namespace cx;

static const int kSendCount = 3;

int main(int argc, char** argv) {
    if (argc <= 3) {
        printf("usage: %s <host> <port> <message>\n", argv[0]);
        exit(1);
    }
    char* host = argv[1];
    int port = atoi(argv[2]);
    char* msg = argv[3];

    std::shared_ptr<TCPSocket> socket = ConnectWithTCP(host, port, 5000);
    if (socket != nullptr) {
        socket->SetSocketTimeout(10000);
        for (int i = 0; i < kSendCount; i++) {
            socket->Write(msg, strlen(msg));
            socket->Write("\n", 1);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        socket->Close();
    }
}
