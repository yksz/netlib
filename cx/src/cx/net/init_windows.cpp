#include "cx/net/init.h"
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

namespace cx {
namespace internal {

static void cleanup() {
    WSACleanup();
}

static void initOnce() {
    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 0), &data) != 0) {
        fprintf(stderr, "ERROR: WSAStartup: %d\n", WSAGetLastError());
    } else {
        atexit(cleanup);
    }
}

void init() {
    static std::once_flag flag;
    std::call_once(flag, initOnce);
}

} // namespace internal
} // namespace cx
