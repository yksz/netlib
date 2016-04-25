#include "net/resolver.h"
#include <string>
#include <gtest/gtest.h>

using namespace net;

TEST(LookupAddress, Localhost) {
    std::string addr;
    error err = LookupAddress("localhost", &addr);

    EXPECT_EQ(error::nil, err);
    EXPECT_EQ("127.0.0.1", addr);
}

TEST(LookupAddress, LocalIPv4) {
    std::string addr;
    error err = LookupAddress("127.0.0.1", &addr);

    EXPECT_EQ(error::nil, err);
    EXPECT_EQ("127.0.0.1", addr);
}

TEST(LookupAddress, Nullptr) {
    ASSERT_DEATH({ LookupAddress("localhost", nullptr); }, "");
}
