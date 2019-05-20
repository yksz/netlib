#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include "netlib/error.h"

namespace net {

struct NetworkInterface {
    int Index;
    std::string Name;
    std::array<uint8_t, 6> HardwareAddress;
    bool IsUp;
    bool IsLoopback;
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
