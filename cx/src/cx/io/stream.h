#pragma once

#include <cstddef>

namespace cx {

struct Closer {
    virtual ~Closer() {};
    virtual void Close() = 0;
};

struct Reader {
    virtual ~Reader() {};
    virtual int Read(char* buf, size_t len) = 0;
};

struct Writer {
    virtual ~Writer() {};
    virtual int Write(const char* buf, size_t len) = 0;
};

} // namespace cx
