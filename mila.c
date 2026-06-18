// This project is licensed under the GNU Affero General Public License

/*
 * MiLa
 * A modern programming language
 * the smallest it can get.
 * Welcome to the MiLa Language Implementation.
 */

#include <stdalign.h>
#define _GNU_SOURCE

#include "blr.c"
#include "ml_dict.c"
#include "ml_primitives.c"
#include <stddef.h>
#include <sys/types.h>
#if !(defined(__GNUC__) || defined(__clang__))
#error "MiLa only supports GCC and Clang."
#endif

#include <stdatomic.h>
#include <signal.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#include <psapi.h>
#include <windows.h>
#else
#include <dlfcn.h>
#include <limits.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#ifndef SAFE_BUILD
#include "ml_paths.c"
#endif

#include "ml_string.c"

#include "ml_builtins.c"

#include "ml_threading.c"

#undef MILA_PROTO
#include "ml_maths.c"

#include "mila.h"

CleanupRegistry *cleanup_registry = NULL;

void print_memory_usage()
{
    size_t memory_usage = 0;

#if defined(_WIN32) || defined(_WIN64)
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    memory_usage = pmc.WorkingSetSize;
#else
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    memory_usage = usage.ru_maxrss * 1024;
#endif

    // Convert to appropriate unit
    double memory_usage_d = (double)memory_usage;
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    while (memory_usage_d >= 1024 && unit_index < 4)
    {
        memory_usage_d /= 1024;
        unit_index++;
    }

    printf("Memory usage: %.2f %s\n", memory_usage_d, units[unit_index]);
}

#if defined(_WIN32) || defined(_WIN64)
unsigned long get_process_id(void)
{
    return GetCurrentProcessId();
}
#else
unsigned long get_process_id(void)
{
    return (unsigned long)getpid();
}
#endif

path_list *search_path = NULL;

// Cleanup

CleanupRegistry *make_cleanup_registry()
{
    CleanupRegistry *registry = (CleanupRegistry *)mila_malloc(sizeof(CleanupRegistry));
    registry->count = 0;
    registry->size = 0;
    registry->registry = NULL;
    return registry;
}

CleanupRegistryEntry *make_cleanup_entry(char *name, void (*fn)(Env *))
{
    CleanupRegistryEntry *entry = (CleanupRegistryEntry *)mila_malloc(sizeof(CleanupRegistryEntry));
    entry->name = mila_strdup(name);
    entry->fn = fn;
    return entry;
}

void free_cleanup_registry(CleanupRegistry *registry)
{
    for (size_t index = 0; index < registry->count; ++index)
    {
        mila_free(registry->registry[index]->name);
        mila_free(registry->registry[index]);
    }
    mila_free(registry->registry);
    mila_free(registry);
}

// ---------- Value representation ----------

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

inline void *mila_malloc(size_t size)
{
    void *ptr = malloc(size);
    memset(ptr, 0, size);
    return ptr;
}
inline void *mila_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}
inline void mila_free(void *ptr)
{
    free(ptr);
}

void float_to_string(float f, char *buf, size_t bufsize)
{
    // Step 1: try %g with max precision
    snprintf(buf, bufsize, "%.10g", f); // 9 digits for float

    if (!isdigit(buf[strlen(buf) - 1]))
        return;

    // Step 2: check if there's a decimal point or exponent
    if (strchr(buf, '.') == NULL && strchr(buf, 'e') == NULL &&
        strchr(buf, 'E') == NULL)
    {
        // integer-looking float, force .0
        size_t len = strlen(buf);
        if (len + 2 < bufsize)
        { // enough room for ".0\0"
            buf[len] = '.';
            buf[len + 1] = '0';
            buf[len + 2] = '\0';
        }
    }
}

FunctionV *functionv_copy(const FunctionV *src)
{
    if (!src)
        return NULL;

    FunctionV *dst = malloc(sizeof(FunctionV));
    if (!dst)
        return NULL;

    dst->params = NULL;
    dst->contextuals = NULL;
    dst->body_src = NULL;
    dst->name = NULL;
    dst->closure = src->closure;

    if (src->params)
    {
        size_t n = 0;
        while (src->params[n])
            n++;

        dst->params = malloc((n + 1) * sizeof(char *));
        if (!dst->params)
        {
            free(dst);
            return NULL;
        }

        for (size_t i = 0; i < n; i++)
        {
            dst->params[i] = mila_strdup(src->params[i]);
            if (!dst->params[i])
            {
                for (size_t j = 0; j < i; j++)
                    free(dst->params[j]);
                free(dst->params);
                free(dst);
                return NULL;
            }
        }
        dst->params[n] = NULL;
    }

    if (src->contextuals)
    {
        size_t n = 0;
        while (src->contextuals[n])
            n++;

        dst->contextuals = malloc((n + 1) * sizeof(char *));
        if (!dst->contextuals)
        {
            if (dst->params)
            {
                for (size_t i = 0; dst->params[i]; i++)
                    free(dst->params[i]);
                free(dst->params);
            }
            free(dst);
            return NULL;
        }

        for (size_t i = 0; i < n; i++)
        {
            dst->contextuals[i] = mila_strdup(src->contextuals[i]);
            if (!dst->contextuals[i])
            {
                for (size_t j = 0; j < i; j++)
                    free(dst->contextuals[j]);
                free(dst->contextuals);
                if (dst->params)
                {
                    for (size_t k = 0; dst->params[k]; k++)
                        free(dst->params[k]);
                    free(dst->params);
                }
                free(dst);
                return NULL;
            }
        }
        dst->contextuals[n] = NULL;
    }

    if (src->body_src)
    {
        dst->body_src = mila_strdup(src->body_src);
        if (!dst->body_src)
        {
            if (dst->contextuals)
            {
                for (size_t i = 0; dst->contextuals[i]; i++)
                    free(dst->contextuals[i]);
                free(dst->contextuals);
            }
            if (dst->params)
            {
                for (size_t i = 0; dst->params[i]; i++)
                    free(dst->params[i]);
                free(dst->params);
            }
            free(dst);
            return NULL;
        }
    }

    if (src->name)
    {
        dst->name = mila_strdup(src->name);
        if (!dst->name)
        {
            if (dst->body_src)
                free(dst->body_src);
            if (dst->contextuals)
            {
                for (size_t i = 0; dst->contextuals[i]; i++)
                    free(dst->contextuals[i]);
                free(dst->contextuals);
            }
            if (dst->params)
            {
                for (size_t i = 0; dst->params[i]; i++)
                    free(dst->params[i]);
                free(dst->params);
            }
            free(dst);
            return NULL;
        }
    }

    return dst;
}

NativeFunctionV *nativefn_copy(const NativeFunctionV *src)
{
    if (!src)
        return NULL;

    NativeFunctionV *dst = malloc(sizeof(NativeFunctionV));
    if (!dst)
        return NULL;

    dst->fn = src->fn;
    dst->userdata = src->userdata;
    dst->name = NULL;

    if (src->name)
    {
        dst->name = mila_strdup(src->name);
        if (!dst->name)
        {
            free(dst);
            return NULL;
        }
    }

    return dst;
}

Value *val_copy(Value *src)
{
    if (!src)
        return vnull();
    if (src->method_table && src->method_table[UMethodCopy])
        return ((unary_method)src->method_table[UMethodCopy])(src);
    if (src->method_table && src->method_table[UMethodCopyShallow])
        return ((unary_method)src->method_table[UMethodCopyShallow])(src);

    if (!src)
        return NULL;

    Value *copy = mila_malloc(sizeof(Value));
    if (!copy)
        return NULL;

    /* Copy structure layout */
    copy->type = src->type;
    copy->refcount = 1;
    copy->type_name = mila_strdup(src->type_name);
    copy->method_table = src->method_table;

    /* Deep copy based on type */
    switch (src->type)
    {
    case T_STRING:
        /* Strings: duplicate the string buffer */
        copy->v = (void *)mila_strdup(GET_STRING(src));
        break;

    case T_BOOL:
        /* Primitives: direct copy */
        copy->v = (void *)src->v;
        break;

    case T_INT:
        copy->v->i = GET_INTEGER(src);
        break;

    case T_UINT:
        copy->v->ui = src->v->ui;
        break;

    case T_FLOAT:
        copy->v->f = src->v->f;
        break;
    case T_BFLOAT:
        copy->v->bf = src->v->bf;
        break;
    case T_BINT:
        copy->v->bi = src->v->bi;
        break;
    case T_ERROR:
        /* Error messages: duplicate the message */
        if (src->v)
        {
            copy->v = (void *)mila_strdup(GET_STRING(src));
        }
        break;

    case T_TAGGED_ERROR:
        /* Tagged errors: duplicate message and copy type */
        copy->v->tagged_error.type = src->v->tagged_error.type;
        if (src->v->tagged_error.message)
        {
            copy->v->tagged_error.message = mila_strdup(src->v->tagged_error.message);
        }
        break;

    case T_FUNCTION:
        /* Functions: retain reference (shared ownership) */
        copy->v = (void *)functionv_copy(GET_FUNCTION(src));
        break;
    case T_NATIVE:
        /* Native functions: retain reference */
        copy->v = (void *)nativefn_copy(GET_NATIVE(src));
        break;

    case T_OPAQUE:
    {
        /* Opaques: share the pointer but increment if refcounted */
        Value *fn = GET_OVERLOAD(src, OVERLOAD_COPY);
        if (fn)
            return call_function_with(NULL, fn, val_retain(src), NULL);
        else
            copy->v = src->v;
    }
    break;
    case T_NONE:
    case T_NULL:
        copy->type = src->type;
        break;

    default:
        val_release(copy);
        return verror("Type %s doesnt support copying and may cause memory leaks if done so.", GET_TYPENAME(src));
    }

    return copy;
}
Value *val_copy_shallow(Value *src)
{
    if (!src)
        return vnull();
    if (src->method_table && src->method_table[UMethodCopyShallow])
        return ((unary_method)src->method_table[UMethodCopyShallow])(src);

    if (!src)
        return NULL;

    Value *copy = mila_malloc(sizeof(Value));
    if (!copy)
        return NULL;

    /* Copy structure layout */
    copy->type = src->type;
    copy->refcount = 1;
    copy->type_name = mila_strdup(src->type_name);
    copy->method_table = src->method_table;

    /* Deep copy based on type */
    switch (src->type)
    {
    case T_STRING:
        /* Strings: duplicate the string buffer */
        copy->v = (void *)mila_strdup(GET_STRING(src));
        break;

    case T_BOOL:
        /* Primitives: direct copy */
        copy->v = src->v;
        break;

    case T_INT:
        copy->v->i = GET_INTEGER(src);
        break;

    case T_UINT:
        copy->v->ui = src->v->ui;
        break;

    case T_FLOAT:
        copy->v->f = src->v->f;
        break;
    case T_BFLOAT:
        copy->v->bf = src->v->bf;
        break;
    case T_BINT:
        copy->v->bi = src->v->bi;
        break;
    case T_ERROR:
        /* Error messages: duplicate the message */
        if (src->v)
        {
            copy->v = (void *)mila_strdup(GET_STRING(src));
        }
        break;

    case T_TAGGED_ERROR:
        /* Tagged errors: duplicate message and copy type */
        copy->v->tagged_error.type = src->v->tagged_error.type;
        if (src->v->tagged_error.message)
        {
            copy->v->tagged_error.message = mila_strdup(src->v->tagged_error.message);
        }
        break;

    case T_FUNCTION:
        /* Functions: retain reference (shared ownership) */
        copy->v = (void *)functionv_copy(GET_FUNCTION(src));
        break;
    case T_NATIVE:
        /* Native functions: retain reference */
        copy->v = (void *)nativefn_copy(GET_NATIVE(src));
        break;

    case T_OPAQUE:
    {
        /* Opaques: share the pointer but increment if refcounted */
        Value *fn = GET_OVERLOAD(src, OVERLOAD_COPY);
        if (fn)
            return call_function_with(NULL, fn, val_retain(src), NULL);
        else
            copy->v = src->v;
    }
    break;
    case T_NONE:
    case T_NULL:
        copy->type = src->type;
        break;

    default:
        val_release(copy);
        return verror("Type %s doesnt support copying and may cause memory leaks if done so.", GET_TYPENAME(src));
    }

    return copy;
}

Value *val_new(ValueType t)
{
    Value *p = mila_malloc(sizeof(Value));
    p->type = t;
    p->refcount = 1;
    p->type_name = NULL;
    p->method_table = NULL;
    p->owns_table = 1;
    p->v = (ValueValue *)malloc(sizeof(ValueValue));
#ifdef MILA_DEBUG
    printf("  ++ %s type allocated!\n     pointer: %p\n", MILA_GET_TYPENAME(p),
           p);
#endif
    return p;
}

Value *val_new_raw(ValueType t)
{
    Value *p = mila_malloc(sizeof(Value));
    p->type = t;
    p->refcount = 1;
    p->type_name = NULL;
    p->method_table = NULL;
    p->owns_table = 1;
    p->v = NULL;
#ifdef MILA_DEBUG
    printf("  ++ %s raw type allocated!\n     pointer: %p\n", MILA_GET_TYPENAME(p),
           p);
#endif
    return p;
}

void val_allocate_table(Value *v)
{
    v->method_table =
        (MethodTable *)mila_malloc(sizeof(MethodTable) * MethodTotalCount);
    memset(v->method_table, 0, sizeof(MethodTable) * MethodTotalCount);
}

MethodTable *val_make_table(void)
{
    MethodTable *t =
        (MethodTable *)mila_malloc(sizeof(MethodTable) * MethodTotalCount);
    memset(t, 0, sizeof(MethodTable) * MethodTotalCount);
    return t;
}

void val_set_table(Value *v, MethodTable *t)
{
    v->owns_table = 0;
    v->method_table = t;
    // TODO: future update
    // for (unsigned char i=0; i < 255; ++i) {
    //     if (t[i]) {
    //         v->table_offset = i;
    //     }
    // }
}

void val_set_method(Value *v, MethodType t, void *func)
{
    v->method_table[t] = func;
}

void val_unset_method(Value *v, MethodType t) { v->method_table[t] = NULL; }

void val_set_method_table(MethodTable *v, MethodType t, void *func)
{
    v[t] = func;
}

void val_unset_method_table(MethodTable *v, MethodType t) { v[t] = NULL; }

// Helpers to create typed values or check their truthiness

inline int is_truthy(Value *value)
{
    switch (GET_TYPE(value))
    {
    case T_INT:
        return GET_INTEGER(value) ? 1 : 0;
    case T_FLOAT:
        return GET_FLOAT(value) ? 1 : 0;
    case T_UINT:
        return GET_UINTEGER(value) ? 1 : 0;
    case T_BINT:
        return GET_BINTEGER(value) ? 1 : 0;
    case T_BFLOAT:
        return !b_ff_is_zero(GET_BFLOAT(value));
    case T_BOOL:
        return GET_BOOL(value) ? 1 : 0;
    case T_STRING:
        return strlen(GET_STRING(value)) ? 1 : 0;
    case T_FUNCTION:
    case T_NATIVE:
        return 1;
    case T_OPAQUE:
    {
        if (value->type_name && strcmp(value->type_name, MILA_LPREFIX "dict") == 0)
        {
#ifdef MILA_DEBUG
            printf("  ?? Recieved candidate for overloading!\n  `");
            print_value(value);
            puts("`");
#endif
            Value *fn = dict_get_str((Dict *)GET_OPAQUE(value), OVERLOAD_TO_BOOL);
            if (fn)
            {
                Value *tmp = call_function_with(NULL, fn, val_retain(value), NULL);
                int res = is_truthy(tmp);
                val_release(tmp);
                return res;
            }
        }
        return GET_OPAQUE(value) ? 1 : 0;
    case T_ERROR:
    case T_TAGGED_ERROR:
    case T_RETURN:
    case T_BREAK:
    case T_CONTINUE:
        return 1;
    }
    break;
    default:;
    }
    return 0;
}

int match_range(const char **pat, char c)
{
    int negate = 0;
    if (**pat == '!')
    {
        negate = 1;
        (*pat)++;
    }

    int matched = 0;
    while (**pat && **pat != ']')
    {
        if ((*pat)[1] == '-' && (*pat)[2] && (*pat)[2] != ']')
        {
            // Handle range a-z
            if (c >= **pat && c <= (*pat)[2])
                matched = 1;
            *pat += 3; // skip 'a-z'
        }
        else
        {
            if (c == **pat)
                matched = 1;
            (*pat)++;
        }
    }

    if (**pat == ']')
        (*pat)++; // skip closing bracket
    return negate ? !matched : matched;
}

int match(const char *pattern, const char *str)
{
    while (*pattern)
    {
        if (*pattern == '*')
        {
            pattern++;
            if (!*pattern)
                return 1;
            while (*str)
            {
                if (match(pattern, str))
                    return 1;
                str++;
            }
            return 0;
        }
        else if (*pattern == '?')
        {
            if (!*str)
                return 0;
            pattern++;
            str++;
        }
        else if (*pattern == '[')
        {
            pattern++;
            if (!*str || !match_range(&pattern, *str))
                return 0;
            str++;
        }
        else
        {
            if (*pattern != *str)
                return 0;
            pattern++;
            str++;
        }
    }
    return !*str;
}

inline Value *vnull() { return val_new_raw(T_NULL); }
inline Value *vnone() { return val_new_raw(T_NONE); }
inline Value *vbreak() { return val_new_raw(T_BREAK); }
inline Value *vcontinue() { return val_new_raw(T_CONTINUE); }

#ifndef SAFE_BUILD
__attribute__((format(printf, 2, 3))) Value *vtagged_error(ErrorType err,
                                                           char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    // First pass: find length
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int len = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (len < 0)
    {
        va_end(ap);
        return NULL;
    }

    char *buf = mila_malloc(len + 1);
    if (!buf)
    {
        va_end(ap);
        Value *v = val_new_raw(T_ERROR);
        v->v = (void *)mila_strdup("verror could not allocate memory!");
        return v;
    }

    vsnprintf(buf, len + 1, fmt, ap);
    va_end(ap);
    Value *v = val_new(T_TAGGED_ERROR);
    v->v->tagged_error.message = buf;
    v->v->tagged_error.type = err;
    v->v->tagged_error.return_code = -1;
    return v;
}

__attribute__((format(printf, 3, 4))) Value *vtagged_coded_error(ErrorType err, int ret_code,
                                                                 char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    // First pass: find length
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int len = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (len < 0)
    {
        va_end(ap);
        return NULL;
    }

    char *buf = mila_malloc(len + 1);
    if (!buf)
    {
        va_end(ap);
        Value *v = val_new_raw(T_ERROR);
        v->v = (void *)mila_strdup("verror could not allocate memory!");
        return v;
    }

    vsnprintf(buf, len + 1, fmt, ap);
    va_end(ap);
    Value *v = val_new(T_TAGGED_ERROR);
    v->v->tagged_error.message = buf;
    v->v->tagged_error.type = err;
    v->v->tagged_error.return_code = ret_code;
    return v;
}

__attribute__((format(printf, 1, 2))) Value *verror(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    // First pass: find length
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int len = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (len < 0)
    {
        va_end(ap);
        return NULL;
    }

    char *buf = mila_malloc(len + 1);
    if (!buf)
    {
        va_end(ap);
        Value *v = val_new_raw(T_ERROR);
        v->v = (void *)mila_strdup("verror could not allocate memory!");
        return v;
    }

    vsnprintf(buf, len + 1, fmt, ap);
    va_end(ap);
    Value *v = val_new_raw(T_ERROR);
    v->v = (void *)buf;
    return v;
}
#endif

__attribute__((format(printf, 1, 2))) inline Value *vstring_fmt(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    // First pass: find length
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int len = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (len < 0)
    {
        va_end(ap);
        return NULL;
    }

    char *buf = mila_malloc(len + 1);
    if (!buf)
    {
        va_end(ap);
        Value *v = val_new_raw(T_ERROR);
        v->v = (void *)mila_strdup("vstring_fmt could not allocate memory!");
        return v;
    }

    vsnprintf(buf, len + 1, fmt, ap);
    va_end(ap);
    Value *v = val_new_raw(T_STRING);
    v->v = (void *)buf;
    return v;
}

inline Value *vint(long x)
{
    Value *v = val_new(T_INT);
    v->v->i = x;
    return v;
}

inline Value *vuint(unsigned long x)
{
    Value *v = val_new(T_UINT);
    v->v->ui = x;
    return v;
}

inline Value *vfloat(double f)
{
    Value *v = val_new(T_FLOAT);
    v->v->f = f;
    return v;
}

inline Value *vbint(__int128 x)
{
    Value *v = val_new(T_BINT);
    v->v->bi = x;
    return v;
}

inline Value *vbfloat(mila_float128_internal f)
{
    Value *v = val_new(T_BFLOAT);
    v->v->bf = f;
    return v;
}

inline Value *vbool(int b)
{
    Value *v = val_new_raw(T_BOOL);
    v->v = (void *)(b ? 1L : 0L);
    return v;
}

inline Value *vstring_dup(const char *restrict s)
{
    Value *v = val_new_raw(T_STRING);
    v->v = (void *)mila_strdup(s ? s : "");
    return v;
}

inline Value *vstring_take(char *s)
{
    Value *v = val_new_raw(T_STRING);
    v->v = (void *)s;
    return v;
}

// String
inline Value *vstring_slice(const char *restrict src, size_t start, size_t len)
{
    size_t n = strlen(src);
    if (start > n)
        return verror("Out of bounds string slice!"); // empty string

    if (start + len > n)
        len = n - start;

    char *buf = mila_malloc(len + 1);
    if (!buf)
        return vnull();

    memcpy(buf, src + start, len);
    buf[len] = '\0';

    return vstring_take(buf);
}

inline Value *vstring_index(const char *restrict src, size_t index)
{
    size_t n = strlen(src);
    if (index >= n)
        return vnull();

    char *buf = mila_malloc(sizeof(char) * 2);
    if (!buf)
        return vnull();

    buf[0] = src[index];
    buf[1] = '\0';

    return vstring_take(buf);
}

inline Value *vstring_replace(const char *restrict src, const char *restrict needle, const char *restrict replacement)
{
    if (!*needle)
        return vstring_dup(src); // can't match empty substring

    size_t src_len = strlen(src);
    size_t n_len = strlen(needle);
    size_t r_len = strlen(replacement);

    // Count occurrences
    size_t count = 0;
    const char *p = src;
    while ((p = strstr(p, needle)))
    {
        count++;
        p += n_len;
    }

    // Allocate output buffer
    size_t new_len = src_len + count * (r_len - n_len);
    char *buf = mila_malloc(new_len + 1);
    if (!buf)
        return vnull();

    // Build replacement
    char *out = buf;
    p = src;

    while (1)
    {
        const char *match = strstr(p, needle);
        if (!match)
        {
            strcpy(out, p);
            break;
        }

        size_t seg = match - p;
        memcpy(out, p, seg);
        out += seg;

        memcpy(out, replacement, r_len);
        out += r_len;

        p = match + n_len;
    }

    return vstring_take(buf);
}
inline Value *vowned_opaque(void *p)
{
    Value *v = val_new_raw(T_OWNED_OPAQUE);
    v->v = (void *)p;
    return v;
}
inline Value *vopaque(void *p)
{
    Value *v = val_new_raw(T_OPAQUE);
    v->v = (void *)p;
    return v;
}
inline Value *vweak_opaque(void *p)
{
    Value *v = val_new_raw(T_WEAK_OPAQUE);
    v->v = (void *)p;
    return v;
}
inline Value *vopaque_extra(void *p, VPrinter dis, const char *type_name)
{
    Value *v = vopaque(p);
    if (dis && v)
    {
        val_allocate_table(v);
        val_set_method(v, UMethodToString, dis);
    }
    v->type_name = mila_strdup(type_name);
    return v;
}
inline Value *vowned_opaque_extra(void *p, VPrinter dis, const char *type_name)
{
    Value *v = vowned_opaque(p);
    if (dis && v)
    {
        val_allocate_table(v);
        val_set_method(v, UMethodToString, dis);
    }
    v->type_name = mila_strdup(type_name);
    return v;
}
inline Value *vnative(NativeFn fn, const char *name)
{
    Value *v = val_new_raw(T_NATIVE);
    NativeFunctionV *native_function = (NativeFunctionV *)mila_malloc(sizeof(NativeFunctionV));
    native_function->fn = fn;
    native_function->userdata = NULL;
    native_function->name = name ? mila_strdup(name) : NULL;
    v->v = (void *)native_function;
    return v;
}
// vfunction creation
inline Value *vfunction(char **params, char **defaults, char **contextuals, Env *closure,
                        char *body_src)
{
    Value *v = val_new_raw(T_FUNCTION);
    FunctionV *function = (FunctionV *)mila_malloc(sizeof(FunctionV));
    function->params = params;
    function->defaults = defaults;
    function->contextuals = contextuals;
    function->body_src = body_src;
    function->closure = closure;
    function->name = NULL;
    v->v = (void *)function;
    return v;
}
inline Value *vtruthy(Value *value) { return vbool(is_truthy(value)); }

int malloc_sprintf(char **strp, const char *fmt, ...)
{
    if (!strp)
        return -1;

    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);
    int add_size = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);

    if (add_size < 0)
    {
        va_end(args);
        return -1;
    }

    // If *strp is NULL, treat it as empty string
    size_t old_len = 0;
    if (*strp)
    {
        // We trust the caller has either a valid string or NULL.
        old_len = strlen(*strp);
    }

    char *newbuf =
        (char *)mila_realloc(*strp ? *strp : NULL, old_len + add_size + 1);
    if (!newbuf)
    {
        va_end(args);
        return -1;
    }

    *strp = newbuf;

    if (vsnprintf(*strp + old_len, add_size + 1, fmt, args) < 0)
    {
        va_end(args);
        return -1;
    }

    va_end(args);
    return (int)(old_len + add_size);
}

Value *to_string(Value *v)
{
    char *s = as_c_string(v);
    return vstring_take(s);
}

