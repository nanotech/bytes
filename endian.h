#ifndef endian_h
#define endian_h
#if defined(__linux__)
#  include <endian.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#  include <sys/endian.h>
#elif defined(__APPLE__)
#  include <libkern/OSByteOrder.h>
#  define le64toh(x) OSSwapLittleToHostInt64(x)
#  define htole64(x) OSSwapHostToLittleInt64(x)
#endif
#endif
