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
#include <string.h>
#include <uchar.h>

#include "ml_dict.c"
#include "ml_ll.c"

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
#endif

#include <time.h>

#define MILA_EDITION 202603
#define MILA_VERSION 2
#define MILA_LPREFIX "canon:"

// Define meta tables

MethodTable *file_meta = NULL;
MethodTable *dict_meta = NULL;
MethodTable *list_meta = NULL;
MethodTable *array_meta = NULL;
MethodTable *range_meta = NULL;
MethodTable *istring_meta = NULL;

Value *self_free(Value *self) {
  val_release(self);
  return NULL;
}

// ---------- Native functions ----------

double get_unix_timestamp() {
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

char *read_input(void) {
  size_t bufsize = 64; // initial buffer size
  size_t len = 0;      // number of chars read
  char *buffer = mila_malloc(bufsize);
  if (!buffer) {
    fprintf(stderr, "read_input: Allocation failed.\n");
    return NULL;
  }

  int c;
  while ((c = getchar()) != EOF && c != '\n') {
    buffer[len++] = (char)c;

    // resize if we're about to overflow
    if (len + 1 >= bufsize) {
      bufsize *= 2;
      char *newbuf = mila_realloc(buffer, bufsize);
      if (!newbuf) {
        mila_free(buffer);
        fprintf(stderr, "Reallocation failed.\n");
        return NULL;
      }
      buffer = newbuf;
    }
  }

  buffer[len] = '\0';
  return buffer;
}

Value *native_pop_start(Env *env, int argc, Value **argv) {
  (void)env;
  (void)argc;
  if (!match_types(argv, T_STRING, T_ARG_END))
    return vnull();
  char *raw_string = argv[0]->v.s;
  char ch = *raw_string; // get first char

  char *copy = mila_strdup(raw_string + 1);

  mila_free(argv[0]->v.s);
  argv[0]->v.s = copy;

  return vstring_dup((char[]){ch, 0});
}

Value *native_pop_end(Env *env, int argc, Value **argv) {
  (void)env;
  (void)argc;
  if (!match_types(argv, T_STRING, T_ARG_END))
    return vnull();
  char *raw_string = argv[0]->v.s;
  char ch = *(raw_string + strlen(raw_string) - 1); // get last char

  uint64_t size = strlen(raw_string) - 1;
  char *copy = (char *)mila_malloc(sizeof(char) * size);
  memcpy(copy, raw_string, size);

  mila_free(argv[0]->v.s);
  argv[0]->v.s = copy;

  return vstring_dup((char[]){ch, 0});
}

Value *native_ascii_from_int(Env *env, int argc, Value **argv) {
  (void)env;
  (void)argc;
  if (!match_types(argv, T_INT, T_ARG_END))
    return vnull();
  return vstring_dup((char[]){(char)argv[0]->v.i, '\0'});
}

Value *native_ascii_from_string(Env *env, int argc, Value **argv) {
  (void)env;
  (void)argc;
  if ((!match_types(argv, T_STRING, T_ARG_END)) || strlen(argv[0]->v.s) != 1)
    return vnull();
  return vint(argv[0]->v.s[0]);
}

Value *native_str_slice(Env *env, int argc, Value **argv) {
  (void)env;
  (void)argc;
  if (!match_types(argv, T_STRING, T_INT, T_INT, T_ARG_END))
    return vnull();
  return vstring_slice(argv[0]->v.s, argv[1]->v.i, argv[2]->v.i);
}

Value *native_str_copy(Env *env, int argc, Value **argv) {
  (void)env;
  (void)argc;
  if (!match_types(argv, T_STRING))
    return vnull();
  return vstring_dup(argv[0]->v.s);
}

Value *native_str_index(Env *env, int argc, Value **argv) {
  (void)env;
  (void)argc;
  if (!match_types(argv, T_STRING, T_INT, T_ARG_END))
    return vnull();
  return vstring_index(argv[0]->v.s, argv[1]->v.i);
}

Value *native_str_patch(Env *env, int argc, Value **argv) {
  (void)env;
  (void)argc;
  if (!match_types(argv, T_STRING, T_STRING, T_STRING, T_ARG_END))
    return vnull();
  return vstring_replace(argv[0]->v.s, argv[1]->v.s, argv[2]->v.s);
}

Value *native_str_len(Env *env, int argc, Value **argv) {
  (void)env;
  (void)argc;
  if (!match_types(argv, T_STRING, T_ARG_END))
    return vnull();
  return vint(strlen(argv[0]->v.s));
}

Value *native_bitwise_and(Env *env, int argc, Value **argv) {
  (void)env;
  (void)argc;
  if (!match_types(argv, T_INT, T_INT, T_ARG_END))
    return vnull();
  return vint(argv[0]->v.i & argv[1]->v.i);
}

Value *native_bitwise_or(Env *env, int argc, Value **argv) {
  (void)env;
  (void)argc;
  if (!match_types(argv, T_INT, T_INT, T_ARG_END))
    return vnull();
  return vint(argv[0]->v.i | argv[1]->v.i);
}

Value *native_bitwise_xor(Env *env, int argc, Value **argv) {
  (void)env;
  (void)argc;
  if (!match_types(argv, T_INT, T_INT, T_ARG_END))
    return vnull();
  return vint(argv[0]->v.i ^ argv[1]->v.i);
}

Value *native_not(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1)
    return vnull();
  return is_truthy(argv[0]) ? vbool(0) : vbool(1);
}

Value *native_print(Env *env, int argc, Value **argv) {
  (void)env;
  for (int i = 0; i < argc; i++) {
    if (i)
      printf(" ");
    print_value(argv[i]);
  }
  return vnull();
}

Value *native_printr(Env *env, int argc, Value **argv) {
  (void)env;
  for (int i = 0; i < argc; i++) {
    print_value(argv[i]);
  }
  return vnull();
}

Value *native_println(Env *env, int argc, Value **argv) {
  (void)env;
  for (int i = 0; i < argc; i++) {
    if (i)
      printf(" ");
    print_value(argv[i]);
  }
  putchar(10);
  return vnull();
}

