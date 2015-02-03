#ifndef bytes_h
#define bytes_h

#include <stddef.h>
#include <stdbool.h>
#include <strings.h>
#include <assert.h>

#include "endian.h"

#ifdef BYTES_INTERNAL
#define BYTES_INTERNAL_FIELD(NAME) NAME
#else
#define BYTES_INTERNAL_FIELD(NAME) INTERNAL__##NAME
#endif

// Core {{{

typedef struct bytes {
    void *BYTES_INTERNAL_FIELD(data);
    size_t BYTES_INTERNAL_FIELD(length);
} bytes;

static inline struct bytes bytes_construct_unsafe(void *data, size_t length) {
    return (struct bytes){ .BYTES_INTERNAL_FIELD(data) = data, .BYTES_INTERNAL_FIELD(length) = length };
}

// Constructors {{{

#define BYTES_STRUCT(SP) bytes_construct_unsafe((SP), sizeof *(SP))
#define BYTES_ARRAY(A) bytes_construct_unsafe((A), sizeof(A))
#define BYTES_LITERAL(S) bytes_construct_unsafe(("" S ""), sizeof(S) - 1)

#define BYTES_ON_STACK(NAME, SIZE) \
    uint8_t bytes_contents__##NAME[SIZE] = {0}; \
    struct bytes NAME = BYTES_ARRAY(bytes_contents__##NAME)

// }}}

static inline uint8_t *bytes_mutable_data(struct bytes b) {
    return b.BYTES_INTERNAL_FIELD(data);
}

static inline const uint8_t *bytes_data(struct bytes b) {
    return b.BYTES_INTERNAL_FIELD(data);
}

static inline size_t bytes_length(struct bytes b) {
    return b.BYTES_INTERNAL_FIELD(length);
}

static inline bool bytes_equal(struct bytes a, struct bytes b) {
    return bytes_length(a) == bytes_length(b)
        && memcmp(bytes_data(a), bytes_data(b), bytes_length(b)) == 0;
}

static inline bool bytes_take(struct bytes *to, struct bytes from, size_t n) {
    if (bytes_length(from) < n) return false;
    *to = bytes_construct_unsafe(bytes_mutable_data(from), n);
    return true;
}

static inline bool bytes_drop(struct bytes *to, struct bytes from, size_t n) {
    if (bytes_length(from) < n) return false;
    *to = bytes_construct_unsafe(bytes_mutable_data(from) + n, bytes_length(from) - n);
    return true;
}

static inline bool bytes_copy(struct bytes to, struct bytes from) {
    if (bytes_length(to) < bytes_length(from)) return false;
    memcpy(bytes_mutable_data(to), bytes_data(from), bytes_length(from));
    return true;
}

// }}}

// Utilities {{{

static inline bool bytes_slice(struct bytes *to, struct bytes from, size_t offset, size_t length) {
    return bytes_drop(to, from, offset) && bytes_take(to, *to, length);
}

static inline bool bytes_copy_slice(struct bytes to, struct bytes from, size_t offset, size_t length) {
    return bytes_slice(&from, from, offset, length) && bytes_copy(to, from);
}

static inline bool bytes_copy_u64_le(struct bytes b, uint64_t x) {
    x = htole64(x);
    return bytes_copy(b, BYTES_STRUCT(&x));
}

// }}}

// Builder {{{

typedef struct bytes_builder {
    struct bytes BYTES_INTERNAL_FIELD(target);
    size_t BYTES_INTERNAL_FIELD(offset);
} bytes_builder;

static inline void bytes_builder_init(struct bytes_builder *bb, struct bytes b) {
    *bb = (struct bytes_builder){
        .BYTES_INTERNAL_FIELD(target) = b,
        .BYTES_INTERNAL_FIELD(offset) = 0,
    };
}

static inline bool bytes_builder_append(struct bytes_builder *bb, struct bytes b) {
    struct bytes target = bb->BYTES_INTERNAL_FIELD(target);
    bool valid = bytes_drop(&target, target, bb->BYTES_INTERNAL_FIELD(offset));
    assert(valid); (void)valid;
    bool ok = bytes_copy(target, b);
    if (ok) bb->BYTES_INTERNAL_FIELD(offset) += bytes_length(b);
    return ok;
}

static inline struct bytes bytes_builder_built_bytes(const struct bytes_builder *bb) {
    struct bytes built = bb->BYTES_INTERNAL_FIELD(target);
    bool valid = bytes_take(&built, built, bb->BYTES_INTERNAL_FIELD(offset));
    assert(valid); (void)valid;
    return built;
}

// }}}

#endif