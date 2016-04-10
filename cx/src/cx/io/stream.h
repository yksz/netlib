#pragma once

#include <cstddef>
#include "cx/error.h"

namespace cx {

struct Closer {
    virtual ~Closer() {};
    virtual error Close() = 0;
};

struct Reader {
    virtual ~Reader() {};
    /**
     * @param[in] buf
     * @param[in] len
     * @param[out] nbytes
     */
    virtual error Read(char* buf, size_t len, int* nbytes) = 0;
    error ReadFully(char* buf, size_t len);
    error ReadLine(char* buf, size_t len);
};

struct Writer {
    virtual ~Writer() {};
    /**
     * @param[in] buf
     * @param[in] len
     * @param[out] nbytes
     */
    virtual error Write(const char* buf, size_t len, int* nbytes) = 0;
    error WriteFully(const char* buf, size_t len);
};

} // namespace cx
