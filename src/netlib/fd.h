#pragma once

#if defined(_WIN32) || defined(_WIN64)
 #include <winsock2.h>
#endif // defined(_WIN32) || defined(_WIN64)

namespace net {

#if defined(_WIN32) || defined(_WIN64)
using SocketFD = SOCKET;
#else
using SocketFD = int;
#endif // defined(_WIN32) || defined(_WIN64)

} // namespace net
