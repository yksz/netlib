#pragma once

#include <array>
#include <string>
#include <vector>
#include "cx/error.h"

namespace cx {

struct NetworkInterface {
    int index;
    std::string name;
    std::array<unsigned char, 6> hardwareAddress;
    int mtu;
    bool isLoopback;
};

/**
 * @param[out] infs
 */
error GetNetworkInterfaces(std::vector<NetworkInterface>* infs);

} // namespace cx
