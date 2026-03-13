/*
 * Welcome to the MiLa Language Library
 * Functions for MiLa
 * These are the folliwing
 *   Text IO
 *   File IO
 *   Array methods
 *   Dict methods
 *   String methods
 *   Byte methods (ascii)
 *   Math
 *   Bitwise
 */

#pragma once

#include <math.h>

#include "ml_dict.c"
#include "ml_ll.c"

#ifdef ML_LIB
#define ML_ALREADY
#endif

#define ML_LIB
#ifndef MILA_USE_MILA_C
#include "mila.h"
#else
#include "mila.c"
#endif

#ifndef ML_ALREADY
#undef ML_LIB
#endif

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/time.h>
#include <dlfcn.h>
#include <unistd.h>
#include <limits.h>
#endif

#define MILA_EDITION 202603
#define MILA_VERSION 2
#define MILA_LPREFIX "canon:"

// Define meta tables

MethodTable *file_meta = NULL;
MethodTable *dict_meta = NULL;
MethodTable *list_meta = NULL;
MethodTable *array_meta = NULL;
MethodTable *range_meta = NULL;
MethodTable *self_free_meta = NULL;

Value* self_free(Value* self) {
    val_release(self);
    return NULL;
}

// ---------- Native functions ----------

double get_unix_timestamp()
{
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    // Convert FILETIME (100-ns intervals since 1601) to Unix epoch
    uint64_t t = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    t -= 116444736000000000ULL; // difference between 1601 and 1970
    return t / 1e7;             // convert 100-ns units to seconds
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + tv.tv_usec / 1e6;
#endif
}

// String

Value *vstring_slice(const char *src, size_t start, size_t len)
{
    size_t n = strlen(src);
    if (start > n)
        return vstring_dup(""); // empty string

    if (start + len > n)
        len = n - start;

    char *buf = mila_malloc(len + 1);
    if (!buf)
        return vnull();

    memcpy(buf, src + start, len);
    buf[len] = '\0';

    return vstring_take(buf);
}

Value *vstring_index(const char *src, size_t index)
{
    size_t n = strlen(src);
    if (index >= n)
        return vnull();

    char *buf = mila_malloc(2);
    if (!buf)
        return vnull();

    buf[0] = src[index];
    buf[1] = '\0';

    return vstring_take(buf);
}

Value *vstring_replace(const char *src,
                       const char *needle,
                       const char *repl)
{
    if (!*needle)
        return vstring_dup(src); // can't match empty substring

    size_t src_len = strlen(src);
    size_t n_len = strlen(needle);
    size_t r_len = strlen(repl);

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

        memcpy(out, repl, r_len);
        out += r_len;

        p = match + n_len;
    }

    return vstring_take(buf);
}

char *read_input(void)
{
    size_t bufsize = 64; // initial buffer size
    size_t len = 0;      // number of chars read
    char *buffer = mila_malloc(bufsize);
    if (!buffer)
    {
        fprintf(stderr, "read_input: Allocation failed.\n");
        return NULL;
    }

    int c;
    while ((c = getchar()) != EOF && c != '\n')
    {
        buffer[len++] = (char)c;

        // resize if we're about to overflow
        if (len + 1 >= bufsize)
        {
            bufsize *= 2;
            char *newbuf = realloc(buffer, bufsize);
            if (!newbuf)
            {
                free(buffer);
                fprintf(stderr, "Reallocation failed.\n");
                return NULL;
            }
            buffer = newbuf;
        }
    }

    buffer[len] = '\0';
    return buffer;
}

Value *native_pop_start(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_STRING, T_ARG_END))
        return vnull();
    char *raw_string = argv[0]->v.s;
    char ch = *raw_string; // get first char

    char *copy = strdup(raw_string + 1);

    free(argv[0]->v.s);
    argv[0]->v.s = copy;

    return vstring_dup((char[]){ch, 0});
}

Value *native_pop_end(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_STRING, T_ARG_END))
        return vnull();
    char *raw_string = argv[0]->v.s;
    char ch = *(raw_string + strlen(raw_string) - 1); // get last char

    uint64_t size = strlen(raw_string) - 1;
    char *copy = (char *)mila_malloc(sizeof(char) * size);
    memcpy(copy, raw_string, size);

    free(argv[0]->v.s);
    argv[0]->v.s = copy;

    return vstring_dup((char[]){ch, 0});
}

Value *native_to_ascii(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_INT, T_ARG_END))
        return vnull();
    return vstring_dup((char[]){argv[0]->v.i, '\0'});
}