Value *native_input(Env *env, int argc, Value **argv) {
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

Value *list_to_iter(Value *self) {
  Value **iter = ll_to_iter(self->v.opaque);
  return vopaque(iter);
}

Value *list_repr(Value *self) {
  LinkedList *lst = (LinkedList *)self->v.opaque;
  if (lst->size > MAX_ITEMS_DISPLAYED)
    return vstring_fmt("list(%zu items)", lst->size);
  Value **iter = ll_to_iter(lst);

  char *buffer = mila_strdup("list(");
  for (int i = 0; iter[i]; i++) {
    char *repr = as_c_string_repr(iter[i]);
    if (i < lst->size - 1)
      our_asprintf(&buffer, "%s, ", repr);
    else
      our_asprintf(&buffer, "%s", repr);
    val_release(iter[i]);
    mila_free(repr);
  }
  our_asprintf(&buffer, ")");
  mila_free(iter);
  return vstring_take(buffer);
}

Value *list_str(Value *self) {
  LinkedList *lst = (LinkedList *)self->v.opaque;
  Value **iter = ll_to_iter(lst);

  char *buffer = mila_strdup("list(");
  for (int i = 0; iter[i]; i++) {
    char *repr = as_c_string_repr(iter[i]);
    if (i < lst->size - 1)
      our_asprintf(&buffer, "%s, ", repr);
    else
      our_asprintf(&buffer, "%s", repr);
    val_release(iter[i]);
    mila_free(repr);
  }
  our_asprintf(&buffer, ")");
  mila_free(iter);
  return vstring_take(buffer);
}

Value *native_list_new(Env *e, int argc, Value **argv) {
  (void)e;
  LinkedList *list = ll_create();
  for (int i = 0; i < argc; i++) {
    ll_append(list, val_retain(argv[i]));
  }
  Value *res = vopaque_extra(list, NULL, MILA_LPREFIX "list");
  val_set_table(res, list_meta);
  return res;
}

Value *native_list_set(Env *e, int argc, Value **argv) {
  (void)e;
  (void)argc;
  ll_set(GET_OPAQUE(argv[0]), GET_INTEGER(argv[1]), val_retain(argv[2]));
  return NULL;
}

Value *native_list_get(Env *e, int argc, Value **argv) {
  (void)e;
  (void)argc;
  return ll_get(GET_OPAQUE(argv[0]), GET_INTEGER(argv[1]));
}

Value *set_list(Value *self, Value *index, Value *value) {
  ll_set(self->v.opaque, index->v.i, val_retain(value));
  return NULL;
}

Value *get_list(Value *self, Value *index) {
  return ll_get(self->v.opaque, index->v.i);
}

Value *native_list_len(Env *e, int argc, Value **argv) {
  (void)e;
  if (argc != 1)
    return verror("list.len(l): requires one argument!");
  LinkedList *ll = (LinkedList *)argv[0]->v.opaque;
  return vint(ll->size);
}

Value *native_list_pop(Env *e, int argc, Value **argv) {
  if (argc == 1)
    return ll_pop(argv[0]->v.opaque, -1);
  else if (argc == 2)
    return ll_pop(argv[0]->v.opaque, argv[1]->v.i);
  return vtagged_error(E_RUNTIME,
                       "list.pop(l, index?): Missing list argument!");
}

Value *list_free(Value *self) {
  Value **iter = ll_to_iter(self->v.opaque);

  for (int i = 0; iter[i]; i++) {
    val_release(iter[i]);
    val_release(iter[i]);
  }

  mila_free(iter);
  ll_free(self->v.opaque);
  self->type = T_NULL;
  self->v.opaque = NULL;
  return NULL;
}

Value *native_cast_int(Env *env, int argc, Value **argv) {
  (void)env;
  long i = 0;
  if (argc == 1 && argv[0]->type == T_STRING) {
    char *end;
    i = strtol(argv[0]->v.s, &end, 10);

    if (*end != '\0') {
      char *buffer = NULL;
      our_asprintf(&buffer, "cast.int(str): Got bad part \"%s\"...", end);
      Value *tmp = vtagged_error(E_TYPE_ERROR, "%s\n", buffer);
      mila_free(buffer);
      i = 0;
      return tmp;
    }
  } else {
    return verror("cast.int(str): Expected 1 argument (str) string.\n");
    i = 0;
  }
  return vint(i);
}

Value *native_cast_float(Env *env, int argc, Value **argv) {
  (void)env;
  double f = 0;
  if (argc == 1 && argv[0]->type == T_STRING) {
    char *end;
    f = strtod(argv[0]->v.s, &end);

    if (*end != '\0') {
      char *buffer = NULL;
      our_asprintf(&buffer, "cast.float(str): Got bad part \"%s\"...", end);
      Value *tmp = vtagged_error(E_TYPE_ERROR, "%s\n", buffer);
      mila_free(buffer);
      f = 0;
    }
  } else {
    return verror("cast.float(str): Expected 1 argument (str) string.\n");
    f = 0;
  }
  return vfloat(f);
}

Value *native_cast_int_to_uint(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc == 1 && argv[0]->type == T_INT) {
    return vuint(argv[0]->v.ui);
  } else {
    return verror("cast.i2u(int): Expected 1 argument (int) int. Got %s\n",
                  MILA_GET_TYPENAME(argv[0]));
  }
}

Value *native_cast_uint_to_int(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc == 1 && argv[0]->type == T_UINT) {
    return vint(argv[0]->v.i);
  } else {
    return verror("cast.u2i(uint): Expected 1 argument (uint) uint. Got %s\n",
                  MILA_GET_TYPENAME(argv[0]));
  }
}

Value *native_cast_int_to_float(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc == 1 && argv[0]->type == T_INT) {
    return vfloat(to_double(argv[0]));
  } else {
    return verror("cast.i2f(int): Expected 1 argument (int) int. Got %s\n",
                  MILA_GET_TYPENAME(argv[0]));
  }
}

Value *native_cast_float_to_int(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc == 1 && argv[0]->type == T_FLOAT) {
    return vint((long)argv[0]->v.f);
  } else {
    return verror(
        "cast.f2i(float): Expected 1 argument (float) float. Got %s\n",
        MILA_GET_TYPENAME(argv[0]));
  }
}

Value *native_cast_string(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc == 1) {
    return vstring_take(as_c_string(argv[0]));
  } else {
    return verror("cast.string(any): Expected 1 argument (any) any.\n");
  }
  return vnull();
}

Value *native_type_of(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1) {
    return verror("typeof(any): Expected 1 argument (any) any.\n");
  }
  if (argv[0]->type_name)
    return vstring_dup(argv[0]->type_name);
  return vstring_dup(MILA_GET_TYPENAME(argv[0]));
}

#ifndef VMM_BUILD

Value *file_printer(Value *self) {
  char *buffer = NULL;
  if (!self || self->type != T_OPAQUE) {
    our_asprintf(&buffer, "<not-a-file>");
    return vstring_take(buffer);
  }
  FILE *f = (FILE *)self->v.opaque;
  if (!f) {
    our_asprintf(&buffer, "<file:closed>");
  } else {
    our_asprintf(&buffer, "<file:%p>", f);
  }
  return vstring_take(buffer);
}

Value *native_open(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 2 || argv[0]->type != T_STRING || argv[1]->type != T_STRING) {
    return verror("= open(filename, mode) expects 2 string args.\n");
  }
  char *path = argv[0]->v.s;
  if (!search_path) {
    char *path = path_list_find(search_path, argv[0]->v.s);
    if (!path) {
      return verror("= open(filename, mode) did not find the file.\n");
    }
  }
  char *res = path_list_find(search_path, path);
  if (!res)
    return verror("File %s not found!", path);

  FILE *f = fopen(res, argv[1]->v.s);
  if (!f) {
    if (res)
      mila_free(res);
    perror(NULL);
    return vnull();
  }

  if (res)
    mila_free(res);
  Value *v = vopaque(f);
  val_set_table(v, file_meta);
  return v;
}

Value *native_fclose(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1 || argv[0]->type != T_OPAQUE) {
    return verror("= fclose(file) expects 1 file handle arg.\n");
  }
  FILE *f = (FILE *)argv[0]->v.opaque;
  if (f) {
    fclose(f);
    argv[0]->v.opaque = NULL; // Prevent double close
  }
  return vnull();
}

Value *native_fflush(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1 || argv[0]->type != T_OPAQUE) {
    return verror("= fflush(file) expects 1 file handle arg.\n");
  }
  FILE *f = (FILE *)argv[0]->v.opaque;
  if (f) {
    fflush(f);
    argv[0]->v.opaque = NULL; // Prevent double close
  }
  return vnull();
}

Value *native_fprint(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 2 || argv[0]->type != T_OPAQUE || argv[1]->type != T_STRING) {
    return verror("= fprint(file, string) expects (handle, string).\n");
  }
  FILE *f = (FILE *)argv[0]->v.opaque;
  if (!f) {
    return verror("= fprint: file handle is closed or invalid.\n");
  }
  const char *s = argv[1]->v.s;
  size_t written = fwrite(s, 1, strlen(s), f);
  return vint(written);
}