char *as_c_string(Value *v)
{
    char *buffer = NULL;
    if (!v)
    {
        return mila_strdup("cnull");
    }
    if (v->method_table && v->method_table[UMethodToString])
    {
        Value *str = ((unary_method)v->method_table[UMethodToString])(v);
        char *res = mila_strdup(GET_STRING(str));
        val_kill(str);
        return res;
    }
    if (v->method_table && v->method_table[UMethodToRepr])
    {
        Value *str = ((unary_method)v->method_table[UMethodToRepr])(v);
        char *res = mila_strdup(GET_STRING(str));
        val_kill(str);
        return res;
    }
    switch (v->type)
    {
    case T_NULL:
        malloc_sprintf(&buffer, "null");
        break;
    case T_NONE:
        malloc_sprintf(&buffer, "none");
        break;
    case T_ERROR:
        malloc_sprintf(&buffer, "<error:%s>", GET_ERROR_MESSAGE(v));
        break;
    case T_TAGGED_ERROR:
        malloc_sprintf(&buffer, "<error[%s]:%s>", GET_TAGGED_ERROR_TYPENAME(v),
                       GET_TAGGED_ERROR_MESSAGE(v));
        break;

    case T_INT:
        malloc_sprintf(&buffer, "%ld", GET_INTEGER(v));
        break;
    case T_FLOAT:
    {
        char buf[MAX_NUMBER_DIGITS] = {0};
        float_to_string(v->v->f, buf, sizeof(buf));
        malloc_sprintf(&buffer, "%s", buf);
        break;
    }
    case T_STRING:
        malloc_sprintf(&buffer, "%s", GET_STRING(v) ? GET_STRING(v) : "");
        break;
    case T_BOOL:
        malloc_sprintf(&buffer, "%s", GET_BOOL(v) ? "true" : "false");
        break;
    case T_FUNCTION:
    {
        char *args = mila_strdup("");
        for (int i = 0; GET_FUNCTION(v)->params[i]; ++i)
        {
            malloc_sprintf(&args, "%s", GET_FUNCTION(v)->params[i]);
            if (GET_FUNCTION(v)->defaults[i])
            {
                malloc_sprintf(&args, "=%s", GET_FUNCTION(v)->defaults[i]);
            }
            if (GET_FUNCTION(v)->params[i + 1])
            {
                malloc_sprintf(&args, ",");
            }
        }
        malloc_sprintf(&buffer, "<function:%s(%s) at %p>",
                       GET_FUNCTION(v)->name ? GET_FUNCTION(v)->name : "[lambda]", args, GET_FUNCTION(v));
        free(args);
    }
    break;
    case T_NATIVE:
        malloc_sprintf(&buffer, "<native:%s at %p>",
                       GET_NATIVE(v)->name ? GET_NATIVE(v)->name : "???",
                       GET_NATIVE(v)->fn);
        break;
    case T_OPAQUE:
        if (v->type_name)
            malloc_sprintf(&buffer, "<opaque:%p %s>", v->v, v->type_name);
        else
            malloc_sprintf(&buffer, "<opaque:%p>", v->v);
        break;
    case T_WEAK_OPAQUE:
        if (v->type_name)
            malloc_sprintf(&buffer, "<weak opaque:%p %s>", v->v, v->type_name);
        else
            malloc_sprintf(&buffer, "<weak opaque:%p>", v->v);
        break;
    case T_OWNED_OPAQUE:
        if (v->type_name)
            malloc_sprintf(&buffer, "<owned opaque:%p %s>", v->v, v->type_name);
        else
            malloc_sprintf(&buffer, "<owned opaque:%p>", v->v);
        break;
    case T_UINT:
        malloc_sprintf(&buffer, "%luu", v->v->ui);
        break;
    case T_BINT:
    {
        char *s = i128toa(v->v->bi);
        malloc_sprintf(&buffer, "%s~", s);
        free(s);
        break;
    }
    case T_BFLOAT:
    {
        char *s = b_ff_to_string(v->v->bf);
        malloc_sprintf(&buffer, "%s~", s);
        free(s);
        break;
    }
    case T_RETURN:
    {
        char *str = as_c_string_repr((Value *)v->v);
        malloc_sprintf(&buffer, "<return:%s>", str);
        mila_free(str);
    }
    break;
    default:
        malloc_sprintf(&buffer, "???");
    }
    return buffer;
}

char *as_c_string_repr(Value *v)
{
    char *buffer = NULL;
    if (!v)
    {
        return mila_strdup("cnull");
    }
    if (v->method_table && v->method_table[UMethodToRepr])
    {
        Value *str = ((unary_method)v->method_table[UMethodToRepr])(v);
        char *res = mila_strdup(GET_STRING(str));
        val_kill(str);
        return res;
    }
    if (v->method_table && v->method_table[UMethodToString])
    {
        Value *str = ((unary_method)v->method_table[UMethodToString])(v);
        char *res = mila_strdup(GET_STRING(str));
        val_kill(str);
        return res;
    }
    switch (v->type)
    {
    case T_STRING:
        malloc_sprintf(&buffer, "\"");
        uint8_t *text = (uint8_t *)GET_STRING(v);
        size_t len = strlen(GET_STRING(v));
        for (size_t i = 0; i < len; ++i)
        {
            switch (text[i])
            {
            case '\a':
                malloc_sprintf(&buffer, "\\a");
                break;
            case '\t':
                malloc_sprintf(&buffer, "\\t");
                break;
            case '\n':
                malloc_sprintf(&buffer, "\\n");
                break;
            case '\v':
                malloc_sprintf(&buffer, "\\v");
                break;
            case '\f':
                malloc_sprintf(&buffer, "\\f");
                break;
            case '\r':
                malloc_sprintf(&buffer, "\\r");
                break;
            case '"':
                malloc_sprintf(&buffer, "\\\"");
                break;
            default:
                if (isprint(text[i]))
                    malloc_sprintf(&buffer, "%c", text[i]);
                else
                    malloc_sprintf(&buffer, "\\x%02x", text[i]);
            }
        }
        malloc_sprintf(&buffer, "\"");
        break;
    default:
    {
        char *tmp = as_c_string(v);
        malloc_sprintf(&buffer, "%s", tmp);
        mila_free(tmp);
    }
    }
    return buffer;
}

int raw_print_value_repr(Value *v);

int raw_print_value(Value *v)
{
    if (!v)
    {
        return printf("cnull");
    }
    if (v->type_name && strcmp(v->type_name, MILA_LPREFIX "dict") == 0)
    {
        Value *fn = dict_get_str((Dict *)v->v, ":display");
        if (fn)
        {
            val_release(call_function_with(NULL, fn, val_retain(v), NULL));
            fflush(stdout);
            return 0;
        }
    }
    if (v->method_table && v->method_table[UMethodToString])
    {
        Value *str = ((unary_method)v->method_table[UMethodToString])(v);
        char *res = mila_strdup(GET_STRING(str));
        val_kill(str);
        int i = printf("%s", res);
        free(res);
        return i;
    }
    if (v->method_table && v->method_table[UMethodToRepr])
    {
        Value *str = ((unary_method)v->method_table[UMethodToRepr])(v);
        char *res = mila_strdup(GET_STRING(str));
        val_kill(str);
        int i = printf("%s", res);
        free(res);
        return i;
    }
    switch (v->type)
    {
    case T_NULL:
        return printf("null");
    case T_NONE:
        return printf("none");
    case T_ERROR:
        return printf("<error:%s>", GET_ERROR_MESSAGE(v));
    case T_TAGGED_ERROR:
        return printf("<error[%s]:%s>", GET_TAGGED_ERROR_TYPENAME(v),
                      GET_TAGGED_ERROR_MESSAGE(v));

    case T_INT:
        return printf("%ld", GET_INTEGER(v));
    case T_FLOAT:
    {
        char buf[MAX_NUMBER_DIGITS] = {0};
        float_to_string(v->v->f, buf, sizeof(buf));
        return printf("%s", buf);
    }
    case T_STRING:
        return printf("%s", GET_STRING(v) ? GET_STRING(v) : "");
    case T_BOOL:
        return printf("%s", v->v ? "true" : "false");
    case T_FUNCTION:
    {
        char *args = mila_strdup("");
        for (int i = 0; GET_FUNCTION(v)->params[i]; ++i)
        {
            malloc_sprintf(&args, "%s", GET_FUNCTION(v)->params[i]);
            if (GET_FUNCTION(v)->defaults[i])
            {
                malloc_sprintf(&args, "=%s", GET_FUNCTION(v)->defaults[i]);
            }
            if (GET_FUNCTION(v)->params[i + 1])
            {
                malloc_sprintf(&args, ",");
            }
        }
        int i = printf("<function:%s(%s) at %p>",
                       GET_FUNCTION(v)->name ? GET_FUNCTION(v)->name : "[lambda]", args, GET_FUNCTION(v));
        free(args);
        return i;
    }
    case T_NATIVE:
        return printf("<native:%s at %p>",
                      GET_NATIVE(v)->name ? GET_NATIVE(v)->name : "???",
                      GET_NATIVE(v)->fn);
    case T_OPAQUE:
        if (v->type_name)
            return printf("<opaque:%p %s>", v->v, v->type_name);
        else
            return printf("<opaque:%p>", v->v);
    case T_WEAK_OPAQUE:
        if (v->type_name)
            return printf("<weak opaque:%p %s>", v->v, v->type_name);
        else
            return printf("<weak opaque:%p>", v->v);
    case T_OWNED_OPAQUE:
        if (v->type_name)
            return printf("<owned opaque:%p %s>", v->v, v->type_name);
        else
            return printf("<owned opaque:%p>", v->v);
    case T_UINT:
        return printf("%luu", v->v->ui);
    case T_BINT:
    {
        char *s = i128toa(v->v->bi);
        int i = printf("%s~", s);
        free(s);
        return i;
    }
    case T_BFLOAT:
    {
        char *s = b_ff_to_string(v->v->bf);
        int i = printf("%s~", s);
        free(s);
        return i;
    }
    case T_RETURN:
    {
        return printf("<return:");
        raw_print_value((Value *)v->v);
        return printf(">");
    }
    default:
        return printf("???");
    }
}

int raw_print_value_repr(Value *v)
{
    if (!v)
    {
        return printf("cnull");
    }

    if (v->type_name && strcmp(v->type_name, MILA_LPREFIX "dict") == 0)
    {
        Value *fn = dict_get_str((Dict *)v->v, ":display");
        if (fn)
            val_release(call_function_with(NULL, fn, val_retain(v), NULL));
    }
    if (v->method_table && v->method_table[UMethodToRepr])
    {
        Value *str = ((unary_method)v->method_table[UMethodToRepr])(v);
        char *res = mila_strdup(GET_STRING(str));
        val_kill(str);
        int i = printf("%s", res);
        free(res);
        return i;
    }
    if (v->method_table && v->method_table[UMethodToString])
    {
        Value *str = ((unary_method)v->method_table[UMethodToString])(v);
        char *res = mila_strdup(GET_STRING(str));
        val_kill(str);
        int i = printf("%s", res);
        free(res);
        return i;
    }
    switch (v->type)
    {
    case T_STRING:;
        int first = printf("\"");
        uint8_t *text = (uint8_t *)GET_STRING(v);
        size_t len = strlen(GET_STRING(v));
        for (size_t i = 0; i < len; ++i)
        {
            switch (text[i])
            {
            case '\a':
                first += printf("\\a");
                break;
            case '\t':
                first += printf("\\t");
                break;
            case '\n':
                first += printf("\\n");
                break;
            case '\v':
                first += printf("\\v");
                break;
            case '\f':
                first += printf("\\f");
                break;
            case '\r':
                first += printf("\\r");
                break;
            case '"':
                first += printf("\\\"");
                break;
            default:
                if (isprint(text[i]))
                    first += printf("%c", text[i]);
                else
                    first += printf("\\x%02x", text[i]);
                break;
            }
        }
        return first + printf("\"");
    default:
    {
        return raw_print_value(v);
    }
    }
}

char *as_c_string_raw(Value *v)
{
    char *buffer = NULL;
    if (!v)
    {
        return mila_strdup("cnull");
    }
    switch (v->type)
    {
    case T_NULL:
        malloc_sprintf(&buffer, "null");
        break;
    case T_NONE:
        malloc_sprintf(&buffer, "none");
        break;
    case T_ERROR:
        malloc_sprintf(&buffer, "<error:%s>", GET_ERROR_MESSAGE(v));
        break;

    case T_INT:
        malloc_sprintf(&buffer, "%ld", GET_INTEGER(v));
        break;
    case T_FLOAT:
    {
        char buf[MAX_NUMBER_DIGITS] = {0};
        float_to_string(v->v->f, buf, sizeof(buf));
        malloc_sprintf(&buffer, "%s", buf);
        break;
    }
    case T_STRING:
        malloc_sprintf(&buffer, "%s", GET_STRING(v) ? GET_STRING(v) : "");
        break;
    case T_BOOL:
        malloc_sprintf(&buffer, "%s", v->v ? "true" : "false");
        break;
    case T_FUNCTION:
        malloc_sprintf(&buffer, "<function:%s at %p>",
                       GET_FUNCTION(v)->name ? GET_FUNCTION(v)->name : "[lambda]", GET_FUNCTION(v));
        break;
    case T_NATIVE:
        malloc_sprintf(&buffer, "<native:%s at %p>",
                       GET_NATIVE(v)->name ? GET_NATIVE(v)->name : "???",
                       GET_NATIVE(v)->fn);
        break;
    case T_OPAQUE:
        if (v->type_name)
            malloc_sprintf(&buffer, "<opaque:%p %s>", v->v, v->type_name);
        else
            malloc_sprintf(&buffer, "<opaque:%p>", v->v);
        break;
    case T_OWNED_OPAQUE:
        if (v->type_name)
            malloc_sprintf(&buffer, "<owned opaque:%p %s>", v->v, v->type_name);
        else
            malloc_sprintf(&buffer, "<owned opaque:%p>", v->v);
        break;
    case T_UINT:
        malloc_sprintf(&buffer, "%lu", v->v->ui);
        malloc_sprintf(&buffer, "u");
        break;
    case T_RETURN:
    {
        char *str = as_c_string_repr_raw((Value *)v->v);
        malloc_sprintf(&buffer, "<return:%s>", str);
        mila_free(str);
    }
    break;
    default:
        malloc_sprintf(&buffer, "???");
    }
    return buffer;
}

char *as_c_string_repr_raw(Value *v)
{
    char *buffer = NULL;
    if (!v)
    {
        return mila_strdup("cnull");
    }
    switch (v->type)
    {
    case T_STRING:
        malloc_sprintf(&buffer, "\"");
        uint8_t *text = (uint8_t *)GET_STRING(v);
        size_t len = strlen(GET_STRING(v));
        for (size_t i = 0; i < len; ++i)
        {
            switch (text[i])
            {
            case '\a':
                malloc_sprintf(&buffer, "\\a");
                break;
            case '\t':
                malloc_sprintf(&buffer, "\\t");
                break;
            case '\n':
                malloc_sprintf(&buffer, "\\n");
                break;
            case '\v':
                malloc_sprintf(&buffer, "\\v");
                break;
            case '\f':
                malloc_sprintf(&buffer, "\\f");
                break;
            case '\r':
                malloc_sprintf(&buffer, "\\r");
                break;
            case '"':
                malloc_sprintf(&buffer, "\\\"");
                break;
            default:
                if (isprint(text[i]))
                    malloc_sprintf(&buffer, "%c", text[i]);
                else
                    malloc_sprintf(&buffer, "\\x%02x", text[i]);
            }
        }
        malloc_sprintf(&buffer, "\"");
        break;
    default:
    {
        char *tmp = as_c_string_raw(v);
        malloc_sprintf(&buffer, "%s", tmp);
        mila_free(tmp);
    }
    }
    return buffer;
}

// print value (for debug / native print)
int print_value(Value *v)
{
    if (!v)
    {
        return printf("cnull");
    }
    return raw_print_value(v);
}

int print_value_repr(Value *v)
{
    if (!v)
    {
        return printf("cnull");
    }
    return raw_print_value_repr(v);
}

int print_value_debug(Value *v)
{
    if (!v)
    {
        return printf("cnull");
    }
    char *txt = as_c_string_repr_raw(v);
    int i = printf("%s", txt);
    mila_free(txt);
    printf(" (%i)\n", v->refcount);
    return i;
}

inline Value *val_retain(Value *v)
{
#ifdef MILA_DEBUG
    if (v)
    {
        printf("  ?? val_retain:\n     type: %s\n     refcount ++%i -> %i\n     "
               "value: ",
               MILA_GET_TYPENAME(v), v->refcount, v->refcount + 1);
        print_value_repr(v);
        puts("");
    }
    else
    {
        printf("  !! val_release:\n     JUST ATTEMPTED TO FREE A NULL VALUE!\n");
    }
#endif
    if (!v)
        return NULL;
    if (GET_TYPE(v) == T_WEAK_OPAQUE)
        return v;
    v->refcount++;
    return v;
}

// release
inline void val_release(Value *v)
{
    if (!v)
        return;
    if (GET_TYPE(v) == T_WEAK_OPAQUE)
        return;
#ifdef MILA_DEBUG
    printf("  -- val_release:\n     type: %s\n     refcount --%i -> %i\n     "
           "%s\n     value: ",
           GET_TYPENAME(v), v->refcount, v->refcount - 1,
           v->refcount - 1 <= 0 ? "will be freed after" : "will survive");
    print_value_repr(v);
    puts("");
#endif
    v->refcount--;
    if (v->refcount <= 0)
    {
        if (v->method_table && v->method_table[UMethodFree])
        {
            ((unary_method)v->method_table[UMethodFree])(v);
            goto cleanup;
        }
        // mila_free internals
        if (v->type == T_STRING && GET_STRING(v))
            mila_free(GET_STRING(v));
        if (v->type == T_ERROR && GET_ERROR_MESSAGE(v))
            mila_free(GET_ERROR_MESSAGE(v));
        if (v->type == T_TAGGED_ERROR && v->v->tagged_error.message)
            mila_free(v->v->tagged_error.message);
        if (v->type == T_FUNCTION)
        {
            if (GET_FUNCTION(v)->params)
            {
                char **p = GET_FUNCTION(v)->params;
                for (int i = 0; p[i]; ++i)
                    mila_free(p[i]);
                mila_free(p);
            }
            if (GET_FUNCTION(v)->defaults)
            {
                char **d = GET_FUNCTION(v)->defaults;
                for (int i = 0; d[i]; ++i)
                    mila_free(d[i]);
                mila_free(d);
            }
            if (GET_FUNCTION(v)->contextuals)
            {
                char **p = GET_FUNCTION(v)->contextuals;
                for (int i = 0; p[i]; ++i)
                    mila_free(p[i]);
                mila_free(p);
            }
            if (GET_FUNCTION(v)->body_src)
                mila_free(GET_FUNCTION(v)->body_src);
            if (GET_FUNCTION(v)->name)
                mila_free(GET_FUNCTION(v)->name);
            env_free(GET_FUNCTION(v)->closure);
            mila_free(GET_FUNCTION(v));
        }
        if (v->type == T_NATIVE)
        {
            if (GET_NATIVE(v)->name)
                mila_free(GET_NATIVE(v)->name);
            mila_free(GET_NATIVE(v));
        }
        if (v->type == T_OWNED_OPAQUE)
        {
            if (v->v)
                mila_free(v->v);
        }
    cleanup:;
        if (v->type_name)
            mila_free(v->type_name);
        if (v->method_table && v->owns_table)
            mila_free(v->method_table);
        switch (GET_TYPE(v))
        {
        case T_ERROR:
        case T_RETURN:
        case T_CONTINUE:
        case T_BREAK:
        case T_FUNCTION:
        case T_NATIVE:
        case T_OPAQUE:
        case T_OWNED_OPAQUE:
        case T_WEAK_OPAQUE:
        case T_NONE:
        case T_NULL:
        case T_BOOL:

        case T_STRING:
            break;
        default:
            if (v->v)
                mila_free(v->v);
        }
        mila_free(v);
    }
}

void val_kill(Value *v)
{
    if (!v)
        return;
#ifdef MILA_DEBUG
    printf("  -- val_kill:\n     type: %s\n     refcount %i -> 0 (forced)\n     "
           "%s\n     value: ",
           GET_TYPENAME(v), v->refcount,
           v->refcount - 1 <= 0 ? "will be freed after" : "will survive");
    print_value_repr(v);
    puts("");
#endif
    if (v->method_table && v->method_table[UMethodKill])
    {
        ((unary_method)v->method_table[UMethodKill])(v);
        goto cleanup;
    }
    if (v->method_table && v->method_table[UMethodFree])
    {
        ((unary_method)v->method_table[UMethodFree])(v);
        goto cleanup;
    }
    // mila_free internals
    if (v->type == T_STRING && GET_STRING(v))
        mila_free(GET_STRING(v));
    if (v->type == T_ERROR && GET_ERROR_MESSAGE(v))
        mila_free(GET_ERROR_MESSAGE(v));
    if (v->type == T_TAGGED_ERROR && v->v->tagged_error.message)
        mila_free(v->v->tagged_error.message);
    if (v->type == T_FUNCTION)
    {
        if (GET_FUNCTION(v)->params)
        {
            char **p = GET_FUNCTION(v)->params;
            for (int i = 0; p[i]; ++i)
                mila_free(p[i]);
            mila_free(p);
        }
        if (GET_FUNCTION(v)->contextuals)
        {
            char **p = GET_FUNCTION(v)->contextuals;
            for (int i = 0; p[i]; ++i)
                mila_free(p[i]);
            mila_free(p);
        }
        if (GET_FUNCTION(v)->body_src)
            mila_free(GET_FUNCTION(v)->body_src);
        if (GET_FUNCTION(v)->name)
            mila_free(GET_FUNCTION(v)->name);
        env_free(GET_FUNCTION(v)->closure);
        mila_free(GET_FUNCTION(v));
    }
    if (v->type == T_NATIVE)
    {
        if (GET_NATIVE(v)->name)
            mila_free(GET_NATIVE(v)->name);
        mila_free(GET_NATIVE(v));
    }
    if (v->type == T_RETURN)
    {
        val_kill((Value *)v->v);
    }
cleanup:;
    mila_free(v->type_name);
    if (v->method_table && v->owns_table)
        mila_free(v->method_table);
    switch (GET_TYPE(v))
    {
    case T_FUNCTION:
    case T_NATIVE:
    case T_BOOL:
    case T_OPAQUE:
    case T_NONE:
    case T_NULL:
    case T_ERROR:
    case T_RETURN:
    case T_CONTINUE:
    case T_BREAK:

    case T_STRING:
        break;
    default:
        if (v->v)
            mila_free(v->v);
    }
    mila_free(v);
}

void val_kill_incomplete(Value *v)
{
    if (!v)
        return;
#ifdef MILA_DEBUG
    printf("  -- val_kill_incomplete:\n     type: %s\n     refcount %i -> 0 "
           "(forced)\n     %s\n     value: ",
           GET_TYPENAME(v), v->refcount,
           v->refcount - 1 <= 0 ? "will be freed after" : "will survive");
    print_value_repr(v);
    puts("");
#endif
    if (v->method_table && v->method_table[UMethodKill])
    {
        ((unary_method)v->method_table[UMethodKill])(v);
        goto cleanup;
    }
    if (v->method_table && v->method_table[UMethodFree])
    {
        ((unary_method)v->method_table[UMethodFree])(v);
        goto cleanup;
    }
    // mila_free internals
    if (v->type == T_STRING && GET_STRING(v))
        mila_free(GET_STRING(v));
    if (v->type == T_ERROR && GET_ERROR_MESSAGE(v))
        mila_free(GET_ERROR_MESSAGE(v));
    if (v->type == T_TAGGED_ERROR && v->v->tagged_error.message)
        mila_free(v->v->tagged_error.message);
    if (v->type == T_FUNCTION && GET_FUNCTION(v))
    {
        if (GET_FUNCTION(v)->params)
        {
            char **p = GET_FUNCTION(v)->params;
            for (int i = 0; p[i]; ++i)
                mila_free(p[i]);
            mila_free(p);
        }
        if (GET_FUNCTION(v)->contextuals)
        {
            char **p = GET_FUNCTION(v)->contextuals;
            for (int i = 0; p[i]; ++i)
                mila_free(p[i]);
            mila_free(p);
        }
        if (GET_FUNCTION(v)->body_src)
            mila_free(GET_FUNCTION(v)->body_src);
        if (GET_FUNCTION(v)->name)
            mila_free(GET_FUNCTION(v)->name);
        env_free(GET_FUNCTION(v)->closure);
        mila_free(GET_FUNCTION(v));
    }
    if (v->type == T_NATIVE)
    {
        if (GET_NATIVE(v)->name)
            mila_free(GET_NATIVE(v)->name);
        mila_free(GET_NATIVE(v));
    }
    if (v->type == T_RETURN)
    {
        val_kill((Value *)v->v);
    }
cleanup:;
    mila_free(v->type_name);
    if (v->method_table && v->owns_table)
        mila_free(v->method_table);
}

// Environment (simple linked list of frames + variables)

Env *env_new(Env *parent)
{
    Env *e = mila_malloc(sizeof(Env));
    e->vars = NULL;
    e->contextual_vars = NULL;
    e->parent = parent;
    return e;
}

void env_copy(Env *dest, Env *src)
{
    if (!src || !dest)
        return;
    Var *v = src->vars;
    while (v)
    {
        Var *nx = v->next;
        env_set_local(dest, v->name, v->value);
        v = nx;
    }
}

void env_free(Env *e)
{
    if (!e)
        return;
    Var *v = e->vars;
    while (v)
    {
        Var *nx = v->next;
        mila_free(v->name);
        val_release(v->value);
        mila_free(v);
        v = nx;
    }
    v = e->contextual_vars;
    while (v)
    {
        Var *nx = v->next;
        mila_free(v->name);
        mila_free(v);
        v = nx;
    }
    mila_free(e);
}

void env_dump(Env *e)
{
    if (!e)
        return;
    Var *v = e->vars;
    while (v)
    {
        Var *nx = v->next;
        if (!v->value)
        {
            printf("'%s' dangling!\n", v->name);
            v = nx;
            continue;
        }
        if (v->value)
        {
            char *res = as_c_string_repr(v->value);
            printf("%s = %s (%i)\n", v->name, res, v->value->refcount);
            mila_free(res);
        }
        v = nx;
    }
    v = e->contextual_vars;
    while (v)
    {
        Var *nx = v->next;
        if (!v->value)
        {
            printf("%s dangling\n", v->name);
            v = nx;
            continue;
        }
        if (v->value)
        {
            char *res = as_c_string_repr(v->value);
            printf("%s = %s (%i)\n", v->name, res, v->value->refcount);
            mila_free(res);
        }
        v = nx;
    }
}

void env_kill(Env *e)
{
    if (!e)
        return;
    Var *v = e->vars;
    while (v)
    {
        Var *nx = v->next;
        mila_free(v->name);
        val_kill(v->value);
        mila_free(v);
        v = nx;
    }
    v = e->contextual_vars;
    while (v)
    {
        Var *nx = v->next;
        mila_free(v->name);
        mila_free(v);
        v = nx;
    }
    mila_free(e);
}

Value *env_get(Env *e, const char *name)
{
    for (Env *cur = e; cur; cur = cur->parent)
    {
        for (Var *v = cur->vars; v; v = v->next)
        {
            if (strcmp(v->name, name) == 0)
            {
                return v->value;
            }
        }
    }
    return NULL;
}

Value *env_get_contextual(Env *e, const char *name)
{
    if (!(e && e->contextual_vars))
        return NULL;
    for (Env *cur = e; cur; cur = cur->parent)
    {
        for (Var *v = cur->contextual_vars; v; v = v->next)
        {
            if (strcmp(v->name, name) == 0)
            {
                return v->value;
            }
        }
    }
    return NULL;
}

int env_set_local_contextual(Env *e, const char *name, Value *val)
{
    // set or create in current frame
    for (Var *v = e->contextual_vars; v; v = v->next)
    {
        if (strcmp(v->name, name) == 0)
        {
            v->value = val;
            return 0;
        }
    }
    Var *nv = mila_malloc(sizeof(Var));
    nv->name = mila_strdup(name);
    nv->next = e->contextual_vars;
    e->contextual_vars = nv;
    return 1;
}

