/*
 * Builtins for MiLa
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

#include "ml_dict.c"

#ifdef ML_LIB
#define ML_ALREADY
#endif

#define ML_LIB
#include "ml.h"

#ifndef ML_ALREADY
#undef ML_LIB
#endif

// DLL/SO Support

int load_library(Env *env, const char *libpath)
{
#ifdef _WIN32
    HMODULE lib = LoadLibraryA(libpath);
    if (!lib)
    {
        fprintf(stderr, "LoadLibraryA('%s') failed (err=%lu)\n", libpath, GetLastError());
        return -1;
    }

    const char *const *names =
        (const char *const *)GetProcAddress(lib, "lib_functions");

    if (!names)
    {
        fprintf(stderr, "Symbol 'lib_functions' not found\n");
        FreeLibrary(lib);
        return -2;
    }

    for (size_t i = 0; names[i] != NULL; i++)
    {
        FARPROC f = GetProcAddress(lib, names[i]);
        if (!f)
        {
            fprintf(stderr, "Warning: function '%s' not found in '%s'\n",
                    names[i], libpath);
            continue;
        }
        env_register_native(env, names[i], (void *)f);
    }

    return 0;

#else // POSIX
    void *lib = dlopen(libpath, RTLD_LAZY);
    if (!lib)
    {
        fprintf(stderr, "dlopen('%s') failed: %s\n", libpath, dlerror());
        return -1;
    }

    dlerror();
    const char *const *names =
        (const char *const *)dlsym(lib, "lib_functions");

    const char *err = dlerror();
    if (err)
    {
        fprintf(stderr, "dlsym 'lib_functions' error: %s\n", err);
        dlclose(lib);
        return -2;
    }

    for (size_t i = 0; names[i] != NULL; i++)
    {
        dlerror();
        void *f = dlsym(lib, names[i]);
        const char *err2 = dlerror();
        if (err2)
        {
            fprintf(stderr, "Warning: '%s' not found in '%s'\n",
                    names[i], libpath);
            continue;
        }

        env_register_native(env, names[i], f);
    }

    return 0;
#endif
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

    char *buf = malloc(len + 1);
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

    char *buf = malloc(2);
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
    char *buf = malloc(new_len + 1);
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
    char *buffer = malloc(bufsize);
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
    if (!match_types(argv, T_STRING, T_ARG_END))
        return vnull();
    char *raw_string = argv[0]->v.s;
    char ch = *(raw_string + strlen(raw_string) - 1); // get last char

    uint64_t size = strlen(raw_string) - 1;
    char *copy = (char *)malloc(sizeof(char) * size);
    memcpy(copy, raw_string, size);

    free(argv[0]->v.s);
    argv[0]->v.s = copy;

    return vstring_dup((char[]){ch, 0});
}

Value *native_to_ascii(Env *env, int argc, Value **argv)
{
    if (!match_types(argv, T_INT, T_ARG_END))
        return vnull();
    return vstring_dup((char[]){argv[0]->v.i, '\0'});
}

Value *native_from_ascii(Env *env, int argc, Value **argv)
{
    if ((!match_types(argv, T_STRING, T_ARG_END)) || strlen(argv[0]->v.s) != 1)
        return vnull();
    return vint(argv[0]->v.s[0]);
}

Value *native_str_slice(Env *env, int argc, Value **argv)
{
    if (!match_types(argv, T_STRING, T_INT, T_INT, T_ARG_END))
        return vnull();
    return vstring_slice(argv[0]->v.s, argv[1]->v.i, argv[2]->v.i);
}

Value *native_str_index(Env *env, int argc, Value **argv)
{
    if (!match_types(argv, T_STRING, T_INT, T_ARG_END))
        return vnull();
    return vstring_index(argv[0]->v.s, argv[1]->v.i);
}

Value *native_str_patch(Env *env, int argc, Value **argv)
{
    if (!match_types(argv, T_STRING, T_STRING, T_STRING, T_ARG_END))
        return vnull();
    return vstring_replace(argv[0]->v.s, argv[1]->v.s, argv[2]->v.s);
}

Value *native_str_length(Env *env, int argc, Value **argv)
{
    if (!match_types(argv, T_STRING, T_ARG_END))
        return vnull();
    return vint(strlen(argv[0]->v.s));
}

Value *native_print(Env *env, int argc, Value **argv)
{
    for (int i = 0; i < argc; i++)
    {
        if (i)
            printf(" ");
        print_value(argv[i]);
    }
    return vnull();
}

Value *native_bitwise_and(Env *env, int argc, Value **argv)
{
    if (!match_types(argv, T_INT, T_INT, T_ARG_END))
        return vnull();
    return vint(argv[0]->v.i & argv[1]->v.i);
}

Value *native_bitwise_or(Env *env, int argc, Value **argv)
{
    if (!match_types(argv, T_INT, T_INT, T_ARG_END))
        return vnull();
    return vint(argv[0]->v.i | argv[1]->v.i);
}

Value *native_bitwise_xor(Env *env, int argc, Value **argv)
{
    if (!match_types(argv, T_INT, T_INT, T_ARG_END))
        return vnull();
    return vint(argv[0]->v.i ^ argv[1]->v.i);
}

Value *native_not(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return vnull();
    return is_truthy(argv[0]) ? vbool(0) : vbool(1);
}

Value *native_printr(Env *env, int argc, Value **argv)
{
    for (int i = 0; i < argc; i++)
    {
        print_value(argv[i]);
    }
    return vnull();
}

Value *native_println(Env *env, int argc, Value **argv)
{
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
    if (argc == 1)
        print_value(argv[0]);
    else if (argc == 0)
        return vnull();
    else
    {
        return verror("input(prompt): Expected 1 argument (prompt) string.\n");
        
    }

    char *res = read_input();
    return res ? vstring_take(res) : vnull();
}

Value *native_cast_int(Env *env, int argc, Value **argv)
{
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

Value *native_cast_string(Env *env, int argc, Value **argv)
{
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
    if (argc != 1)
    {
        return verror("typeof(any): Expected 1 argument (any) any.\n");
        
    }
    if (*argv[0]->type_name)
        return vstring_dup(argv[0]->type_name);
    switch (argv[0]->type)
    {
    case T_STRING:
        return vstring_dup("string");
    case T_INT:
        return vstring_dup("int");
    case T_FLOAT:
        return vstring_dup("float");
    case T_OPAQUE:
        return vstring_dup("opaque");
    case T_FUNCTION:
    case T_NATIVE:
        return vstring_dup("function");
    case T_BOOL:
        return vstring_dup("bool");
    case T_NULL:
        return vstring_dup("null");
    case T_NONE:
        return vstring_dup("none");
    default:
        return vstring_dup("unknown");
    }
}

Value *native_xtype_of(Env *env, int argc, Value **argv)
{
    if (argc != 1)
    {
        return verror("_typeof(any): Expected 1 argument (any) any.\n");
        
    }
    if (*argv[0]->type_name)
        return vstring_dup(argv[0]->type_name);
    switch (argv[0]->type)
    {
    case T_STRING:
        return vstring_dup("string");
    case T_INT:
        return vstring_dup("int");
    case T_FLOAT:
        return vstring_dup("float");
    case T_OPAQUE:
        return vstring_dup("opaque");
    case T_FUNCTION:
        return vstring_dup("function");
    case T_NATIVE:
        return vstring_dup("native");
    case T_BOOL:
        return vstring_dup("bool");
    case T_NULL:
        return vstring_dup("null");
    case T_NONE:
        return vstring_dup("none");
    default:
        return vstring_dup("unknown");
    }
}

char *file_printer(Value *self)
{
    char *buffer = NULL;
    if (!self || self->type != T_OPAQUE)
    {
        our_asprintf(&buffer, "<not-a-file>");
        return buffer;
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
    return buffer;
}

Value *native_open(Env *env, int argc, Value **argv)
{
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
    FILE *f = fopen(path, argv[1]->v.s);
    if (!f)
    {
        return vnull();
    }
    Value *v = vopaque(f);
    v->display = file_printer;
    return v;
}

Value *native_fclose(Env *env, int argc, Value **argv)
{
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

Value *native_fprint(Env *env, int argc, Value **argv)
{
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

    char *buf = malloc(n + 1);
    if (!buf)
        return vnull();

    size_t read_bytes = fread(buf, 1, n, f);
    buf[read_bytes] = '\0';

    return vstring_take(buf);
}

Value *native_fseek(Env *env, int argc, Value **argv)
{
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

char *array_printer(Value *self)
{
    char *buffer = NULL;
    if (!self || self->type != T_OPAQUE)
    {
        our_asprintf(&buffer, "<not-an-array>");
        return buffer;
    }

    Array *arr = (Array *)self->v.opaque;
    if (!arr)
    {
        our_asprintf(&buffer, "<null-array-data>");
        return buffer;
    }

    our_asprintf(&buffer, "Array{");
    for (int i = 0; i < arr->size; i++)
    {
        Value *slot = arr->array[i];
        if (!slot)
        {
            our_asprintf(&buffer, "c-null");
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
    our_asprintf(&buffer, "}");
    return buffer;
}

Value *native_new_array(Env *env, int argc, Value **argv)
{
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
    Array *array = malloc(sizeof(Array));
    array->size = size;
    array->array = malloc(sizeof(Value *) * size);

    for (int i = 0; i < size; i++)
    {
        array->array[i] = vnull(); /* each slot is a Value* with refcount==1 */
    }

    res->v.opaque = array;
    res->display = array_printer;
    strcpy(res->type_name, "array");

    return res;
}