Value *native_fread(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 2 || argv[0]->type != T_OPAQUE || argv[1]->type != T_INT) {
    return verror("= fread(file, num_bytes) expects (handle, int).\n");
  }
  FILE *f = (FILE *)argv[0]->v.opaque;
  if (!f) {
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

Value *native_fseek(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 3 || argv[0]->type != T_OPAQUE || argv[1]->type != T_INT ||
      argv[2]->type != T_INT) {
    return verror(
        "= fseek(file, offset, whence) expects (handle, int, int).\n");
  }
  FILE *f = (FILE *)argv[0]->v.opaque;
  if (!f) {
    return verror("= fseek: file handle is closed or invalid.\n");
  }
  long offset = argv[1]->v.i;
  int whence = (int)argv[2]->v.i;
  int c_whence;

  switch (whence) {
  case 0:
  case 1:
  case 2:
    c_whence = whence;
    break;
  default:
    return verror("= fseek: invalid whence %d (must be 0-SEEK_SET, 1-SEEK_CUR, "
                  "or 2-SEEK_END).\n",
                  whence);
  }

  int res = fseek(f, offset, c_whence);
  return vint(res);
}

Value *native_ftell(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1 || argv[0]->type != T_OPAQUE) {
    return verror("= ftell(file) expects 1 file handle arg.\n");
  }
  FILE *f = (FILE *)argv[0]->v.opaque;
  if (!f) {
    return verror("ftell: file handle is closed or invalid.\n");
  }
  long pos = ftell(f);
  return vint(pos);
}

#endif

/* Array Functions: store Value* per slot so we keep proper refcounts */
typedef struct {
  Value **array; /* array of Value* */
  int size;
} Array;

Value *array_to_str(Value *self) {
  char *buffer = NULL;
  if (!self || self->type != T_OPAQUE) {
    our_asprintf(&buffer, "<not-an-array>");
    return vstring_take(buffer);
  }

  Array *arr = (Array *)self->v.opaque;
  if (!arr) {
    our_asprintf(&buffer, "<null-array-data>");
    return vstring_take(buffer);
  }

  our_asprintf(&buffer, "array.from(");
  for (int i = 0; i < arr->size; i++) {
    Value *slot = arr->array[i];
    if (!slot) {
      our_asprintf(&buffer, "?null?");
    } else {
      char *s = as_c_string_repr(slot);
      our_asprintf(&buffer, "%s", s);
      mila_free(s);
    }
    if (i < arr->size - 1)
      our_asprintf(&buffer, ", ");
  }
  our_asprintf(&buffer, ")");
  return vstring_take(buffer);
}

Value *array_to_repr(Value *self) {
  char *buffer = NULL;
  if (!self || self->type != T_OPAQUE) {
    our_asprintf(&buffer, "<not-an-array>");
    return vstring_take(buffer);
  }

  Array *arr = (Array *)self->v.opaque;
  if (!arr) {
    our_asprintf(&buffer, "<null-array-data>");
    return vstring_take(buffer);
  }

  if (arr->size > MAX_ITEMS_DISPLAYED)
    return vstring_fmt("array(%i items)", arr->size);

  our_asprintf(&buffer, "array.from(");
  for (int i = 0; i < arr->size; i++) {
    Value *slot = arr->array[i];
    if (!slot) {
      our_asprintf(&buffer, "?null?");
    } else {
      char *s = as_c_string_repr(slot);
      our_asprintf(&buffer, "%s", s);
      mila_free(s);
    }
    if (i < arr->size - 1)
      our_asprintf(&buffer, ", ");
  }
  our_asprintf(&buffer, ")");
  return vstring_take(buffer);
}

Value *array_to_iter(Value *self) {
  Value *arrv = self;
  if (arrv->type != T_OPAQUE) {
    return verror("array<UMethodToIter>: first arg must be an array (opaque)");
  }

  Array *arr = (Array *)arrv->v.opaque;
  if (!arr) {
    return verror("array<UMethodToIter>: null array data");
  }

  Value **values = (Value **)mila_malloc(sizeof(Value *) * (arr->size + 1));
  values[arr->size] = NULL;

  int i = 0;
  for (int t = 0; t < arr->size; ++t) {
    if (arr->array[t] == NULL)
      continue;
    values[i++] = val_retain(arr->array[t]);
  }

  return vopaque(values);
}

typedef struct {
  long start;
  long end;
  long step;
} Range;

size_t range_len(long start, long stop, long step) {
  if (step == 0)
    return 0;
  if (step > 0) {
    if (start >= stop)
      return 0;
    return (stop - start + step - 1) / step;
  } else {
    if (start <= stop)
      return 0;
    return (start - stop - step - 1) / (-step);
  }
}

Value *range_to_iter(Value *self) {
  Range *data = (Range *)(self->v.opaque);
  Value **v = (Value **)mila_malloc(
      sizeof(Value *) * (range_len(data->start, data->end, data->step) + 1));
  long index = 0;
  for (long i = data->start; i < data->end; i += data->step) {
    v[index++] = vint(i);
  }
  v[index] = NULL;
  return vopaque(v);
}

Value *range_to_str(Value *self) {
  Range *data = (Range *)(self->v.opaque);
  return vstring_fmt("range(%zu, %zu, %zu)", data->start, data->end,
                     data->step);
}

Value *range_free(Value *self) {
  mila_free(self->v.opaque);
  return NULL;
}

Value *native_range(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc == 1 && argv[0]->type == T_INT) {
    Range *r = (Range *)mila_malloc(sizeof(Range));
    r->start = 0;
    r->end = argv[0]->v.i;
    r->step = 1;
    Value *res = vopaque_extra(r, NULL, MILA_LPREFIX "range");
    val_set_table(res, range_meta);
    return res;
  }
  if (argc == 2 && argv[0]->type == T_INT && argv[1]->type == T_INT) {
    Range *r = (Range *)mila_malloc(sizeof(Range));
    r->start = argv[0]->v.i;
    r->end = argv[1]->v.i;
    r->step = 1;
    Value *res = vopaque_extra(r, NULL, MILA_LPREFIX "range");
    val_set_table(res, range_meta);
    return res;
  }
  if (argc == 3 && argv[0]->type == T_INT && argv[1]->type == T_INT &&
      argv[2]->type == T_INT) {
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

Value *native_new_array(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1) {
    return verror("array(size): Requires one argument, array size (int)\n");
  }
  if (!match_types(argv, T_INT, T_ARG_END)) {
    return verror("array(size): Expected the argument type int\n");
  }

  int size = (int)argv[0]->v.i;
  if (size < 0) {
    return verror("array(size): negative size\n");
  }

  Value *res = val_new(T_OPAQUE);
  Array *array = mila_malloc(sizeof(Array));
  array->size = size;
  array->array = mila_malloc(sizeof(Value *) * size);

  for (int i = 0; i < size; i++) {
    array->array[i] = NULL;
  }

  res->v.opaque = array;
  res->type_name = mila_strdup(MILA_LPREFIX "array");
  val_set_table(res, array_meta);
  return res;
}

Value *native_from_array(Env *env, int argc, Value **argv) {
  (void)env;

  int size = argc;

  Value *res = val_new(T_OPAQUE);
  Array *array = mila_malloc(sizeof(Array));
  array->size = size;
  array->array = mila_malloc(sizeof(Value *) * size);

  for (int i = 0; i < size; i++) {
    array->array[i] = val_retain(argv[i]);
  }

  res->v.opaque = array;
  res->type_name = mila_strdup(MILA_LPREFIX "array");
  val_set_table(res, array_meta);
  return res;
}