int env_set_contextual(Env *e, const char *name, Value *val)
{
    // assign to nearest visible frame that contains name, else set local
    for (Env *cur = e; cur; cur = cur->parent)
    {
        for (Var *v = cur->contextual_vars; v; v = v->next)
        {
            if (strcmp(v->name, name) == 0)
            {
                v->value = val;
                return 0;
            }
        }
    }
    // not found, set locally
    env_set_local_contextual(e, name, val);
    return 1;
}

int env_set_local_raw_contextual(Env *e, const char *name, Value *val)
{
    // set or create in current frame
    for (Var *v = e->contextual_vars; v; v = v->next)
    {
        if (strcmp(v->name, name) == 0)
        {
            v->value = val;
            return 0; // found
        }
    }
    Var *nv = mila_malloc(sizeof(Var));
    nv->name = mila_strdup(name);
    nv->value = val;
    nv->next = e->contextual_vars;
    e->contextual_vars = nv;
    return 1; // not found
}

int env_set_raw_contextual(Env *e, const char *name, Value *val)
{
    // assign to nearest visible frame that contains name, else set local
    for (Env *cur = e; cur; cur = cur->parent)
    {
        for (Var *v = cur->contextual_vars; v; v = v->next)
        {
            if (strcmp(v->name, name) == 0)
            {
                v->value = val;
                return 0;
            }
        }
    }
    // not found, set locally
    env_set_local_raw_contextual(e, name, val);
    return 1;
}

int env_set_local(Env *e, const char *name, Value *val)
{
    // set or create in current frame
    for (Var *v = e->vars; v; v = v->next)
    {
        if (strcmp(v->name, name) == 0)
        {
            val_release(v->value);
            v->value = val_retain(val);
            return 0;
        }
    }

    Var *nv = mila_malloc(sizeof(Var));
    nv->name = mila_strdup(name);
    nv->value = val_retain(val);
    nv->next = e->vars;
    e->vars = nv;
    return 1;
}

int env_set(Env *e, const char *name, Value *val)
{
    // assign to nearest visible frame that contains name, else set local
    for (Env *cur = e; cur; cur = cur->parent)
    {
        for (Var *v = cur->vars; v; v = v->next)
        {
            if (strcmp(v->name, name) == 0)
            {
                val_release(v->value);
                v->value = val_retain(val);
                return 1;
            }
        }
    }
    // not found, set locally
    env_set_local(e, name, val);
    return 0;
}

int env_set_local_raw(Env *e, const char *name, Value *val)
{
    // set or create in current frame
    for (Var *v = e->vars; v; v = v->next)
    {
        if (strcmp(v->name, name) == 0)
        {
            val_release(v->value);
            v->value = val;
            return 0;
        }
    }
    Var *nv = mila_malloc(sizeof(Var));
    nv->name = mila_strdup(name);
    nv->value = val;
    nv->next = e->vars;
    e->vars = nv;
    return 1;
}

int env_set_raw(Env *e, const char *name, Value *val)
{
    // assign to nearest visible frame that contains name, else set local
    for (Env *cur = e; cur; cur = cur->parent)
    {
        for (Var *v = cur->vars; v; v = v->next)
        {
            if (strcmp(v->name, name) == 0)
            {
                val_release(v->value);
                v->value = val;
                return 0;
            }
        }
    }
    // not found, set locally
    env_set_local_raw(e, name, val);
    return 1;
}

int env_remove(Env *env, const char *name)
{
    if (!env || !env->vars)
        return 1;

    Var *prev = NULL;
    Var *cur = env->vars;

    while (cur)
    {
        if (strcmp(cur->name, name) == 0)
        {
            if (prev)
                prev->next = cur->next;
            else
                env->vars = cur->next;
            mila_free(cur->name);
            mila_free(cur);
            return 0;
        }

        prev = cur;
        cur = cur->next;
    }
    return 1;
}

int env_remove_contextual(Env *env, const char *name)
{
    if (!env || !env->contextual_vars)
        return 1;

    Var *prev = NULL;
    Var *cur = env->contextual_vars;

    while (cur)
    {
        if (strcmp(cur->name, name) == 0)
        {
            if (prev)
                prev->next = cur->next;
            else
                env->contextual_vars = cur->next;
            mila_free(cur->name);
            mila_free(cur);
            return 0;
        }

        prev = cur;
        cur = cur->next;
    }
    return 1;
}

#ifndef SAFE_BUILD

int load_library(Env *env, const char *libpath)
{
    int fail = 1;
    void *lib = dlopen(libpath, RTLD_LAZY);
    if (!lib)
    {
        fprintf(stderr, "dlopen('%s') failed: %s\n", libpath, dlerror());
        return -1;
    }

    dlerror();
    void (*init_func)(Env *) = dlsym(lib, "_mila_lib_init");
    const char *err = dlerror();
    if (!err)
    {
        fail = 0;
        init_func(env);
    }
    void (*deinit_func)(Env *) = dlsym(lib, "_mila_lib_deinit");
    err = dlerror();
    if (!err)
    {
        fail = 0;
        char *name = NULL;
        malloc_sprintf(&name, "%s:_mila_lib_deinit", libpath);
        CleanupRegistryEntry *entry = make_cleanup_entry(name, deinit_func);
        da_append(cleanup_registry, entry);
        free(name);
    }

    const NativeEntry *entries =
        (const NativeEntry *)dlsym(lib, "lib_function_entries");

    err = dlerror();
    if (!err)
    {
        for (size_t i = 0; entries[i].name && entries[i].func; i++)
        {
            env_register_native(env, entries[i].name, entries[i].func);
        }
        return 0;
    }

    const char *const *names = (const char *const *)dlsym(lib, "lib_functions");

    err = dlerror();
    if (!err)
    {
        for (size_t i = 0; names[i] != NULL; i++)
        {
            dlerror();
            void *f = dlsym(lib, names[i]);
            const char *err2 = dlerror();
            if (err2)
            {
                fprintf(stderr, "Warning: '%s' not found in '%s'\n", names[i], libpath);
                continue;
            }
            env_register_native(env, names[i], f);
        }
        return 0;
    }

    if (!fail)
        return 0;

    fprintf(stderr,
            "dlsym 'lib_functions' error: %s\nMust have a simple 'const char* "
            "lib_functions' and list the function names (last item must be "
            "NULL)\nor also 'const NativeEntry lib_function_entries[]' which "
            "takes in (NativeEntry){.name, .func} with the last item being "
            "(NativeEntry){NULL, NULL}\n",
            err);
    dlclose(lib);
    return -2;
}
int load_library_noisy(Env *env, const char *libpath)
{
    int fail = 1;
    void *lib = dlopen(libpath, RTLD_LAZY);
    if (!lib)
    {
        fprintf(stderr, "dlopen('%s') failed: %s\n", libpath, dlerror());
        return -1;
    }

    dlerror();
    void (*init_func)(Env *) = dlsym(lib, "_mila_lib_init");
    const char *err = dlerror();
    if (!err)
    {
        fail = 0;
        char *name = NULL;
        malloc_sprintf(&name, "%s:_mila_lib_init", libpath);
        printf("Loaded init: %s\n", name);
        free(name);
        init_func(env);
    }
    void (*deinit_func)(Env *) = dlsym(lib, "_mila_lib_deinit");
    err = dlerror();
    if (!err)
    {
        fail = 0;
        char *name = NULL;
        malloc_sprintf(&name, "%s:_mila_lib_deinit", libpath);
        printf("Loaded cleanup: %s\n", name);
        CleanupRegistryEntry *entry = make_cleanup_entry(name, deinit_func);
        da_append(cleanup_registry, entry);
        free(name);
    }

    const NativeEntry *entries =
        (const NativeEntry *)dlsym(lib, "lib_function_entries");

    err = dlerror();
    if (!err)
    {
        for (size_t i = 0; entries[i].name && entries[i].func; i++)
        {
            fprintf(stderr, "from lib_function_entries, Loaded symbol '%s'\n",
                    entries[i].name);
            env_register_native(env, entries[i].name, entries[i].func);
        }
        return 0;
    }

    const char *const *names = (const char *const *)dlsym(lib, "lib_functions");

    err = dlerror();
    if (!err)
    {
        for (size_t i = 0; names[i] != NULL; i++)
        {
            dlerror();
            void *f = dlsym(lib, names[i]);
            const char *err2 = dlerror();
            if (err2)
            {
                fprintf(stderr, "Warning: '%s' not found in '%s'\n", names[i], libpath);
                continue;
            }
            fprintf(stderr, "from lib_functions, Loaded symbol '%s'\n", names[i]);
            env_register_native(env, names[i], f);
        }
        return 0;
    }

    if (!fail)
        return 0;

    fprintf(stderr,
            "dlsym 'lib_functions' error: %s\nMust have a simple 'const char* "
            "lib_functions' and list the function names (last item must be "
            "NULL)\nor also 'const NativeEntry lib_function_entries[]' which "
            "takes in (NativeEntry){.name, .func} with the last item being "
            "(NativeEntry){NULL, NULL}\n",
            err);
    dlclose(lib);
    return -2;
}

#endif

// helper to bind native into environment with a name
void env_register_native(Env *env, const char *name, NativeFn fn)
{
    Value *nv = vnative(fn, name);
    env_set_local_raw(env, name, nv);
}

// ---------- Parser/Evaluator that directly reads source and evaluates (no
// separate lexer) ----------

Src *src_new(const char *s)
{
    Src *S = mila_malloc(sizeof(Src));
    S->len = strlen(s);
    S->src = mila_strdup(s);
    S->pos = 0;
    return S;
}
void src_free(Src *s)
{
    if (!s)
        return;
    mila_free(s->src);
    mila_free(s);
}

// helpers
inline char src_peek(Src *s) { return s->pos < s->len ? s->src[s->pos] : '\0'; }
inline char src_get(Src *s)
{
    char c = s->pos < s->len ? s->src[s->pos++] : '\0';
    return c;
}
inline int src_eof(Src *s) { return s->pos >= s->len; }

inline void skip_block(Src *s)
{
    skip_ws(s);
    // body is block; extract substring from '{' to matching '}'
    if (src_peek(s) != '{')
    {
        return;
    }
    int depth = 0;
    size_t i = s->pos;
    for (; i < s->len; ++i)
    {
        char ch = s->src[i];
        if (ch == '{')
            depth++;
        else if (ch == '}')
        {
            depth--;
            if (depth == 0)
            {
                i++;
                break;
            }
        }
        else if (ch == '"')
        {
            val_release(parse_string(s));
        }
        else if (ch == '/' && src_peek(s) == '/')
        {
            // skip comments
            i++;
            while (i < s->len && s->src[i] != '\n')
                i++;
        }
        if (ch == '/' && s->pos + 1 < s->len && s->src[s->pos + 1] == '*')
        {
            // block comment
            src_get(s);
            src_get(s);
            while (!src_eof(s))
            {
                char a = src_get(s);
                if (a == '*' && src_peek(s) == '/')
                {
                    src_get(s);
                    break;
                }
            }
            continue;
        }
    }
    if (i > s->len)
        i = s->len;
    s->pos = i;
}

inline void skip_expr(Src *s)
{
    skip_ws(s);
    // body is block; extract substring from '{' to matching '}'
    int depth = 1;
    size_t i = s->pos;
    for (; i < s->len; ++i)
    {
        char ch = s->src[i];
        if (ch == '(')
            depth++;
        else if (ch == ')')
        {
            depth--;
            if (depth == 0)
            {
                i++;
                break;
            }
        }
        else if (ch == '"')
        {
            val_release(parse_string(s));
        }
    }
    if (i > s->len)
        i = s->len;
    s->pos = i;
}

inline void skip_stmt(Src *s)
{
    skip_ws(s);
    size_t i = s->pos;
    for (; i < s->len; ++i)
    {
        char ch = s->src[i];
        if (ch == ';')
        {
            i++;
            break;
        }
        else if (ch == '"')
        {
            // skip string literal
            i++;
            while (i < s->len && s->src[i] != '"')
            {
                if (s->src[i] == '\\' && i + 1 < s->len)
                    i += 2;
                else
                    i++;
            }
        }
    }
    if (i > s->len)
        i = s->len;
    s->pos = i;
}

inline void skip_ws(Src *s)
{
    for (;;)
    {
        char c = src_peek(s);
        if (c == '\0')
            return;
        if (isspace((unsigned char)c))
        {
            src_get(s);
            continue;
        }
        if (c == '/' && s->pos + 1 < s->len && s->src[s->pos + 1] == '/')
        {
            // line comment
            src_get(s);
            src_get(s);
            while (!src_eof(s) && src_get(s) != '\n')
            {
            }
            continue;
        }
        if (c == '/' && s->pos + 1 < s->len && s->src[s->pos + 1] == '*')
        {
            // block comment
            src_get(s);
            src_get(s);
            while (!src_eof(s))
            {
                char a = src_get(s);
                if (a == '*' && src_peek(s) == '/')
                {
                    src_get(s);
                    break;
                }
            }
            continue;
        }
        break;
    }
}

int match_char(Src *s, char c)
{
    skip_ws(s);
    if (src_peek(s) == c)
    {
        src_get(s);
        return 1;
    }
    return 0;
}

// Return type: const char* (NULL = success, non-NULL = error message)
const char *skip_expr_prec(Src *s, int min_prec);

const char *skip_fn_call_args(Src *s)
{
    if (!match_char(s, '('))
        return ERR_EXPECTED_PAREN;
    skip_ws(s);
    if (match_char(s, ')'))
        return ERR_SUCCESS;

    for (;;)
    {
        const char *err = skip_expr_prec(s, 1);
        if (err)
            return err;

        skip_ws(s);
        if (match_char(s, ','))
        {
            skip_ws(s);
            continue;
        }
        if (match_char(s, ')'))
            return ERR_SUCCESS;
        return ERR_EXPECTED_COMMA;
    }
}

const char *skip_parse_block(Src *s)
{
    if (!match_char(s, '{'))
        return ERR_EXPECTED_BRACE;

    while (!src_eof(s))
    {
        skip_ws(s);
        if (match_char(s, '}'))
            return ERR_SUCCESS;
        const char *err = skip_parse_statement(s);
        if (err)
            return err;
    }
    return ERR_BLOCK_UNCLOSED;
}

const char *skip_primary(Src *s)
{
    skip_ws(s);
    char c = src_peek(s);

    // Numbers
    if (isdigit((unsigned char)c) ||
        ((c == '-' || c == '+') && s->pos + 1 < s->len && isdigit((unsigned char)s->src[s->pos + 1])))
    {
        if (c == '-' || c == '+')
            src_get(s);
        while (isdigit((unsigned char)src_peek(s)) || src_peek(s) == '.' ||
               src_peek(s) == 'x' || src_peek(s) == 'X' ||
               isxdigit((unsigned char)src_peek(s)) || src_peek(s) == '~')
            src_get(s);
        if (src_peek(s) == 'u' || src_peek(s) == 'U')
            src_get(s);
        if (src_peek(s) == '%')
            src_get(s);
        return ERR_SUCCESS;
    }

    // Strings
    if (c == '"')
    {
        src_get(s);
        while (!src_eof(s))
        {
            char ch = src_get(s);
            if (ch == '"')
                return ERR_SUCCESS;
            if (ch == '\\')
                src_get(s);
        }
        return ERR_STRING_UNCLOSED;
    }

    // Lists/dicts with element validation
    if (c == '[')
    {
        src_get(s);
        skip_ws(s);
        if (src_peek(s) != ']')
        {
            for (;;)
            {
                const char *err = skip_expr_prec(s, 1);
                if (err)
                    return err;
                skip_ws(s);
                if (match_char(s, ',') || match_char(s, '='))
                {
                    skip_ws(s);
                    continue;
                }
                if (match_char(s, ']'))
                    return ERR_SUCCESS;
                return ERR_EXPECTED_BRACKET;
            }
        }
        else
        {
            src_get(s);
        }
        return ERR_SUCCESS;
    }

    // Parenthesized expressions
    if (c == '(')
    {
        src_get(s);
        const char *err = skip_expr_prec(s, 1);
        if (err)
            return err;
        skip_ws(s);
        if (!match_char(s, ')'))
            return ERR_PAREN_UNCLOSED;

        skip_ws(s);

        // Function call on expression
        if (src_peek(s) == '(')
        {
            return skip_fn_call_args(s);
        }

        // Subscripts on expression
        if (src_peek(s) == '[')
        {
            while (src_peek(s) == '[')
            {
                src_get(s);
                err = skip_expr_prec(s, 1);
                if (err)
                    return err;
                if (!match_char(s, ']'))
                    return ERR_BRACKET_UNCLOSED;
            }
        }

        // Method call on expression
        if (src_peek(s) == ':')
        {
            src_get(s);
            if (src_peek(s) == ':') src_get(s);
            char *method = parse_ident(s);
            if (!method)
                return ERR_INVALID_IDENT;
            mila_free(method);
            if (src_peek(s) == '(')
                return skip_fn_call_args(s);
        }

        return ERR_SUCCESS;
    }

    // Blocks
    if (c == '{')
        return skip_parse_block(s);

    // Dedent blocks
    if (c == '!' && s->pos + 1 < s->len && s->src[s->pos + 1] == '{')
    {
        src_get(s);
        return skip_parse_block(s);
    }

    // Function literals - validate body recursively
    if (is_keyword_at(s, "fn"))
    {
        s->pos += 2;
        FunctionParameters *params = parse_param_list(s);
        if (!params)
            return ERR_INVALID_PARAM_LIST;
        for (int i = 0; params->params[i]; i++)
        {
            mila_free(params->params[i]);
            if (params->defaults[i])
                mila_free(params->defaults[i]);
        }
        mila_free(params->params);
        mila_free(params->defaults);
        mila_free(params);

        char **ctx = parse_context_list(s);
        if (ctx)
        {
            for (int i = 0; ctx[i]; i++)
                mila_free(ctx[i]);
            mila_free(ctx);
        }

        if (match_char(s, ':'))
        {
            char **closure = parse_context_list(s);
            if (closure)
            {
                for (int i = 0; closure[i]; i++)
                    mila_free(closure[i]);
                mila_free(closure);
            }
        }

        if (is_keyword_at(s, "->"))
        {
            s->pos += 2;
            skip_ws(s);
            if (src_peek(s) == '"')
                val_release(parse_string(s));
        }

        return skip_parse_block(s);
    }

    // Identifiers with potential calls/subscripts/methods
    if (is_ident_start(c))
    {
        char *id = parse_ident(s);
        if (!id)
            return ERR_INVALID_IDENT;
        mila_free(id);

        skip_ws(s);

        // Function call - validate all arguments
        if (src_peek(s) == '(')
        {
            return skip_fn_call_args(s);
        }

        // Subscripts
        while (src_peek(s) == '[')
        {
            src_get(s);
            const char *err = skip_expr_prec(s, 1);
            if (err)
                return err;
            if (!match_char(s, ']'))
                return ERR_BRACKET_UNCLOSED;
        }

        // Method call
        if (src_peek(s) == ':')
        {
            src_get(s);
            if (src_peek(s) == ':') src_get(s);
            char *method = parse_ident(s);
            if (!method)
                return ERR_INVALID_IDENT;
            mila_free(method);
            if (src_peek(s) == '(')
                return skip_fn_call_args(s);
        }

        return ERR_SUCCESS;
    }

    skip_ws(s);
    if (src_eof(s))
        return ERR_SUCCESS;
    else
        return ERR_INVALID_EXPR;
}

const char *skip_expr_prec(Src *s, int min_prec)
{
    const char *err = skip_primary(s);
    if (err)
        return err;

    for (;;)
    {
        int saved_pos = s->pos;
        skip_ws(s);
        char a = src_peek(s);
        if (a == '\0')
            return ERR_SUCCESS;

        char b = s->pos + 1 < s->len ? s->src[s->pos + 1] : '\0';
        MethodType op = MethodNone;
        int prec = 0;

        if (a == '|' && b == '|')
        {
            s->pos += 2;
            op = BMethodOr;
            prec = 2;
        }
        else if (a == '&' && b == '&')
        {
            s->pos += 2;
            op = BMethodAnd;
            prec = 1;
        }
        else if (a == '=' && b == '=')
        {
            s->pos += 2;
            op = BMethodEq;
            prec = 4;
        }
        else if (a == '!' && b == '=')
        {
            s->pos += 2;
            op = BMethodNe;
            prec = 4;
        }
        else if (a == '<' && b == '=')
        {
            s->pos += 2;
            op = BMethodLE;
            prec = 6;
        }
        else if (a == '>' && b == '=')
        {
            s->pos += 2;
            op = BMethodGE;
            prec = 6;
        }
        else if (a == '>' && b == '>')
        {
            s->pos += 2;
            op = BMethodRShift;
            prec = 5;
        }
        else if (a == '<' && b == '<')
        {
            s->pos += 2;
            op = BMethodLShift;
            prec = 5;
        }
        else if (a == '?' && b == '?')
        {
            s->pos += 2;
            op = BMethodDefault;
            prec = 3;
        }
        else if (a == '=' && b == '>')
        {
            s->pos += 2;
            op = BMethodGlob;
            prec = 9;
        }
        else if (a == '+')
        {
            s->pos++;
            op = BMethodAdd;
            prec = 7;
        }
        else if (a == '-')
        {
            s->pos++;
            op = BMethodSub;
            prec = 7;
        }
        else if (a == '*')
        {
            s->pos++;
            op = BMethodMul;
            prec = 8;
        }
        else if (a == '/')
        {
            s->pos++;
            op = BMethodDiv;
            prec = 8;
        }
        else if (a == '%')
        {
            s->pos++;
            op = BMethodMod;
            prec = 8;
        }
        else if (a == '<')
        {
            s->pos++;
            op = BMethodLess;
            prec = 6;
        }
        else if (a == '>')
        {
            s->pos++;
            op = BMethodGreat;
            prec = 6;
        }

        if (op == MethodNone)
        {
            s->pos = saved_pos;
            return ERR_SUCCESS;
        }

        if (prec < min_prec)
        {
            s->pos = saved_pos;
            return ERR_SUCCESS;
        }

        err = skip_expr_prec(s, prec + 1);
        if (err)
            return err;
    }
}

const char *skip_parse_expr(Src *s)
{
    return skip_expr_prec(s, 1);
}

const char *skip_parse_statement(Src *s)
{
    skip_ws(s);

    if (is_keyword_at(s, "var"))
    {
        s->pos += 3;
        char *id = parse_ident(s);
        if (!id)
            return ERR_INVALID_IDENT;
        mila_free(id);
        if (match_char(s, ':'))
        {
            skip_ws(s);
            if (src_peek(s) != '"')
                return ERR_EXPECTED_TYPE_ANNOTATION;
            val_release(parse_string(s));
        }
        if (match_char(s, '='))
        {
            const char *err = skip_expr_prec(s, 1);
            if (err)
                return err;
        }
        return match_char(s, ';') ? ERR_SUCCESS : ERR_EXPECTED_SEMICOLON;
    }

    if (is_keyword_at(s, "set"))
    {
        s->pos += 3;
        char *id = parse_ident(s);
        if (!id)
            return ERR_INVALID_IDENT;
        mila_free(id);
        while (match_char(s, '['))
        {
            const char *err = skip_expr_prec(s, 1);
            if (err)
                return err;
            if (!match_char(s, ']'))
                return ERR_BRACKET_UNCLOSED;
        }
        skip_ws(s);
        if (src_peek(s) == '+' || src_peek(s) == '-' || src_peek(s) == '*' ||
            src_peek(s) == '/' || src_peek(s) == '%')
            src_get(s);
        if (!match_char(s, '='))
            return ERR_EXPECTED_EQUALS;
        const char *err = skip_expr_prec(s, 1);
        if (err)
            return err;
        match_char(s, ';');
        return ERR_SUCCESS;
    }

    if (is_keyword_at(s, "return"))
    {
        s->pos += 6;
        const char *err = skip_expr_prec(s, 1);
        if (err)
            return err;
        return match_char(s, ';') ? ERR_SUCCESS : ERR_EXPECTED_SEMICOLON;
    }

    if (is_keyword_at(s, "if"))
    {
        s->pos += 2;
        if (!match_char(s, '('))
            return ERR_EXPECTED_PAREN;
        const char *err = skip_expr_prec(s, 1);
        if (err)
            return err;
        if (!match_char(s, ')'))
            return ERR_PAREN_UNCLOSED;
        if (match_char(s, '{'))
        {
            s->pos--;
            err = skip_parse_block(s);
            if (err)
                return err;
        }
        else
        {
            err = skip_parse_statement(s);
            if (err)
                return err;
        }
        while (is_keyword_at(s, "elif"))
        {
            s->pos += 4;
            if (!match_char(s, '('))
                return ERR_EXPECTED_PAREN;
            err = skip_expr_prec(s, 1);
            if (err)
                return err;
            if (!match_char(s, ')'))
                return ERR_PAREN_UNCLOSED;
            if (match_char(s, '{'))
            {
                s->pos--;
                err = skip_parse_block(s);
                if (err)
                    return err;
            }
            else
            {
                err = skip_parse_statement(s);
                if (err)
                    return err;
            }
        }
        if (is_keyword_at(s, "else"))
        {
            s->pos += 4;
            if (match_char(s, '{'))
            {
                s->pos--;
                err = skip_parse_block(s);
                if (err)
                    return err;
            }
            else
            {
                err = skip_parse_statement(s);
                if (err)
                    return err;
            }
        }
        return ERR_SUCCESS;
    }

    if (is_keyword_at(s, "while"))
    {
        s->pos += 5;
        if (!match_char(s, '('))
            return ERR_EXPECTED_PAREN;
        const char *err = skip_expr_prec(s, 1);
        if (err)
            return err;
        if (!match_char(s, ')'))
            return ERR_PAREN_UNCLOSED;
        if (match_char(s, '{'))
        {
            s->pos--;
            return skip_parse_block(s);
        }
        return skip_parse_statement(s);
    }

    if (is_keyword_at(s, "foreach"))
    {
        s->pos += 7;
        char *id = parse_ident(s);
        if (!id)
            return ERR_INVALID_IDENT;
        if (strcmp(id, "yield") == 0)
        {
            mila_free(id);
            id = parse_ident(s);
            if (!id)
                return ERR_INVALID_IDENT;
        }
        mila_free(id);
        if (!match_char(s, ':'))
            return ERR_EXPECTED_COLON;
        const char *err = skip_expr_prec(s, 1);
        if (err)
            return err;
        if (match_char(s, '{'))
        {
            s->pos--;
            return skip_parse_block(s);
        }
        return skip_parse_statement(s);
    }

    if (is_keyword_at(s, "fn"))
    {
        s->pos += 2;
        char *name = parse_ident(s);
        if (!name)
            return ERR_INVALID_IDENT;
        mila_free(name);
        FunctionParameters *params = parse_param_list(s);
        if (!params)
            return ERR_INVALID_PARAM_LIST;
        for (int i = 0; params->params[i]; i++)
        {
            mila_free(params->params[i]);
            if (params->defaults[i])
                mila_free(params->defaults[i]);
        }
        mila_free(params->params);
        mila_free(params->defaults);
        mila_free(params);

        char **ctx = parse_context_list(s);
        if (ctx)
        {
            for (int i = 0; ctx[i]; i++)
                mila_free(ctx[i]);
            mila_free(ctx);
        }

        if (match_char(s, ':'))
        {
            char **closure = parse_context_list(s);
            if (closure)
            {
                for (int i = 0; closure[i]; i++)
                    mila_free(closure[i]);
                mila_free(closure);
            }
        }

        if (is_keyword_at(s, "->"))
        {
            s->pos += 2;
            skip_ws(s);
            if (src_peek(s) == '"')
                val_release(parse_string(s));
        }

        if (match_char(s, '{'))
        {
            s->pos--;
            return skip_parse_block(s);
        }
        return skip_parse_statement(s);
    }

    if (is_keyword_at(s, "object"))
    {
        s->pos += 6;
        char *name = parse_ident(s);
        if (!name)
            return ERR_INVALID_IDENT;
        mila_free(name);
        if (is_keyword_at(s, "with"))
        {
            s->pos += 4;
            char *obj = parse_ident(s);
            if (!obj)
                return ERR_INVALID_IDENT;
            mila_free(obj);
        }
        return skip_parse_block(s);
    }

    if (is_keyword_at(s, "block"))
    {
        s->pos += 5;
        char *bname = parse_ident(s);
        if (!bname)
            return ERR_INVALID_IDENT;
        mila_free(bname);
        return skip_parse_block(s);
    }

    if (is_keyword_at(s, "namespace"))
    {
        s->pos += 9;
        char *ns = parse_ident(s);
        if (!ns)
            return ERR_INVALID_IDENT;
        mila_free(ns);
        return skip_parse_block(s);
    }

    if (is_keyword_at(s, "catch"))
    {
        s->pos += 5;
        char *cid = parse_ident(s);
        if (!cid)
            return ERR_INVALID_IDENT;
        mila_free(cid);
        return skip_parse_block(s);
    }

    if (src_peek(s) == '{')
    {
        return skip_parse_block(s);
    }

    const char *err = skip_expr_prec(s, 1);
    if (err)
        return err;
    match_char(s, ';');
    return ERR_SUCCESS;
}

