// Like assert.h, allow multiple inclusion with different NDEBUG.
#ifndef NDEBUG
#undef DEBUG_ONLY
#define DEBUG_ONLY(inside) inside
#else
#undef DEBUG_ONLY
#define DEBUG_ONLY(inside)
#endif

#undef DEBUG_ONLY_ASSERT
#define DEBUG_ONLY_ASSERT(inside) DEBUG_ONLY(assert(inside))