Value *native_set_array(Env *env, int argc, Value **argv)
{
    if (argc != 3)
    {
        return verror("array.set(array, index, value): requires 3 args\n");
        
    }

    Value *arrv = argv[0];
    if (arrv->type != T_OPAQUE)
    {
        return verror("array.set(array, index, value): first arg must be an array (opaque)\n");
        
    }

    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array.set(array, index, value): null array data\n");
        
    }

    if (argv[1]->type != T_INT)
    {
        return verror("array.set(array, index, value): index must be int\n");
        
    }

    int idx = (int)argv[1]->v.i;
    if (idx < 0 || idx >= arr->size)
    {
        return verror("array.set(array, index, value): index %d out of bounds (size %d)\n", idx, arr->size);
        
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
    if (argc != 2)
    {
        return verror("array.get(array, index): requires 2 args\n");
        
    }

    Value *arrv = argv[0];
    if (arrv->type != T_OPAQUE)
    {
        return verror("array.get(array, index): first arg must be an array (opaque)\n");
        
    }

    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array.get(array, index): null array data\n");
        
    }

    if (argv[1]->type != T_INT)
    {
        return verror("array.get(array, index): index must be int\n");
        
    }

    int idx = (int)argv[1]->v.i;
    if (idx < 0 || idx >= arr->size)
    {
        return verror("array.get(array, index): index %d out of bounds (size %d)\n", idx, arr->size);
        
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
    if (argc != 1)
    {
        return verror("array.len(array): requires 1 arg\n");
        
    }

    Value *arrv = argv[0];
    if (arrv->type != T_OPAQUE)
    {
        return verror("array.len(array): first arg must be an array (opaque)\n");
        
    }

    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array.len(array): null array data\n");
        
    }

    return vint(arr->size);
}

