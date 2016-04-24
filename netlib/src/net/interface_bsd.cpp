#include "net/interface.h"
#include <cerrno>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>

namespace net {

error GetNetworkInterfaces(std::vector<NetworkInterface>* infs) {
    struct ifaddrs* ifaddrs;
    if (getifaddrs(&ifaddrs) == -1) {
        return toError(errno);
    }
    for (auto ifa = ifaddrs; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family != AF_LINK) {
            continue;
        }
        NetworkInterface inf = {0};
        inf.index = if_nametoindex(ifa->ifa_name);
        inf.name = ifa->ifa_name;
        struct sockaddr_dl* sdl = (struct sockaddr_dl*) ifa->ifa_addr;
        switch (sdl->sdl_type) {
            case IFT_ETHER:
            {
                unsigned char* macAddr = (unsigned char*) LLADDR(sdl);
                for (int i = 0; i < inf.hardwareAddress.size(); i++) {
                    inf.hardwareAddress[i] = macAddr[i];
                }
                break;
            }
            case IFT_LOOP:
                inf.isLoopback = true;
                break;
        }
        inf.isUp = ifa->ifa_flags & IFF_UP;
        infs->push_back(inf);
    }
    freeifaddrs(ifaddrs);
    return error::nil;
}

} // namespace net
