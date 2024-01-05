#define EXPORT __attribute__((visibility("default")))

// Those rely on the bulk memory proposal and clang -mbulk-memory flag.
#define memcpy(dst, src, size) __builtin_memcpy(dst, src, size)
#define memset(dst, value, size) __builtin_memset(dst, value, size)
