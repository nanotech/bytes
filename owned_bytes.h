#ifndef owned_bytes_h
#define owned_bytes_h

#include "bytes.h"

#include <memory>
#include <stddef.h>
#include <stdint.h>

struct owned_bytes {
    std::unique_ptr<uint8_t[]> data;
    size_t length;

    owned_bytes(size_t n) : data(new uint8_t[n]), length(n) {}
    owned_bytes(bytes b) : owned_bytes(bytes_length(b)) { bytes_copy(get(), b); }

    bytes get() { return bytes_construct_unsafe(data.get(), length); }
};

#endif
