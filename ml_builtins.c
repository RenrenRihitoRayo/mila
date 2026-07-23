// This project is licensed under the GNU Affero General Public License

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
 *   JSON and MJSON (de)serializers
 *   Threading
 *   Platform Information
 *   Shelling
 *   and more...
 */

#pragma once

#include <math.h>
#include <string.h>
#include <uchar.h>

#include "ml_json.c"
#include "ml_paths.c"

#ifdef ML_LIB
#define ML_ALREADY
#endif

#define ML_LIB
#ifndef MILA_USE_C
#include "mila.h"
#else
#include "mila.c"
#endif

#ifndef ML_ALREADY
#undef ML_LIB
#endif

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <dlfcn.h>
#include <limits.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

#include <time.h>

#include "ml_primitives.c"
#include "ml_platform_specific.c"
#include "ml_commons.h"

#ifndef ML_NO_THREADING
#include "ml_threading.h"
#endif

Value *self_free(Value *self)
{
    val_release(self);
    return NULL;
}

// ---------- Native functions ----------

char *read_input(void)
{
    size_t bufsize = 64; // initial buffer size
    size_t len = 0;      // number of chars read
    char *buffer = mila_malloc(bufsize);
    if (!buffer)
    {
        fprintf(stderr, "read_input: Allocation failed.");
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
            char *newbuf = mila_realloc(buffer, bufsize);
            if (!newbuf)
            {
                mila_free(buffer);
                fprintf(stderr, "Reallocation failed.");
                return NULL;
            }
            buffer = newbuf;
        }
    }

    buffer[len] = '\0';
    return buffer;
}

Value *native_bitwise_and(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_INT, T_INT, T_ARG_END))
        return vnull();
    return vint(argv[0]->v->i & argv[1]->v->i);
}

Value *native_bitwise_or(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_INT, T_INT, T_ARG_END))
        return vnull();
    return vint(argv[0]->v->i | argv[1]->v->i);
}

Value *native_bitwise_xor(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_INT, T_INT, T_ARG_END))
        return vnull();
    return vint(argv[0]->v->i ^ argv[1]->v->i);
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

Value *native_abort(Env *env, int argc, Value **argv)
{
    abort();
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
        ;
    else
        return verror("input(prompt): Expected 1 argument (prompt) string.");

    char *res = read_input();
    return res ? vstring_take(res) : vnull();
}

Value *native_cast_int(Env *env, int argc, Value **argv)
{
    (void)env;
    long i = 0;
    if (argc == 1 && argv[0]->type == T_STRING)
    {
        char *end;
        i = strtol(GET_STRING(argv[0]), &end, 10);

        if (*end != '\0')
        {
            char *buffer = NULL;
            malloc_sprintf(&buffer, "cast.int(str): Got bad part \"%s\"...", end);
            Value *tmp = vtagged_error(E_TYPE_ERROR, "%s", buffer);
            mila_free(buffer);
            i = 0;
            return tmp;
        }
    }
    else
    {
        return verror("cast.int(str): Expected 1 argument (str) string.");
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
        f = strtod(GET_STRING(argv[0]), &end);

        if (*end != '\0')
        {
            char *buffer = NULL;
            malloc_sprintf(&buffer, "cast.float(str): Got bad part \"%s\"...", end);
            Value *tmp = vtagged_error(E_TYPE_ERROR, "%s", buffer);
            mila_free(buffer);
            return tmp;
        }
    }
    else
    {
        return verror("cast.float(str): Expected 1 argument (str) string.");
    }
    return vfloat(f);
}

Value *native_cast_int_to_uint(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc == 1 && argv[0]->type == T_INT)
    {
        return vuint(argv[0]->v->ui);
    }
    else
    {
        return verror("cast.i2u(int): Expected 1 argument (int) int. Got %s",
                      GET_TYPENAME(argv[0]));
    }
}

Value *native_cast_uint_to_int(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc == 1 && argv[0]->type == T_UINT)
    {
        return vint(argv[0]->v->i);
    }
    else
    {
        return verror("cast.u2i(uint): Expected 1 argument (uint) uint. Got %s",
                      GET_TYPENAME(argv[0]));
    }
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
        return verror("cast.i2f(int): Expected 1 argument (int) int. Got %s",
                      GET_TYPENAME(argv[0]));
    }
}

Value *native_cast_float_to_int(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc == 1 && argv[0]->type == T_FLOAT)
    {
        return vint((long)argv[0]->v->f);
    }
    else
    {
        return verror(
            "cast.f2i(float): Expected 1 argument (float) float. Got %s",
            GET_TYPENAME(argv[0]));
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
        return verror("cast.str(any): Expected 1 argument (any) any.");
    }
    return vnull();
}

Value *native_type_of(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
    {
        return verror("typeof(any): Expected 1 argument (any) any.");
    }
    if (argv[0]->type_name)
        return vstring_dup(argv[0]->type_name);
    return vstring_dup(GET_TYPENAME(argv[0]));
}

Value *native_type_of_extra(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
    {
        return verror("_typeof(any): Expected 1 argument (any) any.");
    }
    if (argv[0]->type_name)
        return vstring_dup(argv[0]->type_name);
    return vstring_dup(GET_TYPENAME(argv[0]));
}

Value *native_is_numeric(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
    {
        return verror("is_numeric(any): Expected 1 argument (any) any.");
    }
    if (argv[0]->type_name)
        return vstring_dup(argv[0]->type_name);
    return vbool(is_number(argv[0]));
}

#ifndef ML_NO_FILE_IO

Value *file_printer(Value *self)
{
    char *buffer = NULL;
    if (!self || self->type != T_OPAQUE)
    {
        malloc_sprintf(&buffer, "<not-a-file>");
        return vstring_take(buffer);
    }
    FILE *f = (FILE *)self->v;
    if (!f)
    {
        malloc_sprintf(&buffer, "<file:closed>");
    }
    else
    {
        malloc_sprintf(&buffer, "<file:%p>", f);
    }
    return vstring_take(buffer);
}

Value *native_open(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2 || argv[0]->type != T_STRING || argv[1]->type != T_STRING)
    {
        return verror("open(filename, mode) expects 2 string args.");
    }
    char *path = GET_STRING(argv[0]);
    if (!mila_search_path)
    {
        char *path = path_list_find(mila_search_path, GET_STRING(argv[0]));
        if (!path)
        {
            return verror("open(filename, mode) did not find the file.");
        }
    }
    char *res = NULL, *og_res = path_list_find(mila_search_path, path);
    if (!og_res)
        res = path_list_find_alternative(mila_search_path, path);
    else
        res = og_res;

    FILE *f = fopen(res, GET_STRING(argv[1]));
    if (!f)
    {
        mila_free(res);
        perror(NULL);
        return vnull();
    }

    mila_free(res);
    Value *v = vopaque(f);
    val_set_table(v, file_meta);
    v->type_name = strdup(MILA_LPREFIX "file");
    return v;
}

Value *native_fdopen(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2 || argv[0]->type != T_INT || argv[1]->type != T_STRING)
    {
        return verror("fdopen(filedescriptor, mode) expects 2 string args.");
    }
    char *path = GET_STRING(argv[0]);
    if (!mila_search_path)
    {
        char *path = path_list_find(mila_search_path, GET_STRING(argv[0]));
        if (!path)
        {
            return verror("fdopen(filedescriptor, mode) did not find the file.");
        }
    }

    FILE *f = fdopen((int)GET_INTEGER(argv[0]), GET_STRING(argv[1]));
    if (!f)
    {
        perror(NULL);
        return vnull();
    }

    Value *v = vopaque(f);
    val_set_table(v, file_meta);
    v->type_name = strdup(MILA_LPREFIX "file");
    return v;
}

