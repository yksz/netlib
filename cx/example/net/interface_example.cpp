#include <cstdio>
#include <array>
#include "cx/net/interface.h"

using namespace cx;

static std::string formatMACAddr(std::array<unsigned char, 6> addr, char delim) {
    char s[18] = {0};
    sprintf(s, "%.2x%c%.2x%c%.2x%c%.2x%c%.2x%c%.2x",
            addr[0], delim, addr[1], delim, addr[2], delim,
            addr[3], delim, addr[4], delim, addr[5]);
    return s;
}

static void printNetworkInterface(const NetworkInterface& inf) {
    printf("\n");
    printf("Index          : %d\n", inf.index);
    printf("Name           : %s\n", inf.name.c_str());
    printf("HardwareAddress: %s\n", formatMACAddr(inf.hardwareAddress, ':').c_str());
    printf("Up             : %s\n", inf.isUp ? "true" : "false");
    printf("Loopback       : %s\n", inf.isLoopback ? "true" : "false");
}

int main(int argc, char** argv) {
    char* name = NULL;
    if (argc > 1) {
        name = argv[1];
    }

    if (name != NULL) {
        NetworkInterface inf;
        if (GetNetworkInterfaceByName(name, &inf) == error::nil) {
            printNetworkInterface(inf);
            return 0;
        }
    }

    std::vector<NetworkInterface> infs;
    error err = GetNetworkInterfaces(&infs);
    if (err != error::nil) {
        printf("%s\n", GetErrorMessage(err));
        return 1;
    }
    for (auto& i : infs) {
        printNetworkInterface(i);
    }
    return 0;
}
