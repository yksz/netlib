#include <cstdio>
#include <cstring>
#include <iostream>
#include "cx/io/binary.h"

using namespace cx;

int main(void) {
    char buf[32];

    // write
    ByteConverter w(buf, sizeof(buf), ByteOrder_BIG_ENDIAN);
    const char* str1 = "abcd";
    w.Put(str1, strlen(str1) + 1);
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
    printf("%s\n", str2);
    printf("%d\n", r.GetUint8());
    printf("%d\n", r.GetUint16());
    printf("%d\n", r.GetUint32());
    printf("%ld\n", r.GetUint64());
    printf("%f\n", r.GetFloat());
    printf("%f\n", r.GetDouble());
    return 0;
}
