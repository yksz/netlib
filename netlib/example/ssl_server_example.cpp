#include <cstdio>
#include <iostream>
#include <memory>
#include <thread>
#include "net/ssl.h"

using namespace net;

static const uint16_t kPort = 8443;

static void handle(const std::shared_ptr<SSLSocket>& clientSock) {
    std::cout << "ThreadID = " << std::this_thread::get_id() << std::endl;

    char buf[8];
    while (true) {
        error err = clientSock->ReadLine(buf, sizeof(buf));
        if (err == error::eof) {
            break;
        }
        if (err != error::nil) {
            printf("%s\n", error::Message(err));
            break;
        }
        printf("%s", buf);
    }
    clientSock->Close();
}

int main(int argc, char** argv) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc <= 2) {
        printf("usage: %s <certfile> <keyfile>\n", argv[0]);
        exit(1);
    }
    std::string cert = argv[1];
    std::string key = argv[2];

    SSLConfig config = {};
    config.CertFile = cert;
    config.KeyFile = key;

    std::shared_ptr<SSLListener> listener;
    error err = ListenSSL(kPort, config, &listener);
    if (err != error::nil) {
        printf("%s\n", error::Message(err));
        return 1;
    }
    printf("Listening on port %d\n", kPort);
    for (;;) {
        std::shared_ptr<SSLSocket> socket;
        error err = listener->Accept(&socket);
        if (err != error::nil) {
            printf("%s\n", error::Message(err));
            continue;
        }
        printf("%s connected\n", socket->RemoteAddress().c_str());
        std::thread th([=]() {
            handle(socket);
        });
        th.detach();
    }
    listener->Close();
    return 0;
}