Value *native_from_ascii(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if ((!match_types(argv, T_STRING, T_ARG_END)) || strlen(argv[0]->v.s) != 1)
        return vnull();
    return vint(argv[0]->v.s[0]);
}

Value *native_str_slice(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_STRING, T_INT, T_INT, T_ARG_END))
        return vnull();
    return vstring_slice(argv[0]->v.s, argv[1]->v.i, argv[2]->v.i);
}

Value *native_str_index(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_STRING, T_INT, T_ARG_END))
        return vnull();
    return vstring_index(argv[0]->v.s, argv[1]->v.i);
}

Value *native_str_patch(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_STRING, T_STRING, T_STRING, T_ARG_END))
        return vnull();
    return vstring_replace(argv[0]->v.s, argv[1]->v.s, argv[2]->v.s);
}

Value *native_str_length(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_STRING, T_ARG_END))
        return vnull();
    return vint(strlen(argv[0]->v.s));
}

Value *native_bitwise_and(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_INT, T_INT, T_ARG_END))
        return vnull();
    return vint(argv[0]->v.i & argv[1]->v.i);
}

Value *native_bitwise_or(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_INT, T_INT, T_ARG_END))
        return vnull();
    return vint(argv[0]->v.i | argv[1]->v.i);
}

Value *native_bitwise_xor(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_INT, T_INT, T_ARG_END))
        return vnull();
    return vint(argv[0]->v.i ^ argv[1]->v.i);
}

Value *native_not(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return vnull();
    return is_truthy(argv[0]) ? vbool(0) : vbool(1);
}

Value *native_print(Env *env, int argc, Value **argv)
{
    (void)env;
    for (int i = 0; i < argc; i++)
    {
        if (i)
            printf(" ");
        print_value(argv[i]);
    }
    return vnull();
}

Value *native_printr(Env *env, int argc, Value **argv)
{
    (void)env;
    for (int i = 0; i < argc; i++)
    {
        print_value(argv[i]);
    }
    return vnull();
}

Value *native_println(Env *env, int argc, Value **argv)
{
    (void)env;
    for (int i = 0; i < argc; i++)
    {
        if (i)
            printf(" ");
        print_value(argv[i]);
    }
    putchar(10);
    return vnull();
}

Value *native_input(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc == 1)
        print_value(argv[0]);
    else if (argc == 0)
        return vnull();
    else
        return verror("input(prompt): Expected 1 argument (prompt) string.\n");

    char *res = read_input();
    return res ? vstring_take(res) : vnull();
}

Value *list_to_iter(Value* self)
{
    Value** iter = ll_to_iter(self->v.opaque);
    return vopaque(iter);
}

Value* list_display(Value* self)
{
    LinkedList* lst = (LinkedList*)self->v.opaque;
    Value** iter = ll_to_iter(lst);
    char* buffer = strdup("list(");
    for (int i = 0; iter[i]; i++) {
        char* repr = as_c_string_repr(iter[i]);
        if (i < lst->size-1) our_asprintf(&buffer, "%s, ", repr);
        else our_asprintf(&buffer, "%s", repr);
    }
    our_asprintf(&buffer, ")");
    return vstring_take(buffer);
}

Value *native_list_new(Env *e, int argc, Value **argv)
{
    (void)e;
    LinkedList* list = ll_create();
    for (int i=0; i < argc; i++) {
        ll_append(list, argv[i]);
    }
    Value* res = vopaque_extra(list, NULL, MILA_LPREFIX "list");
    val_set_table(res, list_meta);
    return res;
}

Value* list_free(Value* self)
{
    Value** iter = ll_to_iter(self->v.opaque);
    
    for (int i=0; iter[i]; i++) {
        val_release(iter[i]);
    }
    
    free(iter);
    ll_free(self->v.opaque);
    self->type = T_NULL;
    self->v.opaque = NULL;
    return vnull();
}

Value *native_list_free(Env* e, int argc, Value** argv)
{
    (void)e;
    if (argc != 1 || argv[0]->type != T_OPAQUE)
        verror("list.free(lst): List opaque pointer is needed.");
    Value** iter = ll_to_iter(argv[0]->v.opaque);
    
    for (int i=0; iter[i]; i++) {
        val_release(iter[i]);
    }
    
    free(iter);
    ll_free(argv[0]->v.opaque);
    argv[0]->type = T_NULL;
    argv[0]->v.opaque = NULL;
    return vnull();
}

