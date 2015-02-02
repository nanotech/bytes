#ifndef endian_h
#define endian_h
#if defined(__linux__)
#  include <endian.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#  include <sys/endian.h>
#elif defined(__OpenBSD__)
#  include <sys/types.h>
#  define le64toh(x) letoh64(x)
#elif defined(__APPLE__)
#  include <libkern/OSByteOrder.h>
#  define le64toh(x) OSSwapLittleToHostInt64(x)
#  define htole64(x) OSSwapHostToLittleInt64(x)
#endif
#endif