Value *native_set_array(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 3) {
    return verror("array.set(array, index, value): requires 3 args");
  }

  Value *arrv = argv[0];
  if (arrv->type != T_OPAQUE) {
    return verror(
        "array.set(array, index, value): first arg must be an array (opaque)");
  }

  Array *arr = (Array *)arrv->v.opaque;
  if (!arr) {
    return verror("array.set(array, index, value): null array data");
  }

  if (argv[1]->type != T_INT) {
    return verror("array.set(array, index, value): index must be int");
  }

  int idx = (int)argv[1]->v.i;
  if (idx < 0 || idx >= arr->size) {
    return verror(
        "array.set(array, index, value): index %d out of bounds (size %d)", idx,
        arr->size);
  }

  Value *old = arr->array[idx];
  if (old)
    val_release(old);

  arr->array[idx] = argv[2];
  val_retain(argv[2]);

  return vnull();
}

Value *native_get_array(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 2) {
    return verror("array.get(array, index): requires 2 args");
  }

  Value *arrv = argv[0];
  if (arrv->type != T_OPAQUE) {
    return verror(
        "array.get(array, index): first arg must be an array (opaque)");
  }

  Array *arr = (Array *)arrv->v.opaque;
  if (!arr) {
    return verror("array.get(array, index): null array data");
  }

  if (argv[1]->type != T_INT) {
    return verror("array.get(array, index): index must be int");
  }

  int idx = (int)argv[1]->v.i;
  if (idx < 0 || idx >= arr->size) {
    return verror("array.get(array, index): index %d out of bounds (size %d)",
                  idx, arr->size);
  }

  Value *val = arr->array[idx];
  if (val)
    val_retain(val);
  else
    return vnull();

  return val;
}

Value *native_len_array(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1) {
    return verror("array.len(array): requires 1 arg");
  }

  Value *arrv = argv[0];
  if (arrv->type != T_OPAQUE) {
    return verror("array.len(array): first arg must be an array (opaque)");
  }

  Array *arr = (Array *)arrv->v.opaque;
  if (!arr) {
    return verror("array.len(array): null array data");
  }

  return vint(arr->size);
}

Value *get_array(Value *self, Value *index) {
  Value *arrv = self;
  Array *arr = (Array *)arrv->v.opaque;
  if (!arr) {
    return verror("array.get(array, index): null array data");
  }

  if (index->type != T_INT) {
    return verror("array.get(array, index): index must be int");
  }

  int idx = (int)index->v.i;
  if (idx < 0 || idx >= arr->size) {
    return verror("array.get(array, index): index %d out of bounds (size %d)",
                  idx, arr->size);
  }

  Value *val = arr->array[idx];

  return val ? val : vnull();
}

Value *set_array(Value *self, Value *index, Value *val) {
  Value *arrv = self;
  Array *arr = (Array *)arrv->v.opaque;
  if (!arr) {
    return verror("array.set(array, index, value): null array data");
  }

  if (index->type != T_INT) {
    return verror("array.set(array, index, value): index must be int");
  }

  int idx = (int)index->v.i;
  if (idx < 0 || idx >= arr->size) {
    return verror(
        "array.set(array, index, value): index %d out of bounds (size %d)", idx,
        arr->size);
  }

  Value *old = arr->array[idx];
  if (old)
    val_release(old);
  arr->array[idx] = val_retain(val);
}

Value *native_free_array(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1) {
    return verror("array.free(array): requires 1 arg");
  }

  Value *arrv = argv[0];
  if (arrv->type != T_OPAQUE) {
    return verror("array.free(array): first arg must be an array (opaque)");
  }

  Array *arr = (Array *)arrv->v.opaque;
  if (!arr) {
    return verror("array.free(array): null array data");
  }

  for (int i = 0; i < arr->size; i++)
    if (arr->array[i])
      val_release(arr->array[i]);
  mila_free(arr->array);
  mila_free(arr);

  return vnull();
}

Value *free_array(Value *self) {
  Value *arrv = self;
  if (arrv->type != T_OPAQUE) {
    return NULL;
  }

  Array *arr = (Array *)arrv->v.opaque;
  if (!arr) {
    return NULL;
  }

  for (int i = 0; i < arr->size; i++)
    val_release(arr->array[i]);
  mila_free(arr->array);
  mila_free(arr);

  return NULL;
}

Value *native_report(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc == 1 && argv[0]->type == T_STRING)
    return verror("%s", argv[0]->v.s);
  else if (argc == 0)
    return verror("No details given.");
  else
    return verror("report(message): Invalid number of arguments given.");
}

Value *native_report_tagged(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc == 2) {
    return vtagged_error((ErrorType)GET_INTEGER(argv[0]), "%s",
                         GET_STRING(argv[1]));
  } else if (argc == 1) {
    return vtagged_error(E_GENERIC, "No details given.");
  } else
    return verror("report(tag, message): Invalid number of arguments given.");
}

Value *native_exit(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc == 1 && argv[0]->type == T_INT)
    exit((int)argv[0]->v.i);
  else if (argc == 0)
    exit(0);
  else {
    return verror("invalid number of arguments given.");
  }
  return vnull();
}

Value *native_get_time(Env *env, int argc, Value **argv) {
  (void)argc;
  (void)argv;
  (void)env;
  if (argc != 0) {
    return verror("invalid number of arguments given.\n");
  }
  return vfloat(get_unix_timestamp());
}

#ifndef VMM_BUILD
Value *native_run(Env *env, int argc, Value **argv) {
  if (argc != 1 || argv[0]->type != T_STRING) {
    return verror("invalid number of arguments given or incorrect types.");
  }

  if (search_path) {
    char *path = path_list_find(search_path, argv[0]->v.s);
    if (!path) {
      return verror("run(filename) did not find the file.");
    }
    Env *frame = env_new(env);
    Value *by = env_get(env, "__name__");
    env_set_local(frame, "__importer__", by ? by : vstring_dup("???"));
    Value *res = run_file_keep_res(path, frame);
    if (MILA_GET_TYPE(res) == T_ERROR) {
      return res;
    }
    mila_free(path);
    env_free(frame);
    return res;
  }

  return vnull();
}

Value *native_load(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1 || (!argv[0]) || argv[0]->type != T_STRING) {
    return verror("invalid number of arguments given or incorrect types.");
  }

  char *new_path = path_list_find(search_path, argv[0]->v.s);
  if (!new_path)
    return verror("problem loading file %s\n", argv[0]->v.s);
  if (load_library(env, new_path)) {
    mila_free(new_path);
    return verror("problem loading file %s\n", argv[0]->v.s);
  }
  mila_free(new_path);
  return vnull();
}

Value *native_eval(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1 || argv[0]->type != T_STRING) {
    return verror("invalid number of arguments given or incorrect types.");
  }

  return eval_str(argv[0]->v.s, env);
}
#endif

Value *native_new_dict(Env *env, int argc, Value **argv) {
  (void)env;
  if (((argc % 2) != 0) && argc != 0) {
    return verror(
        "dict(...): Provide even number of arguments or none at all! Got %i.",
        argc);
  }

  Dict *d = dict_create();

  for (int i = 0; i < argc;) {
    Value *key = argv[i++];
    Value *value = argv[i++];
    dict_set(d, key, value);
  }

  if (d) {
    Value *v = vopaque(d);
    val_set_table(v, dict_meta);
    return v;
  }
  return verror("couldnt make a dict.");
}

Value *native_set_dict(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 3) {
    return verror("invalid number of arguments given or incorrect types.");
  }

  dict_set(argv[0]->v.opaque, argv[1], val_retain(argv[2]));
  return vnull();
}

Value *native_get_dict(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 2) {
    return verror("invalid number of arguments given or incorrect types.");
  }

  Value *v = dict_get(argv[0]->v.opaque, argv[1]);
  return v ? v : vnull();
}

