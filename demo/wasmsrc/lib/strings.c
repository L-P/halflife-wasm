#include <stddef.h>

size_t strlen(const char* str) {
    if (str == NULL) {
        return 0;
    }

    for (size_t i = 0; ; i++) {
        if (str[i] == '\0') {
            return i;
        }
    }
}

bool str_has_prefix(const char* str, const char* prefix) {
    if (str == NULL || prefix == NULL) {
        return false;
    }

    for (size_t i = 0;; i++) {
        if (prefix[i] == '\0') {
            return true;
        }

        if (str[i] == '\0') {
            return false;
        }

        if (str[i] != prefix[i]) {
            return false;
        }
    }
}

int atoi(const char* str) {
    if (str == NULL || strlen(str) < 1) {
        return 0;
    }

    int ret = 0;

    for (size_t i = 0; ; i++) {
        if (i == 0 && str[i] == '-') {
            continue;
        }

        if (str[i] == '\0') {
            break;
        }

        if (str[i] < '0' || str[i] > '9') {
            break;
        }

        ret = ret * 10 + (str[i] - '0');
    }

    return str[0] == '-' ? -ret : ret;
}

int strcmp(const char* lhs, const char* rhs) {
    if (lhs == NULL && rhs == NULL) {
        return 0;
    }

    if (lhs == NULL ^ rhs == NULL) {
        return lhs == NULL ? -1 : 1;
    }

    for (size_t i = 0; ; i++) {
        if (lhs[i] == rhs[i]) {
            if (lhs[i] == '\0') {
                return 0;
            }

            continue;
        }

        if (lhs[i] < rhs[i]) {
            return -1;
        }

        if (lhs[i] > rhs[i]) {
            return 1;
        }
    }
}