Value *native_cast_int(Env *env, int argc, Value **argv)
{
    (void)env;
    long i = 0;
    if (argc == 1 && argv[0]->type == T_STRING)
    {
        char *end;
        i = strtol(argv[0]->v.s, &end, 10);

        if (*end != '\0')
        {
            char *buffer = NULL;
            our_asprintf(&buffer, "cast.int(str): Got bad part \"%s\"...", end);
            return verror("%s\n", buffer);
            free(buffer);
            i = 0;
        }
    }
    else
    {
        return verror("cast.int(str): Expected 1 argument (str) string.\n");
        i = 0;
    }
    return vint(i);
}

Value *native_cast_float(Env *env, int argc, Value **argv)
{
    (void)env;
    double f = 0;
    if (argc == 1 && argv[0]->type == T_STRING)
    {
        char *end;
        f = strtod(argv[0]->v.s, &end);

        if (*end != '\0')
        {
            char *buffer = NULL;
            our_asprintf(&buffer, "cast.float(str): Got bad part \"%s\"...", end);
            return verror("%s\n", buffer);
            free(buffer);
            f = 0;
        }
    }
    else
    {
        return verror("cast.float(str): Expected 1 argument (str) string.\n");
        f = 0;
    }
    return vfloat(f);
}

Value *native_cast_int_to_float(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc == 1 && argv[0]->type == T_INT)
    {
        return vfloat(to_double(argv[0]));
    }
    else
    {
        return verror("cast.i2f(int): Expected 1 argument (int) int.\n");
    }
}

Value *native_cast_float_to_int(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc == 1 && argv[0]->type == T_FLOAT)
    {
        return vint((long)argv[0]->v.f);
    }
    else
    {
        return verror("cast.f2i(int): Expected 1 argument (float) float.\n");
    }
}

Value *native_cast_string(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc == 1)
    {
        return vstring_take(as_c_string(argv[0]));
    }
    else
    {
        return verror("cast.string(any): Expected 1 argument (any) any.\n");
    }
    return vnull();
}

Value *native_type_of(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
    {
        return verror("typeof(any): Expected 1 argument (any) any.\n");
    }
    if (argv[0]->type_name)
        return vstring_dup(argv[0]->type_name);
    return vstring_dup(MILA_GET_TYPENAME(argv[0]));
}

Value *file_printer(Value *self)
{
    char *buffer = NULL;
    if (!self || self->type != T_OPAQUE)
    {
        our_asprintf(&buffer, "<not-a-file>");
        return vstring_take(buffer);
    }
    FILE *f = (FILE *)self->v.opaque;
    if (!f)
    {
        our_asprintf(&buffer, "<file:closed>");
    }
    else
    {
        our_asprintf(&buffer, "<file:%p>", f);
    }
    return vstring_take(buffer);
}

Value *native_open(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2 || argv[0]->type != T_STRING || argv[1]->type != T_STRING)
    {
        return verror("= open(filename, mode) expects 2 string args.\n");
    }
    char *path = argv[0]->v.s;
    if (!search_path)
    {
        char *path = path_list_find(search_path, argv[0]->v.s);
        if (!path)
        {
            return verror("= open(filename, mode) did not find the file.\n");
        }
    }
    char *res = path_list_find(search_path, path);
    if (!res)
        return verror("File %s not found!", path);

    FILE *f = fopen(res, argv[1]->v.s);
    if (!f)
    {
        if (res)
            free(res);
        perror(NULL);
        return vnull();
    }

    if (res)
        free(res);
    Value *v = vopaque(f);
    val_set_table(v, file_meta);
    return v;
}

Value *native_fclose(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || argv[0]->type != T_OPAQUE)
    {
        return verror("= fclose(file) expects 1 file handle arg.\n");
    }
    FILE *f = (FILE *)argv[0]->v.opaque;
    if (f)
    {
        fclose(f);
        argv[0]->v.opaque = NULL; // Prevent double close
    }
    return vnull();
}

Value *native_fflush(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || argv[0]->type != T_OPAQUE)
    {
        return verror("= fflush(file) expects 1 file handle arg.\n");
    }
    FILE *f = (FILE *)argv[0]->v.opaque;
    if (f)
    {
        fflush(f);
        argv[0]->v.opaque = NULL; // Prevent double close
    }
    return vnull();
}

Value *native_fprint(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2 || argv[0]->type != T_OPAQUE || argv[1]->type != T_STRING)
    {
        return verror("= fprint(file, string) expects (handle, string).\n");
    }
    FILE *f = (FILE *)argv[0]->v.opaque;
    if (!f)
    {
        return verror("= fprint: file handle is closed or invalid.\n");
    }
    const char *s = argv[1]->v.s;
    size_t written = fwrite(s, 1, strlen(s), f);
    return vint(written);
}

