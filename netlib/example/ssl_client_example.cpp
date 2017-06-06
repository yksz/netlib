#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <memory>
#include <thread>
#include "net/ssl.h"

using namespace net;

static const int kSendCount = 3;
static const int kConnTimeout =  5000; // ms
static const int kSockTimeout = 10000; // ms

int main(int argc, char** argv) {
    if (argc <= 3) {
        printf("usage: %s <host> <port> <message> [certfile]\n", argv[0]);
        exit(1);
    }
    char* host = argv[1];
    int port = atoi(argv[2]);
    char* msg = argv[3];
    std::string cert = argc > 4 ? argv[4] : "";

    SSLConfig config = {};
    if (cert.empty()) {
        config.InsecureSkipVerify = true;
    } else {
        config.CertFile = cert;
    }

    std::shared_ptr<SSLSocket> socket;
    error err = ConnectSSL(host, port, kConnTimeout, config, &socket);
    if (err != error::nil) {
        printf("%s\n", error::Message(err));
        return 1;
    }
    socket->SetTimeout(kSockTimeout);
    for (int i = 0; i < kSendCount; i++) {
        socket->WriteFull(msg, strlen(msg));
        socket->WriteFull("\n", 1);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    socket->Close();
    return 0;
}
