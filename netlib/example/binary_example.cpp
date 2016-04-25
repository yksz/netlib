#include <cstdio>
#include <iostream>
#include "net/binary.h"

using namespace net;

int main(void) {
    char buf[32];

    // write
    ByteConverter w(buf, sizeof(buf), ByteOrder::BigEndian);
    const char str1[] = "abcd";
    w.Put(str1, sizeof(str1));
    w.PutUint8(1);
    w.PutUint16(2);
    w.PutUint32(3);
    w.PutUint64(4);
    w.PutFloat(5.0f);
    w.PutDouble(6.0);

    // read
    ByteConverter r(buf, sizeof(buf), ByteOrder::BigEndian);
    char str2[5] = {0};
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