Value *native_fread(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2 || argv[0]->type != T_OPAQUE || argv[1]->type != T_INT)
    {
        return verror("= fread(file, num_bytes) expects (handle, int).\n");
    }
    FILE *f = (FILE *)argv[0]->v.opaque;
    if (!f)
    {
        return verror("= fread: file handle is closed or invalid.\n");
    }
    long n = argv[1]->v.i;
    if (n <= 0)
        return vstring_dup("");

    char *buf = mila_malloc(n + 1);
    if (!buf)
        return vnull();

    size_t read_bytes = fread(buf, 1, n, f);
    buf[read_bytes] = '\0';

    return vstring_take(buf);
}

Value *native_fseek(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 3 || argv[0]->type != T_OPAQUE || argv[1]->type != T_INT || argv[2]->type != T_INT)
    {
        return verror("= fseek(file, offset, whence) expects (handle, int, int).\n");
    }
    FILE *f = (FILE *)argv[0]->v.opaque;
    if (!f)
    {
        return verror("= fseek: file handle is closed or invalid.\n");
    }
    long offset = argv[1]->v.i;
    int whence = (int)argv[2]->v.i;
    int c_whence;

    switch (whence)
    {
    case 0:
    case 1:
    case 2:
        c_whence = whence;
        break;
    default:
        return verror("= fseek: invalid whence %d (must be 0-SEEK_SET, 1-SEEK_CUR, or 2-SEEK_END).\n", whence);
    }

    int res = fseek(f, offset, c_whence);
    return vint(res);
}

Value *native_ftell(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || argv[0]->type != T_OPAQUE)
    {
        return verror("= ftell(file) expects 1 file handle arg.\n");
    }
    FILE *f = (FILE *)argv[0]->v.opaque;
    if (!f)
    {
        return verror("ftell: file handle is closed or invalid.\n");
    }
    long pos = ftell(f);
    return vint(pos);
}

/* Array Functions: store Value* per slot so we keep proper refcounts */
typedef struct
{
    Value **array; /* array of Value* */
    int size;
} Array;

Value *array_to_str(Value *self)
{
    char *buffer = NULL;
    if (!self || self->type != T_OPAQUE)
    {
        our_asprintf(&buffer, "<not-an-array>");
        return vstring_take(buffer);
    }

    Array *arr = (Array *)self->v.opaque;
    if (!arr)
    {
        our_asprintf(&buffer, "<null-array-data>");
        return vstring_take(buffer);
    }

    our_asprintf(&buffer, "array.from(");
    for (int i = 0; i < arr->size; i++)
    {
        Value *slot = arr->array[i];
        if (!slot)
        {
            our_asprintf(&buffer, "?null?");
        }
        else
        {
            char *s = as_c_string_repr(slot);
            our_asprintf(&buffer, "%s", s);
            free(s);
        }
        if (i < arr->size - 1)
            our_asprintf(&buffer, ", ");
    }
    our_asprintf(&buffer, ")");
    return vstring_take(buffer);
}

Value *array_to_iter(Value *self)
{
    Value *arrv = self;
    if (arrv->type != T_OPAQUE)
    {
        return verror("array<UMethodToIter>: first arg must be an array (opaque)");
    }

    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array<UMethodToIter>: null array data");
    }

    Value **values = (Value **)mila_malloc(sizeof(Value *) * (arr->size + 1));
    values[arr->size] = NULL;

    int i = 0;
    for (int t = 0; t < arr->size; ++t)
    {
        if (arr->array[t] == NULL)
            continue;
        values[i++] = val_retain(arr->array[t]);
    }

    return vopaque(values);
}

typedef struct
{
    long start;
    long end;
    long step;
} Range;

size_t range_len(long start, long stop, long step)
{
    if (step == 0)
        return 0;
    if (step > 0)
    {
        if (start >= stop)
            return 0;
        return (stop - start + step - 1) / step;
    }
    else
    {
        if (start <= stop)
            return 0;
        return (start - stop - step - 1) / (-step);
    }
}

Value *range_to_iter(Value *self)
{
    Range *data = (Range *)(self->v.opaque);
    Value **v = (Value **)mila_malloc(sizeof(Value *) * (range_len(data->start, data->end, data->step) + 1));
    long index = 0;
    for (long i = 0; i < data->end; i += data->step)
    {
        Value* n = vint(i);
        v[index++] = n;
    }
    v[index] = NULL;
    return vopaque(v);
}

Value* range_free(Value* self) {
    free(self->v.opaque);
    return NULL;
}