Value *native_free_array(Env *env, int argc, Value **argv)
{
    if (argc != 1)
    {
        return verror("array.free(array): requires 1 arg\n");
        
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

Value *native_report(Env *env, int argc, Value **argv)
{
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
    if (argc != 0)
    {
        return verror("invalid number of arguments given.\n");
        
    }
    return vfloat(get_unix_timestamp());
}

Value *native_run(Env *env, int argc, Value **argv)
{
    if (argc != 1 || argv[0]->type != T_STRING)
    {
        return verror("invalid number of arguments given or incorrect types.\n");
        
    }

    char *path = argv[0]->v.s;

    if (!search_path)
    {
        char *path = path_list_find(search_path, argv[0]->v.s);
        if (!path)
        {
            return verror("run(filename) did not find the file.\n");
            
        }
    }

    if (run_file(path, env))
        return verror("problem running file %s\n", argv[0]->v.s);

    return vnull();
}

Value *native_load(Env *env, int argc, Value **argv)
{
    if (argc != 1 || argv[0]->type != T_STRING)
    {
        return verror("invalid number of arguments given or incorrect types.\n");
        
    }

    if (load_library(env, argv[0]->v.s))
        return verror("problem loading file %s\n", argv[0]->v.s);

    return vnull();
}

Value *native_eval(Env *env, int argc, Value **argv)
{
    if (argc != 1 || argv[0]->type != T_STRING)
    {
        return verror("invalid number of arguments given or incorrect types.\n");
        
    }

    return eval_str(argv[0]->v.s, env);
}

Value *native_new_dict(Env *env, int argc, Value **argv)
{
    if (argc != 0)
    {
        return verror("invalid number of arguments given or incorrect types.\n");
        
    }

    Dict *d = dict_create();
    if (d)
    {
        Value *v = vopaque_extra(d, dict_display, "dict");
        return v;
    }
    return verror("couldnt make a dict.\n");
    
}

Value *native_set_dict(Env *env, int argc, Value **argv)
{
    if (argc != 3)
    {
        return verror("invalid number of arguments given or incorrect types.\n");
        
    }

    dict_set(argv[0]->v.opaque, argv[1]->v.s, argv[2]);
    return vnull();
}

Value *native_get_dict(Env *env, int argc, Value **argv)
{
    if (argc != 2)
    {
        return verror("invalid number of arguments given or incorrect types.\n");
        
    }

    Value *v = dict_get(argv[0]->v.opaque, argv[1]->v.s);
    return v ? v : vnull();
}

Value *native_rem_dict(Env *env, int argc, Value **argv)
{
    if (argc != 2)
    {
        return verror("invalid number of arguments given or incorrect types.\n");
        
    }

    dict_remove(argv[0]->v.opaque, argv[1]->v.s);
    return vnull();
}

Value *native_free_dict(Env *env, int argc, Value **argv)
{
    if (argc != 1)
    {
        return verror("invalid number of arguments given or incorrect types.\n");
    }

    dict_free(argv[0]->v.opaque);
    return vnull();
}

Value *native_system(Env *env, int argc, Value **argv)
{
    if (argc != 1 || argv[0]->type != T_STRING)
    {
        return verror("invalid number of arguments given or incorrect types.\n");
    }
    return vint(system(argv[0]->v.opaque));
}

Value *native_floor(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return vnull();
    double x = argv[0]->v.f;
    return vfloat(floor(x));
}

Value *native_ceil(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return vnull();
    double x = argv[0]->v.f;
    return vfloat(ceil(x));
}

Value *native_sqrt(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return vnull();
    double x = argv[0]->v.f;
    return vfloat(sqrt(x));
}

Value *native_sin(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return vnull();
    double x = argv[0]->v.f;
    return vfloat(sin(x));
}

Value *native_cos(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return vnull();
    double x = argv[0]->v.f;
    return vfloat(cos(x));
}

Value *native_tan(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return vnull();
    double x = argv[0]->v.f;
    return vfloat(tan(x));
}

Value *native_atan2(Env *env, int argc, Value **argv)
{
    if (argc != 2)
        return vnull();
    double y = argv[0]->v.f;
    double x = argv[1]->v.f;
    return vfloat(atan2(y, x));
}

Value *native_pow(Env *env, int argc, Value **argv)
{
    if (argc != 2)
        return vnull();
    return vint(pow(argv[0]->v.i, argv[1]->v.i));
}

// helper to bind native into environment with a name
void env_register_native(Env *env, const char *name, NativeFn fn)
{
    Value *nv = vnative(fn, name);
    env_set_local(env, name, nv);
    val_release(nv);
}

Value *vopaque_extra(void *p, Printer dis, const char *type)
{
    Value *v = vopaque(p);
    v->display = dis;
    strcpy(v->type_name, type);
    return v;
}

// Minimal MiLa Core
// We do not support objects,
// the dots are namespaces.
void env_register_builtins(Env *g)
{
    // Text IO
    env_register_native(g, "print", native_print);
    env_register_native(g, "printr", native_printr);
    env_register_native(g, "println", native_println);
    env_register_native(g, "input", native_input);
    // Logic
    env_register_native(g, "and", native_bitwise_and);
    env_register_native(g, "or", native_bitwise_or);
    env_register_native(g, "xor", native_bitwise_xor);
    env_register_native(g, "not", native_not);
    // File IO
    env_register_native(g, "open", native_open);
    env_register_native(g, "fclose", native_fclose);
    env_register_native(g, "fprint", native_fprint);
    env_register_native(g, "fread", native_fread);
    env_register_native(g, "fseek", native_fseek);
    env_register_native(g, "ftell", native_ftell);
    env_set(g, "SEEK_SET", vint(SEEK_SET));
    env_set(g, "SEEK_END", vint(SEEK_END));
    env_set(g, "SEEK_CUR", vint(SEEK_CUR));
    env_set(g, "stderr", vopaque_extra(stderr, NULL, "'stderr fd'"));
    env_set(g, "stdout", vopaque_extra(stdout, NULL, "'stdout fd'"));
    // Array
    env_register_native(g, "array", native_new_array);
    env_register_native(g, "array.set", native_set_array);
    env_register_native(g, "array.get", native_get_array);
    env_register_native(g, "array.len", native_len_array);
    env_register_native(g, "array.free", native_free_array);
    // Dicts
    env_register_native(g, "dict", native_new_dict);
    env_register_native(g, "dict.set", native_set_dict);
    env_register_native(g, "dict.get", native_get_dict);
    env_register_native(g, "dict.rem", native_rem_dict);
    env_register_native(g, "dict.free", native_free_dict);
    // Casting
    env_register_native(g, "cast.int", native_cast_int);
    env_register_native(g, "cast.float", native_cast_float);
    env_register_native(g, "cast.string", native_cast_string);
    env_register_native(g, "typeof", native_type_of);
    env_register_native(g, "_typeof", native_xtype_of);
    // String
    env_register_native(g, "str.slice", native_str_slice);
    env_register_native(g, "str.index", native_str_index);
    env_register_native(g, "str.patch", native_str_patch);
    env_register_native(g, "str.length", native_str_length);
    env_register_native(g, "str.pop_f", native_pop_start);
    env_register_native(g, "str.pop_b", native_pop_end);
    // ASCII
    env_register_native(g, "ascii.from", native_from_ascii);
    env_register_native(g, "ascii.to", native_to_ascii);
    // Math
    env_register_native(g, "floor", native_floor);
    env_register_native(g, "ceil", native_ceil);
    env_register_native(g, "sqrt", native_sqrt);
    env_register_native(g, "sin", native_sin);
    env_register_native(g, "cos", native_cos);
    env_register_native(g, "tan", native_tan);
    env_register_native(g, "atan2", native_atan2);
    env_register_native(g, "pow", native_pow);

    /*
     * _typeof differentiates between native and non native functions
     * this is for very specific use cases
     */
    // Error handling
    env_register_native(g, "report", native_report);
    env_register_native(g, "exit", native_exit);
    // Time measurement
    env_register_native(g, "get_time", native_get_time);
    // OS Stuff
    env_register_native(g, "system", native_system);
    // Modules
    env_register_native(g, "run", native_run);   // runs file
    env_register_native(g, "load", native_load); // loads dlls or so file
    env_register_native(g, "eval", native_eval); // runs string
}
