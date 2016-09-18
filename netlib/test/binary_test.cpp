#include "net/binary.h"
#include <string>
#include <gtest/gtest.h>

using namespace net;

TEST(ByteBuffer, PutAndGetSigned) {
    char buf[32];

    // write
    ByteBuffer w(buf, sizeof(buf), ByteOrder::BigEndian);
    const char str1[] = "abcd";
    w.Put(str1, sizeof(str1));
    w.PutInt8(-1);
    w.PutInt16(-2);
    w.PutInt32(-3);
    w.PutInt64(-4);
    w.PutFloat(-5.0f);
    w.PutDouble(-6.0);

    // read
    ByteBuffer r(buf, sizeof(buf), ByteOrder::BigEndian);
    char str2[5] = {0};
    r.Get(str2, sizeof(str2));
    EXPECT_STREQ(str1, str2);
    EXPECT_EQ(-1, r.GetInt8());
    EXPECT_EQ(-2, r.GetInt16());
    EXPECT_EQ(-3, r.GetInt32());
    EXPECT_EQ(-4, r.GetInt64());
    EXPECT_EQ(-5.0f, r.GetFloat());
    EXPECT_EQ(-6.0, r.GetDouble());
}

TEST(ByteBuffer, PutAndGetUnsigned) {
    char buf[32];

    // write
    ByteBuffer w(buf, sizeof(buf), ByteOrder::BigEndian);
    const unsigned char str1[] = "abcd";
    w.Put(str1, sizeof(str1));
    w.PutUint8(1);
    w.PutUint16(2);
    w.PutUint32(3);
    w.PutUint64(4);
    w.PutFloat(5.0f);
    w.PutDouble(6.0);

    // read
    ByteBuffer r(buf, sizeof(buf), ByteOrder::BigEndian);
    unsigned char str2[5] = {0};
    r.Get(str2, sizeof(str2));
    for (int i = 0; i < sizeof(str1); i++) {
        EXPECT_EQ(str1[i], str2[i]);
    }
    EXPECT_EQ(1, r.GetUint8());
    EXPECT_EQ(2, r.GetUint16());
    EXPECT_EQ(3, r.GetUint32());
    EXPECT_EQ(4, r.GetUint64());
    EXPECT_EQ(5.0f, r.GetFloat());
    EXPECT_EQ(6.0, r.GetDouble());
}
