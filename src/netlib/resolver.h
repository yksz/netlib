#pragma once

#include <cstdint>
#include <string>
#include "netlib/error.h"
#include "netlib/fd.h"

namespace net {

/**
 * @param[in] host
 * @param[out] addr
 */
error LookupAddress(const std::string& host, std::string* addr);

/**
 * @param[in] fd
 * @param[out] port
 */
error LookupPort(const SocketFD& fd, uint16_t* port);

/**
 * @param[out] host
 */
error LookupLocalHostName(std::string* host);

/**
 * @param[out] addr
 */
error LookupLocalHostAddress(std::string* addr);

} // namespace net
