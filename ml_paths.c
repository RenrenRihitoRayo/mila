#pragma once
#include "mila.h"
#include "ml_string.c"
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#endif


#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#define PATH_GETCWD _getcwd
#else
#include <unistd.h>
#define PATH_GETCWD getcwd
#endif

typedef struct path_list path_list;
struct path_list {
    char **items;
    int count;
    int capacity;
};

// =========================================================
// internal: simple helpers
// =========================================================

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
    char var[1024];
    if (len >= sizeof(var)) return NULL;
    memcpy(var, name, len);
    var[len] = '\0';
    return getenv(var);
}

// Expand $VAR and ${VAR}
static void expand_env(char **bufptr) {
    char *in = *bufptr;
    size_t outcap = strlen(in) * 2 + 64; // generous
    char *out = mila_malloc(outcap);
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

    mila_free(in);
    *bufptr = out;
}

void path_join(char *out, size_t outsize, int count, ...) {
    if (!out || outsize == 0) return;

    out[0] = '\0'; // start empty

    va_list args;
    va_start(args, count);

    for (int i = 0; i < count; i++) {
        const char *part = va_arg(args, const char*);
        if (!part || !*part) continue;

        // Remove trailing slash from out
        size_t len = strlen(out);
        while (len > 0 && (out[len-1] == '/' || out[len-1] == '\\')) {
            out[len-1] = '\0';
            len--;
        }

        // Remove leading slash from part (except for first segment)
        const char *start = part;
        if (len > 0 && (*start == '/' || *start == '\\')) start++;

        // Append slash if needed
        if (len > 0 && (out[len-1] != '/' && out[len-1] != '\\')) {
#ifdef _WIN32
            strncat(out, "\\", outsize - strlen(out) - 1);
#else
            strncat(out, "/", outsize - strlen(out) - 1);
#endif
        }

        // Append the segment
        strncat(out, start, outsize - strlen(out) - 1);
    }

    va_end(args);

    normalize_slashes(out);
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

    char *out = mila_malloc(hl + rl + 1);
    strcpy(out, home);
    strcat(out, in + 1);

    mila_free(in);
    *bufptr = out;
}

// full transform
char *transform_path(const char *input) {
    if (!input) return NULL;

    char *buf = mila_strdup(input);

    expand_home(&buf);
    expand_env(&buf);
    normalize_slashes(buf);

    return buf;
}

// =========================================================
// path_list management
// =========================================================

path_list *path_list_new(void) {
    path_list *p = mila_malloc(sizeof(path_list));
    if (!p) return NULL;
    p->count = 0;
    p->capacity = 4;
    p->items = mila_malloc(sizeof(char*) * p->capacity);
    if (!p->items) {
        mila_free(p);
        return NULL;
    }
    return p;
}

// Get the directory part of a path
void path_dirname(const char *path, char *out, size_t outsize) {
    if (!path || !*path) {
        strncpy(out, ".", outsize);
        out[outsize-1] = '\0';
        return;
    }

    char tmp[1024];
    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp)-1] = '\0';

    normalize_slashes(tmp);

    // remove trailing slashes
    size_t len = strlen(tmp);
    while (len > 1 && (tmp[len-1] == '/' || tmp[len-1] == '\\')) {
        tmp[len-1] = '\0';
        len--;
    }

    // find last slash
    char *slash = strrchr(tmp, '/');
#ifdef _WIN32
    if (!slash) slash = strrchr(tmp, '\\');
#endif

    if (!slash) {
        // no slash found
        strncpy(out, ".", outsize);
    } else if (slash == tmp) {
        // path like "/foo"
        strncpy(out, tmp, outsize);
    } else {
        *slash = '\0';
        strncpy(out, tmp, outsize);
    }

    out[outsize-1] = '\0';
}

// Get the file/base name part of a path
void path_basename(const char *path, char *out, size_t outsize) {
    if (!path || !*path) {
        strncpy(out, ".", outsize);
        out[outsize-1] = '\0';
        return;
    }

    char tmp[1024];
    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp)-1] = '\0';

    normalize_slashes(tmp);

    // remove trailing slashes
    size_t len = strlen(tmp);
    while (len > 1 && (tmp[len-1] == '/' || tmp[len-1] == '\\')) {
        tmp[len-1] = '\0';
        len--;
    }

    // find last slash
    char *slash = strrchr(tmp, '/');
#ifdef _WIN32
    if (!slash) slash = strrchr(tmp, '\\');
#endif

    if (!slash)
        strncpy(out, tmp, outsize);
    else
        strncpy(out, slash + 1, outsize);

    out[outsize-1] = '\0';
}

void path_list_free(path_list *pl) {
    if (!pl) return;
    for (int i = 0; i < pl->count; i++)
        mila_free(pl->items[i]);
    mila_free(pl->items);
    mila_free(pl);
}

int path_list_add(path_list *pl, const char *path) {
    if (!pl || !path) return 0;

    if (pl->count == pl->capacity) {
        pl->capacity *= 2;
        char **ni = mila_realloc(pl->items, sizeof(char*) * pl->capacity);
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
            mila_free(pl->items[i]);
            for (int j = i; j < pl->count - 1; j++)
                pl->items[j] = pl->items[j+1];
            pl->count--;
            mila_free(t);
            return 1;
        }
    }

    mila_free(t);
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

        char *full = mila_malloc(need);
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
        puts(full);
        if (file_exists(full)) {
            mila_free(tfile);
            return full;
        }
        mila_free(full);
    }
    mila_free(tfile);
    return NULL;
}

// other


// Return malloc'd absolute path of current working directory. Caller must mila_free.
char* path_get_cwd(void) {
    char *cwd = PATH_GETCWD(NULL, 0); // allocate buffer
    if (!cwd) return NULL;

    normalize_slashes(cwd);
    return cwd;
}