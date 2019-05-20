#pragma once

#include <cstddef>
#include "netlib/error.h"

namespace net {

struct Closer {
    virtual ~Closer() {}
    virtual error Close() = 0;
};

struct Reader {
    virtual ~Reader() {}
    /**
     * @param[in] buf
     * @param[in] len
     * @param[out] nbytes
     */
    virtual error Read(char* buf, size_t len, int* nbytes) = 0;
    error ReadFull(char* buf, size_t len);
    /**
     * Read a single line including the line end ('\n').
     * Read a len-1 bytes if the line end is not found,
     * and a null-character ('\0') is implicity appended
     * at the end of buf.
     *
     * @param[in] buf
     * @param[in] len
     */
    error ReadLine(char* buf, size_t len);
};

struct Writer {
    virtual ~Writer() {}
    /**
     * @param[in] buf
     * @param[in] len
     * @param[out] nbytes
     */
    virtual error Write(const char* buf, size_t len, int* nbytes) = 0;
    error WriteFull(const char* buf, size_t len);
};

struct ReadWriter : public Reader, public Writer {};

struct ReadWriteCloser : public ReadWriter, public Closer {};

} // namespace net
