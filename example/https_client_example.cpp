#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include "netlib/ssl.h"

using namespace net;

static const int kPort = 443;
static const int kConnTimeout =  5000; // ms
static const int kSockTimeout = 10000; // ms

int main(int argc, char** argv) {
    if (argc <= 1) {
        printf("usage: %s <host>\n", argv[0]);
        exit(1);
    }
    std::string host = argv[1];

    std::shared_ptr<SSLSocket> socket;
    // connect
    {
        SSLConfig config = {};
        error err = ConnectSSL(host, kPort, kConnTimeout, config, &socket);
        if (err != error::nil) {
            fprintf(stderr, "%s\n", error::Message(err));
            return 1;
        }
        socket->SetTimeout(kSockTimeout);
    }
    // send request
    {
        std::stringstream ss;
        ss << "GET / HTTP/1.1\r\n"
            << "Host: " + host + "\r\n"
            << "Connection: close\r\n"
            << "\r\n";
        std::string msg = ss.str();
        socket->WriteFull(msg.c_str(), msg.size());
    }
    // recv response
    {
        char buf[256];
        while (true) {
            error err = socket->ReadLine(buf, sizeof(buf));
            if (err == error::eof) {
                break;
            }
            if (err != error::nil) {
                fprintf(stderr, "%s\n", error::Message(err));
                break;
            }
            printf("%s", buf);
        }
    }
    return 0;
}
