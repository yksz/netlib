#include "net/binary.h"
#include <string>
#include <gtest/gtest.h>

using namespace net;

TEST(ByteConverter, PutAndGet) {
    char buf[32];

    // write
    ByteConverter w(buf, sizeof(buf), ByteOrder_BIG_ENDIAN);
    const char str1[] = "abcd";
    w.Put(str1, sizeof(str1));
    w.PutUint8(1);
    w.PutUint16(2);
    w.PutUint32(3);
    w.PutUint64(4);
    w.PutFloat(5.0f);
    w.PutDouble(6.0);

    // read
    ByteConverter r(buf, sizeof(buf), ByteOrder_BIG_ENDIAN);
    char str2[5] = {};
    r.Get(str2, sizeof(str2));
    EXPECT_EQ(std::string(str1), std::string(str2));
    EXPECT_EQ(1, r.GetUint8());
    EXPECT_EQ(2, r.GetUint16());
    EXPECT_EQ(3, r.GetUint32());
    EXPECT_EQ(4, r.GetUint64());
    EXPECT_EQ(5.0f, r.GetFloat());
    EXPECT_EQ(6.0, r.GetDouble());
}
