#include "cx/io/ioutil.h"
#include <cassert>

namespace cx {

bool Reader::ReadFully(char* buf, size_t len) {
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

bool Writer::WriteFully(const char* buf, size_t len) {
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
