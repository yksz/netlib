#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include "net/error.h"

namespace net {

struct NetworkInterface {
    int index;
    std::string name;
    std::array<uint8_t, 6> hardwareAddress;
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

} // namespace net