Value *native_fileno(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || strcmp(GET_TYPENAME(argv[0]), MILA_LPREFIX "file") != 0)
    {
        return verror("fileno(file) expects 1 string argument.");
    }

    return vint(fileno((FILE *)GET_OPAQUE(argv[0])));
}

Value *native_fdredirect(Env *env, int argc, Value **argv)
{
    if (argc != 2)
        return verror("fredirect(oldfd, newfd): Expects two arguments.");
    int oldfd = 0, newfd = 0;
    if (strcmp(GET_TYPENAME(argv[0]), MILA_LPREFIX "file") == 0)
    {
        oldfd = fileno((FILE *)GET_OPAQUE(argv[0]));
    }
    else if (GET_TYPE(argv[0]) == T_INT)
    {
        oldfd = (int)GET_INTEGER(argv[0]);
    }
    else
    {
        return verror("fredirect(oldfd, newfd): Expected oldfd to be a file or a file descriptor, got %s", GET_TYPENAME(argv[0]));
    }
    if (strcmp(GET_TYPENAME(argv[1]), MILA_LPREFIX "file") == 0)
    {
        newfd = fileno((FILE *)GET_OPAQUE(argv[1]));
    }
    else if (GET_TYPE(argv[1]) == T_INT)
    {
        newfd = (int)GET_INTEGER(argv[1]);
    }
    else
    {
        return verror("fredirect(oldfd, newfd): Expected newfd to be a file or a file descriptor, got %s", GET_TYPENAME(argv[1]));
    }
    int og_fd = dup(oldfd);
    int err = dup2(oldfd, newfd);
    return vint(og_fd);
}

Value *native_fclose(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || argv[0]->type != T_OPAQUE)
    {
        return verror("fclose(file) expects 1 file handle arg.");
    }
    FILE *f = (FILE *)GET_OPAQUE(argv[0]);
    if (f)
    {
        fclose(f);
    }
    return vnull();
}

Value *native_close(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || argv[0]->type != T_INT)
    {
        return verror("close(file) expects 1 fd arg.");
    }
    close(GET_INTEGER(argv[0]));
    return vnull();
}

Value *native_fflush(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || argv[0]->type != T_OPAQUE)
    {
        return verror("fflush(file) expects 1 file handle arg.");
    }
    FILE *f = (FILE *)argv[0]->v;
    if (f)
    {
        fflush(f);
    }
    return vnull();
}

Value *native_file_exists(Env *env, int argc, Value **argv)
{
    if (argc != 1 || GET_TYPE(argv[0]) != T_STRING)
        verror("file.exists(f: \"string\"): Expects a path!");
    char *file = path_list_find(mila_search_path, GET_STRING(argv[0]));
    if (file && file_exists(file))
    {
        free(file);
        return vbool(1);
    }
    free(file);
    return vbool(0);
}

Value *native_file_is_file(Env *env, int argc, Value **argv)
{
    if (argc != 1 || GET_TYPE(argv[0]) != T_STRING)
        verror("file.is_file(f: \"string\"): Expects a path!");
    char *file = path_list_find(mila_search_path, GET_STRING(argv[0]));
    if (file && is_file(file))
    {
        free(file);
        return vbool(1);
    }
    free(file);
    return vbool(0);
}

Value *native_file_is_dir(Env *env, int argc, Value **argv)
{
    if (argc != 1 || GET_TYPE(argv[0]) != T_STRING)
        verror("file.is_dir(f: \"string\"): Expects a path!");
    char *file = path_list_find(mila_search_path, GET_STRING(argv[0]));
    if (file && is_dir(file))
    {
        free(file);
        return vbool(1);
    }
    free(file);
    return vbool(0);
}

Value *native_file_list_dir(Env *e, int argc, Value **argv)
{
    Value *l = call_native_with(NULL, native_list_new, NULL);
    DIR *dir;
    struct dirent *entry;
    struct stat st;

    char *dir_name = argc == 1 ? GET_STRING(argv[0]) : ".";
    dir = opendir(dir_name);
    if (dir == NULL)
    {
        return verror("opendir failed");
    }

    while ((entry = readdir(dir)) != NULL)
    {
        // skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char name[2048];
        if (dir_name[strlen(dir_name) - 1] != '/')
            sprintf(name, "%s/%s", dir_name, entry->d_name);
        else
            sprintf(name, "%s%s", dir_name, entry->d_name);

        // optional: check if it's file or dir
        if (stat(name, &st) == 0)
        {
            if (S_ISDIR(st.st_mode))
            {
                val_release(call_native_with(NULL, native_list_append, val_retain(l), make_dict(vstring_dup("name"), vstring_dup(entry->d_name), vstring_dup("type"), vstring_dup("d"), NULL), NULL));
            }
            else
            {
                val_release(call_native_with(NULL, native_list_append, val_retain(l), make_dict(vstring_dup("name"), vstring_dup(entry->d_name), vstring_dup("type"), vstring_dup("f"), NULL), NULL));
            }
        }
        else
        {
            val_release(call_native_with(NULL, native_list_append, val_retain(l), make_dict(vstring_dup("name"), vstring_dup(entry->d_name), vstring_dup("type"), vstring_dup("?"), NULL), NULL));
        }
    }

    closedir(dir);
    return l;
}

Value *native_fprint(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2 || argv[0]->type != T_OPAQUE || argv[1]->type != T_STRING)
    {
        return verror("fprint(file, string) expects (handle, string).");
    }
    FILE *f = (FILE *)argv[0]->v;
    if (!f)
    {
        return verror("fprint: file handle is closed or invalid.");
    }
    const char *s = GET_STRING(argv[1]);
    size_t written = fwrite(s, 1, strlen(s), f);
    return vint(written);
}

Value *native_fprint_bytes(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2 || argv[0]->type != T_OPAQUE || strcmp(GET_TYPENAME(argv[1]), MILA_LPREFIX "list") != 0)
    {
        return verror("fprint_bytes(file, bytes) expects (handle, list[int]).");
    }
    FILE *f = (FILE *)argv[0]->v;
    if (!f)
    {
        return verror("fprint_bytes(file, bytes): file handle is closed or invalid.");
    }
    Value **list = ll_to_iter((LinkedList *)GET_OPAQUE(argv[1]));
    long size = GET_INTEGER(list[0]);
    for (long i = 1; i < size; i++)
    {
        char c = GET_INTEGER(list[i]);
        size_t written = fwrite(&c, 1, 1, f);
        val_release(list[i]);
    }
    val_release(list[0]);
    return vint(size);
}

Value *native_fread(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2 || argv[0]->type != T_OPAQUE || argv[1]->type != T_INT)
    {
        return verror("fread(file, num_bytes) expects (handle, int).");
    }
    FILE *f = (FILE *)argv[0]->v;
    if (!f)
    {
        return verror("fread(file, num_bytes): file handle is closed or invalid.");
    }
    long n = argv[1]->v->i;
    if (n <= 0)
        return vstring_dup("");

    char *buf = mila_malloc(n + 1);
    if (!buf)
        return vnull();

    size_t read_bytes = fread(buf, 1, n, f);
    buf[read_bytes] = '\0';

    return vstring_take(buf);
}

