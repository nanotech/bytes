#ifndef bytes_h
#define bytes_h

#include <stddef.h>
#include <stdbool.h>
#include <strings.h>

#include "endian.h"

#ifdef BYTES_INTERNAL
#define BYTES_INTERNAL_FIELD(NAME) NAME
#else
#define BYTES_INTERNAL_FIELD(NAME) INTERNAL__##NAME
#endif

struct bytes {
    void *BYTES_INTERNAL_FIELD(data);
    size_t BYTES_INTERNAL_FIELD(length);
};

static inline uint8_t *bytes_mutable_data(struct bytes b) {
    return b.BYTES_INTERNAL_FIELD(data);
}

static inline const uint8_t *bytes_data(struct bytes b) {
    return b.BYTES_INTERNAL_FIELD(data);
}

static inline size_t bytes_length(struct bytes b) {
    return b.BYTES_INTERNAL_FIELD(length);
}

static inline bool bytes_copy_u64_le(struct bytes b, uint64_t x) {
    if (bytes_length(b) < sizeof(uint64_t)) return false;
    x = htole64(x);
    memcpy(bytes_mutable_data(b), &x, sizeof(uint64_t));
    return true;
}

#define BYTES_STRUCT(SP) (struct bytes){ .BYTES_INTERNAL_FIELD(data) = (SP), .BYTES_INTERNAL_FIELD(length) = sizeof *(SP) }
#define BYTES_ARRAY(S) (struct bytes){ .BYTES_INTERNAL_FIELD(data) = (S), .BYTES_INTERNAL_FIELD(length) = sizeof(S) }
#define BYTES_LITERAL(S) BYTES_ARRAY("" S "")

#define BYTES_ON_STACK(NAME, SIZE) \
    uint8_t bytes_contents__##NAME[SIZE] = {0}; \
    struct bytes NAME = BYTES_ARRAY(bytes_contents__##NAME)


struct bytes_builder {
    struct bytes BYTES_INTERNAL_FIELD(target);
    size_t BYTES_INTERNAL_FIELD(offset);
};

static inline void bytes_builder_init(struct bytes_builder *bb, struct bytes b) {
    *bb = (struct bytes_builder){
        .BYTES_INTERNAL_FIELD(target) = b,
        .BYTES_INTERNAL_FIELD(offset) = 0,
    };
}

static inline bool bytes_builder_append(struct bytes_builder *bb, struct bytes b) {
    if (bytes_length(bb->BYTES_INTERNAL_FIELD(target)) < bb->BYTES_INTERNAL_FIELD(offset) + bytes_length(b)) return false;
    memcpy(bytes_mutable_data(bb->BYTES_INTERNAL_FIELD(target)) + bb->BYTES_INTERNAL_FIELD(offset), bytes_data(b), bytes_length(b));
    bb->BYTES_INTERNAL_FIELD(offset) += bytes_length(b);
    return true;
}

static inline struct bytes bytes_builder_built_bytes(struct bytes_builder *bb) {
    return (struct bytes){
        .BYTES_INTERNAL_FIELD(data) = bb->BYTES_INTERNAL_FIELD(target).BYTES_INTERNAL_FIELD(data),
        .BYTES_INTERNAL_FIELD(length) = bb->BYTES_INTERNAL_FIELD(offset),
    };
}

#endif
