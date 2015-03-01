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

#ifdef __cplusplus
#define bytes_construct_unsafe(DATA, LEN) (bytes{ reinterpret_cast<void *>(DATA), (LEN) })
#else
#define bytes_construct_unsafe(DATA, LEN) ((bytes){ (DATA), (LEN) })
#endif

// Constructors {{{

#define BYTES_STRUCT(SP) bytes_construct_unsafe((SP), sizeof *(SP))
#define BYTES_ARRAY(A) bytes_construct_unsafe((A), sizeof(A))
#define BYTES_LITERAL(S) bytes_construct_unsafe(("" S ""), sizeof(S) - 1)

#define BYTES_ON_STACK(NAME, SIZE) \
    uint8_t bytes_contents__##NAME[SIZE] = {0}; \
    struct bytes NAME = BYTES_ARRAY(bytes_contents__##NAME)

static inline struct bytes bytes_alloc(void *(*alloc)(size_t), size_t length) {
    return bytes_construct_unsafe(alloc(length), length);
}

// }}}

static inline uint8_t *bytes_mutable_data(struct bytes b) {
    return (uint8_t *)b.BYTES_INTERNAL_FIELD(data);
}

static inline const uint8_t *bytes_data(struct bytes b) {
    return (const uint8_t *)b.BYTES_INTERNAL_FIELD(data);
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

static inline void *bytes_internal_as_struct(bytes b, size_t sz) {
    return bytes_take(&b, b, sz) ? bytes_mutable_data(b) : NULL;
}

#define BYTES_AS_STRUCT(SP, B) ((*SP) = (__typeof__(*SP))bytes_internal_as_struct((B), sizeof **(SP)), (*SP) != NULL)

static inline bool bytes_copy(struct bytes to, struct bytes from) {
    if (bytes_length(to) < bytes_length(from)) return false;
    memcpy(bytes_mutable_data(to), bytes_data(from), bytes_length(from));
    return true;
}

static inline void bytes_zero(struct bytes b) {
    memset(bytes_mutable_data(b), 0, bytes_length(b));
}

// }}}

// Utilities {{{

static inline void bytes_drop_all(struct bytes *b) {
    bool valid = bytes_drop(b, *b, bytes_length(*b));
    assert(valid); (void)valid;
}

static inline bool bytes_slice(struct bytes *to, struct bytes from, size_t offset, size_t length) {
    return bytes_drop(to, from, offset) && bytes_take(to, *to, length);
}

static inline bool bytes_copy_slice(struct bytes to, struct bytes from, size_t offset, size_t length) {
    return bytes_slice(&from, from, offset, length) && bytes_copy(to, from);
}

#define BYTES_INT_TYPES_MAP_0(XX, S, T, E) \
    XX(8, S, T, E)                         \
    XX(16, S, T, E)                        \
    XX(32, S, T, E)                        \
    XX(64, S, T, E)
#define BYTES_INT_TYPES_MAP_1(XX, E)      \
    BYTES_INT_TYPES_MAP_0(XX, i, int, E)  \
    BYTES_INT_TYPES_MAP_0(XX, u, uint, E)
#define BYTES_INT_TYPES_MAP(XX)   \
    BYTES_INT_TYPES_MAP_1(XX, be) \
    BYTES_INT_TYPES_MAP_1(XX, le)

// Defines bytes_copy_{u,i}{16,32,64}_{le,be} for serializing integers
#define XX(N, S, T, E)                                                       \
    static inline bool bytes_copy_##S##N##_##E(struct bytes b, T##N##_t x) { \
        x = (T##N##_t)hto##E##N((uint##N##_t)x);                             \
        return bytes_copy(b, BYTES_STRUCT(&x));                              \
    }
BYTES_INT_TYPES_MAP(XX)
#undef XX

// Defines bytes_read_{u,i}{16,32,64}_{le,be} for deserializing integers
#define XX(N, S, T, E)                                                        \
    static inline bool bytes_read_##S##N##_##E(struct bytes b, T##N##_t *x) { \
        return bytes_copy_slice(BYTES_STRUCT(x), b, 0, sizeof *x)             \
               && (*x = (T##N##_t)E##N##toh((uint##N##_t)*x), true);          \
    }
BYTES_INT_TYPES_MAP(XX)
#undef XX

// }}}

// Builder {{{

typedef struct bytes_builder {
    struct bytes BYTES_INTERNAL_FIELD(target);
    size_t BYTES_INTERNAL_FIELD(offset);
} bytes_builder;

static inline struct bytes_builder bytes_builder_from(struct bytes b) {
    struct bytes_builder bb;
    bb.BYTES_INTERNAL_FIELD(target) = b;
    bb.BYTES_INTERNAL_FIELD(offset) = 0;
    return bb;
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

// Buffer {{{

typedef struct bytes_buffer {
    struct bytes bytes;
    struct bytes BYTES_INTERNAL_FIELD(original);
} bytes_buffer;

static inline struct bytes_buffer bytes_buffer_from(struct bytes b) {
    struct bytes_buffer bb;
    bb.bytes = b;
    bb.BYTES_INTERNAL_FIELD(original) = b;
    return bb;
}

static inline void bytes_buffer_reset(struct bytes_buffer *bb) {
    struct bytes original = bb->BYTES_INTERNAL_FIELD(original);
    assert(bytes_data(bb->bytes) >= bytes_data(original));
    assert(bytes_data(bb->bytes) + bytes_length(bb->bytes) <= bytes_data(original) + bytes_length(original));
    bb->bytes = original;
    bytes_zero(bb->bytes);
}

static inline void bytes_buffer_swap(struct bytes_buffer *a, struct bytes_buffer *b) {
    struct bytes_buffer tmp = *a;
    *a = *b;
    *b = tmp;
}

#define BYTE_BUFFER_ON_STACK(NAME, SIZE) \
    uint8_t bytes_contents__##NAME[SIZE] = {0}; \
    struct bytes_buffer NAME = bytes_buffer_from(BYTES_ARRAY(bytes_contents__##NAME))

// }}}


#endif
