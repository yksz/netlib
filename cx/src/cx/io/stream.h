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
    bool ReadFully(char* buf, size_t len);
    bool ReadLine(char* buf, size_t len);
};

struct Writer {
    virtual ~Writer() {};
    virtual int Write(const char* buf, size_t len) = 0;
    bool WriteFully(const char* buf, size_t len);
};

} // namespace cx