Value *set_dict(Value *self, Value *name, Value *val) {
  dict_set(self->v.opaque, name, val);
  return NULL;
}

Value *get_dict(Value *self, Value *name) {
  Value *v = dict_get(self->v.opaque, name);
  return v;
}

Value *native_rem_dict(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 2) {
    return verror("invalid number of arguments given or incorrect types.");
  }

  dict_remove(argv[0]->v.opaque, argv[1]);
  return vnull();
}

Value *free_dict(Value *self) {
  dict_free(self->v.opaque);
  return NULL;
}

Value *native_free_dict(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1) {
    return verror("invalid number of arguments given or incorrect types.");
  }

  dict_free(argv[0]->v.opaque);
  return vnull();
}

Value *native_system(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1 || argv[0]->type != T_STRING) {
    return verror("invalid number of arguments given or incorrect types.");
  }
  return vint(system(argv[0]->v.opaque));
}

Value *native_floor(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1)
    return verror("floor(i): Requires one arguments");
  double x = argv[0]->v.f;
  return vfloat(floor(x));
}

Value *native_ceil(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1)
    return verror("ceil(i): Requires one arguments");
  double x = argv[0]->v.f;
  return vfloat(ceil(x));
}

Value *native_sqrtf(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1)
    return verror("sqrt(i): Requires one arguments");
  double x = argv[0]->v.f;
  return vfloat(sqrtf(x));
}

Value *native_sqrt(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1)
    return verror("sqrt(i): Requires one arguments");
  double x = argv[0]->v.i;
  return vfloat(sqrt(x));
}

Value *native_sin(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1)
    return verror("sin(i): Requires one arguments");
  double x = argv[0]->v.f;
  return vfloat(sin(x));
}

Value *native_cos(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1)
    return verror("cos(i): Requires one arguments");
  double x = argv[0]->v.f;
  return vfloat(cos(x));
}

Value *native_tan(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1)
    return verror("tan(i): Requires one arguments");
  double x = argv[0]->v.f;
  return vfloat(tan(x));
}

Value *native_atan2(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 2)
    return verror("atan2(i, i): Requires two arguments");
  double y = argv[0]->v.f;
  double x = argv[1]->v.f;
  return vfloat(atan2(y, x));
}

Value *native_pow(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 2)
    return verror("pow(base, exp): Requires two arguments");
  return vint(pow(argv[0]->v.i, argv[1]->v.i));
}

Value *native_vars_set(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 2)
    return verror("vars.set(name, val): Requires two arguments");
  env_set_local(env, argv[0]->v.s, argv[1]);
  return vnull();
}

Value *native_vars_get(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1)
    return verror("vars.get(name): Requires one argument");
  return env_get(env, argv[0]->v.s);
}

Value *native_vfree(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1)
    return verror("vfree(value): Requires one argument");
  val_release(argv[0]);
  return vnull();
}

Value *native_vars_local(Env *env, int argc, Value **argv) {
  (void)env;
  (void)argv;
  if (argc != 0)
    return verror("vars.local(): Requires no arguments");
  for (Var *v = env->vars; v; v = v->next) {
    printf("%s", v->name);
    if (v->next) {
      printf(", ");
    }
  }
  putchar(10);
  return vnull();
}

Value *native_vars_global(Env *env, int argc, Value **argv) {
  (void)env;
  (void)argv;
  if (argc != 0)
    return verror("vars.local(): Requires no arguments");
  for (Env *cur = env; cur; cur = cur->parent) {
    for (Var *v = cur->vars; v; v = v->next) {
      printf("%s", v->name);
      if (v->next) {
        printf(", ");
      }
    }
  }
  putchar(10);
  return vnull();
}

Value *native_meep(Env *e, int argc, Value **argv) {
  (void)argc;
  (void)argv;
  env_dump(e);
  return vnull();
}

Value *native_list_append(Env *env, int argc, Value **argv) {
  ll_append(argv[0]->v.opaque, val_retain(argv[1]));
  return vnull();
}

Value *native_list_contains(Env *env, int argc, Value **argv) {
  (void)env;
  Value **list = ll_to_iter(GET_OPAQUE(argv[0]));
  for (long i = 0; list[i]; ++i) {
    Value *value = binary_op(argv[1], BMethodEq, list[i]);
    if (is_truthy(value)) {
      val_release(value);
      for (; list[i]; ++i)
        val_release(list[i]);
      mila_free(list);
      return vbool(1);
    }
    val_release(value);
    val_release(list[i]);
  }
  mila_free(list);
  return vbool(0);
}

Value *native_repr(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1)
    return verror("repr(value): Expected at least one argument!");
  return vstring_take(as_c_string_repr(argv[0]));
}

Value *native_repr_raw(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1)
    return verror("repr_repr(value): Expected at least one argument!");
  return vstring_take(as_c_string_repr_raw(argv[0]));
}

Value *native_str(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 1)
    return verror("str(value): Expected at least one argument!");
  return vstring_take(as_c_string(argv[0]));
}

Value *env_free_builtins() {
  mila_free(dict_meta);
  mila_free(array_meta);
  mila_free(list_meta);
  mila_free(file_meta);
  mila_free(range_meta);

  return NULL;
}

Value *native_own(Env *e, int argc, Value **argv) {
  (void)e;
  if (argc == 1 && MILA_GET_TYPE(argv[0]) == T_OPAQUE) {
    Value *ptr = argv[0];
    // NOTE: not recomended to directly access value fields!
    argv[0]->type = T_OWNED_OPAQUE;
    return val_retain(argv[0]);
  }
  return verror("own(ptr): Must have a pointer to convert into owned opaque!");
}

Value *native_unown(Env *e, int argc, Value **argv) {
  (void)e;
  if (argc == 1 && MILA_GET_TYPE(argv[0]) == T_OWNED_OPAQUE) {
    Value *ptr = argv[0];
    // NOTE: not recomended to directly access value fields!
    argv[0]->type = T_OPAQUE;
    return val_retain(argv[0]);
  }
  return verror(
      "unown(ptr): Must have an owned pointer to convert into unowned opaque!");
}

Value *native_istring(Env *e, int argc, Value **argv) {
  if (argc == 1) {
    char *str = as_c_string(argv[0]);
    Value *ptr = vowned_opaque(str);
    ptr->type_name = mila_strdup(MILA_LPREFIX "istring");
    val_set_table(ptr, istring_meta);
    return ptr;
  }
  return verror("istring(v): Needs at least one argument!");
}

Value *istring_to_iter(Value *self) {
  char *str = (char *)self->v.opaque;
  size_t slen = strlen(str);
  Value **iter = (Value **)mila_malloc(sizeof(Value *) * (slen + 1));
  for (size_t i = 0; i < slen; ++i) {
    iter[i] = vstring_dup((char[]){str[i], 0});
  }
  iter[slen] = NULL;
  return vopaque(iter);
}

Value *istring_get(Value *self, Value *index) {
  char *str = (char *)self->v.opaque;
  return vstring_dup((char[]){str[GET_INTEGER(index)], 0});
}

Value *istring_to_str(Value *self) { return vstring_dup(self->v.opaque); }

Value *native_rand(Env *e, int argc, Value **argv) { return vfloat(rand()); }

Value *native_fabs(Env *e, int argc, Value **argv) {
  if (argc != 1 || MILA_GET_TYPE(argv[0]) != T_FLOAT)
    return verror("fabs(num): argument must be a float!");
  return vfloat(fabs(GET_FLOAT(argv[0])));
}

Value *native_abs(Env *e, int argc, Value **argv) {
  if (argc != 1 || MILA_GET_TYPE(argv[0]) != T_INT)
    return verror("abs(num): argument must be an integer!");
  return vint(abs((int)GET_INTEGER(argv[0])));
}

