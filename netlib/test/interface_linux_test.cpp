#include "net/interface.h"
#include <array>
#include <string>
#include <gtest/gtest.h>

using namespace net;

TEST(GetNetworkInterfaceByName, LoopbackAddress) {
    const std::string name = "lo";

    NetworkInterface inf;
    error err = GetNetworkInterfaceByName(name, &inf);

    EXPECT_EQ(error::nil, err);
    EXPECT_EQ(name, inf.Name);
    std::array<unsigned char, 6> macAddr{0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    EXPECT_EQ(macAddr, inf.HardwareAddress);
    EXPECT_EQ(true, inf.IsUp);
    EXPECT_EQ(true, inf.IsLoopback);
}
