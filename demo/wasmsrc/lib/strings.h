// Returns the length of the string excluding the NUL char.
// A NULL string length is 0.
size_t strlen(const char* str);

// Compares two string lexically (ASCII-speaking) and returns 0 if the two
// strings are identical.
// A NULL string only matches another NULL string and is considered lower than
// any string.
int strcmp(const char *lhs, const char *rhs);

// Parses a base 10 integer. Any non-decimal digit will end the parsing and the
// integer parsed up to this point will be returned.
int atoi(const char* str);

// Returns true if str starts with the given prefix.
// If any of the strings are NULL the result is false.
// An empty prefix will always match.
bool str_has_prefix(const char* str, const char* prefix);