const char *skip_parse_source(Src *s)
{
    // Skip shebang: #!/path/to/mila
    if (src_peek(s) == '#')
    {
        src_get(s);
        if (src_get(s) != '!')
            return "Invalid shebang";
        while (!src_eof(s) && src_get(s) != '\n')
            ;
    }

    skip_ws(s);

    // Parse optional main function: !fn(args) -> "type" { body }
    if (src_peek(s) == '!')
    {
        src_get(s);
        if (is_keyword_at(s, "fn"))
        {
            s->pos += 2;
            FunctionParameters *fnp = parse_param_list(s);
            if (!fnp)
                return ERR_INVALID_PARAM_LIST;
            for (int i = 0; fnp->params[i]; i++)
            {
                mila_free(fnp->params[i]);
                if (fnp->defaults[i])
                    mila_free(fnp->defaults[i]);
            }
            mila_free(fnp->params);
            mila_free(fnp->defaults);
            mila_free(fnp);

            // Optional return type annotation
            if (is_keyword_at(s, "->"))
            {
                s->pos += 2;
                skip_ws(s);
                if (src_peek(s) != '"')
                    return ERR_EXPECTED_TYPE_ANNOTATION;
                val_release(parse_string(s));
            }
        }
    }

    while (!src_eof(s))
    {
        const char *err = skip_parse_statement(s);
        if (err)
            return err;
    }
    return ERR_SUCCESS;
}

int syn_check(Src *s)
{
    const char *err = skip_parse_source(s);
    if (err)
    {
        size_t line = 1, col = 0;
        size_t line_start = 0;

        for (size_t i = 0; i < s->pos && i < s->len; ++i)
        {
            if (s->src[i] == '\n')
            {
                line++;
                line_start = i + 1;
                col = 0;
            }
            else
            {
                col++;
            }
        }

        size_t line_end = line_start;
        while (line_end < s->len && s->src[line_end] != '\n')
            line_end++;

        fprintf(stderr, "Syntax Error at line %zu, column %zu:\n", line, col);
        fprintf(stderr, "  %s\n", err);
        fprintf(stderr, "  `");
        for (size_t i = line_start; i < line_end; ++i)
            fprintf(stderr, "%c", s->src[i]);
        fprintf(stderr, "`\n  ");
        for (size_t i = 1; i < col - 1; ++i)
            fprintf(stderr, " ");
        fprintf(stderr, "^\n");
        return 1;
    }
    else
    {
        fprintf(stderr, "Success!\n");
        return 0;
    }
}

int is_ident_start(char c)
{
    return isalpha((unsigned char)c) || c == '_' || c == '.' || c == '\'';
}

inline char *parse_ident_string(Src *s)
{
    skip_ws(s);
    if (src_peek(s) != '\'')
        return NULL;
    src_get(s); // consume opening "

    size_t cap = 256;
    size_t len = 0;
    char *buf = mila_malloc(cap);
    if (!buf)
        return NULL;

    while (!src_eof(s))
    {
        char c = src_get(s);
        if (c == '\'')
            break;

        // Grow buffer if needed
        if (len + 1 >= cap)
        {
            cap = cap + (int)(cap * 0.3); // grow by 30%
            char *tmp = mila_realloc(buf, cap);
            if (!tmp)
            {
                mila_free(buf);
                return NULL;
            }
            buf = tmp;
        }
        buf[len++] = c;
    }

    // shrink to exact size
    char *res = mila_realloc(buf, len + 1);
    if (!res)
        res = buf; // if mila_realloc fails, keep original buffer

    res[len] = '\0';
    return res;
}

// parse identifier
inline char *parse_ident(Src *s)
{
    skip_ws(s);
    int st = s->pos;
    char c = src_peek(s);

    if (c == '\'')
        return parse_ident_string(s);

    if (!is_ident_start(c))
        return NULL;
    src_get(s);
    while (isalnum((unsigned char)src_peek(s)) || src_peek(s) == '_' ||
           src_peek(s) == '.' || src_peek(s) == '?')
        src_get(s);
    int en = s->pos;
    int n = en - st;
    char *res = mila_malloc(n + 1);
    memcpy(res, s->src + st, n);
    res[n] = 0;

    if (res[0] == '.' && s->cur_namespace != NULL)
    {
        char *r = mila_strdup(s->cur_namespace);
        malloc_sprintf(&r, ".%s", res + 1);
        mila_free(res);
        return r;
    }

    return res;
}

char *i128toa(__int128 value)
{
    char tmp[50]; // enough to hold 128-bit decimal
    int i = 0;

    int neg = value < 0;
    unsigned __int128 v = neg ? -value : value;

    // Convert digits backwards
    do
    {
        tmp[i++] = '0' + (v % 10);
        v /= 10;
    } while (v != 0);

    if (neg)
        tmp[i++] = '-';

    // Allocate string
    char *str = malloc(i + 1);
    if (!str)
        return NULL;

    // Reverse into allocated string
    for (int j = 0; j < i; j++)
    {
        str[j] = tmp[i - j - 1];
    }
    str[i] = '\0';

    return str;
}

__int128 atoi128(char *s)
{
    while (isspace(*s))
        s++;

    int sign = 1;
    if (*s == '-')
    {
        sign = -1;
        s++;
    }
    else if (*s == '+')
        s++;

    __int128 result = 0;

    while (*s)
    {
        if (*s < '0' || *s > '9')
            break;

        int digit = *s - '0';
        result = result * 10 + digit;

        s++;
    }

    return sign * result;
}

// parse number (int or float)
inline Value *parse_number(Src *s)
{
    skip_ws(s);
    int st = s->pos;
    _Bool seen_dot = 0;
    _Bool is_unsigned = 0;
    _Bool is_percent = 0;
    char base = 10;
    _Bool big = 0;
    if (src_peek(s) == '-')
    {
        src_get(s);
    }
    // either normal number or hex
    while (isdigit((unsigned char)src_peek(s)) || src_peek(s) == '.' ||
           src_peek(s) == 'x' || src_peek(s) == 'X' ||
           isxdigit((unsigned char)src_peek(s)) ||
           src_peek(s) == '~')
    {
        if (src_peek(s) == '.')
        {
            if (seen_dot || base == 16) // cant have hex literal have decimal points (that
                                        // would be cool though)
            {
                break;
            }
            seen_dot = 1;
        }
        else if (src_peek(s) == 'x' || src_peek(s) == 'X')
        {
            if (base == 16 || seen_dot)
            {
                break;
            }
            base = 16;
        }
        else if (src_peek(s) == '~')
        {
            if (big)
            {
                break;
            }
            big = 1;
        }
        src_get(s);
    }

    if (src_peek(s) == 'u' || src_peek(s) == 'U')
    {
        is_unsigned = 1;
        src_get(s);
    }
    if (src_peek(s) == '%')
    {
        is_percent = 1;
        src_get(s);
    }

    int en = s->pos;
    char tmp[MAX_NUMBER_DIGITS];
    uint64_t len = en - st;
    if (len <= 0)
        return NULL;

    if (len < (int)sizeof(tmp) && !big)
    {
        memcpy(tmp, s->src + st, len);
        tmp[len] = 0;
        if (seen_dot)
        {
            double f = atof(tmp);
            if (is_percent)
                f /= 100.0;
            return vfloat(f);
        }
        else
        {
            long i = strtol(tmp, NULL, base);
            if (is_percent)
                return vfloat((double)i / 100.0);
            return is_unsigned ? vuint(i > 0 ? i : -i) : vint(i);
        }
    }
    else
    {
        // implement arbritrarily sized integers in the future
        char *buf = mila_malloc(len + 1);
        memcpy(buf, s->src + st, len);
        buf[len] = 0;
        Value *r;
        if (seen_dot)
        {
            mila_float128_internal f = b_ff_from_string(buf);
            if (is_percent)
                f = b_ff_div(f, b_ff_from_double(100.0));
            r = vbfloat(f);
        }
        else
        {
            long tmp = atoi128(buf);
            if (is_percent)
                return vbfloat(b_ff_div(b_ff_from_i128(tmp), b_ff_from_double(100.0)));
            r = vbint(tmp);
        }
        mila_free(buf);
        return r;
    }
}

char *strip(char *str)
{
    char *s = str;
    int len = strlen(str);
    while (isspace(*s))
        s++;
    while (isspace(*(s + len - 1)))
        len--;
    char *final = (char *)malloc(sizeof(char) * len + 1);
    strncpy(final, s, len);
    return final;
}

char *dedent(char *str)
{
    if (str == NULL)
    {
        return NULL;
    }
    size_t len = strlen(str);
    if (len == 0)
    {
        return calloc(1, sizeof(char)); // Return empty string
    }
    // Find the minimum indentation (leading spaces/tabs on non-empty lines)
    size_t min_indent = SIZE_MAX;
    size_t i = 0;
    while (i < len)
    {
        size_t line_indent = 0;
        while (i < len && (str[i] == ' ' || str[i] == '\t'))
        {
            line_indent++;
            i++;
        }
        if (i < len && str[i] != '\n' && str[i] != '\r')
        {
            if (line_indent < min_indent)
            {
                min_indent = line_indent;
            }
        }
        while (i < len && str[i] != '\n')
        {
            i++;
        }
        if (i < len && str[i] == '\n')
        {
            i++;
        }
    }
    if (min_indent == SIZE_MAX)
    {
        min_indent = 0;
    }
    char *result = malloc(len + 1);
    if (result == NULL)
    {
        return NULL;
    }
    size_t result_pos = 0;
    i = 0;
    while (i < len)
    {
        size_t spaces_to_skip = 0;
        while (spaces_to_skip < min_indent && i < len &&
               (str[i] == ' ' || str[i] == '\t'))
        {
            spaces_to_skip++;
            i++;
        }
        while (i < len && str[i] != '\n')
        {
            result[result_pos++] = str[i++];
        }
        if (i < len && str[i] == '\n')
        {
            result[result_pos++] = str[i++];
        }
    }
    result[result_pos] = '\0';
    return result;
}

// parse string literal (double quotes)
inline Value *parse_string(Src *s)
{
    skip_ws(s);
    if (src_peek(s) != '"')
        return verror("String unterminated!");
    src_get(s); // consume opening "
    char do_dedent = 0;
    size_t cap = 256;
    size_t len = 0;
    char *buf = mila_malloc(cap);
    if (!buf)
        return verror("Allocation failure");
    while (!src_eof(s))
    {
        char b = 0;
        unsigned char c = src_get(s);
        if (c == '"')
            break;
        if (c == '\\')
        {
            char n = src_get(s);
            switch (n)
            {
            case 'n':
                c = '\n';
                break;
            case 't':
                c = '\t';
                break;
            case 'r':
                c = '\r';
                break;
            case 'f':
                c = '\f';
                break;
            case 'v':
                c = '\v';
                break;
            case 'a':
                c = '\a';
                break;
            case '\\':
                c = '\\';
                break;
            case 'N':
            {
                if (src_peek(s) == '{')
                {
                    src_get(s);
                    char text[MILA_N_ESCAPE_DIGITS + 1] = "";
                    int offset = 0;
                    while (src_peek(s) != '}')
                        text[offset++] = src_get(s);
                    src_get(s);
                    uint32_t cp = atoi(text);
                    if (!cp)
                    {
                        mila_free(buf);
                        return verror("NULL BYTE ISNT SUPPORTED.");
                    }
                    if (cp < 0x80)
                    {
                        c = (char)cp;
                    }
                    else if (cp < 0x800)
                    {
                        if (len + 2 >= cap)
                        {
                            cap = cap + (int)(cap * 0.3);
                            char *tmp = mila_realloc(buf, cap);
                            if (!tmp)
                            {
                                mila_free(buf);
                                return verror("Allocation failure!");
                            }
                            buf = tmp;
                        }
                        buf[len++] = 0xC0 | (cp >> 6);
                        buf[len++] = 0x80 | (cp & 0x3F);
                        continue;
                    }
                    else if (cp < 0x10000)
                    {
                        if (len + 3 >= cap)
                        {
                            cap = cap + (int)(cap * 0.3);
                            char *tmp = mila_realloc(buf, cap);
                            if (!tmp)
                            {
                                mila_free(buf);
                                return verror("Allocation failure!");
                            }
                            buf = tmp;
                        }
                        buf[len++] = 0xE0 | (cp >> 12);
                        buf[len++] = 0x80 | ((cp >> 6) & 0x3F);
                        buf[len++] = 0x80 | (cp & 0x3F);
                        continue;
                    }
                    else
                    {
                        if (len + 4 >= cap)
                        {
                            cap = cap + (int)(cap * 0.3);
                            char *tmp = mila_realloc(buf, cap);
                            if (!tmp)
                            {
                                mila_free(buf);
                                return verror("Allocation failure!");
                            }
                            buf = tmp;
                        }
                        buf[len++] = 0xF0 | (cp >> 18);
                        buf[len++] = 0x80 | ((cp >> 12) & 0x3F);
                        buf[len++] = 0x80 | ((cp >> 6) & 0x3F);
                        buf[len++] = 0x80 | (cp & 0x3F);
                        continue;
                    }
                }
                else
                {
                    char code[3] = {src_get(s), src_get(s), 0};
                    c = atoi(code);
                    if (!c)
                    {
                        mila_free(buf);
                        return verror("NULL BYTE ISNT SUPPORTED.");
                    }
                }
            }
            break;
            case 'x':
            { // \xhh (hex) escape codes
                char code[3] = {src_get(s), src_get(s), 0};
                c = strtol(code, NULL, 16);
                if (!c)
                {
                    mila_free(buf);
                    return verror("NULL BYTE ISNT SUPPORTED.");
                }
            }
            break;
            case 'u':
            { // \uXXXX (16-bit unicode)
                char code[5] = {src_get(s), src_get(s), src_get(s), src_get(s), 0};
                uint32_t cp = strtol(code, NULL, 16);
                if (!cp)
                {
                    mila_free(buf);
                    return verror("NULL BYTE ISNT SUPPORTED.");
                }
                if (cp < 0x80)
                {
                    c = cp;
                }
                else if (cp < 0x800)
                {
                    if (len + 2 >= cap)
                    {
                        cap = cap + (int)(cap * 0.3);
                        char *tmp = mila_realloc(buf, cap);
                        if (!tmp)
                        {
                            mila_free(buf);
                            return verror("Allocation failure!");
                        }
                        buf = tmp;
                    }
                    buf[len++] = 0xC0 | (cp >> 6);
                    buf[len++] = 0x80 | (cp & 0x3F);
                    c = 0; // mark as already added
                }
                else
                {
                    if (len + 3 >= cap)
                    {
                        cap = cap + (int)(cap * 0.3);
                        char *tmp = mila_realloc(buf, cap);
                        if (!tmp)
                        {
                            mila_free(buf);
                            return verror("Allocation failure!");
                        }
                        buf = tmp;
                    }
                    buf[len++] = 0xE0 | (cp >> 12);
                    buf[len++] = 0x80 | ((cp >> 6) & 0x3F);
                    buf[len++] = 0x80 | (cp & 0x3F);
                    c = 0;
                }
            }
            break;
            case 'U':
            { // \UXXXXXXXX (32-bit unicode)
                char code[9] = {src_get(s), src_get(s), src_get(s), src_get(s),
                                src_get(s), src_get(s), src_get(s), src_get(s), 0};
                uint32_t cp = strtol(code, NULL, 16);
                if (!cp)
                {
                    mila_free(buf);
                    return verror("NULL BYTE ISNT SUPPORTED.");
                }
                if (cp < 0x80)
                {
                    c = cp;
                }
                else if (cp < 0x800)
                {
                    if (len + 2 >= cap)
                    {
                        cap = cap + (int)(cap * 0.3);
                        char *tmp = mila_realloc(buf, cap);
                        if (!tmp)
                        {
                            mila_free(buf);
                            return verror("Allocation failure!");
                        }
                        buf = tmp;
                    }
                    buf[len++] = 0xC0 | (cp >> 6);
                    buf[len++] = 0x80 | (cp & 0x3F);
                    c = 0;
                }
                else if (cp < 0x10000)
                {
                    if (len + 3 >= cap)
                    {
                        cap = cap + (int)(cap * 0.3);
                        char *tmp = mila_realloc(buf, cap);
                        if (!tmp)
                        {
                            mila_free(buf);
                            return verror("Allocation failure!");
                        }
                        buf = tmp;
                    }
                    buf[len++] = 0xE0 | (cp >> 12);
                    buf[len++] = 0x80 | ((cp >> 6) & 0x3F);
                    buf[len++] = 0x80 | (cp & 0x3F);
                    c = 0;
                }
                else
                {
                    if (len + 4 >= cap)
                    {
                        cap = cap + (int)(cap * 0.3);
                        char *tmp = mila_realloc(buf, cap);
                        if (!tmp)
                        {
                            mila_free(buf);
                            return verror("Allocation failure!");
                        }
                        buf = tmp;
                    }
                    buf[len++] = 0xF0 | (cp >> 18);
                    buf[len++] = 0x80 | ((cp >> 12) & 0x3F);
                    buf[len++] = 0x80 | ((cp >> 6) & 0x3F);
                    buf[len++] = 0x80 | (cp & 0x3F);
                    c = 0;
                }
            }
            break;
            case '0':
            { // \0oo (octal) escape codes
                char code[3] = {src_get(s), src_get(s), 0};
                c = strtol(code, NULL, 8);
                if (!c)
                {
                    mila_free(buf);
                    return verror("NULL BYTE ISNT SUPPORTED.");
                }
            }
            break;
            case '\n':
            {
                c = src_get(s);
                if (c == '"')
                    b = 1;
            }
            break;
            default:
                c = n;
                break;
            }
        }
        if (b)
            break;
        // Grow buffer if needed
        if (len + 1 >= cap)
        {
            cap = cap + (int)(cap * 0.3); // grow by 30%
            char *tmp = mila_realloc(buf, cap);
            if (!tmp)
            {
                mila_free(buf);
                return verror("Allocation failure!");
            }
            buf = tmp;
        }
        if (c)
            buf[len++] = (unsigned char)c;
    }
    if (src_peek(s) == '!')
    {
        src_get(s);
        do_dedent = 1;
    }
    // shrink to exact size
    char *res = mila_realloc(buf, len + 1);
    if (!res)
        res = buf; // if mila_realloc fails, keep original buffer
    res[len] = '\0';
    Value *str = vstring_take(!do_dedent ? res : dedent(res));
    if (do_dedent)
        free(res);
    return str;
}

void src_advance_by(Src *s, size_t amount) { s->pos += amount; }

// parse function literal: fn(a,b){ ... }
inline int is_keyword_at(Src *s, const char *kw)
{
    skip_ws(s);
    int p = s->pos;
    size_t klen = strlen(kw);
    if (p + klen > s->len)
        return 0;
    if (strncmp(s->src + p, kw, klen) == 0)
    {
        char after = s->src[p + klen];
        if (after != '_' &&
            (after == '\0' || isspace((unsigned char)after) || (!isalnum(after))))
            return 1;
    }
    return 0;
}

char *dup_substr(Src *s, int a, int b)
{
    int n = b - a;
    char *r = mila_malloc(n + 1);
    memcpy(r, s->src + a, n);
    r[n] = 0;
    return r;
}

// parse comma-separated identifiers (for parameters)
FunctionParameters *parse_param_list(Src *s)
{
    skip_ws(s);
    if (!match_char(s, '('))
    {
        FunctionParameters *fnp = (FunctionParameters *)mila_malloc(sizeof(FunctionParameters));
        char **p = mila_malloc(sizeof(char *));
        char **d = mila_malloc(sizeof(char *));
        p[0] = NULL;
        d[0] = NULL;
        fnp->params = p;
        fnp->defaults = d;
        return fnp;
    }
    skip_ws(s);
    // empty
    FunctionParameters *fnp = (FunctionParameters *)mila_malloc(sizeof(FunctionParameters));
    char *buffer = NULL;
    fnp->count = 0;
    if (match_char(s, ')'))
    {
        char **p = mila_malloc(sizeof(char *));
        char **d = mila_malloc(sizeof(char *));
        p[0] = NULL;
        d[0] = NULL;
        fnp->params = p;
        fnp->defaults = d;
        return fnp;
    }
    // read identifiers
    char **arr = NULL;
    char **def = NULL;
    int cnt = 0;
    for (;;)
    {
        char *id = parse_ident(s);
        if (!id)
        {
            // error
            // cleanup
            for (int i = 0; i < cnt; i++)
                mila_free(arr[i]);
            mila_free(arr);
            for (int i = 0; i < cnt; i++)
                mila_free(def[i]);
            mila_free(def);
            return NULL;
        }
        if (match_char(s, ':'))
        {
            Value *type = parse_string(s);
            // ignore types for now
            val_kill(type);
        }
        if (match_char(s, '='))
        {
            size_t old_pos = s->pos;
            skip_parse_expr(s);
            size_t new_pos = s->pos;
            size_t len = new_pos - old_pos + 1;
            buffer = (char *)mila_malloc(sizeof(char) * len);
            strncpy(buffer, s->src + old_pos, len);
            buffer[len - 1] = '\0';
        }
        else
            buffer = NULL;
        arr = mila_realloc(arr, sizeof(char *) * (cnt + 2));
        def = mila_realloc(def, sizeof(char *) * (cnt + 2));
        arr[cnt++] = id;
        def[cnt - 1] = buffer;
        arr[cnt] = NULL;
        def[cnt] = NULL;
        if (match_char(s, ','))
            continue;
        if (match_char(s, ')'))
            break;
        break;
    }
    fnp->params = arr;
    fnp->defaults = def;
    fnp->count = cnt;
    return fnp;
}

char **parse_context_list(Src *s)
{
    skip_ws(s);
    if (!match_char(s, '['))
        return NULL;
    skip_ws(s);
    // empty
    if (match_char(s, ']'))
    {
        char **p = mila_malloc(sizeof(char *));
        p[0] = NULL;
        return p;
    }
    // read identifiers
    char **arr = NULL;
    int cnt = 0;
    for (;;)
    {
        char *id = parse_ident(s);
        if (!id)
        {
            // error
            // cleanup
            for (int i = 0; i < cnt; i++)
                mila_free(arr[i]);
            mila_free(arr);
            return NULL;
        }
        arr = mila_realloc(arr, sizeof(char *) * (cnt + 2));
        arr[cnt++] = id;
        arr[cnt] = NULL;
        if (match_char(s, ','))
            continue;
        if (match_char(s, ']'))
            break;
        // error -> break
        break;
    }
    return arr;
}

Value *parse_subscript(Src *s, Env *e)
{
    if (!match_char(s, '['))
    {
        return verror("Subscript was expected!");
    }
    Value *res = eval_expr(s, e);
    if (!match_char(s, ']'))
    {
        return verror("Closing square bracket was expected!");
    }

    return res;
}

// parse block: {...}
Value *eval_block(Src *s, Env *env)
{
    if (!match_char(s, '{'))
    {
        return verror("Expected a block!");
    }
    // new local frame
    Env *frame = env_new(env);
    Value *last = vnull(); // last expression value
    while (1)
    {
        skip_ws(s);
        if (src_peek(s) == '\0')
            break;
        if (match_char(s, '}'))
            break;
        val_release(last);
        Value *st = eval_statement(s, frame);
        last = st;

        if (IS_ERROR(st))
        {
            env_free(frame);
            return st;
        }
        if (IS_CONTROL(st))
        {
            env_free(frame);
            if (st->type == T_RETURN)
                return st;
            HANDLE_CONTROL(st);
        }
    }
    env_free(frame);
    return last;
}

Value *eval_block_raw(Src *s, Env *frame)
{
    if (!match_char(s, '{'))
    {
        return verror("Block was expected!");
    }
    // new local frame
    Value *last = vnull(); // last expression value
    while (1)
    {
        skip_ws(s);
        if (src_peek(s) == '\0')
            break;
        if (match_char(s, '}'))
            break;
        Value *st = eval_statement(s, frame);
        val_release(last);
        last = st;

        if (IS_ERROR(st))
        {
            return st;
        }
        if (IS_CONTROL(st))
        {
            if (st->type == T_RETURN)
                return st;
            HANDLE_CONTROL(st);
        }
    }
    return last;
}

// now we need eval_primary, function call, member? only call expressions and
// binary ops evaluate primary

Value *call_function_with(Env *env, Value *fnval, Value *first, ...)
{
    va_list ap;
    size_t count = 0;

    /* First pass: count */
    va_start(ap, first);
    for (Value *v = first; v != NULL; v = va_arg(ap, Value *))
    {
        count++;
    }
    va_end(ap);

    /* Allocate array (+1 if you want NULL terminator preserved) */
    Value **args = mila_malloc((count + 1) * sizeof(Value *));
    if (!args)
        return NULL;

    /* Second pass: fill */
    va_start(ap, first);
    size_t i = 0;
    for (Value *v = first; v != NULL; v = va_arg(ap, Value *))
    {
        args[i++] = v;
    }
    va_end(ap);

    if (!fnval)
    {
        for (size_t i = 0; i < count; i++)
            val_release(args[i]);
        free(args);
        return verror("Function is NULL!");
    }

    Value *res = call_function(fnval, env, count, args);
    for (size_t i = 0; i < count; ++i)
        val_release(args[i]);
    mila_free(args);
    HANDLE_RETURN(res);
    return res;
}

