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

#define bytes_construct_unsafe(DATA, LEN)     \
    ((bytes){                                 \
        .BYTES_INTERNAL_FIELD(data) = (DATA), \
        .BYTES_INTERNAL_FIELD(length) = (LEN) \
    })

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

static inline void *bytes_internal_as_struct(bytes b, size_t sz) {
    return bytes_take(&b, b, sz) ? bytes_mutable_data(b) : NULL;
}

#define BYTES_AS_STRUCT(SP, B) ((*SP) = bytes_internal_as_struct((B), sizeof **(SP)), (*SP) != NULL)

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

// Buffer {{{

typedef struct bytes_buffer {
    struct bytes bytes;
    struct bytes BYTES_INTERNAL_FIELD(original);
} bytes_buffer;

static inline struct bytes_buffer bytes_buffer_from(struct bytes b) {
    return (struct bytes_buffer){
        .bytes = b,
        .BYTES_INTERNAL_FIELD(original) = b,
    };
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
