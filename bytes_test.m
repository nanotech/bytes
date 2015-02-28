#import <Foundation/Foundation.h>
#import <XCTest/XCTest.h>
#import "bytes.h"

@interface bytes_test : XCTestCase

@end

@implementation bytes_test

- (void)testStringLiteral {
	struct bytes b = BYTES_LITERAL("Hello");
	XCTAssert(bytes_length(b) == 5);
	XCTAssert(memcmp(bytes_data(b), "Hello", 5) == 0);
}

- (void)testEqual {
	char hello[] = {'H', 'e', 'l', 'l', 'o'}; // no trailing null
	XCTAssert(bytes_equal(BYTES_ARRAY(hello), BYTES_LITERAL("Hello")));
	XCTAssertFalse(bytes_equal(BYTES_ARRAY(hello), BYTES_LITERAL("Goodbye")));
	XCTAssertFalse(bytes_equal(BYTES_ARRAY(hello), BYTES_LITERAL("Hello world")));
	XCTAssertFalse(bytes_equal(BYTES_LITERAL("Hello world"), BYTES_ARRAY(hello)));
}

- (void)testCopy {
	BYTES_ON_STACK(a, 6);
	BYTES_ON_STACK(b, 6);
	BYTES_ON_STACK(c, 7);
	XCTAssert(bytes_copy(a, BYTES_LITERAL("aaaaa")));
	XCTAssert(memcmp(bytes_data(a), "aaaaa", 6) == 0);
	XCTAssert(bytes_copy(b, BYTES_LITERAL("bbbbb")));
	XCTAssert(memcmp(bytes_data(b), "bbbbb", 6) == 0);
	XCTAssert(bytes_copy(c, BYTES_LITERAL("cccccc")));
	XCTAssert(memcmp(bytes_data(c), "cccccc", 7) == 0);

	XCTAssert(bytes_copy(a, b));
	XCTAssert(memcmp(bytes_data(a), "bbbbb", 6) == 0);
	XCTAssertFalse(bytes_copy(b, c));
	XCTAssert(memcmp(bytes_data(b), "bbbbb", 6) == 0);
}

- (void)testCopyInts {
    uint8_t xs[8];
    bytes b = BYTES_ARRAY(xs);
    XCTAssert(bytes_copy_i16_be(b, 0x0305));
    XCTAssertEqual(xs[0], 3);
    XCTAssertEqual(xs[1], 5);

    XCTAssert(bytes_copy_i16_le(b, 0x0206));
    XCTAssertEqual(xs[0], 6);
    XCTAssertEqual(xs[1], 2);

    XCTAssert(bytes_copy_u32_be(b, 0x89504e47));
    XCTAssertEqual(xs[0], 0x89);
    XCTAssertEqual(xs[1], 'P');
    XCTAssertEqual(xs[2], 'N');
    XCTAssertEqual(xs[3], 'G');

    XCTAssert(bytes_copy_u32_le(b, 0x76543210UL));
    XCTAssertEqual(xs[0], 0x10);
    XCTAssertEqual(xs[1], 0x32);
    XCTAssertEqual(xs[2], 0x54);
    XCTAssertEqual(xs[3], 0x76);

    XCTAssert(bytes_copy_u64_be(b, 0x7654321001abcdefULL));
    XCTAssertEqual(xs[0], 0x76);
    XCTAssertEqual(xs[1], 0x54);
    XCTAssertEqual(xs[2], 0x32);
    XCTAssertEqual(xs[3], 0x10);
    XCTAssertEqual(xs[4], 0x01);
    XCTAssertEqual(xs[5], 0xab);
    XCTAssertEqual(xs[6], 0xcd);
    XCTAssertEqual(xs[7], 0xef);

    BYTES_ON_STACK(small, 7);
    XCTAssertFalse(bytes_copy_u64_le(small, 0));
}

- (void)testReadInts {
    BYTES_ON_STACK(small, 1);
    BYTES_ON_STACK(b, 8);
    uint32_t xu32 = 0x89504e47;
    XCTAssert(bytes_copy_u32_be(b, xu32));
    uint32_t xu32r = 3;
    XCTAssertFalse(bytes_read_u32_be(small, &xu32r));
    XCTAssertEqual(xu32r, 3U);
    XCTAssert(bytes_read_u32_be(b, &xu32r));
    XCTAssertEqual(xu32r, xu32);
}

- (void)testSlice {
	struct bytes b = BYTES_LITERAL("Hello world!");
	struct bytes w;
	XCTAssert(bytes_drop(&w, b, 6));
	XCTAssert(bytes_equal(w, BYTES_LITERAL("world!")));
	XCTAssert(bytes_take(&w, w, 5));
	XCTAssert(bytes_equal(w, BYTES_LITERAL("world")));

	XCTAssert(bytes_slice(&w, b, 6, 5));
	XCTAssert(bytes_equal(w, BYTES_LITERAL("world")));
}

- (void)testBuilder {
	BYTES_ON_STACK(buf, 12);
	struct bytes_builder u = bytes_builder_from(buf);

	XCTAssert(bytes_equal(bytes_builder_built_bytes(&u), BYTES_LITERAL("")));
	XCTAssert(bytes_builder_append(&u, BYTES_LITERAL("Hello")));
	XCTAssert(bytes_equal(bytes_builder_built_bytes(&u), BYTES_LITERAL("Hello")));
	XCTAssert(bytes_builder_append(&u, BYTES_LITERAL(" world!")));
	XCTAssertFalse(bytes_builder_append(&u, BYTES_LITERAL("!"))); // buf is full
	XCTAssert(bytes_equal(bytes_builder_built_bytes(&u), BYTES_LITERAL("Hello world!")));
}

@end