Value *call_native_with(Env *env, NativeFn fnval, Value *first, ...)
{
    va_list ap;
    size_t count = 0;

    /* First pass: count */
    va_start(ap, first);
    for (Value *v = first; v != NULL; v = va_arg(ap, Value *))
    {
        count++;
    }
    va_end(ap);

    /* Allocate array (+1 if you want NULL terminator preserved) */
    Value **args = mila_malloc((count + 1) * sizeof(Value *));
    if (!args)
        return NULL;

    /* Second pass: fill */
    va_start(ap, first);
    size_t i = 0;
    for (Value *v = first; v != NULL; v = va_arg(ap, Value *))
    {
        args[i++] = v;
    }
    va_end(ap);

    if (!fnval)
    {
        for (size_t i = 0; i < count; i++)
            val_release(args[i]);
        free(args);
        return verror("Function is NULL!");
    }

    Value *res = fnval(env, count, args);
    for (size_t i = 0; i < count; ++i)
        val_release(args[i]);
    mila_free(args);
    HANDLE_RETURN(res);
    return res;
}

Value *call_function_str(Env *env, const char *fnname, Value *first, ...)
{
    va_list ap;
    size_t count = 0;

    /* First pass: count */
    va_start(ap, first);
    for (Value *v = first; v != NULL; v = va_arg(ap, Value *))
    {
        count++;
    }
    va_end(ap);

    /* Allocate array (+1 if you want NULL terminator preserved) */
    Value **args = mila_malloc((count + 1) * sizeof(Value *));
    if (!args)
        return NULL;

    /* Second pass: fill */
    va_start(ap, first);
    size_t i = 0;
    for (Value *v = first; v != NULL; v = va_arg(ap, Value *))
    {
        args[i++] = v;
    }
    va_end(ap);

    Value *fnval = env_get(env, fnname);
    if (!fnval)
    {
        for (size_t i = 0; i < count; i++)
            val_release(args[i]);
        free(args);
        return verror("Function %s does not exist!", fnname);
    }

    Value *res = call_function(fnval, env, count, args);
    for (size_t i = 0; i < count; ++i)
        val_release(args[i]);
    mila_free(args);
    HANDLE_RETURN(res);
    return res;
}

Value **make_args(Value *first, ...)
{
    va_list ap;
    size_t count = 0;

    /* First pass: count */
    va_start(ap, first);
    for (Value *v = first; v != NULL; v = va_arg(ap, Value *))
    {
        count++;
    }
    va_end(ap);

    Value **args = mila_malloc((count) * sizeof(Value *));
    if (!args)
        return NULL;

    /* Second pass: fill */
    va_start(ap, first);
    size_t i = 0;
    for (Value *v = first; v != NULL; v = va_arg(ap, Value *))
    {
        args[i++] = v;
    }
    va_end(ap);
    return args;
}

Value *call_function(Value *fnval, Env *env, int argc, Value **argv)
{
    if (!fnval)
        return verror("Function is NULL!");
    if (fnval->type == T_NATIVE)
    {
        for (int t = 0; t < argc; ++t)
            if (GET_TYPE(argv[t]) == T_ERROR)
                return argv[t];
        Value *result = GET_NATIVE(fnval)->fn(env, argc, argv);
        return result;
    }
    else if (fnval->type == T_FUNCTION)
    {
        // create new environment with closure as parent
        Env *frame = NULL;
        if (GET_FUNCTION(fnval)->closure)
        {
            GET_FUNCTION(fnval)->closure->parent = env;
            frame = env_new(GET_FUNCTION(fnval)->closure);
        }
        else
        {
            frame = env_new(env);
        }
        // bind params
        char **p = GET_FUNCTION(fnval)->params;
        int i = 0;
        for (; p && p[i]; ++i)
        {
            // if fewer args provided, bind null
            if (i < argc && GET_TYPE(argv[i]) == T_ERROR)
            {
                return argv[i];
            }
            Value *a = (i < argc) ? argv[i] : NULL;
            if (a == NULL)
            {
                for (size_t j = argc; GET_FUNCTION(fnval)->defaults[j]; ++j)
                {
                    env_set_raw(frame, strncmp("...", p[j], 3) != 0 ? p[j] : p[j] + 3, eval_str(GET_FUNCTION(fnval)->defaults[j], frame));
                }
                i++;
                break;
            }
            if (strncmp("...", p[i], 3) == 0)
            {
                Value *list = call_function_str(env, "list", NULL);
                env_set_local_raw(frame, p[i] + 3, list);
                val_release(call_function_str(env, "list.append", val_retain(list), val_retain(a), NULL));
                for (i++; i < argc; ++i)
                {
                    val_release(call_function_str(env, "list.append", val_retain(list), val_retain(argv[i]), NULL));
                }
                break;
            }
            else
            {
                env_set_local(frame, p[i], a);
            }
        }
        i--;
        // cursed
        int limit = 0;
        for (int meep = 0; GET_FUNCTION(fnval)->params[meep]; ++meep)
            limit++;
        limit--;
        for (int j = i; j < limit; ++j)
        {
            if (strncmp("...", p[i], 3) == 0)
                env_set_local_raw(frame, p[i] + 3, vnull());
            else
                env_set_local_raw(frame, p[i], vnull());
        }
        // set contextual values
        p = GET_FUNCTION(fnval)->contextuals;
        i = 0;
        for (; p && p[i]; ++i)
        {
            char *name = mila_strdup(p[i]);
            int is_optional = 0;
            if (name[strlen(name) - 1] == '?')
            {
                is_optional = 1;
                name[strlen(name) - 1] = 0;
            }
            Value *a = NULL;
            if (strncmp("@env:", name, 5) == 0)
            {
                a = vopaque_extra(env, NULL, ML("environment"));
                char *new_name = mila_strdup(name + 5);
                free(name);
                env_set_local_raw(frame, new_name, a);
                free(new_name);
                continue;
            }
            else
                a = env_get_contextual(env, name);

            if (!is_optional && a == NULL)
            {
                env_free(frame);
                Value *res =
                    verror("Function %s requires the contextual value `%s`",
                           GET_FUNCTION(fnval)->name ? GET_FUNCTION(fnval)->name : "[lambda]", name);
                mila_free(name);
                return res;
            }
            else if (a)
                env_set_local(frame, name, a);
            mila_free(name);
        }
        // Evaluate body: note body_src contains the body text e.g., "{ ... }"
        Src *child = src_new(GET_FUNCTION(fnval)->body_src);
        // position should start at 0 for the body; body is a block (starts with
        // '{') Evaluate block using the new frame
        Value *res = eval_source(child, frame);
        src_free(child);
        env_free(frame);
        HANDLE_CONTROL(res);
        return res;
    }
    else
    {
        // not callable
        return verror("Attempt to call non-callable value.");
    }
    return vnull();
}

// parse expression with precedence:
// We'll implement: primary, unary, multiplicative(*,/), additive(+,-),
// comparison (<,>,<=,>=,==,!=), logical (&&,||)

// parse primary: numbers, strings, identifiers (variables or function calls),
// parentheses, function literal
Value *eval_primary(Src *s, Env *env)
{
    skip_ws(s);
    char c = src_peek(s);
    if (c == '\0')
        return vnull();
    // number
    if (isdigit((unsigned char)c) ||
        ((c == '+' || c == '-') && isdigit((unsigned char)s->src[s->pos + 1])) ||
        (c == '0' && (s->src[s->pos + 1] == 'x' || s->src[s->pos + 1] == 'X') &&
         isxdigit((unsigned char)s->src[s->pos + 2])))
    {
        return parse_number(s);
    }
    // string
    if (c == '"')
    {
        return parse_string(s);
    }
    if (c == '[')
    {
        src_get(s);
        char is_dict = match_char(s, '@');
        size_t start = s->pos;
        Value **args = NULL;
        Value *list = call_function_str(env, "list", NULL);
        int argc = 0;
        skip_ws(s);
        if (src_peek(s) != ']')
        {
            for (;;)
            {
                Value *a = eval_expr(s, env);
                if (IS_ERROR(a))
                {
                    mila_free(args);
                    val_release(list);
                    return a;
                }
                args = mila_realloc(args, sizeof(Value *) * (argc + 1));
                args[argc++] = a;
                val_release(call_function_str(env, "list.append", val_retain(list), a, NULL));
                skip_ws(s);
                if (match_char(s, ','))
                    continue;
                if (match_char(s, '='))
                    continue;
                if (match_char(s, ']'))
                    break;
                val_release(list);
                mila_free(args);
                int k = 1;
                while (k)
                {
                    if (src_peek(s) == '[')
                        k++;
                    if (src_peek(s) == ']')
                        k--;
                    s->pos++;
                }
                size_t end = s->pos;
                int len = end - start + 1;
                return vtagged_error(
                    E_SYNTAX_ERROR,
                    "Expected a %s or closing bracket!\nAt list `%.*s`",
                    is_dict && argc % 2 ? "colon" : "comma",
                    len, s->src + start - 1);
            }
        }
        else
        {
            src_get(s);
        }
        if (is_dict)
        {
            Value *dict = native_new_dict(env, argc, args);
            val_release(list);
            free(args);
            return dict;
        }
        free(args);
        return list;
    }
    // parentheses
    if (c == '(')
    {
        size_t start = s->pos;
        src_get(s); // consume '('
        Value *expr = eval_expr(s, env);
        skip_ws(s);
        if (src_peek(s) == ')')
            src_get(s);

        if (src_peek(s) == '(')
        {
            // parse args
            src_get(s); // consume '('
            // parse comma separated expressions
            Value **args = NULL;
            int argc = 0;
            skip_ws(s);
            if (src_peek(s) != ')')
            {
                for (;;)
                {
                    Value *a = eval_expr(s, env);
                    if (IS_ERROR(a))
                    {
                        val_release(expr);
                        for (int i = 0; i < argc; i++)
                            val_release(args[i]);
                        mila_free(args);
                        return a;
                    }
                    args = mila_realloc(args, sizeof(Value *) * (argc + 1));
                    args[argc++] = a;
                    if (match_char(s, ','))
                        continue;
                    if (match_char(s, ')'))
                        break;
                    val_release(expr);
                    for (int i = 0; i < argc; i++)
                        val_release(args[i]);
                    mila_free(args);
                    int k = 1;
                    while (k)
                    {
                        if (src_peek(s) == '(')
                            k++;
                        if (src_peek(s) == ')')
                            k--;
                        s->pos++;
                    }
                    size_t end = s->pos;
                    int len = end - start + 1;
                    return vtagged_error(
                        E_SYNTAX_ERROR,
                        "Expected a comma or closing parenthesis!\nAt call `%.*s`", len,
                        s->src + start);
                }
            }
            else
            {
                // empty
                src_get(s); // consume ')'
            }
            // callp
            Value *res = call_function(expr, env, argc, args);
            for (int i = 0; i < argc; i++)
                val_release(args[i]);
            mila_free(args);
            val_release(expr);
            if (GET_TYPE(res) == T_RETURN)
            {
                Value *tmp = (Value *)res->v;
                val_release(res);
                return tmp;
            }
            HANDLE_CONTROL(res);
            return res;
        }
        else if (src_peek(s) == '[')
        {
            Value *obj = expr;

            if (!obj)
            {
                Value *ret = verror("cannot be subscripted as it is cnull");
                return ret;
            }

            // Handle chained subscripts: (expr)[x][y][z]
            while (src_peek(s) == '[')
            {
                Value *index = parse_subscript(s, env);

                if (!obj)
                {
                    val_release(index);
                    Value *ret = verror("cannot be subscripted as it is cnull");
                    return ret;
                }

                if (GET_METHOD(obj, BMethodGetItem))
                {
                    Value *res =
                        ((binary_method)GET_METHOD(obj, BMethodGetItem))(obj, index);
                    val_release(index);
                    Value *tmp = val_retain(res);
                    val_release(obj);
                    obj = tmp;
                }
                else
                {
                    val_release(index);
                    val_release(obj);
                    return verror("Type %s does not support BMethodGetItem!",
                                  GET_TYPENAME(obj));
                }
            }

            return obj;
        }
        else if (src_peek(s) == ':')
        {
            src_get(s); // skip the colon
            if (src_peek(s) == ':') {
                src_get(s);
                char *method = parse_ident(s);
    
                Value *obj = expr;
                Value *attr = vstring_take(method);
                Value *function = NULL;
    
                if (!obj)
                {
                    val_release(attr);
                    Value *ret = verror("cannot be subscripted as it is cnull");
                    return ret;
                }
    
                if (obj->method_table && obj->method_table[BMethodGetItem])
                {
                    function =
                        ((binary_method)obj->method_table[BMethodGetItem])(obj, attr);
                    val_release(attr);
                    if (!function)
                    {
                        Value *err =
                            verror("Attribute %s didnt exist in expression", GET_STRING(attr));
                        return err;
                    }
                }
                else
                {
                    val_release(attr);
                    return verror("Type %s does not support BMethodGetItem!",
                                  GET_TYPENAME(obj));
                }
    
                if (src_peek(s) == '(')
                {
                    // parse args
                    src_get(s); // consume '('
                    // parse comma separated expressions
                    Value **args = NULL;
                    int argc = 0;
                    skip_ws(s);
    
                    // handle (value)(...) calls
                    if (src_peek(s) != ')')
                    {
                        for (;;)
                        {
                            Value *a = eval_expr(s, env);
                            if (IS_ERROR(a))
                            {
                                for (int i = 0; i < argc; i++)
                                    val_release(args[i]);
                                mila_free(args);
                                return a;
                            }
                            args = mila_realloc(args, sizeof(Value *) * (argc + 1));
                            args[argc++] = a;
                            if (match_char(s, ','))
                                continue;
                            if (match_char(s, ')'))
                                break;
                            for (int i = 0; i < argc; i++)
                                val_release(args[i]);
                            mila_free(args);
                            int k = 1;
                            while (k)
                            {
                                if (src_peek(s) == '(')
                                    k++;
                                if (src_peek(s) == ')')
                                    k--;
                                s->pos++;
                            }
                            size_t end = s->pos;
                            int len = end - start + 1;
                            return vtagged_error(
                                E_SYNTAX_ERROR,
                                "Expected a comma or closing parenthesis!\nAt call `%.*s`", len,
                                s->src + start);
                        }
                    }
                    else
                    {
                        // empty
                        src_get(s); // consume ')'
                    }
                    // callp
                    Value *res = call_function(function, env, argc, args);
                    for (int i = 0; i < argc; i++)
                        val_release(args[i]);
                    mila_free(args);
                    HANDLE_RETURN(res);
                    return res;
                }
                else
                {
                    return obj;
                }
            } else {
                char *method = parse_ident(s);
    
                Value *obj = expr;
                Value *attr = vstring_take(method);
                Value *function = NULL;
    
                if (!obj)
                {
                    val_release(attr);
                    Value *ret = verror("cannot be subscripted as it is cnull");
                    return ret;
                }
    
                if (obj->method_table && obj->method_table[BMethodGetItem])
                {
                    function =
                        ((binary_method)obj->method_table[BMethodGetItem])(obj, attr);
                    val_release(attr);
                    if (!function)
                    {
                        Value *err =
                            verror("Attribute %s didnt exist in expression", GET_STRING(attr));
                        return err;
                    }
                }
                else
                {
                    val_release(attr);
                    return verror("Type %s does not support BMethodGetItem!",
                                  GET_TYPENAME(obj));
                }
    
                if (src_peek(s) == '(')
                {
                    // parse args
                    src_get(s); // consume '('
                    // parse comma separated expressions
                    Value **args = mila_malloc(sizeof(Value *));
                    args[0] = val_retain(obj);
                    int argc = 1;
                    skip_ws(s);
    
                    // handle (value)(...) calls
                    if (src_peek(s) != ')')
                    {
                        for (;;)
                        {
                            Value *a = eval_expr(s, env);
                            if (IS_ERROR(a))
                            {
                                for (int i = 0; i < argc; i++)
                                    val_release(args[i]);
                                mila_free(args);
                                return a;
                            }
                            args = mila_realloc(args, sizeof(Value *) * (argc + 1));
                            args[argc++] = a;
                            if (match_char(s, ','))
                                continue;
                            if (match_char(s, ')'))
                                break;
                            for (int i = 0; i < argc; i++)
                                val_release(args[i]);
                            mila_free(args);
                            int k = 1;
                            while (k)
                            {
                                if (src_peek(s) == '(')
                                    k++;
                                if (src_peek(s) == ')')
                                    k--;
                                s->pos++;
                            }
                            size_t end = s->pos;
                            int len = end - start + 1;
                            return vtagged_error(
                                E_SYNTAX_ERROR,
                                "Expected a comma or closing parenthesis!\nAt call `%.*s`", len,
                                s->src + start);
                        }
                    }
                    else
                    {
                        // empty
                        src_get(s); // consume ')'
                    }
                    // callp
                    Value *res = call_function(function, env, argc, args);
                    for (int i = 0; i < argc; i++)
                        val_release(args[i]);
                    mila_free(args);
                    HANDLE_RETURN(res);
                    return res;
                }
                else
                {
                    return obj;
                }
            }
        }
        return expr;
    }
    if (c == '{')
    {
        Value *v = eval_block(s, env);
        match_char(s, '}');
        HANDLE_RETURN(v);
        return v;
    }
    if (c == '!' && s->src[s->pos + 1] == '{')
    {
        src_get(s); // consume '!'
        size_t start = s->pos + 1;
        skip_block(s);
        size_t end = s->pos - 1; // avoid the closing }
        char *buffer = (char *)mila_malloc(sizeof(char) * (end - start) + 1);
        memcpy(buffer, s->src + start, end - start);
        Value *res = vstring_take(dedent(buffer));
        free(buffer);
        return res;
    }
    // function literal
    if (is_keyword_at(s, "fn"))
    {
        // consume keyword
        s->pos += strlen("fn");
        // parse params
        FunctionParameters *params = parse_param_list(s);
        char **contextuals = parse_context_list(s);
        char **names;
        Env *closure = env_new(NULL);
        if (match_char(s, ':'))
        {
            // parse closure bindings
            names = parse_context_list(s); // reused parse_context_list
            for (int i = 0; names[i]; ++i)
            {
                if (strlen(names[i]) > 5 && strncmp("@env:", names[i], 5) == 0)
                {
                    Env *new_env = env_new(NULL);
                    env_copy(new_env, env);
                    env_set_local_raw(closure, names[i] + 5, vopaque_extra(new_env, NULL, ML("environment")));
                }
                else
                    env_set_local(closure, names[i], env_get(env, names[i]));
                mila_free(names[i]);
            }
            mila_free(names);
        }
        if (is_keyword_at(s, "->"))
        {
            s->pos += 2;
            skip_ws(s);
            if (src_peek(s) == '"')
            {
                Value *ret_type = parse_string(s);
                // ignore return type for now
                val_kill(ret_type);
            }
            else
            {
                env_free(closure);
                for (int i = 0; params->params[i]; ++i)
                {
                    mila_free(params->params[i]);
                    mila_free(params->defaults[i]);
                }
                mila_free(params);
                return vtagged_error(E_SYNTAX_ERROR,
                                     "Expected a string literal for the return type.");
            }
        }
        skip_ws(s);
        // body is block; extract substring from '{' to matching '}'
        size_t start = s->pos;
        size_t i = s->pos;
        if (src_peek(s) == '{')
        {
            int depth = 0;
            // find matching brace (we will copy out body)
            for (; i < s->len; ++i)
            {
                char ch = s->src[i];
                if (ch == '{')
                    depth++;
                else if (ch == '}')
                {
                    depth--;
                    if (depth == 0)
                    {
                        i++;
                        break;
                    }
                }
                else if (ch == '"')
                {
                    // skip string literal
                    i++;
                    while (i < s->len && s->src[i] != '"')
                    {
                        if (s->src[i] == '\\' && i + 1 < s->len)
                            i += 2;
                        else
                            i++;
                    }
                }
            }
        }
        else
        {
            skip_parse_statement(s);
            i = s->pos;
        }
        if (i > s->len)
            i = s->len;
        int blen = i - start;
        char *body = mila_malloc(blen + 1);
        memcpy(body, s->src + start, blen);
        body[blen] = 0;
        s->pos = i;
        // create function value with closure get_line_pos(s) current env
        Value *fn = vfunction(params->params, params->defaults, contextuals, closure, body);
        free(params);
        GET_FUNCTION(fn)->name = mila_strdup("[lambda]");
        return fn;
    }
    // identifier or keyword like 'null', 'true', 'false', or bare native name
    if (is_ident_start(c))
    {
        size_t start = s->pos;
        char *id = parse_ident(s);
        if (!id)
            return vnull();
        // keywords
        if (strcmp(id, "null") == 0)
        {
            mila_free(id);
            return vnull();
        }
        if (strcmp(id, "none") == 0)
        {
            mila_free(id);
            return vnone();
        }
        if (strcmp(id, "true") == 0)
        {
            mila_free(id);
            return vbool(1);
        }
        if (strcmp(id, "false") == 0)
        {
            mila_free(id);
            return vbool(0);
        }
        if (strcmp(id, "break") == 0)
        {
            mila_free(id);
            return vbreak();
        }
        if (strcmp(id, "continue") == 0)
        {
            mila_free(id);
            return vcontinue();
        }
        // look ahead: function call? subscript?
        skip_ws(s);
        if (src_peek(s) == '(')
        {
            // parse args
            src_get(s); // consume '('
            // parse comma separated expressions
            Value **args = NULL;
            int argc = 0;
            skip_ws(s);

            // handle (value)(...) calls
            if (src_peek(s) != ')')
            {
                for (;;)
                {
                    Value *a = eval_expr(s, env);
                    if (IS_ERROR(a))
                    {
                        mila_free(id);
                        for (int i = 0; i < argc; i++)
                            val_release(args[i]);
                        mila_free(args);
                        return a;
                    }
                    args = mila_realloc(args, sizeof(Value *) * (argc + 1));
                    args[argc++] = a;
                    if (match_char(s, ','))
                        continue;
                    if (match_char(s, ')'))
                        break;
                    mila_free(id);
                    for (int i = 0; i < argc; i++)
                        val_release(args[i]);
                    mila_free(args);

                    int k = 1;
                    while (k)
                    {
                        if (src_peek(s) == '(')
                            k++;
                        if (src_peek(s) == ')')
                            k--;
                        s->pos++;
                    }
                    size_t end = s->pos;
                    int len = end - start + 1;
                    return vtagged_error(
                        E_SYNTAX_ERROR,
                        "Expected a comma or closing parenthesis!\nAt call `%.*s`", len,
                        s->src + start);
                }
            }
            else
            {
                // empty
                src_get(s); // consume ')'
            }
            // get callee (id)
            Value *callee = env_get(env, id);
            if (!callee)
            {
                Value *res = verror("Undefined function '%s'", id);
                mila_free(id);
                // release args
                for (int i = 0; i < argc; i++)
                    val_release(args[i]);
                mila_free(args);
                return res;
            }
            mila_free(id);
            // callp
#ifdef MILA_DEBUG
            printf("  ?? Call to %s\n", callee->v->native->name);
#endif
            Value *res = call_function(callee, env, argc, args);
            for (int i = 0; i < argc; i++)
                val_release(args[i]);
            mila_free(args);
            HANDLE_RETURN(res);
            return res;
        }
        else if (src_peek(s) == '[')
        {
            Value *obj = env_get(env, id);
            if (!obj)
            {
                Value *ret = verror("%s cannot be subscripted as it is cnull", id);
                mila_free(id);
                return ret;
            }
            mila_free(id);

            // Handle chained subscripts: (expr)[x][y][z]
            while (src_peek(s) == '[')
            {
                Value *index = parse_subscript(s, env);

                if (!obj)
                {
                    val_release(index);
                    Value *ret = verror("cannot be subscripted as it is cnull");
                    return ret;
                }

                if (obj->method_table && obj->method_table[BMethodGetItem])
                {
                    Value *res =
                        ((binary_method)obj->method_table[BMethodGetItem])(obj, index);
                    val_release(index);
                    Value *tmp = res;
                    // val_release(obj);
                    obj = tmp;
                }
                else
                {
                    val_release(index);
                    val_release(obj);
                    return verror("Type %s does not support BMethodGetItem!",
                                  GET_TYPENAME(obj));
                }
            }

            return val_retain(obj);
        }
        else if (src_peek(s) == ':')
        {
            src_get(s); // skip the colon
            if (src_peek(s) == ':') {
                src_get(s);
                char *method = parse_ident(s);
    
                Value *obj = env_get(env, id);
                Value *attr = vstring_take(method);
                Value *function = NULL;
    
                if (!obj)
                {
                    val_release(attr);
                    Value *ret = verror("cannot be subscripted as it is cnull");
                    mila_free(id);
                    return ret;
                }
    
                if (obj->method_table && obj->method_table[BMethodGetItem])
                {
                    function =
                        ((binary_method)obj->method_table[BMethodGetItem])(obj, attr);
                    if (!function)
                    {
                        Value *err = verror("Attribute %s didnt exist in value %s",
                                            GET_STRING(attr), id);
                        val_release(attr);
                        mila_free(id);
                        return err;
                    }
                    val_release(attr);
                }
                else
                {
                    mila_free(id);
                    val_release(attr);
                    return verror("Type %s does not support BMethodGetItem!",
                                  GET_TYPENAME(obj));
                }
    
                if (src_peek(s) == '(')
                {
                    // parse args
                    src_get(s); // consume '('
                    // parse comma separated expressions
                    Value **args = NULL;
                    int argc = 0;
                    skip_ws(s);
    
                    // handle (value)(...) calls
                    if (src_peek(s) != ')')
                    {
                        for (;;)
                        {
                            Value *a = eval_expr(s, env);
                            if (IS_ERROR(a))
                            {
                                mila_free(id);
                                for (int i = 0; i < argc; i++)
                                    val_release(args[i]);
                                mila_free(args);
                                return a;
                            }
                            args = mila_realloc(args, sizeof(Value *) * (argc + 1));
                            args[argc++] = a;
                            if (match_char(s, ','))
                                continue;
                            if (match_char(s, ')'))
                                break;
                            mila_free(id);
                            for (int i = 0; i < argc; i++)
                                val_release(args[i]);
                            mila_free(args);
                            int k = 1;
                            while (k)
                            {
                                if (src_peek(s) == '(')
                                    k++;
                                if (src_peek(s) == ')')
                                    k--;
                                s->pos++;
                            }
                            size_t end = s->pos;
                            int len = end - start + 1;
                            return vtagged_error(
                                E_SYNTAX_ERROR,
                                "Expected a comma or closing parenthesis!\nAt call `%.*s`", len,
                                s->src + start);
                        }
                    }
                    else
                    {
                        // empty
                        src_get(s); // consume ')'
                    }
                    mila_free(id);
                    // callp
                    Value *res = call_function(function, env, argc, args);
                    for (int i = 0; i < argc; i++)
                        val_release(args[i]);
                    mila_free(args);
                    HANDLE_RETURN(res);
                    return res;
                }
                else
                {
                    mila_free(id);
                    return val_retain(function);
                }
            } else {
                char *method = parse_ident(s);
    
                Value *obj = env_get(env, id);
                Value *attr = vstring_take(method);
                Value *function = NULL;
    
                if (!obj)
                {
                    val_release(attr);
                    Value *ret = verror("cannot be subscripted as it is cnull");
                    mila_free(id);
                    return ret;
                }
    
                if (obj->method_table && obj->method_table[BMethodGetItem])
                {
                    function =
                        ((binary_method)obj->method_table[BMethodGetItem])(obj, attr);
                    if (!function)
                    {
                        Value *err = verror("Attribute %s didnt exist in value %s",
                                            GET_STRING(attr), id);
                        val_release(attr);
                        mila_free(id);
                        return err;
                    }
                    val_release(attr);
                }
                else
                {
                    mila_free(id);
                    val_release(attr);
                    return verror("Type %s does not support BMethodGetItem!",
                                  GET_TYPENAME(obj));
                }
    
                if (src_peek(s) == '(')
                {
                    // parse args
                    src_get(s); // consume '('
                    // parse comma separated expressions
                    Value **args = mila_malloc(sizeof(Value *));
                    args[0] = val_retain(obj);
                    int argc = 1;
                    skip_ws(s);
    
                    // handle (value)(...) calls
                    if (src_peek(s) != ')')
                    {
                        for (;;)
                        {
                            Value *a = eval_expr(s, env);
                            if (IS_ERROR(a))
                            {
                                mila_free(id);
                                for (int i = 0; i < argc; i++)
                                    val_release(args[i]);
                                mila_free(args);
                                return a;
                            }
                            args = mila_realloc(args, sizeof(Value *) * (argc + 1));
                            args[argc++] = a;
                            if (match_char(s, ','))
                                continue;
                            if (match_char(s, ')'))
                                break;
                            mila_free(id);
                            for (int i = 0; i < argc; i++)
                                val_release(args[i]);
                            mila_free(args);
                            int k = 1;
                            while (k)
                            {
                                if (src_peek(s) == '(')
                                    k++;
                                if (src_peek(s) == ')')
                                    k--;
                                s->pos++;
                            }
                            size_t end = s->pos;
                            int len = end - start + 1;
                            return vtagged_error(
                                E_SYNTAX_ERROR,
                                "Expected a comma or closing parenthesis!\nAt call `%.*s`", len,
                                s->src + start);
                        }
                    }
                    else
                    {
                        // empty
                        src_get(s); // consume ')'
                    }
                    mila_free(id);
                    // callp
                    Value *res = call_function(function, env, argc, args);
                    for (int i = 0; i < argc; i++)
                        val_release(args[i]);
                    mila_free(args);
                    HANDLE_RETURN(res);
                    return res;
                }
                else
                {
                    mila_free(id);
                    return val_retain(function);
                }
            }
        }
        else
        {
            // variable lookup
            Value *vv = env_get(env, id);
#ifdef MILA_DEBUG
            printf("    ?? read %s\n", id);
#endif
            mila_free(id);
            if (!vv)
            {
                // undefined variable -> null
                return vnull();
            }
            val_retain(vv);
            return vv;
        }
        mila_free(id);
    }
    // fallback
    return vnull();
}

