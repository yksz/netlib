#include "cx/net/interface.h"

namespace cx {

error GetNetworkInterfaceByIndex(int index, NetworkInterface* inf) {
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

} // namespace cx
