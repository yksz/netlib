#include "net/interface.h"
#include <cassert>

namespace net {

error GetNetworkInterfaceByIndex(int index, NetworkInterface* inf) {
    if (inf == nullptr) {
        assert(0 && "inf must not be nullptr");
        return error::illegal_argument;
    }

    std::vector<NetworkInterface> infs;
    error err = GetNetworkInterfaces(&infs);
    if (err != error::nil) {
        return err;
    }
    for (auto& i : infs) {
        if (i.index == index) {
            *inf = i;
            return error::nil;
        }
    }
    return error::not_found;
}

error GetNetworkInterfaceByName(const std::string& name, NetworkInterface* inf) {
    if (inf == nullptr) {
        assert(0 && "inf must not be nullptr");
        return error::illegal_argument;
    }

    std::vector<NetworkInterface> infs;
    error err = GetNetworkInterfaces(&infs);
    if (err != error::nil) {
        return err;
    }
    for (auto& i : infs) {
        if (i.name == name) {
            *inf = i;
            return error::nil;
        }
    }
    return error::not_found;
}

} // namespace net
