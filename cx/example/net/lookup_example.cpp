#include <cstdio>
#include <cstdlib>
#include <string>
#include "cx/net/lookup.h"

using namespace cx;

int main(int argc, char** argv) {
    if (argc <= 1) {
        printf("usage: %s <host>\n", argv[0]);
        exit(1);
    }
    char* host = argv[1];

    std::string addr;
    error err = LookupAddress(host, &addr);
    if (err != error::nil) {
        printf("%s\n", GetErrorMessage(err));
        return 1;
    }
    printf("%s\n", addr.c_str());
    return 0;
}
