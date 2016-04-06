#pragma once

#include <cstddef>

namespace cx {

bool ReadFully(Reader& r, char* buf, size_t len);
bool WriteFully(Writer& w, const char* buf, size_t len);

} // namespace cx
