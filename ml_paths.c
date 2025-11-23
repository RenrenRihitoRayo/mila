#pragma once
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#endif

typedef struct {
    char **items;
    int count;
    int capacity;
} path_list;

// =========================================================
// internal: simple helpers
// =========================================================

static char *str_dup(const char *s) {
    size_t n = strlen(s) + 1;
    char *out = malloc(n);
    if (out) memcpy(out, s, n);
    return out;
}

static int file_exists(const char *p) {
    struct stat st;
    return stat(p, &st) == 0;
}

// =========================================================
// transform_path: expands ~, expands $VAR, normalizes slashes
// =========================================================

static void normalize_slashes(char *buf) {
#ifdef _WIN32
    const char from = '/', to = '\\';
#else
    const char from = '\\', to = '/';
#endif

    char *src = buf, *dst = buf;
    int last_sep = 0;

    while (*src) {
        char c = *src++;

        if (c == from) c = to;

        // collapse duplicate separators
        if (c == to) {
            if (last_sep) continue;
            last_sep = 1;
        } else {
            last_sep = 0;
        }

        *dst++ = c;
    }
    *dst = '\0';
}

static const char *get_env(const char *name, size_t len) {
    char var[256];
    if (len >= sizeof(var)) return NULL;
    memcpy(var, name, len);
    var[len] = '\0';
    return getenv(var);
}

// Expand $VAR and ${VAR}
static void expand_env(char **bufptr) {
    char *in = *bufptr;
    size_t outcap = strlen(in) * 2 + 64; // generous
    char *out = malloc(outcap);
    size_t o = 0;

    for (size_t i = 0; in[i]; ) {
        if (in[i] == '$') {
            if (in[i+1] == '{') {
                size_t j = i + 2;
                while (in[j] && in[j] != '}') j++;
                if (in[j] == '}') {
                    const char *val = get_env(in + i + 2, j - (i + 2));
                    if (val) {
                        size_t vl = strlen(val);
                        memcpy(out + o, val, vl);
                        o += vl;
                    }
                    i = j + 1;
                    continue;
                }
            } else {
                size_t j = i + 1;
                while (isalnum(in[j]) || in[j] == '_') j++;
                if (j > i + 1) {
                    const char *val = get_env(in + i + 1, j - (i + 1));
                    if (val) {
                        size_t vl = strlen(val);
                        memcpy(out + o, val, vl);
                        o += vl;
                    }
                    i = j;
                    continue;
                }
            }
        }

        out[o++] = in[i++];
    }
    out[o] = '\0';

    free(in);
    *bufptr = out;
}

// Expand `~` → HOME or USERPROFILE
static void expand_home(char **bufptr) {
    char *in = *bufptr;

    if (in[0] != '~')
        return;

    const char *home =
#ifdef _WIN32
        getenv("USERPROFILE");
#else
        getenv("HOME");
#endif

    if (!home) return;

    size_t hl = strlen(home);
    size_t rl = strlen(in);

    char *out = malloc(hl + rl + 1);
    strcpy(out, home);
    strcat(out, in + 1);

    free(in);
    *bufptr = out;
}

// full transform
char *transform_path(const char *input) {
    if (!input) return NULL;

    char *buf = str_dup(input);

    expand_home(&buf);
    expand_env(&buf);
    normalize_slashes(buf);

    return buf;
}

// =========================================================
// path_list management
// =========================================================

path_list *path_list_new(void) {
    path_list *p = malloc(sizeof(path_list));
    if (!p) return NULL;
    p->count = 0;
    p->capacity = 4;
    p->items = malloc(sizeof(char*) * p->capacity);
    if (!p->items) {
        free(p);
        return NULL;
    }
    return p;
}

void path_list_free(path_list *pl) {
    if (!pl) return;
    for (int i = 0; i < pl->count; i++)
        free(pl->items[i]);
    free(pl->items);
    free(pl);
}

int path_list_add(path_list *pl, const char *path) {
    if (!pl || !path) return 0;

    if (pl->count == pl->capacity) {
        pl->capacity *= 2;
        char **ni = realloc(pl->items, sizeof(char*) * pl->capacity);
        if (!ni) return 0;
        pl->items = ni;
    }

    char *t = transform_path(path);
    if (!t) return 0;

    pl->items[pl->count++] = t;
    return 1;
}

int path_list_remove(path_list *pl, const char *path) {
    if (!pl || !path) return 0;

    char *t = transform_path(path);
    if (!t) return 0;

    for (int i = 0; i < pl->count; i++) {
        if (strcmp(pl->items[i], t) == 0) {
            free(pl->items[i]);
            for (int j = i; j < pl->count - 1; j++)
                pl->items[j] = pl->items[j+1];
            pl->count--;
            free(t);
            return 1;
        }
    }

    free(t);
    return 0;
}

// =========================================================
// file searching
// =========================================================

char *path_list_find(path_list *pl, const char *file) {
    if (!pl || !file) return NULL;

    char *tfile = transform_path(file);
    if (file_exists(tfile))
        return tfile;

    for (int i = 0; i < pl->count; i++) {
        const char *root = pl->items[i];

        size_t rl = strlen(root);
        size_t fl = strlen(tfile);
        size_t need = rl + 1 + fl + 1;

        char *full = malloc(need);
        strcpy(full, root);

        // add separator if missing
        char sep =
#ifdef _WIN32
            '\\';
#else
            '/';
#endif

        if (rl > 0 && root[rl - 1] != sep)
            full[rl] = sep, full[rl+1] = '\0';

        strcat(full, tfile);

        if (file_exists(full)) {
            free(tfile);
            return full;
        }

        free(full);
    }

    free(tfile);
    return NULL;
}

// other

int path_get_cwd(char *out, size_t n)
{
#if defined(_WIN32) || defined(_WIN64)
    return _getcwd(out, n) ? 0 : -1;
#else
    return getcwd(out, n) ? 0 : -1;
#endif
}