Value *native_fread_bytes(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2 || argv[0]->type != T_OPAQUE || argv[1]->type != T_INT)
    {
        return verror("fread_bytes(file, num_bytes) expects (handle, int).");
    }
    FILE *f = (FILE *)argv[0]->v;
    if (!f)
    {
        return verror("fread_bytes(file, num_bytes): file handle is closed or invalid.");
    }
    long n = argv[1]->v->i;
    if (n <= 0)
        return vstring_dup("");

    char *buf = mila_malloc(n);
    if (!buf)
        return vnull();

    Value *list = make_list(NULL);
    size_t read_bytes = fread(buf, 1, n, f);
    for (long i = 0; i < read_bytes; i++)
    {
        val_release(call_native_with(NULL, native_list_append, val_retain(list), vint(buf[i]), NULL));
    }

    mila_free(buf);

    return list;
}

Value *native_fread_all(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || argv[0]->type != T_OPAQUE)
    {
        return verror("fread_all(file) expects handle.");
    }
    FILE *f = (FILE *)argv[0]->v;
    if (!f)
    {
        return verror("fread_all(file): file handle is closed or invalid.");
    }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = mila_malloc(n + 1);
    if (!buf)
        return vnull();

    size_t read_bytes = fread(buf, 1, n, f);
    buf[read_bytes] = '\0';

    return vstring_take(buf);
}

Value *native_fread_all_bytes(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || argv[0]->type != T_OPAQUE)
    {
        return verror("fread_all(file) expects handle.");
    }
    FILE *f = (FILE *)argv[0]->v;
    if (!f)
    {
        return verror("fread_all(file): file handle is closed or invalid.");
    }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = mila_malloc(n + 1);
    if (!buf)
        return vnull();

    Value *list = make_list(NULL);
    size_t read_bytes = fread(buf, 1, n, f);
    for (long i = 0; i < read_bytes; i++)
    {
        val_release(call_native_with(NULL, native_list_append, val_retain(list), vint(buf[i]), NULL));
    }

    mila_free(buf);

    return list;
}

Value *native_fseek(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 3 || argv[0]->type != T_OPAQUE || argv[1]->type != T_INT ||
        argv[2]->type != T_INT)
    {
        return verror(
            "fseek(file, offset, whence) expects (handle, int, int).");
    }
    FILE *f = (FILE *)argv[0]->v;
    if (!f)
    {
        return verror("fseek(file, offset, whence): file handle is closed or invalid.");
    }
    long offset = argv[1]->v->i;
    int whence = (int)argv[2]->v->i;
    int c_whence;

    switch (whence)
    {
    case 0:
    case 1:
    case 2:
        c_whence = whence;
        break;
    default:
        return verror("fseek(file, offset, whence): invalid whence %d (must be 0-SEEK_SET, 1-SEEK_CUR, "
                      "or 2-SEEK_END).",
                      whence);
    }

    int res = fseek(f, offset, c_whence);
    return vint(res);
}

Value *native_ftell(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || argv[0]->type != T_OPAQUE)
    {
        return verror("ftell(file) expects 1 file handle arg.");
    }
    FILE *f = (FILE *)argv[0]->v;
    if (!f)
    {
        return verror("ftell(file): file handle is closed or invalid.");
    }
    long pos = ftell(f);
    return vint(pos);
}

#endif

Value *native_report(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc == 1 && argv[0]->type == T_STRING)
        return verror("%s", GET_STRING(argv[0]));
    else if (argc == 0)
        return verror("No details given.");
    else
        return verror("report(message): Invalid number of arguments given.");
}

Value *native_report_tagged(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc == 2)
    {
        return vtagged_error((ErrorType)GET_INTEGER(argv[0]), "%s",
                             GET_STRING(argv[1]));
    }
    else if (argc == 1)
    {
        return vtagged_error(E_GENERIC, "No details given.");
    }
    else
        return verror("report(tag, message): Invalid number of arguments given.");
}

Value *native_exit(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc == 1 && argv[0]->type == T_INT)
    {
        return vtagged_coded_error(E_EXIT, argv[0]->v->i, "Exited.");
    }
    else if (argc == 0)
    {
        return vtagged_coded_error(E_EXIT, 0, "Exited.");
    }
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
        return verror("get_time(): invalid number of arguments given.");
    }
    return vfloat(get_unix_timestamp());
}

Value *native_time_sleep(Env *env, int argc, Value **argv)
{
    (void)argc;
    (void)argv;
    (void)env;
    if (argc != 1)
    {
        return verror("time_sleep(time): invalid number of arguments given.");
    }
    if (GET_TYPE(argv[0]) == T_INT)
        sleep(GET_INTEGER(argv[0]));
    else if (GET_TYPE(argv[0]) == T_UINT)
        sleep(GET_UINTEGER(argv[0]));
    return vnull();
}

void sleep_micros(uint64_t microseconds)
{
    struct timespec ts;
    ts.tv_sec = microseconds / 1000000;
    ts.tv_nsec = (microseconds % 1000000) * 1000;
    nanosleep(&ts, NULL);
}

Value *native_time_sleep_ms(Env *env, int argc, Value **argv)
{
    (void)argc;
    (void)argv;
    (void)env;
    if (argc != 1)
    {
        return verror("time_sleep_ms(time): invalid number of arguments given.");
    }
    if (GET_TYPE(argv[0]) == T_INT)
        sleep_micros(GET_INTEGER(argv[0]) * 1000);
    else if (GET_TYPE(argv[0]) == T_UINT)
        sleep_micros(GET_UINTEGER(argv[0]) * 1000);
    return vnull();
}

#ifndef SAFE_BUILD
Value *native_run(Env *env, int argc, Value **argv)
{
    if (argc != 1 || argv[0]->type != T_STRING)
    {
        return verror("run(filename): invalid number of arguments given or incorrect types.");
    }

    if (mila_search_path)
    {
        char *path = path_list_find(mila_search_path, GET_STRING(argv[0]));
        if (!path)
        {
            return verror("run(filename) did not find the file.");
        }
        Env *frame = env_new(env);
        Value *by = env_get(env, "__name__");
        Value *by_path = env_get(env, "__path__");
        Value *by_dir_path = env_get(env, "__dir_path__");
        env_set_local_raw(frame, "__importer__", call_native_with(env, native_new_dict, vstring_dup("name"), val_copy(by), vstring_dup("path"), val_copy(by_path), vstring_dup("dir_path"), val_copy(by_dir_path), NULL));
        Value *res = run_file_keep_res(path, frame);
        if (GET_TYPE(res) == T_ERROR)
        {
            return res;
        }
        mila_free(path);
        env_free(frame);
        return res;
    }

    return vnull();
}

Value *native_require(Env *env, int argc, Value **argv)
{
#ifndef ML_NO_CACHED_MODS
    if (!mila_cached_modules)
    {
        return vtagged_error(E_FATAL, "Interpreter was not initialized properly as 'mila_cached_modules' is not available.");
    }
    if (argc != 1 || argv[0]->type != T_STRING)
    {
        return verror("require(filename): invalid number of arguments given or incorrect types.");
    }

    if (mila_search_path)
    {
        char *path = path_list_find(mila_search_path, GET_STRING(argv[0]));
        if (!path)
        {
            return verror("require(filename) did not find the file.");
        }
        // try full path
        // yes this code wont run if the file doesnt exist.
        // counter measure against people who think they can just
        // mess with internals and call it "meta programming"
#ifndef ML_NO_THREADING
        pthread_mutex_lock(&mila_cached_modules_lock_read);
#endif
        Value *module = call_native_with(NULL, native_get_dict, val_retain(mila_cached_modules), vstring_dup(path), NULL);
#ifndef ML_NO_THREADING
        pthread_mutex_unlock(&mila_cached_modules_lock_read);
#endif
        if (module && GET_TYPE(module) != T_NULL)
        {
            return module;
        }
        // open full path and run the file.
        Env *frame = env_new(env);
        Value *by = env_get(env, "__name__");
        Value *by_path = env_get(env, "__path__");
        Value *by_dir_path = env_get(env, "__dir_path__");
        env_set_local_raw(frame, "__importer__", call_native_with(env, native_new_dict, vstring_dup("name"), val_copy(by), vstring_dup("path"), val_copy(by_path), vstring_dup("dir_path"), val_copy(by_dir_path), NULL));
        Value *res = run_file_keep_res(path, frame);
        if (GET_TYPE(res) == T_ERROR)
        {
            return res;
        }
        mila_free(path);
        env_free(frame);
#ifndef ML_NO_THREADING
        pthread_mutex_lock(&mila_cached_modules_lock);
#endif
        val_release(call_native_with(NULL, native_set_dict, val_retain(mila_cached_modules), vstring_dup(path), val_retain(res), NULL));
#ifndef ML_NO_THREADING
        pthread_mutex_unlock(&mila_cached_modules_lock);
#endif
        
        return res;
    }

    return vnull();
#else
    return native_run(env, argc, argv);
#endif // ML_NO_MOD_CACHE
}

