#include <cstdio>
#include <cstdlib>
#include <string>
#include "net/resolver.h"

using namespace net;

int main(int argc, char** argv) {
    if (argc <= 1) {
        printf("usage: %s <host>\n", argv[0]);
        exit(1);
    }
    char* host = argv[1];

    std::string addr;
    error err = LookupAddress(host, &addr);
    if (err != error::nil) {
        printf("%s\n", ErrorMessage(err));
        return 1;
    }
    printf("%s\n", addr.c_str());
    return 0;
}