Value *native_as_opaque(Env *e, int argc, Value **argv) {
  (void)e;
  if (argc != 1)
    return verror("as_opaque(v): Must have one argument!");
  switch (MILA_GET_TYPE(argv[0])) {
  case T_INT: {
    long *ptr = NULL;
    ptr = (long *)mila_malloc(sizeof(long));
    *ptr = GET_INTEGER(argv[0]);
    return vowned_opaque(ptr);
  } break;
  case T_UINT: {
    unsigned long *ptr = NULL;
    ptr = (unsigned long *)mila_malloc(sizeof(unsigned long));
    *ptr = GET_UINTEGER(argv[0]);
    return vowned_opaque(ptr);
  } break;
  case T_FLOAT: {
    double *ptr = NULL;
    ptr = (double *)mila_malloc(sizeof(double));
    *ptr = GET_FLOAT(argv[0]);
    return vowned_opaque(ptr);
  } break;
  case T_STRING: {
    char *ptr = NULL;
    ptr = (char *)mila_malloc(sizeof(char) * (strlen(GET_STRING(argv[0])) + 1));
    strncpy(ptr, GET_STRING(argv[0]), strlen(GET_STRING(argv[0])));
    ptr[strlen(GET_STRING(argv[0]))] = 0;
    return vowned_opaque(ptr);
  } break;
  case T_OWNED_OPAQUE:
  case T_OPAQUE: {
    return vopaque(GET_OPAQUE(argv[0]));
  } break;
  }
  return verror("Unsupported type %s!", MILA_GET_TYPENAME(argv[0]));
}

// supports all C format specifiers (well at faking it well)
Value *native_printf(Env *e, int argc, Value **argv) {
  if (argc == 0)
    return verror(
        "char_count += printf(fmt, ...): Requires at least one argument.");
  if (argc == 1) {
    print_value(argv[0]);
    return vnull();
  }
  if (argc >= 2) {
    if (MILA_GET_TYPE(argv[0]) != T_STRING)
      return verror("char_count += printf(fmt, ...): Requires first argument "
                    "to be a string!");
    char *fmt = GET_STRING(argv[0]);
    _Bool is_comma = 0;

    int count = 1;
    unsigned long char_count = 0;
    int precision = -1;
    char tmp[20] = {0};

    while (*fmt) {
      if ((*fmt) == '%') {
        fmt++;

        // skip over any flags like -, +, space, #, 0
        while (*fmt == '-' || *fmt == '+' || *fmt == ' ' || *fmt == '#' ||
               *fmt == '0')
          fmt++;

        // skip over a numeric width if present, skip * and coresponding
        // argument
        if (*fmt == '.') {
          fmt++;
          if (*fmt == '*') {
            Value *v = argv[count++];
            precision = (int)GET_INTEGER(v);
            fmt++;
          } else {
            char *start = fmt;
            while (*fmt >= '0' && *fmt <= '9')
              fmt++;
            int len = fmt - start;
            sprintf(tmp, "%.*s", len, start);
            precision = atoi(tmp);
            tmp[0] = '\0'; // reset buffer
          }
        }

        // check for our custom ' comma-separator extension
        _Bool local_comma = is_comma;
        is_comma = 0;
        if (*fmt == '\'') {
          local_comma = 1;
          fmt++;
        }

        // figure out the length modifier so we know how wide to read
        // from an opaque pointer. 0=none 1=h 2=hh 3=l 4=ll 5=z 6=t 7=j
        int length = 0;
        if (*fmt == 'h') {
          fmt++;
          if (*fmt == 'h') {
            length = 2;
            fmt++;
          } // hh - signed/unsigned char
          else
            length = 1; // h  - short
        } else if (*fmt == 'l') {
          fmt++;
          if (*fmt == 'l') {
            length = 4;
            fmt++;
          } // ll - long long
          else
            length = 3; // l  - long / double
        } else if (*fmt == 'z') {
          length = 5;
          fmt++;
        } // size_t
        else if (*fmt == 't') {
          length = 6;
          fmt++;
        } // ptrdiff_t
        else if (*fmt == 'j') {
          length = 7;
          fmt++;
        } // intmax_t

        switch (*fmt) {
        case '\'':
          // comma modifier showed up after the length modifier, handle it
          local_comma = 1;
          fmt++;
          goto reparse_specifier;

        reparse_specifier:;

        case 'd':
        case 'i':
        case 'u':
        case 'f':
        case 'g':
        case 'e':
        case 'E':
        case 'G':
        case 'o': {
          Value *v = argv[count++];

          if (MILA_GET_TYPE(v) == T_OPAQUE ||
              MILA_GET_TYPE(v) == T_OWNED_OPAQUE) {
            void *p = GET_OPAQUE(v);
            switch (*fmt) {
            case 'f':
            case 'e':
            case 'E':
              if (length == 3)
                char_count += printf("%.*lf", precision, *(double *)p);
              else if (length == 4)
                char_count += printf("%.*Lf", precision, *(long double *)p);
              else
                char_count += printf("%.*f", precision, *(float *)p);
              break;
            case 'g':
            case 'G':
              if (length == 3)
                char_count += printf("%.*lg", precision, *(double *)p);
              else if (length == 4)
                char_count += printf("%.*Lg", precision, *(long double *)p);
              else
                char_count += printf("%.*g", precision, *(float *)p);
              break;
            case 'd':
            case 'i':
              switch (length) {
              case 0:
                char_count += printf("%.*d", precision, *(int *)p);
                break;
              case 1:
                char_count += printf("%.*d", precision, *(short *)p);
                break;
              case 2:
                char_count += printf("%.*d", precision, *(signed char *)p);
                break;
              case 3:
                char_count += printf("%.*ld", precision, *(long *)p);
                break;
              case 4:
                char_count += printf("%.*lld", precision, *(long long *)p);
                break;
              case 5:
                char_count += printf("%.*zd", precision, *(ssize_t *)p);
                break;
              case 6:
                char_count += printf("%.*td", precision, *(ptrdiff_t *)p);
                break;
              case 7:
                char_count += printf("%.*jd", precision, *(intmax_t *)p);
                break;
              }
              break;
            case 'u':
              switch (length) {
              case 0:
                char_count += printf("%.*u", precision, *(unsigned int *)p);
                break;
              case 1:
                char_count += printf("%.*u", precision, *(unsigned short *)p);
                break;
              case 2:
                char_count += printf("%.*u", precision, *(unsigned char *)p);
                break;
              case 3:
                char_count += printf("%.*lu", precision, *(unsigned long *)p);
                break;
              case 4:
                char_count +=
                    printf("%.*llu", precision, *(unsigned long long *)p);
                break;
              case 5:
                char_count += printf("%.*zu", precision, *(size_t *)p);
                break;
              case 6:
                char_count += printf("%.*tu", precision, *(ptrdiff_t *)p);
                break;
              case 7:
                char_count += printf("%.*ju", precision, *(uintmax_t *)p);
                break;
              }
              break;
            case 'o':
              switch (length) {
              case 0:
                char_count += printf("%.*o", precision, *(unsigned int *)p);
                break;
              case 1:
                char_count += printf("%.*o", precision, *(unsigned short *)p);
                break;
              case 2:
                char_count += printf("%.*o", precision, *(unsigned char *)p);
                break;
              case 3:
                char_count += printf("%.*lo", precision, *(unsigned long *)p);
                break;
              case 4:
                char_count +=
                    printf("%.*llo", precision, *(unsigned long long *)p);
                break;
              case 5:
                char_count += printf("%.*zo", precision, *(size_t *)p);
                break;
              case 7:
                char_count += printf("%.*jo", precision, *(uintmax_t *)p);
                break;
              }
              break;
            }
          } else {
            // not an opaque pointer, just let our value printer handle it
            if (__builtin_expect(local_comma, 0))
              char_count += print_value_fancy(v);
            else
              char_count += print_value(v);
          }
        } break;

        case 'x': {
          Value *v = argv[count++];
          if (MILA_GET_TYPE(v) == T_OPAQUE ||
              MILA_GET_TYPE(v) == T_OWNED_OPAQUE) {
            void *p = GET_OPAQUE(v);
            switch (length) {
            case 0:
              char_count += printf("%.*x", precision, *(unsigned int *)p);
              break;
            case 1:
              char_count += printf("%.*x", precision, *(unsigned short *)p);
              break;
            case 2:
              char_count += printf("%.*x", precision, *(unsigned char *)p);
              break;
            case 3:
              char_count += printf("%.*lx", precision, *(unsigned long *)p);
              break;
            case 4:
              char_count +=
                  printf("%.*llx", precision, *(unsigned long long *)p);
              break;
            case 5:
              char_count += printf("%.*zx", precision, *(size_t *)p);
              break;
            case 7:
              char_count += printf("%.*jx", precision, *(uintmax_t *)p);
              break;
            }
          } else {
            // non-opaque, just cast whatever numeric type we got
            unsigned int value = 0;
            switch (MILA_GET_TYPE(v)) {
            case T_INT:
              value = (unsigned int)GET_INTEGER(v);
              break;
            case T_UINT:
              value = (unsigned int)GET_UINTEGER(v);
              break;
            case T_FLOAT:
              value = (unsigned int)GET_FLOAT(v);
              break;
            }
            char_count += printf("%.*x", precision, value);
          }
        } break;

        case 'X': {
          Value *v = argv[count++];
          if (MILA_GET_TYPE(v) == T_OPAQUE ||
              MILA_GET_TYPE(v) == T_OWNED_OPAQUE) {
            void *p = GET_OPAQUE(v);
            switch (length) {
            case 0:
              char_count += printf("%.*X", precision, *(unsigned int *)p);
              break;
            case 1:
              char_count += printf("%.*X", precision, *(unsigned short *)p);
              break;
            case 2:
              char_count += printf("%.*X", precision, *(unsigned char *)p);
              break;
            case 3:
              char_count += printf("%.*lX", precision, *(unsigned long *)p);
              break;
            case 4:
              char_count +=
                  printf("%.*llX", precision, *(unsigned long long *)p);
              break;
            case 5:
              char_count += printf("%.*zX", precision, *(size_t *)p);
              break;
            case 7:
              char_count += printf("%.*jX", precision, *(uintmax_t *)p);
              break;
            }
          } else {
            // same as %x but uppercase, same casting logic
            unsigned int value = 0;
            switch (MILA_GET_TYPE(v)) {
            case T_INT:
              value = (unsigned int)GET_INTEGER(v);
              break;
            case T_UINT:
              value = (unsigned int)GET_UINTEGER(v);
              break;
            case T_FLOAT:
              value = (unsigned int)GET_FLOAT(v);
              break;
            }
            char_count += printf("%.*X", precision, value);
          }
        } break;

        case 's': {
          Value *v = argv[count++];
          switch (MILA_GET_TYPE(v)) {
          case T_STRING:
            char_count += printf("%.*s", precision, GET_STRING(v));
            break;
          case T_OPAQUE:
          case T_OWNED_OPAQUE:
            char_count += printf("%.*s", precision, (char *)GET_OPAQUE(v));
            break;
          default:
            if (__builtin_expect(local_comma, 0))
              char_count += print_value_fancy(v);
            else
              char_count += print_value(v);
            break;
          }
        } break;

        case 'c': {
          Value *v = argv[count++];
          char value = '?';
          switch (MILA_GET_TYPE(v)) {
          case T_STRING:
            value = *GET_STRING(v);
            break; // just grab the first character
          case T_OPAQUE:
          case T_OWNED_OPAQUE:
            value = *(char *)GET_OPAQUE(v);
            break;
          default:
            return verror("%%c format specifier only supports opaque pointers "
                          "and strings but got %s!",
                          MILA_GET_TYPENAME(v));
          }
          char_count += printf("%c", value);
        } break;

        case 'p': {
          Value *v = argv[count++];
          void *ptr = NULL;
          switch (MILA_GET_TYPE(v)) {
          case T_OWNED_OPAQUE:
            ptr = GET_OPAQUE(v);
            break;
          default:
            ptr = v;
            break; // print the Value* itself as an address
          }
          char_count += printf("%p", ptr);
        } break;

        case 'n': {
          // write back the number of characters printed so far,
          Value *v = argv[count++];
          if (MILA_GET_TYPE(v) == T_INT) {
            v->type = T_UINT;
            v->v.ui = char_count;
          } else if (MILA_GET_TYPE(v) == T_UINT) {
            v->v.i = char_count;
          } else if (MILA_GET_TYPE(v) == T_OPAQUE) {
            v->type = T_UINT;
            v->v.ui = char_count;
          } else if (MILA_GET_TYPE(v) == T_NONE) {
            v->type = T_UINT;
            v->v.ui = char_count;
          } else if (MILA_GET_TYPE(v) == T_NULL) {
            v->type = T_UINT;
            v->v.ui = char_count;
          }
        } break;

        case '?':
          // printcas normal value
          char_count += print_value(argv[count++]);
          break;

        case '%':
          putchar('%');
          char_count++;
          break;

        default:
          return verror("Invalid format modifier '%%%c'!", *fmt);
        }
      } else {
        putchar(*fmt);
        char_count++;
      }
      fmt++;
    }

    return vnull();
  }

  return vnull();
}