Value *native_invoke(Env *env, int argc, Value **argv)
{
    if (argc != 1 || argv[0]->type != T_STRING)
    {
        return verror("invoke(filename): invalid number of arguments given or incorrect types.");
    }

    if (mila_search_path)
    {
        char *path = path_list_find(mila_search_path, GET_STRING(argv[0]));
        if (!path)
        {
            return verror("invoke(filename) did not find the file.");
        }
        Env *frame = env_new(env);
        Value *by = env_get(env, "__name__");
        Value *by_path = env_get(env, "__path__");
        Value *by_dir_path = env_get(env, "__dir_path__");
        env_set_local_raw(frame, "__importer__", call_native_with(env, native_new_dict, vstring_dup("name"), val_copy(by), vstring_dup("path"), val_copy(by_path), vstring_dup("dir_path"), val_copy(by_dir_path), NULL));
        Value *res = invoke_file_keep_res(path, frame);
        if (GET_TYPE(res) == T_ERROR)
        {
            return res;
        }
        mila_free(path);
        env_free(frame);
        return res;
    }

    return vnull();
}

Value *native_load(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || (!argv[0]) || argv[0]->type != T_STRING)
    {
        return verror("load(file): invalid number of arguments given or incorrect types.");
    }

    char *new_path = path_list_find(mila_search_path, GET_STRING(argv[0]));
    if (!new_path)
        return verror("load(file): problem loading file %s", GET_STRING(argv[0]));
    if (load_library(env, new_path))
    {
        mila_free(new_path);
        return verror("load(file): problem loading file %s", GET_STRING(argv[0]));
    }
    mila_free(new_path);
    return vnull();
}

Value *native_eval(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || argv[0]->type != T_STRING)
    {
        return verror("eval(code): invalid number of arguments given or incorrect types.");
    }

    return eval_str(GET_STRING(argv[0]), env);
}
#endif

Value *native_system(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1 || argv[0]->type != T_STRING)
    {
        return verror("invalid number of arguments given or incorrect types.");
    }
    return vint(system(GET_STRING(argv[0])));
}

#ifndef ML_NO_MATH
Value *native_floor(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("floor(i): Requires one arguments");
    double x = GET_FLOAT(argv[0]);
    return vfloat(floor(x));
}

Value *native_ceil(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("ceil(i): Requires one arguments");
    double x = GET_FLOAT(argv[0]);
    return vfloat(ceil(x));
}

Value *native_sqrtf(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("sqrtf(i): Requires one arguments");
    double x = GET_FLOAT(argv[0]);
    return vfloat(sqrtf(x));
}

Value *native_sqrt(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("sqrt(i): Requires one arguments");
    double x = GET_FLOAT(argv[0]);
    return vfloat(sqrt(x));
}

Value *native_sin(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("sin(i): Requires one arguments");
    double x = GET_FLOAT(argv[0]);
    return vfloat(sin(x));
}

Value *native_cos(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("cos(i): Requires one arguments");
    double x = GET_FLOAT(argv[0]);
    return vfloat(cos(x));
}

Value *native_tan(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("tan(i): Requires one arguments");
    double x = GET_FLOAT(argv[0]);
    return vfloat(tan(x));
}

Value *native_atan2(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2)
        return verror("atan2(i, i): Requires two arguments");
    double y = GET_FLOAT(argv[0]);
    double x = GET_FLOAT(argv[1]);
    return vfloat(atan2(y, x));
}

Value *native_pow(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2)
        return verror("pow(base, exp): Requires two arguments");
    return vint(pow(GET_INTEGER(argv[0]), GET_INTEGER(argv[1])));
}

Value *native_fabs(Env *e, int argc, Value **argv)
{
    if (argc != 1 || GET_TYPE(argv[0]) != T_FLOAT)
        return verror("fabs(num): argument must be a float!");
    return vfloat(fabs(GET_FLOAT(argv[0])));
}

Value *native_abs(Env *e, int argc, Value **argv)
{
    if (argc != 1 || GET_TYPE(argv[0]) != T_INT)
        return verror("abs(num): argument must be an integer!");
    return vint(abs((int)GET_INTEGER(argv[0])));
}
#endif // ML_NO_MATH

Value *native_env_new(Env *env, int argc, Value **argv)
{
    Env *e = env_new(NULL);
    return vopaque_extra(e, NULL, "environment");
}

Value *native_repr(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("repr(value): Expected at least one argument!");
    return vstring_take(as_c_string_repr(argv[0]));
}

Value *native_repr_raw(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("repr_repr(value): Expected at least one argument!");
    return vstring_take(as_c_string_repr_raw(argv[0]));
}

Value *native_str(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
        return verror("str(value): Expected at least one argument!");
    return vstring_take(as_c_string(argv[0]));
}

Value *env_free_builtins()
{
    mila_free(dict_meta);
    mila_free(array_meta);
    mila_free(list_meta);
    mila_free(file_meta);
    mila_free(range_meta);

    return NULL;
}

Value *native_own(Env *e, int argc, Value **argv)
{
    (void)e;
    if (argc == 1 && GET_TYPE(argv[0]) == T_OPAQUE)
    {
        // NOTE: not recomended to directly access value fields!
        argv[0]->type = T_OWNED_OPAQUE;
        return val_retain(argv[0]);
    }
    return verror("own(ptr): Must have a pointer to convert into owned opaque!");
}

Value *native_unown(Env *e, int argc, Value **argv)
{
    (void)e;
    if (argc == 1 && GET_TYPE(argv[0]) == T_OWNED_OPAQUE)
    {
        // NOTE: not recomended to directly access value fields!
        argv[0]->type = T_OPAQUE;
        return val_retain(argv[0]);
    }
    return verror(
        "unown(ptr): Must have an owned pointer to convert into unowned opaque!");
}

Value *native_istring(Env *e, int argc, Value **argv)
{
    if (argc == 1)
    {
        char *str = as_c_string(argv[0]);
        Value *ptr = vowned_opaque(str);
        ptr->type_name = mila_strdup(MILA_LPREFIX "istring");
        val_set_table(ptr, istring_meta);
        return ptr;
    }
    return verror("istring(v): Needs at least one argument!");
}

Value *istring_to_iter(Value *self)
{
    char *str = (char *)self->v;
    size_t slen = strlen(str);
    Value **iter = (Value **)mila_malloc(sizeof(Value *) * (slen + 2));
    size_t i = 1;
    for (; i < slen + 1; ++i)
    {
        iter[i] = vstring_dup((char[]){str[i - 1], 0});
    }
    iter[slen] = NULL;
    iter[0] = vuint((unsigned long)slen);
    return vopaque(iter);
}

