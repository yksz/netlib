#pragma once

#include <string>
#include "net/error.h"

namespace net {

/**
 * @param[in] host
 * @param[out] addr
 */
error LookupAddress(const std::string& host, std::string* addr);

} // namespace net
