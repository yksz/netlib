#include <cstdio>
#include <array>
#include <iostream>
#include "cx/net/interface.h"

using namespace cx;

static std::string formatMACAddr(std::array<unsigned char, 6> addr, char delim) {
	char s[18] = {0};
    snprintf(s, sizeof(s), "%.2x%c%.2x%c%.2x%c%.2x%c%.2x%c%.2x", 
            addr[0], delim, addr[1], delim, addr[2], delim,
            addr[3], delim, addr[4], delim, addr[5]);
    return s;
}

int main(void) {
    std::vector<NetworkInterface> infs;
    error err = GetNetworkInterfaces(&infs);
    if (err != error::nil) {
        printf("%s\n", GetErrorMessage(err));
        return 1;
    }
    for (auto& inf : infs) {
		printf("\n");
        printf("Index          : %d\n", inf.index);
        std::cout << "Name           : " << inf.name << std::endl;
        printf("HardwareAddress: %s\n", formatMACAddr(inf.hardwareAddress, ':'));
        printf("MTU            : %d\n", inf.mtu);
        printf("Loopback       : %s\n", inf.isLoopback ? "true" : "false");
    }
    return 0;
}