Value *native_range(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc == 1 && argv[0]->type == T_INT)
    {
        Range *r = (Range *)mila_malloc(sizeof(Range));
        r->start = 0;
        r->end = argv[0]->v.i;
        r->step = 1;
        Value *res = vopaque_extra(r, NULL, MILA_LPREFIX "range");
        val_set_table(res, range_meta);
        return res;
    }
    if (argc == 2 && argv[0]->type == T_INT && argv[1]->type == T_INT)
    {
        Range *r = (Range *)mila_malloc(sizeof(Range));
        r->start = argv[0]->v.i;
        r->end = argv[1]->v.i;
        r->step = 1;
        Value *res = vopaque_extra(r, NULL, MILA_LPREFIX "range");
        val_set_table(res, range_meta);
        return res;
    }
    if (argc == 3 && argv[0]->type == T_INT && argv[1]->type == T_INT && argv[2]->type == T_INT)
    {
        Range *r = (Range *)mila_malloc(sizeof(Range));
        r->start = argv[0]->v.i;
        r->end = argv[1]->v.i;
        r->step = argv[2]->v.i;
        Value *res = vopaque_extra(r, NULL, MILA_LPREFIX "range");
        val_set_table(res, range_meta);
        return res;
    }
    return vnull();
}

Value *native_new_array(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
    {
        return verror("array(size): Requires one argument, array size (int)\n");
    }
    if (!match_types(argv, T_INT, T_ARG_END))
    {
        return verror("array(size): Expected the argument type int\n");
    }

    int size = (int)argv[0]->v.i;
    if (size < 0)
    {
        return verror("array(size): negative size\n");
    }

    Value *res = val_new(T_OPAQUE);
    Array *array = mila_malloc(sizeof(Array));
    array->size = size;
    array->array = mila_malloc(sizeof(Value *) * size);

    for (int i = 0; i < size; i++)
    {
        array->array[i] = NULL;
    }

    res->v.opaque = array;
    res->type_name = strdup(MILA_LPREFIX "array");
    val_set_table(res, array_meta);
    return res;
}

Value *native_from_array(Env *env, int argc, Value **argv)
{
    (void)env;

    int size = argc;

    Value *res = val_new(T_OPAQUE);
    Array *array = mila_malloc(sizeof(Array));
    array->size = size;
    array->array = mila_malloc(sizeof(Value *) * size);

    for (int i = 0; i < size; i++)
    {
        array->array[i] = val_retain(argv[i]);
    }

    res->v.opaque = array;
    res->type_name = strdup(MILA_LPREFIX "array");
    val_set_table(res, array_meta);
    return res;
}

Value *native_set_array(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 3)
    {
        return verror("array.set(array, index, value): requires 3 args");
    }

    Value *arrv = argv[0];
    if (arrv->type != T_OPAQUE)
    {
        return verror("array.set(array, index, value): first arg must be an array (opaque)");
    }

    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array.set(array, index, value): null array data");
    }

    if (argv[1]->type != T_INT)
    {
        return verror("array.set(array, index, value): index must be int");
    }

    int idx = (int)argv[1]->v.i;
    if (idx < 0 || idx >= arr->size)
    {
        return verror("array.set(array, index, value): index %d out of bounds (size %d)", idx, arr->size);
    }

    Value *old = arr->array[idx];
    if (old)
        val_release(old);

    arr->array[idx] = argv[2];
    val_retain(argv[2]);

    return vnull();
}

Value *native_get_array(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2)
    {
        return verror("array.get(array, index): requires 2 args");
    }

    Value *arrv = argv[0];
    if (arrv->type != T_OPAQUE)
    {
        return verror("array.get(array, index): first arg must be an array (opaque)");
    }

    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array.get(array, index): null array data");
    }

    if (argv[1]->type != T_INT)
    {
        return verror("array.get(array, index): index must be int");
    }

    int idx = (int)argv[1]->v.i;
    if (idx < 0 || idx >= arr->size)
    {
        return verror("array.get(array, index): index %d out of bounds (size %d)", idx, arr->size);
    }

    Value *val = arr->array[idx];
    if (val)
        val_retain(val);
    else
        return vnull();

    return val;
}

Value *native_len_array(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
    {
        return verror("array.len(array): requires 1 arg");
    }

    Value *arrv = argv[0];
    if (arrv->type != T_OPAQUE)
    {
        return verror("array.len(array): first arg must be an array (opaque)");
    }

    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array.len(array): null array data");
    }

    return vint(arr->size);
}

Value *get_array(Value* self, Value* index)
{

    Value *arrv = self;
    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array.get(array, index): null array data");
    }

    if (index->type != T_INT)
    {
        return verror("array.get(array, index): index must be int");
    }

    int idx = (int)index->v.i;
    if (idx < 0 || idx >= arr->size)
    {
        return verror("array.get(array, index): index %d out of bounds (size %d)", idx, arr->size);
    }

    Value *val = arr->array[idx];
    if (val)
        val_retain(val);
    else
        return vnull();

    return val;
}