// helper to convert numeric types and do arithmetic
inline int is_number(Value *v)
{
    return v && (v->type == T_INT || v->type == T_FLOAT || v->type == T_UINT || v->type == T_BINT || v->type == T_BFLOAT);
}

inline double to_double(Value *v)
{
    if (!v)
        return 0.0;
    if (v->type == T_INT)
        return (double)v->v->i;
    if (v->type == T_FLOAT)
        return v->v->f;
    if (v->type == T_UINT)
        return (double)v->v->ui;
    if (v->type == T_BINT)
        return (double)v->v->bi;
    if (v->type == T_BFLOAT)
        return (double)v->v->bi;
    return 0.0;
}

inline mila_float128_internal to_bdouble(Value *v)
{
    if (!v)
        return b_ff_from_double(0.0);
    if (v->type == T_INT)
        return b_ff_from_long(v->v->i);
    if (v->type == T_FLOAT)
        return b_ff_from_double(v->v->f);
    if (v->type == T_UINT)
        return b_ff_from_ulong(v->v->ui);
    if (v->type == T_BINT)
        return b_ff_from_i128(v->v->bi);
    if (v->type == T_BFLOAT)
        return v->v->bf;
    return b_ff_from_double(0.0);
}

inline __int128 to_bint(Value *v)
{
    if (!v)
        return 0.0;
    if (v->type == T_INT)
        return (__int128)v->v->i;
    if (v->type == T_FLOAT)
        return (__int128)v->v->f;
    if (v->type == T_UINT)
        return (__int128)v->v->ui;
    if (v->type == T_BINT)
        return v->v->bi;
    if (v->type == T_BFLOAT)
        return b_ff_to_i128(v->v->bf);
    return 0.0;
}

inline unsigned long to_uint(Value *v)
{
    if (!v)
        return 0.0;
    if (v->type == T_INT)
        return (unsigned long)v->v->i;
    if (v->type == T_FLOAT)
        return (unsigned long)v->v->f;
    if (v->type == T_UINT)
        return v->v->ui;
    if (v->type == T_BINT)
        return (unsigned long)v->v->bi;
    if (v->type == T_BFLOAT)
        return b_ff_to_ulong(v->v->bf);
    return 0.0;
}

FN_UNUSED static inline long to_int(Value *v)
{
    if (!v)
        return 0;
    if (v->type == T_INT)
        return (long)v->v->i;
    if (v->type == T_FLOAT)
        return (long)v->v->f;
    if (v->type == T_UINT)
        return v->v->ui;
    if (v->type == T_BINT)
        return (long)v->v->bi;
    if (v->type == T_BFLOAT)
        return b_ff_to_long(v->v->bf);
    return 0;
}

// binary ops
inline Value *binary_op(Value *a, MethodType op, Value *b)
{
    if (a->method_table && a->method_table[TMethodBinop])
    {
        Value *res = ((trinary_method)a->method_table[op])(a, vint(op), b);
        if (res != NULL)
            return res;
    }
    else if ((a->type == T_NONE || a->type == T_NULL) &&
             (b->type == T_NONE || b->type == T_NULL))
    {
        if (BMethodEq == op)
            return vbool(a->type == b->type);
        if (BMethodNe == op)
            return vbool(a->type != b->type);
    }
    else if (op == BMethodDefault)
    {
        if (a->type == T_NONE || a->type == T_NULL || !is_truthy(a))
        {
            return val_retain(b);
        }
        else
        {
            return val_retain(a);
        }
    }
    else if (BMethodOr == op)
    {
        int res = is_truthy(a) || is_truthy(b);
        return vbool(res);
    }
    else if (BMethodAnd == op)
    {
        int res = is_truthy(a) && is_truthy(b);
        return vbool(res);
    }
    else if (is_number(a) && is_number(b))
    {
        if (a->type == T_BFLOAT || b->type == T_BFLOAT)
        {
            mila_float128_internal ra = to_bdouble(a), rb = to_bdouble(b);
            if (op == BMethodAdd)
                return vbfloat(b_ff_add(ra, rb));
            if (op == BMethodSub)
                return vbfloat(b_ff_sub(ra, rb));
            if (op == BMethodMul)
                return vbfloat(b_ff_mul(ra, rb));
            if (op == BMethodDiv)
                return vbfloat(b_ff_div(ra, rb));
            if (op == BMethodLess)
                return vbool(b_ff_cmp(ra, rb) < 0);
            if (op == BMethodGreat)
                return vbool(b_ff_cmp(ra, rb) > 0);
            if (op == BMethodLE)
                return vbool(b_ff_cmp(ra, rb) <= 0);
            if (op == BMethodGE)
                return vbool(b_ff_cmp(ra, rb) >= 0);
            if (op == BMethodEq)
                return vbool(b_ff_cmp(ra, rb) == 0);
            if (op == BMethodNe)
                return vbool(b_ff_cmp(ra, rb) != 0);
            return vnull();
        }
        else if (a->type == T_BINT || b->type == T_BINT)
        {
            __int128 ra = to_bint(a), rb = to_bint(b);
            if (op == BMethodAdd)
                return vbint(ra + rb);
            if (op == BMethodSub)
                return vbint(ra - rb);
            if (op == BMethodMul)
                return vbint(ra * rb);
            if (op == BMethodDiv)
                return vbint(ra / rb);
            if (op == BMethodLess)
                return vbool(ra < rb);
            if (op == BMethodGreat)
                return vbool(ra > rb);
            if (op == BMethodLE)
                return vbool(ra <= rb);
            if (op == BMethodLShift)
                a->v->bi = ra << rb;
            if (op == BMethodRShift)
                a->v->bi = ra >> rb;
            if (op == BMethodGE)
                return vbool(ra >= rb);
            if (op == BMethodEq)
                return vbool(ra == rb);
            if (op == BMethodNe)
                return vbool(ra != rb);
            return vnull();
        }
        else if (a->type == T_UINT || b->type == T_UINT)
        // treat both numbers as unsigned.
        {
            unsigned long ia = to_uint(a), ib = to_uint(b);
            if (op == BMethodAdd)
                return vuint(ia + ib);
            if (op == BMethodSub)
                return vuint(ia - ib);
            if (op == BMethodMul)
                return vuint(ia * ib);
            if (op == BMethodDiv)
                return vuint(ia / ib);
            if (op == BMethodLess)
                return vbool(ia < ib);
            if (op == BMethodGreat)
                return vbool(ia > ib);
            if (op == BMethodLE)
                return vbool(ia <= ib);
            if (op == BMethodGE)
                return vbool(ia >= ib);
            if (op == BMethodEq)
                return vbool(ia == ib);
            if (op == BMethodNe)
                return vbool(ia != ib);
            if (op == BMethodLShift)
                return vuint(ia << ib);
            if (op == BMethodRShift)
                return vuint(ia >> ib);
            if (op == BMethodMod)
                return vuint(ia % ib);
            return vnull();
        }
        // numeric arithmetic
        else if (a->type == T_FLOAT || b->type == T_FLOAT)
        {
            double ra = to_double(a), rb = to_double(b);
            if (op == BMethodAdd)
                return vfloat(ra + rb);
            if (op == BMethodSub)
                return vfloat(ra - rb);
            if (op == BMethodMul)
                return vfloat(ra * rb);
            if (op == BMethodDiv)
                return vfloat(ra / rb);
            if (op == BMethodLess)
                return vbool(ra < rb);
            if (op == BMethodGreat)
                return vbool(ra > rb);
            if (op == BMethodLE)
                return vbool(ra <= rb);
            if (op == BMethodGE)
                return vbool(ra >= rb);
            if (op == BMethodEq)
                return vbool(ra == rb);
            if (op == BMethodNe)
                return vbool(ra != rb);
            return vnull();
        }
        else
        {
            long ia = a->v->i, ib = b->v->i;
            if (op == BMethodAdd)
                return vint(ia + ib);
            if (op == BMethodSub)
                return vint(ia - ib);
            if (op == BMethodMul)
                return vint(ia * ib);
            if (op == BMethodDiv)
                return vfloat((double)ia / (double)ib);
            if (op == BMethodLess)
                return vbool(ia < ib);
            if (op == BMethodGreat)
                return vbool(ia > ib);
            if (op == BMethodLE)
                return vbool(ia <= ib);
            if (op == BMethodGE)
                return vbool(ia >= ib);
            if (op == BMethodEq)
                return vbool(ia == ib);
            if (op == BMethodNe)
                return vbool(ia != ib);
            if (op == BMethodLShift)
                return vint(ia << ib);
            if (op == BMethodRShift)
                return vint(ia >> ib);
            if (op == BMethodMod)
                return vint(ia % ib);
            return vnull();
        }
    }
    if (strcmp(GET_TYPENAME(a), ML("list")) == 0 && strcmp(GET_TYPENAME(b), ML("list")) == 0)
    {
        if (!(op == BMethodGreat || op == BMethodLess || op == BMethodEq || op == BMethodNe || op == BMethodGE || op == BMethodLE))
            return vnull();
        LinkedList *ll_a = GET_OPAQUE(a), *ll_b = GET_OPAQUE(b);
        int res = 0; // -1 if less, 0 if equal, 1 if greater than
        Value **list_a = ll_to_iter(ll_a);
        Value **list_b = ll_to_iter(ll_b);

        // Find last index (essentially the length)
        size_t last_a = 0, last_b = 0;
        for (; list_a[last_a]; ++last_a)
            ;
        for (; list_b[last_b]; ++last_b)
            ;

        // Compare the two lists
        // Compare by length
        if (last_a > last_b)
        {
            res = 1;
            goto end;
        }
        if (last_a < last_b)
        {
            res = -1;
            goto end;
        }

        // Compare by items
        if (op == BMethodEq)
        {
            res = 0;
            for (size_t index = 0; index < last_a; ++index)
            {
                Value *truth = binary_op(list_a[index], op, list_b[index]);
                if (!is_truthy(truth))
                {
                    res = 1;
                    val_release(truth);
                    break;
                }
                val_release(truth);
            }
            goto end;
        }
        else if (op == BMethodNe)
        {
            res = 1;
            for (size_t index = 0; index < last_a; ++index)
            {
                Value *truth = binary_op(list_a[index], op, list_b[index]);
                if (!is_truthy(truth))
                {
                    res = 0;
                    val_release(truth);
                    break;
                }
                val_release(truth);
            }
            goto end;
        }
        else
        {
            for (size_t index = 0; index < last_a; ++index)
            {
                if (!is_number(list_a[index]) || !is_number(list_b[index]))
                {
                    char *item_repr = as_c_string_repr(!is_number(list_a[index]) ? list_a[index] : list_b[index]);
                    Value *msg = verror("Item %s is not numeric but was used in list numerical comparison!", item_repr);
                    free(item_repr);
                    // Clean up
                    for (size_t i_a = 0; list_a[i_a]; ++i_a)
                        val_release(list_a[i_a]);
                    for (size_t i_b = 0; list_b[i_b]; ++i_b)
                        val_release(list_b[i_b]);
                    free(list_a);
                    free(list_b);
                    return msg;
                }
                double d_a = to_double(list_a[index]);
                double d_b = to_double(list_b[index]);
                if (d_a > d_b)
                {
                    res = 1;
                    break;
                }
                if (d_a < d_b)
                {
                    res = -1;
                    break;
                }
            }
        }

    end:

        // Clean up
        for (size_t i_a = 0; list_a[i_a]; ++i_a)
            val_release(list_a[i_a]);
        for (size_t i_b = 0; list_b[i_b]; ++i_b)
            val_release(list_b[i_b]);
        free(list_a);
        free(list_b);

        switch (op)
        {
        case BMethodLess:
            return res == -1 ? vbool(1) : vbool(0);
        case BMethodGreat:
            return res == 1 ? vbool(1) : vbool(0);
        case BMethodLE:
            return res == -1 || res == 0 ? vbool(1) : vbool(0);
        case BMethodGE:
            return res == 1 || res == 0 ? vbool(1) : vbool(0);
        case BMethodEq:
            return res == 0 ? vbool(1) : vbool(0);
        case BMethodNe:
            return res != 0 ? vbool(1) : vbool(0);
        default:
            return vbool(0);
        }
    }
    else if (BMethodEq == op)
    {
        if (!a || !b)
            return vbool(0);
        if (a->type == T_STRING && b->type == T_STRING)
            return vbool(strcmp(GET_STRING(a), GET_STRING(b)) == 0);
        // fallback pointer equality (document this!!!)
        return vbool(a == b);
    }
    else if (BMethodNe == op)
    {
        Value *eq = binary_op(a, BMethodEq, b);
        int res = (eq->type == T_BOOL && eq->v == NULL);
        val_release(eq);
        return vbool(res);
    }
    // string concatenation for '+'
    else if (op == BMethodAdd && a->type == T_STRING && b->type == T_STRING)
    {
        size_t la = strlen(GET_STRING(a)), lb = strlen(GET_STRING(b));
        char *buf = mila_malloc(la + lb + 1);
        memcpy(buf, GET_STRING(a), la);
        memcpy(buf + la, GET_STRING(b), lb);
        buf[la + lb] = 0;
        return vstring_take(buf);
    }
    else if (op == BMethodAdd && a->type == T_STRING)
    {
        size_t la = strlen(GET_STRING(a));
        char *stringyfied = as_c_string(b);
        if (stringyfied)
        {
            char *buf = mila_malloc(la + strlen(stringyfied) + 1);
            if (!buf)
                return vnull();
            strcpy(buf, GET_STRING(a));
            strcat(buf, stringyfied);
            mila_free(stringyfied);
            return vstring_take(buf);
        }
        return vnull();
    }
    else if (op == BMethodAdd && b->type == T_STRING)
    {
        size_t la = strlen(GET_STRING(b));
        char *stringyfied = as_c_string(a);
        if (stringyfied)
        {
            char *buf = mila_malloc(la + strlen(stringyfied) + 1);
            if (!buf)
                return vnull();
            strcpy(buf, stringyfied);
            strcat(buf, GET_STRING(b));
            mila_free(stringyfied);
            return vstring_take(buf);
        }
        return vnull();
    }
    else if (b->type == T_STRING && a->type == T_STRING && op == BMethodGlob)
    {
        char *string, *pattern;
        string = GET_STRING(a);
        pattern = GET_STRING(b);
        if (match(pattern, string))
            return vbool(1);
        else
            return vbool(0);
    }
    if (a->type_name && strcmp(a->type_name, MILA_LPREFIX "dict") == 0)
    {
        return binary_op_objects(NULL, 1, a, op, b);
    }
    if (b->type_name && strcmp(b->type_name, MILA_LPREFIX "dict") == 0)
    {
        return binary_op_objects(NULL, 0, b, op, a);
    }
    return vnull();
}

Value *binary_op_objects(Env *env, char right, Value *a, MethodType op, Value *b)
{
    if (a->type_name && strcmp(a->type_name, MILA_LPREFIX "dict") != 0)
    {
        char *repr = as_c_string_repr(a);
        Value *err = verror("%s\n of type %s does not support runtime overloading!", repr, GET_TYPENAME(a));
        free(repr);
        return err;
    }
    const char *method = NULL;
    switch (op)
    {
    case BMethodAdd:
        method = right ? OVERLOAD_ADD : OVERLOAD_R_ADD;
        break;
    case BMethodSub:
        method = right ? OVERLOAD_SUB : OVERLOAD_R_SUB;
        break;
    case BMethodMul:
        method = right ? OVERLOAD_MUL : OVERLOAD_R_MUL;
        break;
    case BMethodDiv:
        method = right ? OVERLOAD_DIV : OVERLOAD_R_DIV;
        break;
    case BMethodMod:
        method = right ? OVERLOAD_MOD : OVERLOAD_R_MOD;
        break;
    case BMethodEq:
        method = right ? OVERLOAD_EQ : OVERLOAD_R_EQ;
        break;
    case BMethodNe:
        method = right ? OVERLOAD_NE : OVERLOAD_R_NE;
        break;
    case BMethodLess:
        method = right ? OVERLOAD_LE : OVERLOAD_R_LE;
        break;
    case BMethodGreat:
        method = right ? OVERLOAD_GE : OVERLOAD_R_GE;
        break;
    case BMethodLE:
        method = right ? OVERLOAD_LE : OVERLOAD_R_LE;
        break;
    case BMethodGE:
        method = right ? OVERLOAD_GE : OVERLOAD_R_GE;
        break;
    case BMethodLShift:
        method = right ? OVERLOAD_LSHIFT : OVERLOAD_R_LSHIFT;
        break;
    case BMethodRShift:
        method = right ? OVERLOAD_RSHIFT : OVERLOAD_R_RSHIFT;
        break;
    default:;
    }
    if (!method)
        return vnull();
    Value *fn = dict_get_str((Dict *)a->v, method);
    if (fn)
        return call_function_with(env, fn, val_retain(a), val_retain(b), NULL);
    return vnull();
}

// evaluate expression with precedence climbing
inline int precedence_of(MethodType op)
{
    if (BMethodAnd == op)
        return 1;
    if (BMethodOr == op)
        return 2;
    if (BMethodDefault == op)
        return 3;
    if (BMethodEq == op || BMethodNe == op)
        return 4;
    if (BMethodLShift == op || BMethodRShift == op)
        return 5;
    if (BMethodLE == op || BMethodGE == op || BMethodLess == op ||
        BMethodGreat == op)
        return 6;
    if (BMethodAdd == op || BMethodSub == op)
        return 7;
    if (BMethodMod == op || BMethodMul == op || BMethodDiv == op)
        return 8;
    if (BMethodGlob == op)
        return 9;
    return 0;
}

inline MethodType parse_op(Src *s)
{
    skip_ws(s);
    char a = src_peek(s);
    if (a == '\0')
        return -1;
    char b = s->src[s->pos + 1];
    // two-char ops
    if (a == '|' && b == '|')
    {
        s->pos += 2;
        return BMethodOr;
    }
    if (a == '&' && b == '&')
    {
        s->pos += 2;
        return BMethodAnd;
    }
    if (a == '=' && b == '=')
    {
        s->pos += 2;
        return BMethodEq;
    }
    if (a == '!' && b == '=')
    {
        s->pos += 2;
        return BMethodNe;
    }
    if (a == '<' && b == '=')
    {
        s->pos += 2;
        return BMethodLE;
    }
    if (a == '>' && b == '=')
    {
        s->pos += 2;
        return BMethodGE;
    }
    if (a == '>' && b == '>')
    {
        s->pos += 2;
        return BMethodRShift;
    }
    if (a == '<' && b == '<')
    {
        s->pos += 2;
        return BMethodLShift;
    }
    if (a == '?' && b == '?')
    {
        s->pos += 2;
        return BMethodDefault;
    }
    if (a == '=' && b == '>')
    {
        s->pos += 2;
        return BMethodGlob;
    }
    // single-char ops
    s->pos++;
    switch (a)
    {
    case '+':
        return BMethodAdd;
    case '-':
        return BMethodSub;
    case '*':
        return BMethodMul;
    case '/':
        return BMethodDiv;
    case '%':
        return BMethodMod;
    case '<':
        return BMethodLess;
    case '>':
        return BMethodGreat;
    }
    s->pos--;
    return MethodNone;
}

Value *eval_expr_prec(Src *s, Env *env, int min_prec)
{
    skip_ws(s);
    Value *lhs = eval_primary(s, env);
    if (!lhs)
        return vnull();
    for (;;)
    {
        int saved_pos = s->pos;
        MethodType op = parse_op(s);
        if (op == MethodNone)
            return lhs;
        int prec = precedence_of(op);
        if (prec < min_prec)
        {
            s->pos = saved_pos;
            break;
        }
        // handle right associativity? none needed
        int next_min = prec + 1;
        Value *rhs = eval_expr_prec(s, env, next_min);
        Value *newlhs = binary_op(lhs, op, rhs);
        val_release(lhs);
        val_release(rhs);
        lhs = newlhs;
    }
    return lhs;
}

Value *eval_expr(Src *s, Env *env) { return eval_expr_prec(s, env, 1); }

void clean_elif_chain(Src *s)
{
    while (is_keyword_at(s, "elif"))
    {
        s->pos += strlen("elif");
        if (match_char(s, '('))
            skip_expr(s);
        match_char(s, ')');
        if (match_char(s, '{'))
        {
            s->pos--;
            skip_block(s);
        }
        else
        {
            skip_parse_statement(s);
        }
    }
    if (is_keyword_at(s, "else"))
    {
        s->pos += strlen("else");
        if (match_char(s, '{'))
        {
            s->pos--;
            skip_block(s);
        }
        else
        {
            skip_parse_statement(s);
        }
    }
}

