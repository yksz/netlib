#include "netlib/binary.h"
#include <string>
#include <gtest/gtest.h>

using namespace net;

TEST(ByteBuffer, PutAndGetSigned) {
    char buf[33];

    // write
    ByteBuffer w(buf, sizeof(buf), ByteOrder::BigEndian);
    const char str1[] = "abcd";
    w.Put(str1, sizeof(str1));
    w.PutBool(true);
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
    EXPECT_TRUE(r.GetBool());
    EXPECT_EQ(-1, r.GetInt8());
    EXPECT_EQ(-2, r.GetInt16());
    EXPECT_EQ(-3, r.GetInt32());
    EXPECT_EQ(-4, r.GetInt64());
    EXPECT_EQ(-5.0f, r.GetFloat());
    EXPECT_EQ(-6.0, r.GetDouble());
}

TEST(ByteBuffer, PutAndGetUnsigned) {
    char buf[33];

    // write
    ByteBuffer w(buf, sizeof(buf), ByteOrder::BigEndian);
    const unsigned char str1[] = "abcd";
    w.Put(str1, sizeof(str1));
    w.PutBool(false);
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
    EXPECT_EQ(0, memcmp(str1, str2, sizeof(str1)));
    EXPECT_FALSE(r.GetBool());
    EXPECT_EQ(1, r.GetUint8());
    EXPECT_EQ(2, r.GetUint16());
    EXPECT_EQ(3, r.GetUint32());
    EXPECT_EQ(4, r.GetUint64());
    EXPECT_EQ(5.0f, r.GetFloat());
    EXPECT_EQ(6.0, r.GetDouble());
}

TEST(ByteBuffer, GetInt32_BigEndian) {
    char buf[] = {0x00, 0x00, 0x00, 0x01};
    char copy[4];
    memcpy(copy, buf, sizeof(buf));

    ByteBuffer r(buf, sizeof(buf), ByteOrder::BigEndian);
    EXPECT_EQ(1, r.GetInt32());
    EXPECT_EQ(0, memcmp(buf, copy, sizeof(buf)));
}

TEST(ByteBuffer, GetInt32_LittleEndian) {
    char buf[] = {0x01, 0x00, 0x00, 0x00};
    char copy[4];
    memcpy(copy, buf, sizeof(buf));

    ByteBuffer r(buf, sizeof(buf), ByteOrder::LittleEndian);
    EXPECT_EQ(1, r.GetInt32());
    EXPECT_EQ(0, memcmp(buf, copy, sizeof(buf)));
}

TEST(ByteBuffer, PutInt32_BigEndian) {
    char buf[4] = {0};
    ByteBuffer w(buf, sizeof(buf), ByteOrder::BigEndian);
    w.PutInt32(1);
    EXPECT_EQ(0x00, buf[0]);
    EXPECT_EQ(0x00, buf[1]);
    EXPECT_EQ(0x00, buf[2]);
    EXPECT_EQ(0x01, buf[3]);
}

TEST(ByteBuffer, PutInt32_LittleEndian) {
    char buf[4] = {0};
    ByteBuffer w(buf, sizeof(buf), ByteOrder::LittleEndian);
    w.PutInt32(1);
    EXPECT_EQ(0x01, buf[0]);
    EXPECT_EQ(0x00, buf[1]);
    EXPECT_EQ(0x00, buf[2]);
    EXPECT_EQ(0x00, buf[3]);
}