Value *native_assert(Env *env, int argc, Value **argv) {
  (void)env;
  if (argc != 2)
    return verror("assert(cond, message): Needs two arguments!");
  if (!is_truthy(argv[0]))
    return verror("%s", GET_STRING(argv[1]));
  return vnull();
}

Value *native_srandom(Env *env, int argc, Value **argv) {
  if (argc != 1 || MILA_GET_TYPE(argv[0]) != T_INT)
    return verror("srandom(seed): Expected an integer argument!");
  srand(GET_INTEGER(argv[0]));
  return vnull();
}

Value *native_random(Env *env, int argc, Value **argv) {
  if (argc == 2 && MILA_GET_TYPE(argv[0]) == T_INT &&
      MILA_GET_TYPE(argv[1]) == T_INT) {
    long min = GET_INTEGER(argv[0]);
    long max = GET_INTEGER(argv[1]);
    return vint((rand() % (max - min + 1)) + min);
  }
  return verror("random(lower, upper): Expected two integer arguments.");
}

Value *native_crandom(Env *env, int argc, Value **argv) {
  if (argc != 0)
    verror("crandom(): Expected no arguments");
  return vint(rand());
}

Value *native_get_tm_local(Env *env, int argc, Value **argv) {
  if (argc > 1)
    return verror("get_tm_local(): Expected at most 1 argument!");
  time_t time = argc == 0 ? get_unix_timestamp() : GET_INTEGER(argv[0]);
  struct tm *info = localtime(&time);
  return vopaque(info);
}

Value *native_get_tm_gmt(Env *env, int argc, Value **argv) {
  if (argc > 1)
    return verror("get_tm_gmt(): Expected at most 1 argument!");
  time_t time = argc == 0 ? get_unix_timestamp() : GET_INTEGER(argv[0]);
  struct tm *info = gmtime(&time);
  return vopaque(info);
}

