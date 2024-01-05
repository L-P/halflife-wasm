// Returns the length of the string excluding the NUL char.
// A NULL string length is 0.
size_t strlen(const char* str);

// Compares two string lexically (ASCII-speaking) and returns 0 if the two
// strings are identical.
// A NULL string only matches another NULL string and is considered lower than
// any string.
int strcmp(const char *lhs, const char *rhs);