Value *istring_get(Value *self, Value *index)
{
    char *str = (char *)self->v;
    return vstring_dup((char[]){str[GET_INTEGER(index)], 0});
}

Value *istring_to_str(Value *self) { return vstring_dup(GET_STRING(self)); }

Value *native_rand(Env *e, int argc, Value **argv) { return vfloat(rand()); }

Value *native_as_opaque(Env *e, int argc, Value **argv)
{
    (void)e;
    if (argc != 1)
        return verror("as_opaque(v): Must have one argument!");
    switch (GET_TYPE(argv[0]))
    {
    case T_INT:
    {
        long *ptr = NULL;
        ptr = (long *)mila_malloc(sizeof(long));
        *ptr = GET_INTEGER(argv[0]);
        return vowned_opaque(ptr);
    }
    break;
    case T_UINT:
    {
        unsigned long *ptr = NULL;
        ptr = (unsigned long *)mila_malloc(sizeof(unsigned long));
        *ptr = GET_UINTEGER(argv[0]);
        return vowned_opaque(ptr);
    }
    break;
    case T_FLOAT:
    {
        double *ptr = NULL;
        ptr = (double *)mila_malloc(sizeof(double));
        *ptr = GET_FLOAT(argv[0]);
        return vowned_opaque(ptr);
    }
    break;
    case T_STRING:
    {
        char *ptr = NULL;
        size_t len = strlen(GET_STRING(argv[0]));
        ptr = (char *)mila_malloc(sizeof(char) * (len + 1));
        strncpy(ptr, GET_STRING(argv[0]), len);
        ptr[len] = 0;
        return vowned_opaque(ptr);
    }
    break;
    case T_OWNED_OPAQUE:
    case T_OPAQUE:
    {
        return vopaque(GET_OPAQUE(argv[0]));
    }
    break;
    default:;
    }
    return verror("Unsupported type %s!", GET_TYPENAME(argv[0]));
}

Value *native_assert(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2)
        return verror("assert(cond, message): Needs two arguments!");
    if (!is_truthy(argv[0]))
        return vtagged_error(E_ASSERT, "%s", GET_STRING(argv[1]));
    return vnull();
}

Value *native_srandom(Env *env, int argc, Value **argv)
{
    if (argc != 1 || GET_TYPE(argv[0]) != T_INT)
        return verror("srandom(seed): Expected an integer argument!");
    srand(GET_INTEGER(argv[0]));
    return vnull();
}

Value *native_random(Env *env, int argc, Value **argv)
{
    if (argc == 2 && GET_TYPE(argv[0]) == T_INT &&
        GET_TYPE(argv[1]) == T_INT)
    {
        long min = GET_INTEGER(argv[0]);
        long max = GET_INTEGER(argv[1]);
        return vint((rand() % (max - min + 1)) + min);
    }
    return verror("random(lower, upper): Expected two integer arguments.");
}

Value *native_noise(Env *env, int argc, Value **argv)
{
    if (argc > 5 || argc < 4)
        return verror("noise(start, count, max, min, magnitude?): Expected at least 4 or 5 arguments.");
    long start = GET_INTEGER(argv[0]);
    long count = GET_INTEGER(argv[1]);
    long max = GET_INTEGER(argv[2]);
    long min = GET_INTEGER(argv[3]);
    long mag = argc == 5 ? GET_INTEGER(argv[4]) : 5L;

    long *nums = (long *)malloc(sizeof(long) * count);
    Value *arr = call_function_str(env, "array", vint(count), NULL);
    noise(start, count, max, min, mag, nums);
    for (long i = 0; i < count; ++i)
    {
        val_release(call_function_str(env, "array.set", val_retain(arr), vint(i), vint(nums[i]), NULL));
    }
    free(nums);
    return arr;
}

Value *native_crandom(Env *env, int argc, Value **argv)
{
    if (argc != 0)
        verror("crandom(): Expected no arguments");
    return vint(rand());
}

Value *native_get_tm_local(Env *env, int argc, Value **argv)
{
    if (argc > 1)
        return verror("get_tm_local(): Expected at most 1 argument!");
    time_t time = argc == 0 ? get_unix_timestamp() : GET_INTEGER(argv[0]);
    struct tm *info = localtime(&time);
    return vopaque(info);
}

Value *native_get_tm_gmt(Env *env, int argc, Value **argv)
{
    if (argc > 1)
        return verror("get_tm_gmt(): Expected at most 1 argument!");
    time_t time = argc == 0 ? get_unix_timestamp() : GET_INTEGER(argv[0]);
    struct tm *info = gmtime(&time);
    return vopaque(info);
}

Value *native_strftime(Env *env, int argc, Value **argv)
{
    if (argc != 2)
        return verror("strftime(fmt, tm): Expected two arguments!");
    struct tm *info = GET_OPAQUE(argv[1]);
    char *fmt = GET_STRING(argv[0]);
    char buffer[100] = {0};
    strftime(buffer, sizeof(buffer), fmt, info);
    return vstring_dup(buffer);
}

Value *native_vars_bind(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return verror("vars.bind(name: \"string\"): Expected an argument");
    Env *parent = env->parent ? env->parent : env;
    Value *v = env_get(parent, GET_STRING(argv[0]));
    if (!v)
        return verror("vars.bind(name: \"string\"): Variable %s does exist!", GET_STRING(argv[0]));
    env_set_local(env, GET_STRING(argv[0]), v);
    return vnull();
}

extern path_list *mila_search_path;
Value *native_dump_search_list(Env *env, int argc, Value **argv)
{
    printf("Search Paths:");
    for (int i = 0; i < mila_search_path->count; i++)
    {
        if (i != mila_search_path->count - 1)
            printf("  %s,\n", mila_search_path->items[i]);
        else
            printf("  %s\n", mila_search_path->items[i]);
    }
    return vnull();
}

Value *native_break_point(Env *env, int argc, Value **argv)
{
#if defined(__x86_64__) || defined(__i386__)
    __asm__ volatile("int3");

#elif defined(__aarch64__) || defined(__arm__)
    __asm__ volatile("brk #0");

#elif defined(__riscv)
    __asm__ volatile("ebreak");

#elif defined(__powerpc__) || defined(__ppc__)
    __asm__ volatile("trap");

#elif defined(__mips__)
    __asm__ volatile("break");

#else
/* Fallback: raise a signal that debuggers treat like a breakpoint */
#include <signal.h>
    raise(SIGTRAP);
#endif
    return vnull();
}

Value *native_from_opaque(Env *e, int argc, Value **argv)
{
    (void)e;
    if (argc != 2)
        return verror("from_opaque(str, opaque): Must have one argument!");

    if (strcmp(GET_STRING(argv[0]), "string") == 0)
    {
        return vstring_dup(GET_OPAQUE(argv[1]));
    }
    if (strcmp(GET_STRING(argv[0]), "owned_string") == 0)
    {
        return vstring_take(GET_OPAQUE(argv[1]));
    }
    if (strcmp(GET_STRING(argv[0]), "long") == 0)
    {
        return vint(*(long *)GET_OPAQUE(argv[1]));
    }
    if (strcmp(GET_STRING(argv[0]), "ulong") == 0)
    {
        return vuint(*(unsigned long *)GET_OPAQUE(argv[1]));
    }
    if (strcmp(GET_STRING(argv[0]), "int") == 0)
    {
        return vint(*(int *)GET_OPAQUE(argv[1]));
    }
    if (strcmp(GET_STRING(argv[0]), "uint") == 0)
    {
        return vint(*(int *)GET_OPAQUE(argv[1]));
    }
    if (strcmp(GET_STRING(argv[0]), "float") == 0)
    {
        return vint(*(float *)GET_OPAQUE(argv[1]));
    }
    if (strcmp(GET_STRING(argv[0]), "double") == 0)
    {
        return vint(*(double *)GET_OPAQUE(argv[1]));
    }
    if (strcmp(GET_STRING(argv[0]), "char") == 0)
    {
        return vint(*(char *)GET_OPAQUE(argv[1]));
    }
    return verror("Unsupported type: %s", GET_STRING(argv[0]));
}