Value *native_strftime(Env *env, int argc, Value **argv) {
  if (argc != 2)
    return verror("strftime(fmt, tm): Expected two arguments!");
  struct tm *info = GET_OPAQUE(argv[1]);
  char *fmt = GET_STRING(argv[0]);
  char buffer[100] = {0};
  strftime(buffer, sizeof(buffer), fmt, info);
  return vstring_dup(buffer);
}

// Minimal MiLa Builtins
// We do not support normal objects,
// the dots are namespaces.
// And yes if you remove this file it wont trash the interpreter.
void env_register_builtins(Env *g) {
  // === Setup
  // canonical builtins reports edition (2026 march)
  env_set_raw(g, "__mila_canonical_builtins", vint(202603L));
  // canonical builtins version reports actual version (integer, any changes
  // means ver++)
  env_set_raw(g, "__mila_canonical_builtins_version", vint(1));
  // tell users what implementation it is
  // heres its canon since this is the base implementation.
  env_set_raw(g, "__mila_codename", vstring_dup("canon"));

  // === Misc
  env_register_native(g, "range", native_range);
  env_register_native(g, "as_opaque", native_as_opaque);
  env_register_native(g, "dump_vars", native_meep);
  env_register_native(g, "own", native_own);
  env_register_native(g, "unown", native_unown);
  env_register_native(g, "repr", native_repr);
  env_register_native(g, "repr_raw", native_repr_raw);
  env_register_native(g, "str", native_str);
  env_register_native(g, "random", native_random);
  env_register_native(g, "srandom", native_srandom);
  env_register_native(g, "crandom", native_crandom);
  // === Text IO
  env_register_native(g, "print", native_print);
  env_register_native(
      g, "printf", native_printf); // lovely printf function for C programmers
  env_register_native(g, "printr", native_printr);
  env_register_native(g, "println", native_println);
  env_register_native(g, "input", native_input);
  // === Logic
  env_register_native(g, "and", native_bitwise_and);
  env_register_native(g, "or", native_bitwise_or);
  env_register_native(g, "xor", native_bitwise_xor);
  env_register_native(g, "not", native_not);
#ifndef VMM_BUILD
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
  env_set_raw(g, "stdin", vopaque_extra(stdin, NULL, "'stdin fd'"));
  file_meta = val_make_table();
  val_set_method_table(file_meta, UMethodToString, file_printer);
#endif
  // === Lists

  dict_meta = val_make_table();

  val_set_method_table(dict_meta, UMethodToString, dict_display);
  val_set_method_table(dict_meta, UMethodFree, free_dict);
  val_set_method_table(dict_meta, BMethodGetItem, get_dict);
  val_set_method_table(dict_meta, TMethodSetItem, set_dict);

  list_meta = val_make_table();

  val_set_method_table(list_meta, UMethodToRepr, list_repr);
  val_set_method_table(list_meta, UMethodToString, list_str);
  val_set_method_table(list_meta, UMethodFree, list_free);
  val_set_method_table(list_meta, UMethodToIter, list_to_iter);
  val_set_method_table(list_meta, BMethodGetItem, get_list);
  val_set_method_table(list_meta, TMethodSetItem, set_list);

  array_meta = val_make_table();

  val_set_method_table(array_meta, UMethodToIter, array_to_iter);
  val_set_method_table(array_meta, UMethodToString, array_to_str);
  val_set_method_table(array_meta, UMethodToRepr, array_to_repr);
  val_set_method_table(array_meta, BMethodGetItem, get_array);
  val_set_method_table(array_meta, TMethodSetItem, set_array);
  val_set_method_table(array_meta, UMethodFree, free_array);

  range_meta = val_make_table();

  val_set_method_table(range_meta, UMethodToIter, range_to_iter);
  val_set_method_table(range_meta, UMethodToString, range_to_str);
  val_set_method_table(range_meta, UMethodFree, range_free);

  istring_meta = val_make_table();

  val_set_method_table(istring_meta, UMethodToIter, istring_to_iter);
  val_set_method_table(istring_meta, BMethodGetItem, istring_get);
  val_set_method_table(istring_meta, UMethodToString, istring_to_str);

  env_register_native(g, "list", native_list_new);
  env_register_native(g, "list.set", native_list_set);
  env_register_native(g, "list.get", native_list_get);
  env_register_native(g, "list.pop", native_list_pop);
  env_register_native(g, "list.len", native_list_len);
  env_register_native(g, "list.append", native_list_append);
  env_register_native(g, "list.contains", native_list_contains);
  // === Array
  env_register_native(g, "array", native_new_array);
  env_register_native(g, "array.from", native_from_array);
  env_register_native(g, "array.set", native_set_array);
  env_register_native(g, "array.get", native_get_array);
  env_register_native(g, "array.len", native_len_array);
  // === Dicts
  env_register_native(g, "dict", native_new_dict);
  env_register_native(g, "dict.set", native_set_dict);
  env_register_native(g, "dict.get", native_get_dict);
  env_register_native(g, "dict.rem", native_rem_dict);
  // === Casting
  env_register_native(g, "cast.int", native_cast_int);
  env_register_native(g, "cast.float", native_cast_float);
  env_register_native(g, "cast.string", native_cast_string);
  env_register_native(g, "cast.i2f", native_cast_int_to_float);
  env_register_native(g, "cast.i2u", native_cast_int_to_uint);
  env_register_native(g, "cast.u2i", native_cast_uint_to_int);
  env_register_native(g, "cast.f2i", native_cast_float_to_int);
  env_register_native(g, "typeof", native_type_of);
  // === String
  env_register_native(g, "str.slice", native_str_slice);
  env_register_native(g, "str.index", native_str_index);
  env_register_native(g, "str.patch", native_str_patch);
  env_register_native(g, "str.copy", native_str_copy);
  env_register_native(g, "str.len", native_str_len);
  env_register_native(g, "str.pop_f", native_pop_start);
  env_register_native(g, "str.pop_b", native_pop_end);
  env_register_native(g, "istring", native_istring);
  // === ASCII
  env_register_native(g, "ascii.from_int", native_ascii_from_int);
  env_register_native(g, "ascii.from_string", native_ascii_from_string);
  // === Math
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
  env_set_raw(g, "RAND_MAX", vint(RAND_MAX));
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
  env_register_native(g, "report_tagged", native_report_tagged);
  env_register_native(g, "assert", native_assert);
  env_set_local_raw(g, "E_PRE_RUNTIME", vint(E_PRE_RUNTIME));
  env_set_local_raw(g, "E_RUNTIME", vint(E_RUNTIME));
  env_set_local_raw(g, "E_TYPE_ERROR", vint(E_TYPE_ERROR));
  env_set_local_raw(g, "E_FATAL", vint(E_FATAL));
  env_set_local_raw(g, "E_GENERIC", vint(E_GENERIC));
  env_set_local_raw(g, "E_SYNTAX_ERROR", vint(E_SYNTAX_ERROR));
  env_register_native(g, "exit", native_exit);
  // === Time measurement
  env_register_native(g, "get_time", native_get_time);
  env_register_native(g, "strftime", native_strftime);
  env_register_native(g, "get_tm_gmt", native_get_tm_gmt);
  env_register_native(g, "get_tm_local", native_get_tm_local);
  // === OS Stuff
  env_register_native(g, "system", native_system);
  // === Modules
#ifndef VMM_BUILD
  env_register_native(g, "run", native_run);   // runs file
  env_register_native(g, "load", native_load); // loads dlls or so file
  env_register_native(g, "eval", native_eval); // runs string
#endif
}

#ifndef ML_LIB
void _mila_lib_init(Env *e) { env_register_builtins(e); }
#endif