Value *eval_statement(Src *s, Env *env)
{
    if (is_keyword_at(s, "set"))
    {
        s->pos += strlen("set");
        char *id = parse_ident(s);
        if (!id)
            return verror("Invalid set statement.");
        Value *v = NULL;
        MethodType mt = MethodNone;
        skip_ws(s);

        // Check if this is a subscripted assignment
        if (src_peek(s) == '[')
        {
            // Handle nested subscripts: set a[x][y][z] = value
            Value *obj = env_get(env, id);

            if (!obj)
            {
                Value *ret = verror("%s cannot be subscripted as it is cnull", id);
                mila_free(id);
                return ret;
            }

            val_retain(obj);

            // Collect all subscript indices
            Value **indices = NULL;
            int num_indices = 0;

            while (src_peek(s) == '[')
            {
                Value *index = parse_subscript(s, env);
                indices = mila_realloc(indices, sizeof(Value *) * (num_indices + 1));
                indices[num_indices++] = index;
            }

            skip_ws(s);

            // Parse the assignment or statement
            switch (src_peek(s))
            {
            case '+':
                mt = BMethodAdd;
                break;
            case '-':
                mt = BMethodSub;
                break;
            case '*':
                mt = BMethodMul;
                break;
            case '/':
                mt = BMethodDiv;
                break;
            case '%':
                mt = BMethodMod;
                break;
            }
            if (mt != MethodNone)
                s->pos++;
            if (__builtin_expect(!!match_char(s, '='), 1))
            {
                v = eval_expr(s, env);
                if (mt != MethodNone)
                {
                    // Traverse to the parent object (all but the last index)
                    Value *parent = obj;
                    for (int i = 0; i < num_indices - 1; i++)
                    {
                        if (parent->method_table && parent->method_table[BMethodGetItem])
                        {
                            Value *res =
                                ((binary_method)parent->method_table[BMethodGetItem])(
                                    parent, indices[i]);
                            // val_release(parent);
                            parent = res;

                            if (!parent)
                            {
                                Value *ret = verror("cannot be subscripted at level %d", i + 1);
                                for (int j = 0; j < num_indices; j++)
                                    val_release(indices[j]);
                                mila_free(indices);
                                val_release(obj);
                                val_release(v);
                                mila_free(id);
                                return ret;
                            }
                        }
                        else
                        {
                            Value *ret =
                                verror("Type %s does not support subscripting at level %d!",
                                       GET_TYPENAME(parent), i + 1);
                            for (int j = 0; j < num_indices; j++)
                                val_release(indices[j]);
                            mila_free(indices);
                            val_release(obj);
                            val_release(parent);
                            val_release(v);
                            mila_free(id);
                            return ret;
                        }
                    }

                    // Set the final item using the last index
                    Value *last_index = indices[num_indices - 1];
                    if (parent->method_table && parent->method_table[TMethodSetItem])
                    {
                        Value *inplace =
                            ((binary_method)parent->method_table[BMethodGetItem])(
                                parent, val_retain(last_index));
                        val_release(obj);
                        Value *result = binary_op(inplace, mt, v);
                        ((trinary_method)parent->method_table[TMethodSetItem])(
                            parent, last_index, result);
                        for (int i = 0; i < num_indices; i++)
                            val_release(indices[i]);
                        val_release(last_index);
                        mila_free(indices);
                        val_release(v);
                        mila_free(id);
                        match_char(s, ';');
                        return result;
                    }
                    else
                    {
                        Value *ret = verror("Type %s does not support item assignment!",
                                            GET_TYPENAME(parent));
                        for (int i = 0; i < num_indices; i++)
                            val_release(indices[i]);
                        mila_free(indices);
                        val_release(obj);
                        val_release(parent);
                        val_release(v);
                        mila_free(id);
                        return ret;
                    }
                }
            }
            else if (match_char(s, '='))
            {
                v = eval_expr(s, env);
                match_char(s, ';');
            }
            else if (match_char(s, ':'))
            {
                v = eval_statement(s, env);
            }
            else
            {
                for (int i = 0; i < num_indices; i++)
                    val_release(indices[i]);
                mila_free(indices);
                val_release(obj);
                mila_free(id);
                return verror("Expected = or : after subscripts!");
            }

            if (v && v->type == T_RETURN)
            {
                Value *tmp = v;
                v = (Value *)tmp->v;
                val_release(tmp);
            }

            // Traverse to the parent object (all but the last index)
            Value *parent = obj;
            for (int i = 0; i < num_indices - 1; i++)
            {
                if (parent->method_table && parent->method_table[BMethodGetItem])
                {
                    Value *res = ((binary_method)parent->method_table[BMethodGetItem])(
                        parent, indices[i]);
                    // val_release(parent);
                    parent = res;

                    if (!parent)
                    {
                        Value *ret = verror("cannot be subscripted at level %d", i + 1);
                        for (int j = 0; j < num_indices; j++)
                            val_release(indices[j]);
                        mila_free(indices);
                        val_release(obj);
                        val_release(v);
                        mila_free(id);
                        return ret;
                    }
                }
                else
                {
                    Value *ret =
                        verror("Type %s does not support subscripting at level %d!",
                               GET_TYPENAME(parent), i + 1);
                    for (int j = 0; j < num_indices; j++)
                        val_release(indices[j]);
                    mila_free(indices);
                    val_release(obj);
                    val_release(parent);
                    val_release(v);
                    mila_free(id);
                    return ret;
                }
            }

            // Set the final item using the last index
            Value *last_index = indices[num_indices - 1];
            if (parent->method_table && parent->method_table[TMethodSetItem])
            {
                Value *res = ((trinary_method)parent->method_table[TMethodSetItem])(
                    parent, last_index, v);

                for (int i = 0; i < num_indices; i++)
                    val_release(indices[i]);
                mila_free(indices);
                val_release(obj);
                // val_release(parent);
                val_release(v);
                mila_free(id);
                return val_retain(res);
            }
            else
            {
                Value *ret = verror("Type %s does not support item assignment!",
                                    GET_TYPENAME(parent));

                for (int i = 0; i < num_indices; i++)
                    val_release(indices[i]);
                mila_free(indices);
                val_release(obj);
                val_release(parent);
                val_release(v);
                mila_free(id);
                return ret;
            }
        }

        switch (src_peek(s))
        {
        case '+':
            mt = BMethodAdd;
            break;
        case '-':
            mt = BMethodSub;
            break;
        case '*':
            mt = BMethodMul;
            break;
        case '/':
            mt = BMethodDiv;
            break;
        case '%':
            mt = BMethodMod;
            break;
        }
        if (mt != MethodNone)
            s->pos++;
        if (__builtin_expect(!!match_char(s, '='), 1))
        {
            v = eval_expr(s, env);
            if (mt != MethodNone)
            {
                Value *inplace = env_get(env, id);
                if (!inplace)
                {
                    Value *err = verror(
                        "Variable %s doesnt exist and yet inplace operator was used!",
                        id);
                    mila_free(id);
                    val_release(v);
                    return err;
                }
                Value *res = NULL;
                if (inplace)
                    env_set_raw(env, id, res = binary_op(inplace, mt, v));
                mila_free(id);
                val_release(v);
                match_char(s, ';');
                return val_retain(res);
            }
        }
        else if (match_char(s, ':'))
        {
            v = eval_statement(s, env);
        }
        else
        {
            return verror("Expected a proper set statement!");
        }

        if (v && v->type == T_RETURN)
        {
            Value *tmp = v;
            v = (Value *)tmp->v;
            val_release(tmp);
        }
        else if (v && v->type == T_FUNCTION && !GET_FUNCTION(v)->name)
        {
            GET_FUNCTION(v)->name = mila_strdup(id);
        }
        env_set(env, id, v);
        mila_free(id);
        return v;
    }
    if (is_keyword_at(s, "var"))
    {
        s->pos += strlen("var");
        char *id = parse_ident(s);
        // check for type hints
        if (match_char(s, ':'))
        {
            Value *type = parse_string(s);
            val_release(type); // ignore for now
        }
        if (!id)
            return verror("Invalid var statement.");
        if (match_char(s, ';'))
        {
            // declare none
            env_set_raw(env, id, vnone());
            mila_free(id);
            return vnull();
        }
        Value *v = NULL;
        if (match_char(s, '='))
        {
            v = eval_expr(s, env);
            match_char(s, ';');
        }
        else if (match_char(s, ':'))
        {
            v = eval_statement(s, env);
        }
        else
        {
            free(id);
            return verror("Expected a proper var statement!");
        }

        if (v && v->type == T_RETURN)
        {
            Value *tmp = v;
            v = (Value *)tmp->v;
            val_release(tmp);
        }
        else if (v && v->type == T_FUNCTION && !GET_FUNCTION(v)->name)
        {
            GET_FUNCTION(v)->name = mila_strdup(id);
        }
        env_set_local(env, id, v);
        mila_free(id);
        return v;
    }
    if (is_keyword_at(s, "contextual"))
    {
        s->pos += strlen("contextual");
        char *id = parse_ident(s);
        if (!id)
            return verror("Invalid contextual statement.");
        Value *a = env_get(env, id);
        if (!a)
        {
            Value *res = verror(
                "Variable `%s` cannot become contextual as it doesnt exist!", id);
            mila_free(id);
            return res;
        }
        if (is_keyword_at(s, "as"))
        {
            s->pos += 2;
            char *alias = parse_ident(s);
            if (!alias)
            {
                Value *res = vtagged_error(E_SYNTAX_ERROR,
                                           "Aliased contextual `%s` is incomplete", id);
                free(id);
                return res;
            }
            mila_free(id);
            id = alias;
        }
        env_set_raw_contextual(env, id, a);
        mila_free(id);
        match_char(s, ';');
        return vnull();
    }
    if (is_keyword_at(s, "sync"))
    {
        s->pos += strlen("sync");
        char *id = parse_ident(s);
        if (!id)
            return verror("Invalid sync statement.");
        Value *a = env_get(env, id);
        if (!a)
        {
            Value *res =
                verror("Variable `%s` cannot be synced as it doesnt exist!", id);
            mila_free(id);
            return res;
        }
        Value *val = NULL;
        if (match_char(s, '='))
        {
            val = eval_expr(s, env);
            match_char(s, ';');
        }
        val_kill_incomplete(a);
        a->type = val->type;
        switch (GET_TYPE(val))
        {
        case T_INT:
            a->v->i = GET_INTEGER(val);
            break;
        case T_UINT:
            a->v->ui = GET_UINTEGER(val);
            break;
        case T_BINT:
            a->v->bi = GET_BINTEGER(val);
            break;
        case T_BFLOAT:
            a->v->bf = GET_BFLOAT(val);
            break;
        case T_OWNED_OPAQUE:
        case T_OPAQUE:
            a->v = (void *)GET_OPAQUE(val);
            break;
        case T_STRING:
            a->v = (void *)GET_STRING(val);
            break;
        case T_FUNCTION:
            a->v = (void *)val->v;
            break;
        case T_NATIVE:
            a->v = (void *)val->v;
            break;
        default:
            return vtagged_error(E_FATAL, "Type %s cannot be synced!",
                                 GET_TYPENAME(val));
        }
        mila_free(val);
        mila_free(id);
        return vnull();
    }
    if (is_keyword_at(s, "export"))
    {
        s->pos += strlen("export");
        char *id = parse_ident(s);
        if (!id)
            return verror("Invalid export statement.");
        Value *v = NULL;
        if (match_char(s, '='))
        {
            v = eval_expr(s, env);
            match_char(s, ';');
        }
        else if (match_char(s, ':'))
        {
            v = eval_statement(s, env);
        }
        else
        {
            return verror("Expected a proper export statement!");
        }

        if (v && v->type == T_RETURN)
        {
            Value *tmp = v;
            v = (Value *)tmp->v;
            val_release(tmp);
        }
        if (env->parent)
        {
            env_set(env->parent, id, v);
            env_set_local(env, id, v);
        }
        else
        {
            env_set_local(env, id, v);
            fprintf(stderr, "= Warning: %s not binded to an outer scope!\n", id);
        }
        mila_free(id);
        return v;
    }
    if (is_keyword_at(s, "forget"))
    {
        s->pos += strlen("forget");
        skip_ws(s);
        if (src_peek(s) == '[')
        {
            char **names = parse_context_list(s);
            for (int i = 0; names[i]; ++i)
            {
                // env_set_local_raw_contextual(env, names[i], NULL);
                env_remove_contextual(env, names[i]);
                free(names[i]);
            }
            free(names);
            return vnull();
        }
        char *id = parse_ident(s);

        val_release(env_get(env, id));
        env_remove(env, id);

        mila_free(id);

        match_char(s, ';');
        return vnull();
    }
    if (is_keyword_at(s, "return"))
    {
        s->pos += strlen("return");
        Value *v = eval_expr(s, env);
        match_char(s, ';');
        // wrap as return value: create T_RETURN whose opaque pointer contains the
        // actual Value*
        Value *r = val_new_raw(T_RETURN);
        r->v = (void *)v;
        return r;
    }
    if (is_keyword_at(s, "if"))
    {
        s->pos += strlen("if");
        if (match_char(s, '('))
        {
            Value *cond = eval_expr(s, env);
            match_char(s, ')');
            int truth = is_truthy(cond);
            val_release(cond);
            if (truth)
            {
                Value *res = NULL;
                if (match_char(s, '{'))
                {
                    s->pos--;
                    res = eval_block_raw(s, env);
                }
                else
                    res = eval_statement(s, env);
                clean_elif_chain(s);
                HANDLE_CONTROL(res);
            }
            else
            {
                // skip then clause
                if (match_char(s, '{'))
                {
                    s->pos--;
                    skip_block(s);
                }
                else
                {
                    skip_parse_statement(s);
                }
                // check elifs
                while (is_keyword_at(s, "elif"))
                {
                    s->pos += strlen("elif");
                    if (match_char(s, '('))
                    {
                        Value *cond = eval_expr(s, env);
                        match_char(s, ')');
                        if (is_truthy(cond))
                        {
                            Value *res = NULL;
                            if (match_char(s, '{'))
                            {
                                s->pos--;
                                res = eval_block_raw(s, env);
                            }
                            else
                                res = eval_statement(s, env);

                            clean_elif_chain(s);
                            val_release(cond);
                            HANDLE_CONTROL(res);
                        }
                        else
                        {
                            // skip elif then clause
                            val_release(cond);
                            if (match_char(s, '{'))
                            {
                                s->pos--;
                                skip_block(s);
                            }
                            else
                            {
                                skip_parse_statement(s);
                            }
                        }
                    }
                }
                // run else if it exists
                if (is_keyword_at(s, "else"))
                {
                    s->pos += strlen("else");
                    Value *res = NULL;
                    if (match_char(s, '{'))
                    {
                        s->pos--;
                        res = eval_block_raw(s, env);
                    }
                    else
                        res = eval_statement(s, env);
                    // check for return propagation
                    HANDLE_CONTROL(res);
                }
            }
        }
        return vnull();
    }
    if (is_keyword_at(s, "while"))
    {
        s->pos += strlen("while");
        if (match_char(s, '('))
        {
            uint64_t cond_start_pos = s->pos;
            skip_expr(s);
            s->pos--; // skip_expr consumes the closing parenthesis
            if (!match_char(s, ')'))
                return verror("while: Expected condition to close with a parenthesis!");
            uint64_t body_start_pos = s->pos;
            skip_block(s);
            uint64_t body_end_pos = s->pos;
            Value *bod = vnull();

            // we execute the loop until the condition is false or a control-flow
            // statement is hit
            while (1)
            {
                // Reset position to condition start for re-evaluation
                s->pos = cond_start_pos;

                // Re-evaluate condition
                Value *cond = eval_expr(s, env);
                if (IS_ERROR(cond))
                {
                    val_release(bod);
                    return cond;
                }
                else if (!is_truthy(cond))
                {
                    val_release(cond);
                    if (GET_TYPE(bod) == T_RETURN)
                    {
                        return bod;
                    }
                    val_release(bod);
                    s->pos = body_end_pos;
                    return vnull();
                }
                val_release(cond);
                // reset the position to the start of the body for execution
                s->pos = body_start_pos;
                val_release(bod);
                bod = eval_block(s, env);

                // --- Handle body result ---
                switch (GET_TYPE(bod))
                {
                case T_BREAK:
                {
                    s->pos = body_end_pos;
                    val_release(bod);
                    return vnull();
                }
                break;
                case T_CONTINUE:
                {
                    s->pos = cond_start_pos;
                    continue;
                }
                break;
                case T_RETURN:
                {
                    s->pos = body_end_pos;
                    return bod;
                }
                break;
                case T_TAGGED_ERROR:
                case T_ERROR:
                {
                    s->pos = body_end_pos;
                    return bod;
                }
                break;
                default:;
                }
            }
            return bod;
        }
        return verror("While loops condition must be wrapped in parenthesis!");
    }
    if (is_keyword_at(s, "foreach"))
    {
        s->pos += strlen("foreach");
        skip_ws(s);
        char *id = parse_ident(s);
        if (!id)
            return vnull();
        if (strcmp(id, "yield") == 0)
        {
            free(id);
            id = parse_ident(s);
            if (!match_char(s, ':'))
            {
                mila_free(id);
                return verror("Foreach lacking ':'");
            }
            Value *dummy = NULL;
            Value *iter_obj = eval_expr(s, env);
            if (iter_obj->method_table && iter_obj->method_table[UMethodToGen])
            {
                Value *res = ((unary_method)iter_obj->method_table[UMethodToGen])(iter_obj);
                dummy = iter_obj;
                iter_obj = res;
            }

            if (IS_ERROR(iter_obj))
            {
                mila_free(id);
                return iter_obj;
            }

            uint64_t body_start_pos = s->pos;
            skip_block(s);
            uint64_t body_end_pos = s->pos;
            Value *bod = NULL;
            if (GET_TYPE(iter_obj) != T_INT)
            {
                free(id);
                val_release(iter_obj);
                return verror("Expected an integer for the thread ID!");
            }
            ThreadContext *ctx = thread_registry_get(GET_INTEGER(iter_obj));
            if (!ctx)
            {
                val_release(iter_obj);
                free(id);
                return verror("Thread ID %ld does not exist!", GET_INTEGER(iter_obj));
            }
            Value *v = thread_get_yield(ctx);
            for (; v; v = thread_get_yield(ctx))
            {
                // reset the position to the start of the body for execution
                s->pos = body_start_pos;
                // Env *env = env_new(env);
                env_set_local_raw(env, id, v);
                bod = eval_block_raw(s, env);
                // val_release(v);
                // env_remove(frame, id);

                // env_free(frame);
                // --- Handle body result ---
                switch (GET_TYPE(bod))
                {
                case T_BREAK:
                {
                    s->pos = body_end_pos;
                    val_release(bod);
                    mila_free(id);
                    s->pos = body_end_pos;
                    return vnull();
                }
                break;
                case T_CONTINUE:
                {
                    s->pos = body_start_pos;
                    if (bod)
                        val_release(bod);
                    mila_free(id);
                    continue;
                }
                break;
                case T_RETURN:
                {
                    s->pos = body_end_pos;
                    mila_free(id);
                    return bod;
                }
                break;
                case T_TAGGED_ERROR:
                case T_ERROR:
                {
                    s->pos = body_end_pos;
                    mila_free(id);
                    return bod;
                }
                break;
                default:;
                }
                val_release(bod);
            }
            s->pos = body_end_pos;
            val_release(v);
            val_release(dummy);
            val_release(iter_obj);
            mila_free(id);
            return vnull();
        }
        else
        {
            if (!match_char(s, ':'))
            {
                mila_free(id);
                return verror("Foreach lacking ':'");
            }
            Value *iter_obj = eval_expr(s, env);
            if (IS_ERROR(iter_obj))
            {
                mila_free(id);
                return iter_obj;
            }
            Value **value = NULL;

            // check new improved Iter method first
            if (iter_obj->method_table && iter_obj->method_table[UMethodStepIterInit] && iter_obj->method_table[UMethodStepIter] && iter_obj->method_table[UMethodStepIterClean])
            {
                void *iter_state =
                    ((unary_method)iter_obj->method_table[UMethodStepIterInit])(iter_obj);
                if (!iter_state)
                {
                    mila_free(id);
                    return verror("Iterable initialization returned C null!");
                }

                skip_ws(s);
                uint64_t body_start_pos = s->pos;
                skip_block(s);
                uint64_t body_end_pos = s->pos;
                Value *bod = NULL;

                while (1)
                {
                    Value *v = ((unary_method)iter_obj->method_table[UMethodStepIter])(iter_state);
                    if (!v)
                    {
                        s->pos = body_end_pos;
                        mila_free(id);
                        ((unary_method)iter_obj->method_table[UMethodStepIterClean])(iter_state);
                        val_release(iter_obj);
                        return vnull();
                    }
                    // reset the position to the start of the body for execution
                    s->pos = body_start_pos;
                    Env *frame = env_new(env);
                    env_set_local_raw(frame, id, v);
                    bod = eval_block_raw(s, frame);
                    val_release(v);
                    env_remove(frame, id);

                    env_free(frame);
                    // --- Handle body result ---
                    switch (GET_TYPE(bod))
                    {
                    case T_BREAK:
                    {
                        s->pos = body_end_pos;
                        val_release(bod);
                        mila_free(value);
                        mila_free(id);
                        ((unary_method)iter_obj->method_table[UMethodStepIterClean])(iter_state);
                        val_release(iter_obj);
                        return vnull();
                    }
                    case T_CONTINUE:
                    {
                        s->pos = body_start_pos;
                        if (bod)
                            val_release(bod);
                        mila_free(v);
                        mila_free(value);
                        mila_free(id);
                        ((unary_method)iter_obj->method_table[UMethodStepIterClean])(iter_state);
                        val_release(iter_obj);
                        continue;
                    }
                    case T_RETURN:
                    {
                        s->pos = body_end_pos;
                        mila_free(value);
                        mila_free(id);
                        ((unary_method)iter_obj->method_table[UMethodStepIterClean])(iter_state);
                        val_release(iter_obj);
                        return bod;
                    }
                    case T_TAGGED_ERROR:
                    case T_ERROR:
                    {
                        s->pos = body_end_pos;
                        mila_free(value);
                        mila_free(id);
                        ((unary_method)iter_obj->method_table[UMethodStepIterClean])(iter_state);
                        val_release(iter_obj);
                        return bod;
                    }
                    default:;
                    }

                    val_release(bod);
                }
                s->pos = body_end_pos;
                mila_free(id);
                mila_free(value);
            }
            else if (iter_obj->method_table && iter_obj->method_table[UMethodToIter])
            {
                Value *iter_instance =
                    ((unary_method)iter_obj->method_table[UMethodToIter])(iter_obj);
                if (IS_ERROR(iter_instance))
                {
                    mila_free(id);
                    return iter_instance;
                }
                if (!iter_instance)
                {
                    mila_free(id);
                    return verror("Iterable is cnull!");
                }
                value = (Value **)iter_instance->v;
                if (!value)
                {
                    mila_free(id);
                    return verror("Value returned null!");
                }
                val_kill(iter_instance);

                skip_ws(s);
                val_release(iter_obj);

                uint64_t body_start_pos = s->pos;
                skip_block(s);
                uint64_t body_end_pos = s->pos;
                Value *bod = NULL;

                // we execute the loop until the condition is false or a control-flow
                // statement is hit
                size_t i = 0;
                Value *v = value[i];
                i++;
                for (; v; v = value[i++])
                {
                    // reset the position to the start of the body for execution
                    s->pos = body_start_pos;
                    Env *frame = env_new(env);
                    env_set_local_raw(frame, id, v);
                    bod = eval_block_raw(s, frame);
                    val_release(v);
                    env_remove(frame, id);

                    env_free(frame);
                    // --- Handle body result ---
                    switch (GET_TYPE(bod))
                    {
                    case T_BREAK:
                    {
                        s->pos = body_end_pos;
                        for (; value[i]; ++i)
                            val_release(value[i]);
                        val_release(bod);
                        mila_free(value);
                        mila_free(id);
                        return vnull();
                    }
                    case T_CONTINUE:
                    {
                        s->pos = body_start_pos;
                        if (bod)
                            val_release(bod);
                        for (; value[i]; ++i)
                            val_release(value[i]);
                        mila_free(v);
                        mila_free(value);
                        mila_free(id);
                        continue;
                    }
                    case T_RETURN:
                    {
                        s->pos = body_end_pos;
                        for (; value[i]; ++i)
                            val_release(value[i]);
                        mila_free(value);
                        mila_free(id);
                        return bod;
                    }
                    case T_TAGGED_ERROR:
                    case T_ERROR:
                    {
                        s->pos = body_end_pos;
                        for (; value[i]; ++i)
                            val_release(value[i]);
                        mila_free(value);
                        mila_free(id);
                        return bod;
                    }
                    default:;
                    }
                    val_release(bod);
                }
                s->pos = body_end_pos;
                mila_free(id);
                mila_free(value);
            }
            else
            {
                mila_free(id);
                Value *err = verror("Type %s does not implement UMethodToIter",
                                    GET_TYPENAME(iter_obj));
                val_release(iter_obj);
                return err;
            }
            return vnull();
        }
    }
    if (is_keyword_at(s, "block"))
    {
        s->pos += strlen("block");
        skip_ws(s);
        char *name = parse_ident(s);
        if (!name)
            return verror("Block needs a name!");
        skip_ws(s);
        Env *frame = env_new(env);
        env_set_local_raw(frame, "@block_name", vstring_take(name));
        Value *res = eval_block_raw(s, frame);
        if (IS_ERROR_TAGGED(res))
        {
            Value *new_res =
                vtagged_coded_error(res->v->tagged_error.type, res->v->tagged_error.return_code, "Block %s reported an error: %s", name, res->v->tagged_error.message);
            val_release(res);
            env_free(frame);
            return new_res;
        }
        else if (IS_ERROR(res))
        {
            Value *new_res =
                verror("Block %s reported an error: %s", name, GET_ERROR_MESSAGE(res));
            val_release(res);
            env_free(frame);
            return new_res;
        }
        env_free(frame);
        return res;
    }
    if (is_keyword_at(s, "namespace"))
    {
        s->pos += strlen("namespace");
        skip_ws(s);
        char *old_name = s->cur_namespace;
        s->cur_namespace = parse_ident(s);
        if (!s->cur_namespace)
        {
            s->cur_namespace = old_name;
            return verror("Namespace needs a name!");
        }
        skip_ws(s);
        Value *res = eval_block_raw(s, env);
        mila_free(s->cur_namespace);
        s->cur_namespace = old_name;
        return res;
    }
    if (is_keyword_at(s, "catch"))
    {
        s->pos += strlen("catch");
        char *id = parse_ident(s);
        size_t start = s->pos;
        skip_block(s);
        size_t end = s->pos;
        s->pos = start;
        Value *res = eval_block_raw(s, env);

        if (IS_ERROR(res) && !IS_FATAL(res))
        {
            env_set_local_raw(env, id, vnone());
            if (id)
            {
                if (IS_ERROR_TAGGED(res))
                {
                    Value *msg = vstring_dup(res->v->tagged_error.message);
                    Value *type = vstring_dup(GET_TAGGED_ERROR_TYPENAME(res));
                    Value *dict = call_native_with(env, native_new_dict, vstring_dup("error"), type, vstring_dup("message"), msg, NULL);
                    val_release(res);
                    env_set_local(env, id, dict);
                    free(id);
                    s->pos = end;
                    return dict;
                }
                else
                {
                    Value *msg = vstring_dup(GET_ERROR_MESSAGE(res));
                    Value *dict = call_native_with(env, native_new_dict, vstring_dup("error"), vstring_dup("Generic"), vstring_dup("message"), msg, NULL);
                    val_release(res);
                    env_set_local(env, id, dict);
                    free(id);
                    s->pos = end;
                    return dict;
                }
            }
        }
        else
        {
            return res; // do not catch fatal
        }

        s->pos = end;
        mila_free(id);
        return res;
    }
    if (is_keyword_at(s, "fn"))
    {
        // consume keyword
        s->pos += strlen("fn");
        char *name = parse_ident(s);
        if (!name)
            return verror("Function needs a name!");
        FunctionParameters *params = parse_param_list(s);
        char **contextuals = parse_context_list(s);
        char **names;
        Env *closure = env_new(NULL);
        if (match_char(s, ':'))
        {
            // parse closure bindings
            names = parse_context_list(s);
            for (int i = 0; names[i]; ++i)
            {
                if (strlen(names[i]) > 5 && strncmp("@env:", names[i], 5) == 0)
                {
                    Env *new_env = env_new(NULL);
                    env_copy(new_env, env);
                    env_set_local_raw(closure, names[i] + 5, vopaque_extra(new_env, NULL, ML("environment")));
                }
                else
                    env_set_local(closure, names[i], env_get(env, names[i]));
                mila_free(names[i]);
            }
            mila_free(names);
        }
        if (is_keyword_at(s, "->"))
        {
            s->pos += 2;
            skip_ws(s);
            if (src_peek(s) == '"')
            {
                Value *ret_type = parse_string(s);
                // ignore return type for now
                val_kill(ret_type);
            }
            else
            {
                env_free(closure);
                for (int i = 0; params->params[i]; ++i)
                {
                    mila_free(params->params[i]);
                    mila_free(params->defaults[i]);
                }
                mila_free(params);
                mila_free(name);
                return vtagged_error(E_SYNTAX_ERROR,
                                     "Expected a string literal for the return type.");
            }
        }
        skip_ws(s);
        // body is block; extract substring from '{' to matching '}'
        int depth = 0;
        size_t start = s->pos;
        size_t i = s->pos;
        if (src_peek(s) == '{')
        {
            // find matching brace (we will copy out body)
            for (; i < s->len; ++i)
            {
                char ch = s->src[i];
                if (ch == '{')
                    depth++;
                else if (ch == '}')
                {
                    depth--;
                    if (depth == 0)
                    {
                        i++;
                        break;
                    }
                }
                else if (ch == '"')
                {
                    // skip string literal
                    i++;
                    while (i < s->len && s->src[i] != '"')
                    {
                        if (s->src[i] == '\\' && i + 1 < s->len)
                            i += 2;
                        else
                            i++;
                    }
                }
            }
        }
        else
        {
            skip_parse_statement(s);
            i = s->pos;
        }
        if (i > s->len)
            i = s->len;
        int blen = i - start;
        char *body = mila_malloc(blen + 1);
        memcpy(body, s->src + start, blen);
        body[blen] = 0;
        s->pos = i;
        // create function value with closure get_line_pos(s) current env
        Value *fn = vfunction(params->params, params->defaults, contextuals, closure, body);
        free(params);
        if (!GET_FUNCTION(fn)->name)
            GET_FUNCTION(fn)->name = mila_strdup(name);
        env_set_local(env, name, fn);
        mila_free(name);
        return fn;
    }
    if (is_keyword_at(s, "object"))
    {
        s->pos += strlen("object");
        char *name = parse_ident(s);
        if (!name)
        {
            return vtagged_error(E_SYNTAX_ERROR, "Expected object to have a name!");
        }
        Value *obj;
        if (!is_keyword_at(s, "with"))
            obj = call_native_with(env, native_new_dict, NULL);
        else
        {
            s->pos += strlen("with");
            char *obj_name = parse_ident(s);
            if (!obj_name)
            {
                Value *res = vtagged_error(E_SYNTAX_ERROR, "Expected a name after `with` for object `%s`", name);
                free(name);
                return res;
            }
            obj = call_native_with(env, native_new_dict, NULL);
            Value *with_obj = env_get(env, obj_name);
            if (!obj)
            {
                Value *res = vtagged_error(
                    E_RUNTIME,
                    "Cannot build on top of variable `%s` as it doesnt exist!",
                    obj_name);
                mila_free(obj_name);
                mila_free(name);
                return res;
            }
            else if (IS_ERROR(obj))
                return obj;

            KVPair *entries = NULL;
            size_t count = 0, capacity = 16;
            entries = (KVPair *)mila_malloc(capacity * sizeof(KVPair));
            if (!entries)
                return NULL;

            // Collect all entries
            for (size_t i = 0; i < ((Dict *)with_obj->v)->capacity; i++)
            {
                DictEntry *entry = ((Dict *)with_obj->v)->buckets[i];
                while (entry)
                {
                    if (count >= capacity)
                    {
                        capacity *= 2;
                        KVPair *tmp = (KVPair *)realloc(entries, capacity * sizeof(KVPair));
                        if (!tmp)
                        {
                            mila_free(entries);
                            return NULL;
                        }
                        entries = tmp;
                    }
                    entries[count].key = entry->key;
                    entries[count].value = entry->value;
                    count++;
                    entry = entry->next;
                }
            }

            for (size_t i = count; i > 0; i--)
            {
                dict_set_raw((Dict *)obj->v, entries[i - 1].key,
                             entries[i - 1].value);
            }
            mila_free(entries);
            mila_free(obj_name);
        }
        if (IS_ERROR(obj))
        {
            mila_free(name);
            return obj;
        }
        Env *class_env = env_new(env);
        Value *res = eval_block_raw(s, class_env);
        if (IS_ERROR(res))
        {
            env_free(class_env);
            mila_free(name);
            val_release(obj);
            return res;
        }
        else
            val_release(res);

        for (Var *v = class_env->vars; v; v = v->next)
        {
            Value *name = vstring_dup(v->name);
            dict_set((Dict *)obj->v, name, v->value);
            val_release(name);
        }

        env_set_local_raw(env, name, obj);
        mila_free(name);
        env_free(class_env);
        return val_retain(obj);
    }
    skip_ws(s);
    // block
    if (src_peek(s) == '{')
    {
        Env *frame = env_new(env);
        Value *res = eval_block(s, frame);
        env_free(frame);
        return res;
    }
    // expression statement
    Value *e = eval_expr(s, env);
    match_char(s, ';');
    return e;
}

