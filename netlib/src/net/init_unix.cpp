#include "net/init.h"
#include <csignal>
#include <mutex>

namespace net {
namespace internal {

static void initOnce() {
    signal(SIGPIPE, SIG_IGN);
}

void init() {
    static std::once_flag flag;
    std::call_once(flag, initOnce);
}

} // namespace internal
} // namespace net
