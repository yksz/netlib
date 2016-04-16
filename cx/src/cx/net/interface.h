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
    bool isUp;
    bool isLoopback;
};

/**
 * @param[out] infs
 */
error GetNetworkInterfaces(std::vector<NetworkInterface>* infs);

/**
 * @param[in] index
 * @param[out] inf
 */
error GetNetworkInterfaceByIndex(int index, NetworkInterface* inf);

/**
 * @param[in] name
 * @param[out] inf
 */
error GetNetworkInterfaceByName(const std::string& name, NetworkInterface* inf);

} // namespace cx
