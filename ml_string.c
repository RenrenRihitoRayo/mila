#pragma once

#include "mila.h"
#include <string.h>

char* mila_strdup(const char* s) {
    if (!s) return NULL;

    size_t len = strlen(s);
    char* out = (char*)mila_malloc(len + 1);
    if (!out) return NULL;

    for (size_t i = 0; i <= len; i++) {
        out[i] = s[i];
    }
    return out;
}

char* mila_strndup(const char* s, size_t n) {
    if (!s) return NULL;

    size_t len = 0;
    while (len < n && s[len]) len++;

    char* out = (char*)mila_malloc(len + 1);
    if (!out) return NULL;

    for (size_t i = 0; i < len; i++) {
        out[i] = s[i];
    }
    out[len] = '\0';

    return out;
}

char* mila_strcat_alloc(const char* a, const char* b) {
    if (!a || !b) return NULL;

    size_t len_a = strlen(a);
    size_t len_b = strlen(b);

    char* out = (char*)mila_malloc(len_a + len_b + 1);
    if (!out) return NULL;

    for (size_t i = 0; i < len_a; i++) {
        out[i] = a[i];
    }
    for (size_t i = 0; i < len_b; i++) {
        out[len_a + i] = b[i];
    }

    out[len_a + len_b] = '\0';
    return out;
}

char* mila_strappend(char* base, const char* suffix) {
    if (!suffix) return base;

    size_t len_base = base ? strlen(base) : 0;
    size_t len_suf  = strlen(suffix);

    char* out = (char*)mila_realloc(base, len_base + len_suf + 1);
    if (!out) return NULL;

    for (size_t i = 0; i <= len_suf; i++) {
        out[len_base + i] = suffix[i]; // includes null terminator
    }

    return out;
}