int match_types(Value **args, ...)
{
    va_list types;
    va_start(types, args);
    ValueType current;
    for (int i = 0; (current = va_arg(types, ValueType)) != T_ARG_END; i++)
        if (current != args[i]->type)
            goto f;

    va_end(types);
    return 1;
f:
    va_end(types);
    return 0;
}

// top-level eval of source - runs sequential statements in global env
Value *eval_source(Src *s, Env *env)
{
    Value *last = vnull();
    while (!src_eof(s))
    {
        if (src_eof(s))
            break;
        Value *st = eval_statement(s, env);

        if (st->type != T_NULL)
        {
            val_release(last);
            last = st;
            if (last)
            {
                if (IS_ERROR(last))
                {
                    return last;
                }
                else if (last->type == T_RETURN)
                {
                    Value *res = (Value *)last->v;
                    val_release(last);
                    return res;
                }
            }
        }
        else
        {
            val_release(st);
        }
    }
    return last;
}

Value *eval_str(char *src, Env *env)
{
    Src *S = src_new(src);
    Value *res = eval_source(S, env);
    src_free(S);
    return res;
}

void print_error(Value *v)
{
    if (v->type == T_ERROR)
    {
        fprintf(stderr, "\n= Error: %s\n", GET_ERROR_MESSAGE(v));
    }
    if (v->type == T_TAGGED_ERROR)
    {
        if (v->v->tagged_error.type == E_EXIT)
        {
            fprintf(stderr, "\n= Recieved Exit Signal: %s\n", GET_TAGGED_ERROR_MESSAGE(v));
            return;
        }
        if (v->v->tagged_error.type == E_THREAD_HALT)
            return;
        if (IS_FATAL(v))
            fprintf(stderr, "\n= FATAL ERROR[%s]: %s\n", GET_TAGGED_ERROR_TYPENAME(v),
                    v->v->tagged_error.message);
        else
            fprintf(stderr, "\n= Error[%s]: %s\n", GET_TAGGED_ERROR_TYPENAME(v),
                    v->v->tagged_error.message);
    }
}

int run_file(char *name, Env *env)
{
#ifndef SAFE_BUILD
    char *loc_dir = path_dirname_alloc(name);
    env_set_local_raw(env, "__name__", vstring_take(path_basename_alloc(name)));
    env_set_local_raw(env, "__path__", vstring_dup(name));
    env_set_local_raw(env, "__dir_path__", vstring_dup(loc_dir));
    path_list_add(search_path, loc_dir);
#endif
    char *src_text = NULL;
    FILE *f = fopen(name, "rb");
    if (!f)
    {
        fprintf(stderr, "Cannot open %s\n", name);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    src_text = mila_malloc(size + 1);
    fread(src_text, 1, size, f);
    src_text[size] = 0;
    fclose(f);
    Src *S = src_new(src_text);
    Value *res = eval_source(S, env);
    val_release(res);
    src_free(S);
    mila_free(src_text);
#ifndef SAFE_BUILD
    path_list_remove(search_path, loc_dir);
    free(loc_dir);
#endif
    return 0;
}

Value *run_file_keep_res(char *name, Env *env)
{
#ifndef SAFE_BUILD
    char *loc_dir = path_dirname_alloc(name);
    env_set_local_raw(env, "__name__", vstring_take(path_basename_alloc(name)));
    env_set_local_raw(env, "__path__", vstring_dup(name));
    env_set_local_raw(env, "__dir_path__", vstring_dup(loc_dir));
    path_list_add(search_path, loc_dir);
#endif
    char *src_text = NULL;
    FILE *f = fopen(name, "rb");
    if (!f)
    {
        return verror("Cannot open %s\n", name);
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    src_text = mila_malloc(size + 1);
    fread(src_text, 1, size, f);
    src_text[size] = 0;
    fclose(f);
    Src *S = src_new(src_text);
    Value *res = eval_source(S, env);
    src_free(S);
    mila_free(src_text);
#ifndef SAFE_BUILD
    path_list_remove(search_path, loc_dir);
    free(loc_dir);
#endif
    return res;
}

int invoke_file(char *name, Env *env)
{
#ifndef SAFE_BUILD
    char *_loc_dir = path_dirname_alloc(name);
    char *cwd = path_get_cwd();
    char *loc_dir = path_join_alloc(cwd, _loc_dir, NULL);
    free(_loc_dir);
    free(cwd);
    env_set_local_raw(env, "__name__", vstring_take(path_basename_alloc(name)));
    env_set_local_raw(env, "__path__", vstring_dup(name));
    env_set_local_raw(env, "__dir_path__", vstring_dup(loc_dir));
    path_list_add(search_path, loc_dir);

    char *setup_name = path_list_find(search_path, "init.setup-mila");
    if (setup_name)
    {
        Env *setup_env = env_new(env);

        env_set_raw(setup_env, "setup_for", call_native_with(env, native_new_dict, vstring_dup("name"), vstring_take(path_basename_alloc(name)), vstring_dup("id_name"), vstring_take(path_basename_id_alloc(name)), vstring_dup("path"), vstring_dup(name), vstring_dup("dir_path"), vstring_dup(loc_dir), NULL));

        Value *setup_res = run_file_keep_res(setup_name, setup_env);
        env_free(setup_env);
        if (IS_ERROR(setup_res))
        {
            Value *err = verror("Setup file %s returned %s", setup_name, GET_TAGGED_ERROR_MESSAGE(setup_res));
            free(setup_name);
            print_error(err);
            val_release(setup_res);
            path_list_remove(search_path, loc_dir);
            free(loc_dir);
            return 1;
        }
        free(setup_name);
        val_release(setup_res);
    }
#endif
    char *src_text = NULL;
    FILE *f = fopen(name, "rb");
    if (!f)
    {
        fprintf(stderr, "Cannot open %s\n", name);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    src_text = mila_malloc(size + 1);
    fread(src_text, 1, size, f);
    src_text[size] = 0;
    fclose(f);
    Src *S = src_new(src_text);
    Value *res = eval_source(S, env);
    val_release(res);
    src_free(S);
    mila_free(src_text);
#ifndef SAFE_BUILD
    path_list_remove(search_path, loc_dir);
    free(loc_dir);
#endif
    return 0;
}

Value *invoke_main_file(char *name, Env *env, int argc, char *argv[])
{
#ifndef SAFE_BUILD
    char *_loc_dir = path_dirname_alloc(name);
    char *cwd = path_get_cwd();
    char *loc_dir = path_join_alloc(cwd, _loc_dir, NULL);
    free(_loc_dir);
    free(cwd);
    env_set_local_raw(env, "__name__", vstring_take(path_basename_alloc(name)));
    env_set_local_raw(env, "__path__", vstring_dup(name));
    env_set_local_raw(env, "__dir_path__", vstring_dup(loc_dir));
    path_list_add(search_path, loc_dir);

    char *setup_name = path_list_find(search_path, "init.setup-mila");
    if (setup_name)
    {
        Env *setup_env = env_new(env);

        env_set_raw(setup_env, "setup_for", call_native_with(env, native_new_dict, vstring_dup("name"), vstring_take(path_basename_alloc(name)), vstring_dup("id_name"), vstring_take(path_basename_id_alloc(name)), vstring_dup("path"), vstring_dup(name), vstring_dup("dir_path"), vstring_dup(loc_dir), NULL));

        Value *setup_res = run_file_keep_res(setup_name, setup_env);
        env_free(setup_env);
        free(setup_name);
        if (IS_ERROR(setup_res))
        {
            path_list_remove(search_path, loc_dir);
            free(loc_dir);
            return setup_res;
        }
        val_release(setup_res);
    }
#endif
    char *src_text = NULL;
    FILE *f = fopen(name, "rb");
    if (!f)
    {
        return verror("Cannot open %s\n", name);
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    src_text = mila_malloc(size + 1);
    fread(src_text, 1, size, f);
    src_text[size] = 0;
    fclose(f);
    Src *S = src_new(src_text);

    // no comments
    if (src_peek(S) == '#')
    {
        src_get(S);
        if (!match_char(S, '!'))
            return verror("Invalid shebang!");
        while (src_peek(S) != '\n')
            src_get(S);
    }

    skip_ws(S);
    if (src_peek(S) == '!')
    {
        src_get(S);
        if (is_keyword_at(S, "fn"))
        {
            S->pos += 2;
            FunctionParameters *fnp = parse_param_list(S);
            for (int i = 1; i < argc && fnp->params[i]; ++i)
            {
                env_set_raw(env, fnp->params[i], vstring_dup(argv[i]));
                if (strncmp("...", fnp->params[i], 3) == 0)
                {
                    Value *list = call_native_with(env, native_list_new, NULL);
                    env_set_local_raw(env, fnp->params[i] + 3, list);
                    val_release(call_native_with(env, native_list_append, val_retain(list), vstring_dup(argv[i]), NULL));
                    for (i++; i < argc; ++i)
                    {
                        val_release(call_native_with(env, native_list_new, val_retain(list), vstring_dup(argv[i]), NULL));
                    }
                    break;
                }
                else
                {
                    env_set_local_raw(env, fnp->params[i], vstring_dup(argv[i]));
                }
            }
            size_t pass = argc; // 1 because of argc
            for (size_t i = argc; i < fnp->count && fnp->defaults[i]; ++i)
            {
                if (strncmp("...", fnp->params[i], 3) == 0)
                {
                    env_set_raw(env, fnp->params[i] + 3, eval_str(fnp->defaults[i], env));
                }
                else
                {
                    env_set_raw(env, fnp->params[i], eval_str(fnp->defaults[i], env));
                }
                pass++;
            }
            pass--;
            for (size_t i = 1 + pass; i < fnp->count && fnp->params[i]; ++i)
            {
                if (strncmp("...", fnp->params[i], 3) == 0)
                    env_set_raw(env, fnp->params[i] + 3, vnull()); // avoid passing through to upper env
                else
                    env_set_raw(env, fnp->params[i], vnull()); // avoid passing through to upper env
            }
            for (size_t i = 0; i < fnp->count; ++i)
            {
                if (fnp->defaults[i])
                    mila_free(fnp->defaults[i]);
                mila_free(fnp->params[i]);
            }
            mila_free(fnp->params);
            mila_free(fnp->defaults);
            mila_free(fnp);

            if (is_keyword_at(S, "->"))
            {
                S->pos += 2;
                skip_ws(S);
                if (!match_char(S, '"'))
                {
                    src_free(S);
                    mila_free(src_text);
#ifndef SAFE_BUILD
                    path_list_remove(search_path, loc_dir);
                    free(loc_dir);
#endif
                    return verror("Expected string annotation!");
                }
                S->pos--; // parse_string expects openning quote
                val_release(parse_string(S));
            }
        }
    }

    Value *res = eval_source(S, env);
    src_free(S);
    mila_free(src_text);
#ifndef SAFE_BUILD
    path_list_remove(search_path, loc_dir);
    free(loc_dir);
#endif
    return res;
}

Value *invoke_file_keep_res(char *name, Env *env)
{
#ifndef SAFE_BUILD
    char *_loc_dir = path_dirname_alloc(name);
    char *cwd = path_get_cwd();
    char *loc_dir = path_join_alloc(cwd, _loc_dir, NULL);
    free(_loc_dir);
    free(cwd);
    env_set_local_raw(env, "__name__", vstring_take(path_basename_alloc(name)));
    env_set_local_raw(env, "__path__", vstring_dup(name));
    env_set_local_raw(env, "__dir_path__", vstring_dup(loc_dir));
    path_list_add(search_path, loc_dir);

    char *setup_name = path_list_find(search_path, "init.setup-mila");
    if (setup_name)
    {
        Env *setup_env = env_new(env);

        env_set_raw(setup_env, "setup_for", call_native_with(env, native_new_dict, vstring_dup("name"), vstring_take(path_basename_alloc(name)), vstring_dup("id_name"), vstring_take(path_basename_id_alloc(name)), vstring_dup("path"), vstring_dup(name), vstring_dup("dir_path"), vstring_dup(loc_dir), NULL));

        Value *setup_res = run_file_keep_res(setup_name, setup_env);
        env_free(setup_env);
        free(setup_name);
        if (IS_ERROR(setup_res))
        {
            path_list_remove(search_path, loc_dir);
            free(loc_dir);
            return setup_res;
        }
        val_release(setup_res);
    }
#endif
    char *src_text = NULL;
    FILE *f = fopen(name, "rb");
    if (!f)
    {
        return verror("Cannot open %s\n", name);
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    src_text = mila_malloc(size + 1);
    fread(src_text, 1, size, f);
    src_text[size] = 0;
    fclose(f);
    Src *S = src_new(src_text);
    Value *res = eval_source(S, env);
    src_free(S);
    mila_free(src_text);
#ifndef SAFE_BUILD
    path_list_remove(search_path, loc_dir);
    free(loc_dir);
#endif
    return res;
}

int needs_more(const char *src)
{
    int parens = 0, braces = 0;
    int in_string = 0;

    for (const char *p = src; *p; p++)
    {
        if (*p == '"' && (p == src || *(p - 1) != '\\'))
        {
            in_string = !in_string; // toggle string state
        }
        else if (!in_string)
        {
            if (*p == '(')
                parens++;
            else if (*p == ')')
                parens--;
            else if (*p == '{')
                braces++;
            else if (*p == '}')
                braces--;
        }
    }

    // if inside a string or any unbalanced delimiter, we need more input
    return in_string || parens > 0 || braces > 0;
}

Env *mila_global_init(void)
{
    if (!cleanup_registry)
    {
        cleanup_registry = make_cleanup_registry();
    }
    else
    {
        fprintf(stderr, "mila_init called more than once.\n");
        abort();
    }
    Env *g = env_new(NULL);
    env_register_builtins(g);
    register_thread_builtins(g);
    return g;
}

Env *mila_init(void)
{
    Env *g = env_new(NULL);
    env_register_builtins(g);
    register_thread_builtins(g);
    return g;
}

void mila_deinit(Env *g)
{
    env_free(g);
}

void mila_global_deinit(Env *g)
{
    // we might handle stuff here
    if (cleanup_registry)
    {
        for (size_t index = 0; index < cleanup_registry->count; ++index)
        {
            cleanup_registry->registry[index]->fn(g);
        }
        free_cleanup_registry(cleanup_registry);
        cleanup_registry = NULL;
    }
    else
    {
        fprintf(stderr, "mila_init wasnt called.\n");
        abort();
    }
    env_free(g);
    mila_threads_cleanup();
    env_free_builtins();
    path_list_free(search_path);
}

void print_primitive(char *text)
{
#if defined(_WIN32) || defined(_WIN64)
    _exit(1);
#else
    write(STDOUT_FILENO, text, strlen(text));
#endif
}

void handle_signal(int signal)
{
    mila_global_deinit(NULL);
    _exit(signal);
}

#if !(defined(SAFE_BUILD) || defined(ML_LIB))
int main(int argc, char **argv)
{
    // read file if provided or use built-in demo
    char *src_text = NULL;
    if (argc == 2)
    {
        if (strcmp(argv[1], "--info") == 0)
        {
            printf("MiLa - Info\n"
                   "Version: %ld.%ld.%ld\n\n"
                   "C type sizes in bytes (type, size, alignment):\n"
                   "         char %2lu %2lu\n"
                   "        short %2lu %2lu\n"
                   "          int %2lu %2lu\n"
                   "         long %2lu %2lu\n"
                   "       double %2lu %2lu\n"
                   "        void* %2lu %2lu\n"
                   "    ValueType %2lu %2lu\n"
                   "   ValueValue %2lu %2lu\n"
                   "\nVariable size (metadata):\n"
                   "  %lu Bytes for primitive types\n "
                   "   For types:\n"
                   "     bint, float, bfloat\n"
                   "  %lu For shortcut types\n"
                   "    Values: int, strings, bools, none, null, functions, natives, opaques\n"
                   "  %lu Bytes for types with Value Instance Operator Overloading\n"
                   "    For types like:\n"
                   "      arrays, lists, and dictionaries, and others\n"
                   "Estimated memory:\n"
                   "  t * %lu + n * 40 Bytes\n"
                   "  n = # of vars\n"
                   "  t = # of types\n"
                   "Max num digits (asuming long support):\n"
                   "  %i\n",
                   MILA_EDITION, MILA_VERSION, MILA_PATCH,
                   sizeof(char), alignof(char),
                   sizeof(short), alignof(short),
                   sizeof(int), alignof(int),
                   sizeof(long), alignof(long),
                   sizeof(double), alignof(double),
                   sizeof(void *), alignof(void *),
                   sizeof(ValueType), alignof(ValueType),
                   sizeof(ValueValue), alignof(ValueValue),
                   sizeof(Value) + sizeof(ValueValue),
                   sizeof(Value),
                   sizeof(Value) + sizeof(MethodTable) * MethodTotalCount,
                   sizeof(MethodTable) * MethodTotalCount, MAX_NUMBER_DIGITS);
            return 0;
        }
        else if (strcmp(argv[1], "--version") == 0 ||
                 strcmp(argv[1], "-v") == 0)
        {
            printf("MiLa Specification v1.0\n"
                   "CLI v1.0\n"
                   "API v1.0\n");
            return 0;
        }
        else if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
        {
            printf("MiLa v1.0\n"
                   "  --info         = For internal info as well as version info\n"
                   "  --check        = Syntactically check the file\n"
                   "  --version | -v = Prints version\n"
                   "  --help    | -h = Prints this list\n");
            return 0;
        }
    }
    else if (argc == 3)
    {
        if (strcmp(argv[1], "--check") == 0)
        {
            fprintf(stderr, "Syntax Checker - IN PROGRESS\n");
            FILE *f = fopen(argv[2], "rb");
            if (!f)
            {
                fprintf(stderr, "Cannot open %s: Missing or not a file.\n", argv[1]);
                return 1;
            }
            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            fseek(f, 0, SEEK_SET);
            src_text = mila_malloc(size + 1);
            fread(src_text, 1, size, f);
            src_text[size] = 0;
            fclose(f);
            Src *S = src_new(src_text);
            int err = syn_check(S);
            src_free(S);
            mila_free(src_text);
            return err;
        }
    }
    Value *array = NULL;

    Env *g = mila_global_init();

    signal(SIGABRT, handle_signal);
    signal(SIGFPE, handle_signal);
    signal(SIGILL, handle_signal);
    signal(SIGINT, handle_signal);
    signal(SIGSEGV, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);

    search_path = path_list_new();
    char *cwd = path_get_cwd();
    if (!cwd)
        fprintf(stderr, "current working directory was not determined.");
    else
        path_list_add(search_path, cwd);

    path_list_add(search_path, "~/.local/mila");

    if (argc >= 2 && strcmp(argv[1], "--") != 0)
    {
        FILE *f = fopen(argv[1], "rb");
        if (!f)
        {
            fprintf(stderr, "Cannot open %s: Missing or not a file.\n", argv[1]);
            env_kill(g);
            return 1;
        }

        env_set_raw(g, "argc", vint(argc - 1));
        array = call_function_str(g, "array", vint(argc - 1), NULL);
        for (int i = 1; i < argc; i++)
        {
            Value *str = vstring_dup(argv[i]);
            val_release(call_function_str(g, "array.set", val_retain(array),
                                          vint(i - 1), str, NULL));
        }
        env_set_raw(g, "argv", array);
        env_set_raw(g, "__argv", vopaque(argv));
        free(cwd);

        Value *res = invoke_main_file(argv[1], g, argc, argv);
        if (IS_ERROR(res))
        {
            print_error(res);
        }

        int return_code = 0;

        if (GET_TYPE(res) == T_TAGGED_ERROR)
        {
            switch (res->v->tagged_error.type)
            {
            case E_EXIT:
            {
                if (res->v->tagged_error.return_code == -1)
                {
                    return_code = E_EXIT;
                }
                else
                {
                    return_code = res->v->tagged_error.return_code;
                }
            }
            break;
            default:
                return_code = res->v->tagged_error.type;
                break;
            }
        }

        // cleanup
        val_release(res);

        mila_free(src_text);

        mila_global_deinit(g);
        return return_code;
    }
    else
    {
        free(cwd);
        if (argc > 1 && strcmp(argv[1], "--") == 0)
        {
            array = call_function_str(g, "array", vint(argc - 2), NULL);
            for (int i = 2; i < argc; i++)
            {
                Value *str = vstring_dup(argv[i]);
                val_release(call_function_str(g, "array.set", val_retain(array),
                                              vint(i - 2), str, NULL));
            }
            env_set_raw(g, "argv", array);
        }

        printf("MiLa REPL\n");
        printf("Running MiLa '%s'\n", GET_STRING(env_get(g, "__mila_codename"))
                                          ? GET_STRING(env_get(g, "__mila_codename"))
                                          : "???");

        printf("Edition %ld version %ld (full %ld.%ld.%ld)\n",
               MILA_EDITION,
               MILA_VERSION,
               MILA_EDITION,
               MILA_VERSION,
               MILA_PATCH);

        BlrHistory hist;
        if (blr_history_init(&hist, 100))
        {
            perror("blr_history_init");
            return 1;
        }

        char buffer[2048]; // accumulated snippet
        buffer[0] = 0;

        printf(">>> ");
        fflush(stdout);

        char *line = NULL;
        char *ps = ">>> ";
        while ((line = blr_rec_read(ps, &hist)) != NULL)
        {
            // append line to buffer
            strcat(buffer, line);
            free(line);
            fflush(stdout);

            // debugging
            if (strncmp(buffer, ".mempro", 7) == 0)
            {
                FILE *f = fopen("/proc/self/status", "r");
                char line[256];

                while (fgets(line, sizeof(line), f))
                {
                    if (strncmp(line, "VmRSS:", 6) == 0 ||
                        strncmp(line, "VmData:", 7) == 0 ||
                        strncmp(line, "VmStack:", 8) == 0 ||
                        strncmp(line, "VmExe:", 6) == 0 ||
                        strncmp(line, "VmStk:", 6) == 0)
                    {
                        printf("%s", line);
                    }
                }
                fclose(f);
                printf(">>> ");
                fflush(stdout);
                buffer[0] = 0;
                continue;
            }
            else if (strncmp(buffer, ".load", 5) == 0)
            {
                char *file = path_list_find(search_path, buffer + 6);
                if (!file || load_library_noisy(g, file))
                    printf("Library loading went wrong!\n");
                else
                    mila_free(file);
                printf(">>> ");
                buffer[0] = 0;
                continue;
            }
            else if (strncmp(buffer, ".mem", 4) == 0)
            {
                print_memory_usage();
                buffer[0] = 0;
                printf(">>> ");
                fflush(stdout);
                continue;
            }
            else if (strncmp(buffer, ".quit", 5) == 0)
            {
                break;
            }

            // check if expression is syntactically complete
            if (!needs_more(buffer))
            {
                // evaluate accumulated buffer
                Src *S = src_new(buffer);
                Value *res = eval_source(S, g);

                if (GET_TYPE(res) != T_NULL && !IS_ERROR(res))
                {
                    printf("  : ");
                    raw_print_value_repr(res);
                    putchar('\n');
                }
                else
                {
                    print_error(res);
                    if (IS_ERROR_TAGGED(res) && GET_ERROR_TYPE(res) == E_EXIT)
                    {
                        int code = 0;
                        if (res->v->tagged_error.return_code == -1)
                        {
                            code = E_EXIT;
                        }
                        else
                        {
                            code = res->v->tagged_error.return_code;
                        }
                        src_free(S);
                        val_release(res);
                        blr_history_free(&hist);
                        mila_threads_cleanup();
                        mila_deinit(g);
                        path_list_free(search_path);
                        return code;
                    }
                }

                val_release(res);
                src_free(S);

                // clear buffer
                buffer[0] = 0;

                ps = ">>> ";
                fflush(stdout);
            }
            else
            {
                // prompt for continuation
                ps = "... ";
                fflush(stdout);
            }
        }
        blr_history_free(&hist);
        mila_global_deinit(g);
    }
    return 0;
}
#endif // SAFE_BUILD