Value *native_copy(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
    {
        return verror("copy(value): requires 1 arg");
    }

    return val_copy(argv[0]);
}

Value *native_is(Env *env, int argc, Value **argv)
{
    if (argc != 2)
        return verror("is(a, b): Requires two arguments");
    return vbool(argv[0] == argv[1]);
}

Value *native_json_loads(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return verror("json.loads(str): Expects one argument.");
    Src *s = src_new(GET_STRING(argv[0]));
    Value *res = parse_json(s);
    src_free(s);
    return res;
}

Value *native_json_dumps(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return verror("json.dumps(value): Expects one argument.");
    return vstring_take(mila_to_json(argv[0]));
}

Value *native_mjson_loads(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return verror("mjson.loads(str): Expects one argument.");
    Src *s = src_new(GET_STRING(argv[0]));
    Value *res = parse_mjson(s);
    src_free(s);
    return res;
}

Value *native_mjson_dumps(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return verror("mjson.dumps(value): Expects one argument.");
    return vstring_take(mila_to_mjson(argv[0]));
}

Value *native_hash(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return verror("hash(value): Expects one argument.");
    return vuint(hash_value(argv[0]));
}

Value *native_hash_set_seed(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return verror("hash.set_seed(value): Expects one integer.");
    if (GET_UINTEGER(argv[0]) == 0)
        return verror("hash.set_seed(value): Hash cannot be zero!");
    hash_set_seed(GET_UINTEGER(argv[0]));
    return vnull();
}

Value *native_hash_get_seed(Env *env, int argc, Value **argv)
{
    if (argc != 0)
        return verror("hash._get_seed(): Expects no arguments.");
    return vuint(HASH_SEED);
}

Value *native_sys_get_pid(Env *env, int argc, Value **argv)
{
    if (argc != 0)
        return verror("sys.get_pid(): Expects no arguments.");
    return vuint(get_process_id());
}

Value *native_list_deconstruct_v1(Env *env, int argc, Value **argv)
{
    if (argc != 2)
        return verror("ll_deconstruct(pattern, list): Expected 2 args!");
    if (GET_TYPE(argv[0]) != T_STRING)
        return verror("Pattern must be string");
    if (strcmp(GET_TYPENAME(argv[1]), MILA_LPREFIX "list"))
        return verror("Must be list");

    char *pattern = GET_STRING(argv[0]);
    LinkedList *list = (LinkedList *)GET_OPAQUE(argv[1]);

    char *pat_copy = mila_strdup(pattern);
    char *p = pat_copy;

    if (*p == '[')
        p++;
    char *bracket = strchr(p, ']');
    if (bracket)
        *bracket = '\0';

    char *save_ptr = NULL;
    char *token = strtok_r(p, ",", &save_ptr);
    size_t idx = 0;
    char *spread_name = NULL;
    Value *rest_list = NULL;
    Value *result = call_native_with(env, native_new_dict, NULL);

    while (token)
    {
        while (*token == ' ' || *token == '\t')
            token++;
        char *tok_end = token + strlen(token) - 1;
        while (tok_end > token && (*tok_end == ' ' || *tok_end == '\t'))
        {
            *tok_end = '\0';
            tok_end--;
        }

        if (strncmp(token, "...", 3) == 0)
        {
            spread_name = mila_strdup(token + 3);
            rest_list = call_native_with(env, native_list_new, NULL);
        }
        else if (!spread_name && idx < list->size)
        {
            Value *val = ll_get(list, idx);
            val_release(call_native_with(env, native_set_dict, val_retain(result), vstring_dup(token),
                                         val ? val_retain(val) : vnull(), NULL));
            val_release(val);
            idx++;
        }

        token = strtok_r(NULL, ",", &save_ptr);
    }

    if (spread_name)
    {
        while (idx < list->size)
        {
            Value *val = ll_get(list, idx++);
            val_release(call_native_with(env, native_list_append, val_retain(rest_list),
                                         val ? val_retain(val) : vnull(), NULL));
        }
        val_release(call_native_with(env, native_set_dict, result, vstring_dup(spread_name),
                                     rest_list, NULL));
    }

    mila_free(pat_copy);
    if (spread_name)
        mila_free(spread_name);

    return result;
}

Value *native_list_deconstruct(Env *env, int argc, Value **argv)
{
    if (argc != 2)
        return verror("list.deconstruct(pattern, list): Expected 2 args!");
    if (GET_TYPE(argv[0]) != T_STRING)
        return verror("Pattern must be string");
    if (strcmp(GET_TYPENAME(argv[1]), MILA_LPREFIX "list"))
        return verror("Must be list");

    char *pattern = GET_STRING(argv[0]);
    LinkedList *list = (LinkedList *)GET_OPAQUE(argv[1]);

    char *pat_copy = mila_strdup(pattern);
    char *p = pat_copy;

    if (*p == '[')
        p++;
    char *end = p + strlen(p) - 1;
    while (end > p && (*end == ']' || *end == ' ' || *end == '\t'))
        end--;
    end[1] = '\0';

    size_t idx = 0;
    char *spread_name = NULL;
    Value *rest_list = NULL;
    Value *result = call_native_with(env, native_new_dict, NULL);

    char *curr = p;
    while (*curr)
    {
        while (*curr == ' ' || *curr == '\t')
            curr++;
        if (!*curr)
            break;

        char *tok_start = curr;
        int depth = 0;

        while (*curr && (depth > 0 || (*curr != ',' && *curr != '\0')))
        {
            if (*curr == '[')
                depth++;
            else if (*curr == ']')
                depth--;
            curr++;
        }

        char *tok_end = curr - 1;
        while (tok_end > tok_start && (*tok_end == ' ' || *tok_end == '\t'))
            tok_end--;

        char token_buf[512];
        int tok_len = tok_end - tok_start + 1;
        if (tok_len > 0 && tok_len < 511)
        {
            memcpy(token_buf, tok_start, tok_len);
            token_buf[tok_len] = '\0';

            if (strncmp(token_buf, "...", 3) == 0)
            {
                spread_name = mila_strdup(token_buf + 3);
                rest_list = call_native_with(env, native_list_new, NULL);
            }
            else if (!spread_name && idx < list->size)
            {
                Value *val = ll_get(list, idx);

                if (*token_buf == '[')
                {
                    Value *nested = call_native_with(env, native_list_deconstruct, vstring_dup(token_buf),
                                                     val_retain(val), NULL);
                    if (nested && nested->type == T_OPAQUE)
                    {
                        Dict *nested_dict = (Dict *)GET_OPAQUE(nested);
                        Value **keys = dict_keys(nested_dict);
                        for (size_t i = 0; keys[i]; i++)
                        {
                            Value *v = dict_get(nested_dict, keys[i]);
                            val_release(call_native_with(env, native_set_dict, val_retain(result), keys[i],
                                                         val_retain(v), NULL));
                            val_release(v);
                        }
                        free(keys);
                        val_release(nested);
                    }
                }
                else
                {
                    val_release(call_native_with(env, native_set_dict, val_retain(result), vstring_dup(token_buf),
                                                 val ? val : vnull(), NULL));
                }
                idx++;
            }
        }

        if (*curr == ',')
            curr++;
    }

    if (spread_name)
    {
        while (idx < list->size)
        {
            Value *val = ll_get(list, idx++);
            val_release(call_native_with(env, native_list_append, val_retain(rest_list),
                                         val ? val_retain(val) : vnull(), NULL));
        }
        val_release(call_native_with(env, native_set_dict, val_retain(result), vstring_dup(spread_name),
                                     rest_list, NULL));
        val_release(rest_list);
    }

    mila_free(pat_copy);
    if (spread_name)
        mila_free(spread_name);

    return result;
}

