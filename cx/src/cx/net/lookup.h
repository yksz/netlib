#pragma once

#include <string>
#include "cx/error.h"

namespace cx {

/**
 * @param[in] host
 * @param[out] addr
 */
error LookupAddress(const std::string& host, std::string* addr);

} // namespace cx