Value *set_array(Value* self, Value* index, Value* val)
{

    Value *arrv = self;
    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array.set(array, index, value): null array data");
    }

    if (index->type != T_INT)
    {
        return verror("array.set(array, index, value): index must be int");
    }

    int idx = (int)index->v.i;
    if (idx < 0 || idx >= arr->size)
    {
        return verror("array.set(array, index, value): index %d out of bounds (size %d)", idx, arr->size);
    }

    Value *old = arr->array[idx];
    if (old)
        val_release(old);
    arr->array[idx] = val;

    return vnull();
}

Value *native_free_array(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
    {
        return verror("array.free(array): requires 1 arg");
    }

    Value *arrv = argv[0];
    if (arrv->type != T_OPAQUE)
    {
        return verror("array.free(array): first arg must be an array (opaque)");
    }

    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array.free(array): null array data");
    }

    for (int i = 0; i < arr->size; i++)
        if (arr->array[i])
            val_release(arr->array[i]);
    free(arr->array);
    free(arr);

    return vnull();
}

Value *free_array(Value *self)
{
    Value *arrv = self;
    if (arrv->type != T_OPAQUE)
    {
        return NULL;
    }

    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return NULL;
    }

    for (int i = 0; i < arr->size; i++)
        if (arr->array[i])
            val_release(arr->array[i]);
    free(arr->array);
    free(arr);

    return NULL;
}

Value *native_report(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc == 1 && argv[0]->type == T_STRING)
        return verror("report(message): %s\n", argv[0]->v.s);
    else if (argc == 0)
        return verror("report(message) - No details given.");
    else
        return verror("report(message): Invalid number of arguments given.");
    return vnull();
}

Value *native_exit(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc == 1 && argv[0]->type == T_INT)
        exit((int)argv[0]->v.i);
    else if (argc == 0)
        exit(0);
    else
    {
        return verror("invalid number of arguments given.");
    }
    return vnull();
}

Value *native_get_time(Env *env, int argc, Value **argv)
{
    (void)argc;
    (void)argv;
    (void)env;
    if (argc != 0)
    {
        return verror("invalid number of arguments given.\n");
    }
    return vfloat(get_unix_timestamp());
}

Value *native_run(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || argv[0]->type != T_STRING)
    {
        return verror("invalid number of arguments given or incorrect types.");
    }

    if (search_path)
    {
        char *path = path_list_find(search_path, argv[0]->v.s);
        if (!path)
        {
            return verror("run(filename) did not find the file.");
        }
        if (run_file(path, env))
        {
            return verror("problem running file %s", path);
        }
        free(path);
    }

    return vnull();
}

Value *native_load(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || argv[0]->type != T_STRING)
    {
        return verror("invalid number of arguments given or incorrect types.");
    }

    if (load_library(env, argv[0]->v.s))
        return verror("problem loading file %s\n", argv[0]->v.s);

    return vnull();
}

Value *native_eval(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || argv[0]->type != T_STRING)
    {
        return verror("invalid number of arguments given or incorrect types.");
    }

    return eval_str(argv[0]->v.s, env);
}

Value *native_new_dict(Env *env, int argc, Value **argv)
{
    (void)env;
    if (((argc % 2) != 0) && argc != 0)
    {
        return verror("dict(...): Provide even number of arguments or none at all! Got %i.", argc);
    }

    Dict *d = dict_create();

    for (int i = 0; i < argc;)
    {
        Value *key = argv[i++];
        Value *value = argv[i++];
        dict_set(d, key, value);
    }

    if (d)
    {
        Value *v = vopaque(d);
        val_set_table(v, dict_meta);
        return val_retain(v);
    }
    return verror("couldnt make a dict.");
}

Value *native_set_dict(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 3)
    {
        return verror("invalid number of arguments given or incorrect types.");
    }

    dict_set(argv[0]->v.opaque, argv[1], val_retain(argv[2]));
    return vnull();
}

Value *native_get_dict(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2)
    {
        return verror("invalid number of arguments given or incorrect types.");
    }

    Value *v = dict_get(argv[0]->v.opaque, argv[1]);
    return v ? v : vnull();
}

Value *set_dict(Value* self, Value* name, Value* val)
{
    dict_set(self->v.opaque, name, val);
    return vnull();
}

Value *get_dict(Value* self, Value* name)
{
    Value *v = dict_get(self->v.opaque, name);
    return v;
}

