#include <cstdlib>
#include <iostream>
#include <string>
#include "net/resolver.h"

using namespace net;

int main(int argc, char** argv) {
    std::string host = "localhost";
    if (argc > 1) {
        host = argv[1];
    }

    std::string addr;
    error err;
    if (host == "localhost") {
        err = LookupLocalHostAddress(&addr);
    } else {
        err = LookupAddress(host, &addr);
    }
    if (err != error::nil) {
        std::cout << host << ": " << error::Message(err) << std::endl;
        return 1;
    }
    std::cout << host << ": " << addr << std::endl;
    return 0;
}