#ifdef EXT_SOCK
#include "addon/ml_socket.c"
#endif

#ifdef EXT_HTTP
#include "addon/http/http.c"
#endif

void env_register_builtins(Env *g)
{
    // === Setup

    dict_meta = val_make_table();

    val_set_method_table(dict_meta, UMethodToString, dict_display);
    val_set_method_table(dict_meta, UMethodFree, free_dict);
    val_set_method_table(dict_meta, BMethodGetItem, get_dict);
    val_set_method_table(dict_meta, TMethodSetItem, set_dict);
    val_set_method_table(dict_meta, UMethodCopy, dict_copy);

    list_meta = val_make_table();

    val_set_method_table(list_meta, UMethodToRepr, list_repr);
    val_set_method_table(list_meta, UMethodToString, list_str);
    val_set_method_table(list_meta, UMethodFree, list_free);
    val_set_method_table(list_meta, BMethodGetItem, get_list);
    val_set_method_table(list_meta, TMethodSetItem, set_list);
    val_set_method_table(list_meta, UMethodStepIterInit, ll_iter_init);
    val_set_method_table(list_meta, UMethodStepIter, ll_iter_next);
    val_set_method_table(list_meta, UMethodStepIterClean, ll_iter_cleanup);
    val_set_method_table(list_meta, UMethodCopy, ll_copy);

    array_meta = val_make_table();

    // val_set_method_table(array_meta, UMethodToIter, array_to_iter);
    val_set_method_table(array_meta, UMethodToString, array_to_str);
    val_set_method_table(array_meta, UMethodToRepr, array_to_repr);
    val_set_method_table(array_meta, BMethodGetItem, get_array);
    val_set_method_table(array_meta, TMethodSetItem, set_array);
    val_set_method_table(array_meta, UMethodFree, free_array);
    val_set_method_table(array_meta, UMethodStepIterInit, array_iter_init);
    val_set_method_table(array_meta, UMethodStepIter, array_iter_next);
    val_set_method_table(array_meta, UMethodStepIterClean, array_iter_cleanup);

    range_meta = val_make_table();

    val_set_method_table(range_meta, UMethodToIter, range_to_iter);
    val_set_method_table(range_meta, UMethodToString, range_to_str);
    val_set_method_table(range_meta, UMethodFree, range_free);

    istring_meta = val_make_table();

    val_set_method_table(istring_meta, UMethodToIter, istring_to_iter);
    val_set_method_table(istring_meta, BMethodGetItem, istring_get);
    val_set_method_table(istring_meta, UMethodToString, istring_to_str);

    // canonical builtins reports version
    env_set_raw(g, "__mila_version", make_list(vint(MILA_EDITION), vint(MILA_VERSION), vint(MILA_PATCH), NULL));
#ifdef SAFE_BUILD
    env_set_raw(g, "__mila_codename", vstring_dup("mila:safe_canon"));
#else
    env_set_raw(g, "__mila_codename", vstring_dup("mila:canon"));
#endif

    // === Misc
    env_register_native(g, "range", native_range);
    env_register_native(g, "own", native_own);
    env_register_native(g, "unown", native_unown);
    env_register_native(g, "copy", native_copy);
    env_register_native(g, "repr", native_repr);
    env_register_native(g, "repr_raw", native_repr_raw);
    env_register_native(g, "random", native_random);
    env_register_native(g, "srandom", native_srandom);
    env_register_native(g, "noise", native_noise);
    env_register_native(g, "crandom", native_crandom);
    env_register_native(g, "dump_mila_search_paths", native_dump_search_list);
    env_register_native(g, "is", native_is);
    env_register_native(g, "hash", native_hash);
    env_register_native(g, "hash.set_seed", native_hash_set_seed);
    env_register_native(g, "hash._get_seed", native_hash_get_seed);
    // === Text IO
    env_register_native(g, "print", native_print);
    env_register_native(g, "printr", native_printr);
    env_register_native(g, "println", native_println);
    env_register_native(g, "input", native_input);
    // === Logic and Bitwise
    env_register_native(g, "and", native_bitwise_and);
    env_register_native(g, "or", native_bitwise_or);
    env_register_native(g, "xor", native_bitwise_xor);
    env_register_native(g, "not", native_not);
#ifndef ML_NO_FILE_IO
    // === File IO
    env_register_native(g, "open", native_open);
    env_register_native(g, "fdopen", native_fdopen);
    env_register_native(g, "fdredirect", native_fdredirect);
    env_register_native(g, "fclose", native_fclose);
    env_register_native(g, "close", native_close);
    env_register_native(g, "fprint", native_fprint);
    env_register_native(g, "fprint_bytes", native_fprint_bytes);
    env_register_native(g, "fread", native_fread);
    env_register_native(g, "fread_all", native_fread_all);
    env_register_native(g, "fread_bytes", native_fread_bytes);
    env_register_native(g, "fread_all_bytes", native_fread_all_bytes);
    env_register_native(g, "fseek", native_fseek);
    env_register_native(g, "ftell", native_ftell);
    env_register_native(g, "fflush", native_fflush);
    env_register_native(g, "file.exists", native_file_exists);
    env_register_native(g, "file.is_file", native_file_is_file);
    env_register_native(g, "file.is_dir", native_file_is_dir);
    env_register_native(g, "file.list_dir", native_file_list_dir);
    env_set_raw(g, "SEEK_SET", vint(SEEK_SET));
    env_set_raw(g, "SEEK_END", vint(SEEK_END));
    env_set_raw(g, "SEEK_CUR", vint(SEEK_CUR));
#ifdef _WIN32
    env_set_raw(g, "PATH_SEP", vstring_dup("\\"));
#else
    env_set_raw(g, "PATH_SEP", vstring_dup("/"));
#endif
    env_set_raw(g, "stderr", vopaque_extra(stderr, NULL, "'stderr'"));
    env_set_raw(g, "stdout", vopaque_extra(stdout, NULL, "'stdout'"));
    env_set_raw(g, "stdin", vopaque_extra(stdin, NULL, "'stdin'"));
    env_set_raw(g, "stderr_fd", vint(STDERR_FILENO));
    env_set_raw(g, "stdout_fd", vint(STDOUT_FILENO));
    env_set_raw(g, "stdin_fd", vint(STDIN_FILENO));
    file_meta = val_make_table();
    val_set_method_table(file_meta, UMethodToString, file_printer);
#endif // ML_NO_FILE_IO

    // === Lists
    env_register_native(g, "list", native_list_new);
    env_register_native(g, "list.pop", native_list_pop);
    env_register_native(g, "list.len", native_list_len);
    env_register_native(g, "list.append", native_list_append);
    env_register_native(g, "list.contains", native_list_contains);
    env_register_native(g, "list.index", native_list_index);
    env_register_native(g, "list.slice", native_list_slice);
    env_register_native(g, "list.deconstruct", native_list_deconstruct);
    // === Array
    env_register_native(g, "array", native_new_array);
    env_register_native(g, "array.from", native_from_array);
    env_register_native(g, "array.len", native_len_array);
    // === Dicts
    env_register_native(g, "dict", native_new_dict);
    env_register_native(g, "dict.rem", native_rem_dict);
    env_register_native(g, "dict.keys", native_keys_dict);
    // === Casting
    env_register_native(g, "cast.int", native_cast_int);
    env_register_native(g, "cast.float", native_cast_float);
    env_register_native(g, "cast.str", native_cast_string);
    env_register_native(g, "cast.i2f", native_cast_int_to_float);
    env_register_native(g, "cast.i2u", native_cast_int_to_uint);
    env_register_native(g, "cast.u2i", native_cast_uint_to_int);
    env_register_native(g, "cast.f2i", native_cast_float_to_int);
    env_register_native(g, "typeof", native_type_of);
    env_register_native(g, "_typeof", native_type_of_extra);
    env_register_native(g, "is_numeric", native_is_numeric);
#ifndef ML_NO_C_CAST
    env_register_native(g, "as_opaque", native_as_opaque);
    env_register_native(g, "from_opaque", native_from_opaque);
#endif
    /*
     * _typeof differentiates between native and non native functions
     * this is for very specific use cases
     */
    // === JSON
    env_register_native(g, "mjson.loads", native_mjson_loads);
    env_register_native(g, "mjson.dumps", native_mjson_dumps);
    env_register_native(g, "json.loads", native_json_loads);
    env_register_native(g, "json.dumps", native_json_dumps);
    // === String
    env_register_native(g, "str.slice", native_str_slice);
    env_register_native(g, "str.index", native_str_index);
    env_register_native(g, "str.patch", native_str_patch);
    env_register_native(g, "str.copy", native_str_copy);
    env_register_native(g, "str.len", native_str_len);
    env_register_native(g, "str.pop_f", native_str_pop_start);
    env_register_native(g, "str.pop_b", native_str_pop_end);
    env_register_native(g, "str.split", native_str_split);
    env_register_native(g, "str.join", native_str_join);
    env_register_native(g, "str.startswith", native_str_startsw);
    env_register_native(g, "str.endswith", native_str_endsw);
    env_register_native(g, "str.contains", native_str_contains);
    env_register_native(g, "str.caseless_contains", native_str_contains_caseless);
    env_register_native(g, "str.find", native_str_find);
    env_register_native(g, "str.caseless_find", native_str_caseless_find);
    env_register_native(g, "str.match_replace", native_str_match_replace);
    env_register_native(g, "str.match_find", native_str_match_find);

    env_register_native(g, "istring", native_istring);
    // === ASCII
    env_register_native(g, "ascii.from_int", native_ascii_from_int);
    env_register_native(g, "ascii.from_string", native_ascii_from_string);
    // === Math
#ifndef ML_NO_MATH
    env_register_native(g, "floor", native_floor);
    env_register_native(g, "ceil", native_ceil);
    env_register_native(g, "sqrt", native_sqrt);
    env_register_native(g, "sqrtf", native_sqrtf);
    env_register_native(g, "sin", native_sin);
    env_register_native(g, "cos", native_cos);
    env_register_native(g, "tan", native_tan);
    env_register_native(g, "atan2", native_atan2);
    env_register_native(g, "pow", native_pow);
    env_register_native(g, "rand", native_rand);
    env_register_native(g, "fabs", native_fabs);
    env_register_native(g, "abs", native_abs);
    env_set_raw(g, "INF", vfloat(INFINITY));
    env_set_raw(g, "NINF", vfloat(-INFINITY));
#endif // ML_NO_MATH
    env_set_raw(g, "RAND_MAX", vint(RAND_MAX));
    // === Error handling
    env_register_native(g, "report", native_report);
    env_register_native(g, "report_tagged", native_report_tagged);
    env_register_native(g, "assert", native_assert);
    env_set_local_raw(g, "E_PRE_RUNTIME", vint(E_PRE_RUNTIME));
    env_set_local_raw(g, "E_RUNTIME", vint(E_RUNTIME));
    env_set_local_raw(g, "E_TYPE_ERROR", vint(E_TYPE_ERROR));
    env_set_local_raw(g, "E_FATAL", vint(E_FATAL));
    env_set_local_raw(g, "E_GENERIC", vint(E_GENERIC));
    env_set_local_raw(g, "E_SYNTAX_ERROR", vint(E_SYNTAX_ERROR));
    env_set_local_raw(g, "E_EXIT", vint(E_EXIT));
    env_set_local_raw(g, "E_ASSERT", vint(E_ASSERT));
    env_set_local_raw(g, "E_THREAD_HALT", vint(E_THREAD_HALT));
    env_register_native(g, "exit", native_exit);
    env_register_native(g, "abort", native_abort);
    // === Time measurement
#ifndef ML_NO_TIME
    env_register_native(g, "get_time", native_get_time);
    env_register_native(g, "time_sleep", native_time_sleep);
    env_register_native(g, "time_sleep_ms", native_time_sleep_ms);
    env_register_native(g, "strftime", native_strftime);
    env_register_native(g, "get_tm_gmt", native_get_tm_gmt);
    env_register_native(g, "get_tm_local", native_get_tm_local);
#endif
    // === Debugging
    env_register_native(g, "_breakpoint", native_break_point);
    // === OS Stuff
#ifndef ML_NO_PLATFORM
    env_register_native(g, "system", native_system);
    env_register_native(g, "sys.get_platform", native_sys_get_platform);
    env_register_native(g, "sys.get_arch", native_sys_get_arch);
    env_register_native(g, "sys.get_pid", native_sys_get_pid);
#endif
    // === Modules and Libs
#ifndef ML_NO_EXECUTABLES
    env_register_native(g, "run", native_run);         // runs file
    env_register_native(g, "require", native_require); // runs file if not cached
    env_register_native(g, "invoke", native_run);      // invokes file
    env_register_native(g, "load", native_load);       // loads dlls or so file
    env_register_native(g, "eval", native_eval);       // runs string
#endif
    // === Threading
#ifndef ML_NO_THREADING
    env_register_native(g, "thread.make", native_thread_create);
    env_register_native(g, "thread.join", native_thread_join);
    env_register_native(g, "thread.cancel", native_thread_cancel);
    env_register_native(g, "thread.check_cancel", native_thread_check_cancel);
    env_register_native(g, "thread.set_daemon", native_thread_set_daemon);
    env_register_native(g, "thread.get_pthread_id", native_thread_pthread_id);
    env_register_native(g, "thread.status", native_thread_status);
    env_register_native(g, "thread.mutex", native_make_mutex);
    env_register_native(g, "thread.mutex_unlock", native_mutex_unlock);
    env_register_native(g, "thread.mutex_lock", native_mutex_lock);
    env_register_native(g, "thread.dump", native_thread_dump);
#endif

    // ==== EXTENSIONS (not meant for prod) ====
    // _has_ext standardizes what to check to ensure ext exists
    // this makes sure users dont check function for this
#ifdef EXT_WEB
#include "addon/ml_web.h"
    env_set_local_raw(g, "_has_ext.web", vbool(1));
    env_register_web_ext(g);
#else
    env_set_local_raw(g, "_has_ext.web", vbool(0));
#endif
#ifdef EXT_SOCK
    env_set_local_raw(g, "_has_ext.socket", vbool(1));
    env_register_socket_ext(g);
#else
    env_set_local_raw(g, "_has_ext.socket", vbool(0));
#endif
#ifdef EXT_HTTP
    env_set_local_raw(g, "_has_ext.http", vbool(1));
    env_register_http_ext(g);
#else
    env_set_local_raw(g, "_has_ext.http", vbool(0));
#endif
}