Value *native_rem_dict(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2)
    {
        return verror("invalid number of arguments given or incorrect types.");
    }

    dict_remove(argv[0]->v.opaque, argv[1]);
    return vnull();
}

Value* free_dict(Value* self)
{
    dict_free(self->v.opaque);
    return NULL;
}

Value *native_free_dict(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
    {
        return verror("invalid number of arguments given or incorrect types.");
    }

    dict_free(argv[0]->v.opaque);
    return vnull();
}

Value *native_system(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || argv[0]->type != T_STRING)
    {
        return verror("invalid number of arguments given or incorrect types.");
    }
    return vint(system(argv[0]->v.opaque));
}

Value *native_floor(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("floor(i): Requires one arguments");
    double x = argv[0]->v.f;
    return vfloat(floor(x));
}

Value *native_ceil(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("ceil(i): Requires one arguments");
    double x = argv[0]->v.f;
    return vfloat(ceil(x));
}

Value *native_sqrt(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("sqrt(i): Requires one arguments");
    double x = argv[0]->v.f;
    return vfloat(sqrt(x));
}

Value *native_sin(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("sin(i): Requires one arguments");
    double x = argv[0]->v.f;
    return vfloat(sin(x));
}

Value *native_cos(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("cos(i): Requires one arguments");
    double x = argv[0]->v.f;
    return vfloat(cos(x));
}

Value *native_tan(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("tan(i): Requires one arguments");
    double x = argv[0]->v.f;
    return vfloat(tan(x));
}

Value *native_atan2(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2)
        return verror("atan2(i, i): Requires two arguments");
    double y = argv[0]->v.f;
    double x = argv[1]->v.f;
    return vfloat(atan2(y, x));
}

Value *native_pow(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2)
        return verror("pow(base, exp): Requires two arguments");
    return vint(pow(argv[0]->v.i, argv[1]->v.i));
}

Value *native_vars_set(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2)
        return verror("vars.set(name, val): Requires two arguments");
    env_set_local(env, argv[0]->v.s, argv[1]);
    return vnull();
}

Value *native_vars_get(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("vars.get(name): Requires one argument");
    return env_get(env, argv[0]->v.s);
}

Value *native_vfree(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("vfree(value): Requires one argument");
    val_release(argv[0]);
    return vnull();
}

Value *native_vars_local(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argv;
    if (argc != 0)
        return verror("vars.local(): Requires no arguments");
    for (Var *v = env->vars; v; v = v->next)
    {
        printf("%s", v->name);
        if (v->next)
        {
            printf(", ");
        }
    }
    putchar(10);
    return vnull();
}

Value *native_vars_global(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argv;
    if (argc != 0)
        return verror("vars.local(): Requires no arguments");
    for (Env *cur = env; cur; cur = cur->parent)
    {
        for (Var *v = cur->vars; v; v = v->next)
        {
            printf("%s", v->name);
            if (v->next)
            {
                printf(", ");
            }
        }
    }
    putchar(10);
    return vnull();
}

