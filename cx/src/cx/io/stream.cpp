#include "cx/io/stream.h"
#include <cassert>

namespace cx {

bool Reader::ReadFully(char* buf, size_t len) {
    if (buf == nullptr) {
        assert(0 && "buf must not be nullptr");
        return false;
    }

    size_t offset = 0;
    int size;
    while ((size = Read(&buf[offset], len - offset)) > 0) {
        offset += size;
        if (offset >= len) {
            return true;
        }
    }
    return false;
}

bool Reader::ReadLine(char* buf, size_t len) {
    if (buf == nullptr) {
        assert(0 && "buf must not be nullptr");
        return false;
    }

    size_t offset = 0;
    int size;
    while ((size = Read(&buf[offset], 1)) > 0) {
        if (buf[offset] == '\n') {
            buf[offset + 1] = '\0';
            return true;
        }
        offset += size;
        if (offset + 1 >= len) {
            return true; // buf is full
        }
    }
    return false;
}

bool Writer::WriteFully(const char* buf, size_t len) {
    if (buf == nullptr) {
        assert(0 && "buf must not be nullptr");
        return false;
    }

    size_t offset = 0;
    int size;
    while ((size = Write(&buf[offset], len - offset)) > 0) {
        offset += size;
        if (offset >= len) {
            return true;
        }
    }
    return false;
}

} // namespace cx
