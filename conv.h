#ifndef BYTES_CONV_H
#define BYTES_CONV_H

#include "bytes.h"

#ifdef __cplusplus

template <typename T>
bytes bytes_from(const T &s) {
    static_assert(sizeof s[0] == 1, "T must be a container of bytes");
    return bytes_construct_unsafe(&s[0], s.size());
}

inline bytes bytes_from(bytes b) { return b; }

#endif

#endif