// Minimal MiLa Builtins
// We do not support objects,
// the dots are namespaces.
// And yes if you remove this file it wont trash the interpreter.
void env_register_builtins(Env *g)
{
    // === Setup
    // canonical builtins reports edition (2026 march)
    env_set_raw(g, "__mila_canonical_builtins", vint(202603L));
    // canonical builtins version reports actual version (integer, any changes means ver++)
    env_set_raw(g, "__mila_canonical_builtins_version", vint(1));
    // tell users what implementation it is
    // heres its canon since this is the base implementation.
    env_set_raw(g, "__mila_codename", vstring_dup("canon"));
 
    file_meta = val_make_table();
    
    val_set_method_table(file_meta, UMethodToString, file_printer);
    
    dict_meta = val_make_table();
    
    val_set_method_table(dict_meta, UMethodToString, dict_display);
    val_set_method_table(dict_meta, UMethodFree, free_dict);
    val_set_method_table(dict_meta, BMethodGetItem, get_dict);
    val_set_method_table(dict_meta, TMethodSetItem, set_dict);
    
    list_meta = val_make_table();

    val_set_method_table(list_meta, UMethodToString, list_display);
    val_set_method_table(list_meta, UMethodFree, list_free);
    
    array_meta = val_make_table();
    
    val_set_method_table(array_meta, UMethodToIter, array_to_iter);
    val_set_method_table(array_meta, UMethodToString, array_to_str);
    val_set_method_table(array_meta, UMethodFree, free_array);
    val_set_method_table(array_meta, BMethodGetItem, get_array);
    val_set_method_table(array_meta, TMethodSetItem, set_array);
    
    range_meta = val_make_table();
    
    val_set_method_table(range_meta, UMethodToIter, range_to_iter);
    val_set_method_table(range_meta, UMethodFree, range_free);

    self_free_meta = val_make_table();

    val_set_method_table(self_free_meta, UMethodFree, self_free);

    // === Misc
    env_register_native(g, "range", native_range);
    env_register_native(g, "vfree", native_vfree);
    // === Text IO
    env_register_native(g, "print", native_print);
    env_register_native(g, "printr", native_printr);
    env_register_native(g, "println", native_println);
    env_register_native(g, "input", native_input);
    // === Logic
    env_register_native(g, "and", native_bitwise_and);
    env_register_native(g, "or", native_bitwise_or);
    env_register_native(g, "xor", native_bitwise_xor);
    env_register_native(g, "not", native_not);
    // === File IO
    env_register_native(g, "open", native_open);
    env_register_native(g, "fclose", native_fclose);
    env_register_native(g, "fprint", native_fprint);
    env_register_native(g, "fread", native_fread);
    env_register_native(g, "fseek", native_fseek);
    env_register_native(g, "ftell", native_ftell);
    env_register_native(g, "fflush", native_fflush);
    env_set_raw(g, "SEEK_SET", vint(SEEK_SET));
    env_set_raw(g, "SEEK_END", vint(SEEK_END));
    env_set_raw(g, "SEEK_CUR", vint(SEEK_CUR));
    env_set_raw(g, "stderr", vopaque_extra(stderr, NULL, "'stderr fd'"));
    env_set_raw(g, "stdout", vopaque_extra(stdout, NULL, "'stdout fd'"));
    // === Lists
    env_register_native(g, "list", native_list_new);
    env_register_native(g, "list.free", native_list_free);
    // === Array
    env_register_native(g, "array", native_new_array);
    env_register_native(g, "array.from", native_from_array);
    env_register_native(g, "array.set", native_set_array);
    env_register_native(g, "array.get", native_get_array);
    env_register_native(g, "array.len", native_len_array);
    env_register_native(g, "array.free", native_free_array);
    // === Dicts
    env_register_native(g, "dict", native_new_dict);
    env_register_native(g, "dict.set", native_set_dict);
    env_register_native(g, "dict.get", native_get_dict);
    env_register_native(g, "dict.rem", native_rem_dict);
    env_register_native(g, "dict.free", native_free_dict);
    // === Casting
    env_register_native(g, "cast.int", native_cast_int);
    env_register_native(g, "cast.float", native_cast_float);
    env_register_native(g, "cast.string", native_cast_string);
    env_register_native(g, "cast.i2f", native_cast_int_to_float);
    env_register_native(g, "cast.f2i", native_cast_float_to_int);
    env_register_native(g, "typeof", native_type_of);
    // === String
    env_register_native(g, "str.slice", native_str_slice);
    env_register_native(g, "str.index", native_str_index);
    env_register_native(g, "str.patch", native_str_patch);
    env_register_native(g, "str.length", native_str_length);
    env_register_native(g, "str.pop_f", native_pop_start);
    env_register_native(g, "str.pop_b", native_pop_end);
    // === ASCII
    env_register_native(g, "ascii.from", native_from_ascii);
    env_register_native(g, "ascii.to", native_to_ascii);
    // === Math
    env_register_native(g, "floor", native_floor);
    env_register_native(g, "ceil", native_ceil);
    env_register_native(g, "sqrt", native_sqrt);
    env_register_native(g, "sin", native_sin);
    env_register_native(g, "cos", native_cos);
    env_register_native(g, "tan", native_tan);
    env_register_native(g, "atan2", native_atan2);
    env_register_native(g, "pow", native_pow);
    // === Env
    env_register_native(g, "vars.set", native_vars_set);
    env_register_native(g, "vars.get", native_vars_get);
    env_register_native(g, "vars.local", native_vars_local);
    env_register_native(g, "vars.global", native_vars_global);

    /*
     * _typeof differentiates between native and non native functions
     * this is for very specific use cases
     */
    // === Error handling
    env_register_native(g, "report", native_report);
    env_register_native(g, "exit", native_exit);
    // === Time measurement
    env_register_native(g, "get_time", native_get_time);
    // === OS Stuff
    env_register_native(g, "system", native_system);
    // === Modules
    env_register_native(g, "run", native_run);   // runs file
    env_register_native(g, "load", native_load); // loads dlls or so file
    env_register_native(g, "eval", native_eval); // runs string
}

void _mila_lib_init(Env *e)
{
    env_register_builtins(e);
}