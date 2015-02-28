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
