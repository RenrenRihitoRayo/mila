#include "ml_dict.c"
#include <sys/types.h>
#if !(defined(__GNUC__) || defined(__clang__))
#error "MiLa only supports GCC and Clang."
#endif

/*
 * MiLa
 * A modern programming language
 * the smallest it can get.
 * Welcome to the MiLa Language Kernel.
 */

#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
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

#ifndef VMM_BUILD
#include "ml_paths.c"
#endif

#include "ml_string.c"

#include "ml_builtins.c"


#include "mila.h"

void print_memory_usage() {
  size_t memory_usage = 0;

#ifdef _WIN32
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
  while (memory_usage_d >= 1024 && unit_index < 4) {
    memory_usage_d /= 1024;
    unit_index++;
  }

  printf("Memory usage: %.2f %s\n", memory_usage_d, units[unit_index]);
}

path_list* search_path = NULL;

// ---------- Value representation ----------

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void* mila_malloc(size_t size) {
    void* ptr = malloc(size);
    memset(ptr, 0, size);
    return ptr;
}
void* mila_realloc(void* ptr, size_t size) {
    return realloc(ptr, size);
}
void mila_free(void* ptr) {
    free(ptr);
}

static void insert_commas(char *buf, size_t bufsize) {
  char temp[128];
  strncpy(temp, buf, sizeof(temp));
  temp[sizeof(temp) - 1] = '\0';

  char *dot = strchr(temp, '.');
  int int_len = dot ? (dot - temp) : (int)strlen(temp);

  int commas = (int_len - 1) / 3;
  int new_len = strlen(temp) + commas;

  if ((size_t)new_len + 1 > bufsize)
    return;

  int i = int_len - 1;
  int j = new_len - 1;

  // copy fractional part first
  if (dot) {
    strcpy(buf + (new_len - strlen(dot)), dot);
    j = new_len - strlen(dot) - 1;
  } else {
    buf[new_len] = '\0';
  }

  int count = 0;

  while (i >= 0) {
    buf[j--] = temp[i--];
    if (++count == 3 && i >= 0) {
      buf[j--] = ',';
      count = 0;
    }
  }
}

void float_to_string_fancy(float f, char *buf, size_t bufsize) {
  // Step 1: format without locale tricks
  snprintf(buf, bufsize, "%.10g", f);

  // Step 2: ensure .0 if integer-like
  if (strchr(buf, '.') == NULL && strchr(buf, 'e') == NULL &&
      strchr(buf, 'E') == NULL) {
    size_t len = strlen(buf);
    if (len + 2 < bufsize) {
      buf[len] = '.';
      buf[len + 1] = '0';
      buf[len + 2] = '\0';
    }
  }

  // Step 3: insert commas into integer part
  insert_commas(buf, bufsize);
}

void long_to_string_fancy(long value, char *buf, size_t bufsize) {
  char temp[64];
  snprintf(temp, sizeof(temp), "%ld", value);

  bool negative = temp[0] == '-';
  char *num = negative ? temp + 1 : temp;

  int len = strlen(num);
  int commas = (len - 1) / 3;
  int new_len = len + commas + (negative ? 1 : 0);

  if ((size_t)new_len + 1 > bufsize) {
    if (bufsize > 0)
      buf[0] = '\0';
    return;
  }

  int i = len - 1;
  int j = new_len - 1;
  int count = 0;

  buf[new_len] = '\0';

  while (i >= 0) {
    buf[j--] = num[i--];
    if (++count == 3 && i >= 0) {
      buf[j--] = ',';
      count = 0;
    }
  }

  if (negative) {
    buf[0] = '-';
  }
}

void ulong_to_string_fancy(unsigned long value, char *buf, size_t bufsize) {
  char temp[64];
  snprintf(temp, sizeof(temp), "%lu", value);

  int len = strlen(temp);
  int commas = (len - 1) / 3;
  int new_len = len + commas;

  if ((size_t)new_len + 1 > bufsize) {
    if (bufsize > 0)
      buf[0] = '\0';
    return;
  }

  int i = len - 1;
  int j = new_len - 1;
  int count = 0;

  buf[new_len] = '\0';

  while (i >= 0) {
    buf[j--] = temp[i--];
    if (++count == 3 && i >= 0) {
      buf[j--] = ',';
      count = 0;
    }
  }
}

void float_to_string(float f, char *buf, size_t bufsize) {
  // Step 1: try %g with max precision
  snprintf(buf, bufsize, "%.10g", f); // 9 digits for float

  // Step 2: check if there's a decimal point or exponent
  if (strchr(buf, '.') == NULL && strchr(buf, 'e') == NULL &&
      strchr(buf, 'E') == NULL) {
    // integer-looking float, force .0
    size_t len = strlen(buf);
    if (len + 2 < bufsize) { // enough room for ".0\0"
      buf[len] = '.';
      buf[len + 1] = '0';
      buf[len + 2] = '\0';
    }
  }
}

Value *val_new(ValueType t) {
  Value *p = mila_malloc(sizeof(Value));
  p->type = t;
  p->refcount = 1;
  p->type_name = NULL;
  p->method_table = NULL;
  p->owns_table = 1;
#ifdef MILA_DEBUG
  printf("  ++ %s type allocated!\n     pointer: %p\n", MILA_GET_TYPENAME(p),
         p);
#endif
  return p;
}

void val_allocate_table(Value *v) {
  v->method_table =
      (MethodTable *)mila_malloc(sizeof(MethodTable) * MethodTotalCount);
  memset(v->method_table, 0, sizeof(MethodTable) * MethodTotalCount);
}

MethodTable *val_make_table(void) {
  MethodTable *t =
      (MethodTable *)mila_malloc(sizeof(MethodTable) * MethodTotalCount);
  memset(t, 0, sizeof(MethodTable) * MethodTotalCount);
  return t;
}

void val_set_table(Value *v, MethodTable *t) {
  v->owns_table = 0;
  v->method_table = t;
}

void val_set_method(Value *v, MethodType t, void *func) {
  v->method_table[t] = func;
}

void val_unset_method(Value *v, MethodType t) { v->method_table[t] = NULL; }

void val_set_method_table(MethodTable *v, MethodType t, void *func) {
  v[t] = func;
}

void val_unset_method_table(MethodTable *v, MethodType t) { v[t] = NULL; }

// Helpers to create typed values or check their truthiness

int is_truthy(Value *value) {
  _Bool truth = 0;
  if (value->type == T_BOOL)
    truth = value->v.b;
  else if (value->type == T_INT)
    truth = value->v.i != 0;
  else if (value->type == T_FLOAT)
    truth = value->v.f != 0.0;
  else if (value->type == T_STRING)
    truth = value->v.s != NULL && strlen(value->v.s);
  else if (value->type == T_OPAQUE)
    truth = value->v.opaque;
  else
    truth = (value->type != T_NULL && value->type != T_NONE);
  return truth;
}

int match_range(const char **pat, char c) {
  int negate = 0;
  if (**pat == '!') {
    negate = 1;
    (*pat)++;
  }

  int matched = 0;
  while (**pat && **pat != ']') {
    if ((*pat)[1] == '-' && (*pat)[2] && (*pat)[2] != ']') {
      // Handle range a-z
      if (c >= **pat && c <= (*pat)[2])
        matched = 1;
      *pat += 3; // skip 'a-z'
    } else {
      if (c == **pat)
        matched = 1;
      (*pat)++;
    }
  }

  if (**pat == ']')
    (*pat)++; // skip closing bracket
  return negate ? !matched : matched;
}

int match(const char *pattern, const char *str) {
  while (*pattern) {
    if (*pattern == '*') {
      pattern++;
      if (!*pattern)
        return 1;
      while (*str) {
        if (match(pattern, str))
          return 1;
        str++;
      }
      return 0;
    } else if (*pattern == '?') {
      if (!*str)
        return 0;
      pattern++;
      str++;
    } else if (*pattern == '[') {
      pattern++;
      if (!*str || !match_range(&pattern, *str))
        return 0;
      str++;
    } else {
      if (*pattern != *str)
        return 0;
      pattern++;
      str++;
    }
  }
  return !*str;
}

Value *vnull() { return val_new(T_NULL); }
Value *vnone() { return val_new(T_NONE); }
Value *vbreak() { return val_new(T_BREAK); }
Value *vcontinue() { return val_new(T_CONTINUE); }

#ifndef VMM_BUILD
__attribute__((format(printf, 2, 3))) Value *vtagged_error(ErrorType err,
                                                           char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  // First pass: find length
  va_list ap_copy;
  va_copy(ap_copy, ap);
  int len = vsnprintf(NULL, 0, fmt, ap_copy);
  va_end(ap_copy);

  if (len < 0) {
    va_end(ap);
    return NULL;
  }

  char *buf = mila_malloc(len + 1);
  if (!buf) {
    va_end(ap);
    Value *v = val_new(T_ERROR);
    v->v.message = mila_strdup("verror could not allocate memory!");
    return v;
  }

  vsnprintf(buf, len + 1, fmt, ap);
  va_end(ap);
  Value *v = val_new(T_TAGGED_ERROR);
  v->v.tagged_error.message = buf;
  v->v.tagged_error.type = err;
  return v;
}

__attribute__((format(printf, 1, 2))) Value *verror(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  // First pass: find length
  va_list ap_copy;
  va_copy(ap_copy, ap);
  int len = vsnprintf(NULL, 0, fmt, ap_copy);
  va_end(ap_copy);

  if (len < 0) {
    va_end(ap);
    return NULL;
  }

  char *buf = mila_malloc(len + 1);
  if (!buf) {
    va_end(ap);
    Value *v = val_new(T_ERROR);
    v->v.message = mila_strdup("verror could not allocate memory!");
    return v;
  }

  vsnprintf(buf, len + 1, fmt, ap);
  va_end(ap);
  Value *v = val_new(T_ERROR);
  v->v.message = buf;
  return v;
}
#endif

// __attribute__((format(printf, 1, 2)))
int mila_printf(char *fmt, ...) {
  if (fmt == NULL) {
    fprintf(stderr, "Error: printf(fmt, ...): Format string is NULL\n");
    return 0;
  }

  va_list args;
  va_start(args, fmt);

  _Bool is_comma = 0;
  int count = 0;
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

      // skip over a numeric width if present, skip * and coresponding argument
      if (*fmt == '.') {
        fmt++;
        if (*fmt == '*') {
          Value *v = va_arg(args, Value *);
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

      case '?': {
        Value *v = va_arg(args, Value *);
        if (__builtin_expect(local_comma, 0)) {
          char_count += print_value_fancy(v);
        } else {
          char_count += print_value(v);
        }
      } break;

      case 'd':
      case 'i':
      case 'u':
      case 'f':
      case 'g':
      case 'e':
      case 'E':
      case 'G':
      case 'o': {
        Value *v = va_arg(args, Value *);

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
        Value *v = va_arg(args, Value *);
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
            char_count += printf("%.*llx", precision, *(unsigned long long *)p);
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
        Value *v = va_arg(args, Value *);
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
            char_count += printf("%.*llX", precision, *(unsigned long long *)p);
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
        Value *v = va_arg(args, Value *);
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
        Value *v = va_arg(args, Value *);
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
          return -1;
        }
        char_count += printf("%c", value);
      } break;

      case 'p': {
        Value *v = va_arg(args, Value *);
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
        // write back the number of characters printed so far
        Value *v = va_arg(args, Value *);
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

      case '%':
        putchar('%');
        char_count++;
        break;

      default:
        return -1;
      }
    } else {
      putchar(*fmt);
      char_count++;
    }
    fmt++;
  }

  va_end(args);
  return (int)char_count;
}
__attribute__((format(printf, 1, 2))) Value *vstring_fmt(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  // First pass: find length
  va_list ap_copy;
  va_copy(ap_copy, ap);
  int len = vsnprintf(NULL, 0, fmt, ap_copy);
  va_end(ap_copy);

  if (len < 0) {
    va_end(ap);
    return NULL;
  }

  char *buf = mila_malloc(len + 1);
  if (!buf) {
    va_end(ap);
    Value *v = val_new(T_ERROR);
    v->v.message = mila_strdup("vstring_fmt could not allocate memory!");
    return v;
  }

  vsnprintf(buf, len + 1, fmt, ap);
  va_end(ap);
  Value *v = val_new(T_STRING);
  v->v.message = buf;
  return v;
}
Value *vint(long x) {
  Value *v = val_new(T_INT);
  v->v.i = x;
  return v;
}
Value *vuint(unsigned long x) {
  Value *v = val_new(T_UINT);
  v->v.ui = x;
  return v;
}
Value *vfloat(double f) {
  Value *v = val_new(T_FLOAT);
  v->v.f = f;
  return v;
}
Value *vbool(int b) {
  Value *v = val_new(T_BOOL);
  v->v.b = b ? 1 : 0;
  return v;
}
Value *vstring_dup(const char *s) {
  Value *v = val_new(T_STRING);
  v->v.s = mila_strdup(s ? s : "");
  return v;
}
Value *vstring_take(char *s) {
  Value *v = val_new(T_STRING);
  v->v.s = s;
  return v;
}
// String
Value *vstring_slice(const char *src, size_t start, size_t len) {
  size_t n = strlen(src);
  if (start > n)
    return verror(""); // empty string

  if (start + len > n)
    len = n - start;

  char *buf = mila_malloc(len + 1);
  if (!buf)
    return vnull();

  memcpy(buf, src + start, len);
  buf[len] = '\0';

  return vstring_take(buf);
}
Value *vstring_index(const char *src, size_t index) {
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
Value *vstring_replace(const char *src, const char *needle, const char *repl) {
  if (!*needle)
    return vstring_dup(src); // can't match empty substring

  size_t src_len = strlen(src);
  size_t n_len = strlen(needle);
  size_t r_len = strlen(repl);

  // Count occurrences
  size_t count = 0;
  const char *p = src;
  while ((p = strstr(p, needle))) {
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

  while (1) {
    const char *match = strstr(p, needle);
    if (!match) {
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
Value *vowned_opaque(void *p) {
  Value *v = val_new(T_OWNED_OPAQUE);
  v->v.opaque = p;
  return v;
}
Value *vopaque(void *p) {
  Value *v = val_new(T_OPAQUE);
  v->v.opaque = p;
  return v;
}
Value *vweak_opaque(void *p) {
  Value *v = val_new(T_WEAK_OPAQUE);
  v->v.opaque = p;
  return v;
}
Value *vopaque_extra(void *p, Value *(*dis)(Value *), const char *type_name) {
  Value *v = vopaque(p);
  if (dis && v) {
    val_allocate_table(v);
    val_set_method(v, UMethodToString, dis);
  }
  v->type_name = mila_strdup(type_name);
  return v;
}
Value *vowned_opaque_extra(void *p, Value *(*dis)(Value *),
                           const char *type_name) {
  Value *v = vowned_opaque(p);
  if (dis && v) {
    val_allocate_table(v);
    val_set_method(v, UMethodToString, dis);
  }
  v->type_name = mila_strdup(type_name);
  return v;
}
Value *vnative(NativeFn fn, const char *name) {
  Value *v = val_new(T_NATIVE);
  v->v.native = (NativeFunctionV *)mila_malloc(sizeof(NativeFunctionV));
  v->v.native->fn = fn;
  v->v.native->userdata = NULL;
  v->v.native->name = name ? mila_strdup(name) : NULL;
  return v;
}
Value *vtruthy(Value *value) { return vbool(is_truthy(value)); }

int our_asprintf(char **strp, const char *fmt, ...) {
  if (!strp)
    return -1;

  va_list args;
  va_start(args, fmt);

  va_list args_copy;
  va_copy(args_copy, args);
  int add_size = vsnprintf(NULL, 0, fmt, args_copy);
  va_end(args_copy);

  if (add_size < 0) {
    va_end(args);
    return -1;
  }

  // If *strp is NULL, treat it as empty string
  size_t old_len = 0;
  if (*strp) {
    // We trust the caller has either a valid string or NULL.
    old_len = strlen(*strp);
  }

  char *newbuf =
      (char *)mila_realloc(*strp ? *strp : NULL, old_len + add_size + 1);
  if (!newbuf) {
    va_end(args);
    return -1;
  }

  *strp = newbuf;

  if (vsnprintf(*strp + old_len, add_size + 1, fmt, args) < 0) {
    va_end(args);
    return -1;
  }

  va_end(args);
  return (int)(old_len + add_size);
}

char *as_c_string(Value *v) {
  char *buffer = NULL;
  if (!v) {
    return mila_strdup("cnull");
  }
  if (v->method_table && v->method_table[UMethodToString]) {
    Value *str = ((unary_method)v->method_table[UMethodToString])(v);
    char *res = mila_strdup(str->v.s);
    val_kill(str);
    return res;
  }
  if (v->method_table && v->method_table[UMethodToRepr]) {
    Value *str = ((unary_method)v->method_table[UMethodToRepr])(v);
    char *res = mila_strdup(str->v.s);
    val_kill(str);
    return res;
  }
  switch (v->type) {
  case T_NULL:
    our_asprintf(&buffer, "null");
    break;
  case T_NONE:
    our_asprintf(&buffer, "none");
    break;
  case T_ERROR:
    our_asprintf(&buffer, "<error:%s>", v->v.message);
    break;
  case T_TAGGED_ERROR:
    our_asprintf(&buffer, "<error[%s]:%s>", MILA_GET_ERRORNAME(v),
                 GET_ERROR(v));
    break;
  case T_INT:
    our_asprintf(&buffer, "%ld", v->v.i);
    break;
  case T_FLOAT: {
    char buf[MAX_NUMBER_DIGITS] = {0};
    float_to_string(v->v.f, buf, sizeof(buf));
    our_asprintf(&buffer, "%s", buf);
    break;
  }
  case T_STRING:
    our_asprintf(&buffer, "%s", v->v.s ? v->v.s : "");
    break;
  case T_BOOL:
    our_asprintf(&buffer, "%s", v->v.b ? "true" : "false");
    break;
  case T_FUNCTION:
    our_asprintf(&buffer, "<function:%s at %p>",
                 v->v.fn->name ? v->v.fn->name : "(lambda)", v->v.fn);
    break;
  case T_NATIVE:
    our_asprintf(&buffer, "<native:%s at %p>",
                 v->v.native->name ? v->v.native->name : "???",
                 v->v.native->fn);
    break;
  case T_OPAQUE:
    if (v->type_name)
      our_asprintf(&buffer, "<opaque:%p %s>", v->v.opaque, v->type_name);
    else
      our_asprintf(&buffer, "<opaque:%p>", v->v.opaque);
    break;
  case T_WEAK_OPAQUE:
    if (v->type_name)
      our_asprintf(&buffer, "<weak opaque:%p %s>", v->v.opaque, v->type_name);
    else
      our_asprintf(&buffer, "<weak opaque:%p>", v->v.opaque);
    break;
  case T_OWNED_OPAQUE:
    if (v->type_name)
      our_asprintf(&buffer, "<owned opaque:%p %s>", v->v.opaque, v->type_name);
    else
      our_asprintf(&buffer, "<owned opaque:%p>", v->v.opaque);
    break;
  case T_UINT:
    our_asprintf(&buffer, "%lu", v->v.ui);
    our_asprintf(&buffer, "u");
    break;
  case T_RETURN: {
    char *str = as_c_string_repr(v->v.opaque);
    our_asprintf(&buffer, "<return:%s>", str);
    mila_free(str);
  } break;
  default:
    our_asprintf(&buffer, "???");
  }
  return buffer;
}

char *as_c_string_raw(Value *v) {
  char *buffer = NULL;
  if (!v) {
    return mila_strdup("cnull");
  }
  switch (v->type) {
  case T_NULL:
    our_asprintf(&buffer, "null");
    break;
  case T_NONE:
    our_asprintf(&buffer, "none");
    break;
  case T_ERROR:
    our_asprintf(&buffer, "<error:%s>", v->v.message);
    break;
  case T_INT:
    our_asprintf(&buffer, "%ld", v->v.i);
    break;
  case T_FLOAT: {
    char buf[MAX_NUMBER_DIGITS] = {0};
    float_to_string(v->v.f, buf, sizeof(buf));
    our_asprintf(&buffer, "%s", buf);
    break;
  }
  case T_STRING:
    our_asprintf(&buffer, "%s", v->v.s ? v->v.s : "");
    break;
  case T_BOOL:
    our_asprintf(&buffer, "%s", v->v.b ? "true" : "false");
    break;
  case T_FUNCTION:
    our_asprintf(&buffer, "<function:%s at %p>",
                 v->v.fn->name ? v->v.fn->name : "(lambda)", v->v.fn);
    break;
  case T_NATIVE:
    our_asprintf(&buffer, "<native:%s at %p>",
                 v->v.native->name ? v->v.native->name : "???",
                 v->v.native->fn);
    break;
  case T_OPAQUE:
    if (v->type_name)
      our_asprintf(&buffer, "<opaque:%p %s>", v->v.opaque, v->type_name);
    else
      our_asprintf(&buffer, "<opaque:%p>", v->v.opaque);
    break;
  case T_OWNED_OPAQUE:
    if (v->type_name)
      our_asprintf(&buffer, "<owned opaque:%p %s>", v->v.opaque, v->type_name);
    else
      our_asprintf(&buffer, "<owned opaque:%p>", v->v.opaque);
    break;
  case T_UINT:
    our_asprintf(&buffer, "%lu", v->v.ui);
    our_asprintf(&buffer, "u");
    break;
  case T_RETURN: {
    char *str = as_c_string_repr_raw(v->v.opaque);
    our_asprintf(&buffer, "<return:%s>", str);
    mila_free(str);
  } break;
  default:
    our_asprintf(&buffer, "???");
  }
  return buffer;
}

Value *to_c_string(Value *v) {
  char *s = as_c_string(v);
  return vstring_take(s);
}

char *as_c_string_fancy(Value *v) {
  char *buffer = NULL;
  if (!v) {
    return mila_strdup("cnull");
  }
  if (v->method_table && v->method_table[UMethodToRepr]) {
    Value *str = ((unary_method)v->method_table[UMethodToRepr])(v);
    char *res = mila_strdup(str->v.s);
    val_kill(str);
    return res;
  }
  if (v->method_table && v->method_table[UMethodToString]) {
    Value *str = ((unary_method)v->method_table[UMethodToString])(v);
    char *res = mila_strdup(str->v.s);
    val_kill(str);
    return res;
  }
  switch (v->type) {
  case T_UINT: {
    char out[MAX_NUMBER_DIGITS] = {0};
    ulong_to_string_fancy(v->v.ui, out, sizeof(out));
    our_asprintf(&buffer, "%s", out);
  } break;
  case T_INT: {
    char out[MAX_NUMBER_DIGITS] = {0};
    long_to_string_fancy(v->v.i, out, sizeof(out));
    our_asprintf(&buffer, "%s", out);
  } break;
  case T_FLOAT: {
    char out[MAX_NUMBER_DIGITS] = {0};
    float_to_string_fancy(v->v.f, out, sizeof(out));
    our_asprintf(&buffer, "%s", out);
  } break;
  default: {
    char *tmp = as_c_string(v);
    our_asprintf(&buffer, "%s", tmp);
    mila_free(tmp);
  }
  }
  return buffer;
}

char *as_c_string_repr(Value *v) {
  char *buffer = NULL;
  if (!v) {
    return mila_strdup("cnull");
  }
  if (v->method_table && v->method_table[UMethodToRepr]) {
    Value *str = ((unary_method)v->method_table[UMethodToRepr])(v);
    char *res = mila_strdup(str->v.s);
    val_kill(str);
    return res;
  }
  if (v->method_table && v->method_table[UMethodToString]) {
    Value *str = ((unary_method)v->method_table[UMethodToString])(v);
    char *res = mila_strdup(str->v.s);
    val_kill(str);
    return res;
  }
  switch (v->type) {
  case T_STRING:
    our_asprintf(&buffer, "\"");
    char *temp = v->v.s ? v->v.s : "";
    for (size_t i = 0; i < strlen(temp); ++i) {
      switch (temp[i]) {
      case 7:
        our_asprintf(&buffer, "\\a");
        break;
      case 9:
        our_asprintf(&buffer, "\\t");
        break;
      case 10:
        our_asprintf(&buffer, "\\n");
        break;
      case 11:
        our_asprintf(&buffer, "\\v");
        break;
      case 12:
        our_asprintf(&buffer, "\\f");
        break;
      default:
        our_asprintf(&buffer, "%c", temp[i]);
      }
    }
    our_asprintf(&buffer, "\"");
    break;
  default: {
    char *tmp = as_c_string(v);
    our_asprintf(&buffer, "%s", tmp);
    mila_free(tmp);
  }
  }
  return buffer;
}

char *as_c_string_repr_raw(Value *v) {
  char hex_buf[5];
  char *buffer = NULL;
  if (!v) {
    return mila_strdup("cnull");
  }
  switch (v->type) {
  case T_STRING:
    our_asprintf(&buffer, "\"");
    char *temp = v->v.s ? v->v.s : "";
    for (size_t i = 0; i < strlen(temp); ++i) {
      switch (temp[i]) {
      case 7:
        our_asprintf(&buffer, "\\a");
        break;
      case 9:
        our_asprintf(&buffer, "\\t");
        break;
      case 10:
        our_asprintf(&buffer, "\\n");
        break;
      case 11:
        our_asprintf(&buffer, "\\v");
        break;
      case 12:
        our_asprintf(&buffer, "\\f");
        break;
      case '\x01':
      case '\x02':
      case '\x03':
      case '\x04':
      case '\x05':
      case '\x06':
      case '\x0E':
      case '\x0F':
      case '\x10':
      case '\x11':
      case '\x12':
      case '\x13':
      case '\x14':
      case '\x15':
      case '\x16':
      case '\x17':
      case '\x18':
      case '\x19':
      case '\x1A':
      case '\x1C':
      case '\x1D':
      case '\x1E':
      case '\x1F':
        snprintf(hex_buf, sizeof(hex_buf), "\\x%02X", (unsigned char)temp[i]);
        our_asprintf(&buffer, "%s", hex_buf);
        break;
      default:
        our_asprintf(&buffer, "%c", temp[i]);
      }
    }
    our_asprintf(&buffer, "\"");
    break;
  default: {
    char *tmp = as_c_string_raw(v);
    our_asprintf(&buffer, "%s", tmp);
    mila_free(tmp);
  }
  }
  return buffer;
}

// print value (for debug / native print)
int print_value(Value *v) {
  if (!v) {
    return printf("cnull");
  }
  char *txt;
  if (MILA_GET_TYPE(v) == T_OPAQUE || MILA_GET_TYPE(v) == T_OWNED_OPAQUE)
    txt = as_c_string_repr(v);
  else
    txt = as_c_string(v);
  int i = printf("%s", txt);
  mila_free(txt);
  return i;
}

int print_value_fancy(Value *v) {
  if (!v) {
    return printf("cnull");
  }
  char *txt = as_c_string_fancy(v);
  int i = printf("%s", txt);
  mila_free(txt);
  return i;
}

int print_value_repr(Value *v) {
  if (!v) {
    return printf("cnull");
  }
  char *txt = as_c_string_repr(v);
  int i = printf("%s", txt);
  mila_free(txt);
  return i;
}

int print_value_debug(Value *v) {
  if (!v) {
    return printf("cnull");
  }
  char *txt = as_c_string_repr(v);
  int i = printf("%s", txt);
  mila_free(txt);
  printf(" (%i)\n", v->refcount);
  return i;
}

Value *val_retain(Value *v) {
#ifdef MILA_DEBUG
  if (v) {
    printf("  ?? val_retain:\n     type: %s\n     refcount ++%i -> %i\n     "
           "value: ",
           MILA_GET_TYPENAME(v), v->refcount, v->refcount + 1);
    print_value_repr(v);
    puts("");
  } else {
    printf("  !! val_release:\n     JUST ATTEMPTED TO FREE A NULL VALUE!\n");
  }
#endif
  if (!v)
    return NULL;
  if (MILA_GET_TYPE(v) == T_WEAK_OPAQUE)
    return v;
  v->refcount++;
  return v;
}

// release
void val_release(Value *v) {
  if (!v)
    return;
  if (MILA_GET_TYPE(v) == T_WEAK_OPAQUE)
    return;
#ifdef MILA_DEBUG
  printf("  -- val_release:\n     type: %s\n     refcount --%i -> %i\n     "
         "%s\n     value: ",
         MILA_GET_TYPENAME(v), v->refcount, v->refcount - 1,
         v->refcount - 1 <= 0 ? "will be freed after" : "will survive");
  print_value_repr(v);
  puts("");
#endif
  v->refcount--;
  if (v->refcount <= 0) {
    if (v->method_table && v->method_table[UMethodFree]) {
      ((unary_method)v->method_table[UMethodFree])(v);
      goto cleanup;
    }
    // mila_free internals
    if (v->type == T_STRING && v->v.s)
      mila_free(v->v.s);
    if (v->type == T_ERROR && v->v.message)
      mila_free(v->v.message);
    if (v->type == T_TAGGED_ERROR && v->v.tagged_error.message)
      mila_free(v->v.tagged_error.message);
    if (v->type == T_FUNCTION) {
      if (v->v.fn->params) {
        char **p = v->v.fn->params;
        for (int i = 0; p[i]; ++i)
          mila_free(p[i]);
        mila_free(p);
      }
      if (v->v.fn->contextuals) {
        char **p = v->v.fn->contextuals;
        for (int i = 0; p[i]; ++i)
          mila_free(p[i]);
        mila_free(p);
      }
      if (v->v.fn->body_src)
        mila_free(v->v.fn->body_src);
      if (v->v.fn->name != NULL)
        mila_free(v->v.fn->name);
      env_free(v->v.fn->closure);
      mila_free(v->v.fn);
    }
    if (v->type == T_NATIVE) {
      if (v->v.native->name)
        mila_free(v->v.native->name);
      mila_free(v->v.native);
    }
    if (v->type == T_OWNED_OPAQUE) {
      if (v->v.opaque)
        mila_free(v->v.opaque);
    }
  cleanup:;
    if (v->type_name)
      mila_free(v->type_name);
    if (v->method_table && v->owns_table)
      mila_free(v->method_table);
    mila_free(v);
  }
}

// only release the data IN the value, not the value instance
void val_release_incomplete(Value *v) {
  if (!v)
    return;
  if (MILA_GET_TYPE(v) == T_WEAK_OPAQUE)
    return;
#ifdef MILA_DEBUG
  printf("  -- val_release_incomplete:\n     type: %s\n     refcount --%i -> "
         "%i\n     %s\n     value: ",
         MILA_GET_TYPENAME(v), v->refcount, v->refcount - 1,
         v->refcount - 1 <= 0 ? "will be freed after" : "will survive");
  print_value_repr(v);
  puts("");
#endif
  v->refcount--;
  if (v->refcount <= 0) {
    if (v->method_table && v->method_table[UMethodFree]) {
      ((unary_method)v->method_table[UMethodFree])(v);
      goto cleanup;
    }
    // mila_free internals
    if (v->type == T_STRING && v->v.s)
      mila_free(v->v.s);
    if (v->type == T_ERROR && v->v.message)
      mila_free(v->v.message);
    if (v->type == T_TAGGED_ERROR && v->v.tagged_error.message)
      mila_free(v->v.tagged_error.message);
    if (v->type == T_FUNCTION) {
      if (v->v.fn->params) {
        char **p = v->v.fn->params;
        for (int i = 0; p[i]; ++i)
          mila_free(p[i]);
        mila_free(p);
      }
      if (v->v.fn->contextuals) {
        char **p = v->v.fn->contextuals;
        for (int i = 0; p[i]; ++i)
          mila_free(p[i]);
        mila_free(p);
      }
      if (v->v.fn->body_src)
        mila_free(v->v.fn->body_src);
      if (v->v.fn->name)
        mila_free(v->v.fn->name);
      env_free(v->v.fn->closure);
      mila_free(v->v.fn);
    }
    if (v->type == T_NATIVE) {
      if (v->v.native->name)
        mila_free(v->v.native->name);
      mila_free(v->v.native);
    }
    if (v->type == T_OWNED_OPAQUE) {
      if (v->v.opaque)
        mila_free(v->v.opaque);
    }
  cleanup:;
    if (v->type_name)
      mila_free(v->type_name);
    if (v->method_table && v->owns_table)
      mila_free(v->method_table);
  }
}

void val_kill(Value *v) {
  if (!v)
    return;
#ifdef MILA_DEBUG
  printf("  -- val_kill:\n     type: %s\n     refcount %i -> 0 (forced)\n     "
         "%s\n     value: ",
         MILA_GET_TYPENAME(v), v->refcount,
         v->refcount - 1 <= 0 ? "will be freed after" : "will survive");
  print_value_repr(v);
  puts("");
#endif
  if (v->method_table && v->method_table[UMethodKill]) {
    ((unary_method)v->method_table[UMethodKill])(v);
    goto cleanup;
  }
  if (v->method_table && v->method_table[UMethodFree]) {
    ((unary_method)v->method_table[UMethodFree])(v);
    goto cleanup;
  }
  // mila_free internals
  if (v->type == T_STRING && v->v.s)
    mila_free(v->v.s);
  if (v->type == T_ERROR && v->v.message)
    mila_free(v->v.message);
  if (v->type == T_TAGGED_ERROR && v->v.tagged_error.message)
    mila_free(v->v.tagged_error.message);
  if (v->type == T_FUNCTION) {
    if (v->v.fn->params) {
      char **p = v->v.fn->params;
      for (int i = 0; p[i]; ++i)
        mila_free(p[i]);
      mila_free(p);
    }
    if (v->v.fn->contextuals) {
      char **p = v->v.fn->contextuals;
      for (int i = 0; p[i]; ++i)
        mila_free(p[i]);
      mila_free(p);
    }
    if (v->v.fn->body_src)
      mila_free(v->v.fn->body_src);
    if (v->v.fn->name)
      mila_free(v->v.fn->name);
    env_free(v->v.fn->closure);
    mila_free(v->v.fn);
  }
  if (v->type == T_NATIVE) {
    if (v->v.native->name)
      mila_free(v->v.native->name);
    mila_free(v->v.native);
  }
  if (v->type == T_RETURN) {
    val_kill(v->v.opaque);
  }
cleanup:;
  mila_free(v->type_name);
  if (v->method_table && v->owns_table)
    mila_free(v->method_table);
  mila_free(v);
}

void val_kill_incomplete(Value *v) {
  if (!v)
    return;
#ifdef MILA_DEBUG
  printf("  -- val_kill_incomplete:\n     type: %s\n     refcount %i -> 0 "
         "(forced)\n     %s\n     value: ",
         MILA_GET_TYPENAME(v), v->refcount,
         v->refcount - 1 <= 0 ? "will be freed after" : "will survive");
  print_value_repr(v);
  puts("");
#endif
  if (v->method_table && v->method_table[UMethodKill]) {
    ((unary_method)v->method_table[UMethodKill])(v);
    goto cleanup;
  }
  if (v->method_table && v->method_table[UMethodFree]) {
    ((unary_method)v->method_table[UMethodFree])(v);
    goto cleanup;
  }
  // mila_free internals
  if (v->type == T_STRING && v->v.s)
    mila_free(v->v.s);
  if (v->type == T_ERROR && v->v.message)
    mila_free(v->v.message);
  if (v->type == T_TAGGED_ERROR && v->v.tagged_error.message)
    mila_free(v->v.tagged_error.message);
  if (v->type == T_FUNCTION && v->v.fn) {
    if (v->v.fn->params) {
      char **p = v->v.fn->params;
      for (int i = 0; p[i]; ++i)
        mila_free(p[i]);
      mila_free(p);
    }
    if (v->v.fn->contextuals) {
      char **p = v->v.fn->contextuals;
      for (int i = 0; p[i]; ++i)
        mila_free(p[i]);
      mila_free(p);
    }
    if (v->v.fn->body_src)
      mila_free(v->v.fn->body_src);
    if (v->v.fn->name)
      mila_free(v->v.fn->name);
    env_free(v->v.fn->closure);
    mila_free(v->v.fn);
  }
  if (v->type == T_NATIVE) {
    if (v->v.native->name)
      mila_free(v->v.native->name);
    mila_free(v->v.native);
  }
  if (v->type == T_RETURN) {
    val_kill(v->v.opaque);
  }
cleanup:;
  mila_free(v->type_name);
  if (v->method_table && v->owns_table)
    mila_free(v->method_table);
}

// ---------- Environment (simple linked list of frames + variables) ----------

Env *env_new(Env *parent) {
  Env *e = mila_malloc(sizeof(Env));
  e->vars = NULL;
  e->contextual_vars = NULL;
  e->parent = parent;
  return e;
}

void env_free(Env *e) {
  if (!e)
    return;
  Var *v = e->vars;
  while (v) {
    Var *nx = v->next;
    mila_free(v->name);
    val_release(v->value);
    mila_free(v);
    v = nx;
  }
  v = e->contextual_vars;
  while (v) {
    Var *nx = v->next;
    mila_free(v->name);
    mila_free(v);
    v = nx;
  }
  mila_free(e);
}

void env_dump(Env *e) {
  if (!e)
    return;
  Var *v = e->vars;
  while (v) {
    Var *nx = v->next;
    if (!v->value) {
      printf("%s dangling\n", v->name);
      v = nx;
      continue;
    }
    if (v->value) {
      char *res = as_c_string_repr(v->value);
      printf("%s = %s (%i)\n", v->name, res, v->value->refcount);
      mila_free(res);
    }
    v = nx;
  }
  v = e->contextual_vars;
  while (v) {
    Var *nx = v->next;
    if (!v->value) {
      printf("%s dangling\n", v->name);
      v = nx;
      continue;
    }
    if (v->value) {
      char *res = as_c_string_repr(v->value);
      printf("%s = %s (%i)\n", v->name, res, v->value->refcount);
      mila_free(res);
    }
    v = nx;
  }
}

void env_kill(Env *e) {
  if (!e)
    return;
  Var *v = e->vars;
  while (v) {
    Var *nx = v->next;
    mila_free(v->name);
    val_kill(v->value);
    mila_free(v);
    v = nx;
  }
  v = e->contextual_vars;
  while (v) {
    Var *nx = v->next;
    mila_free(v->name);
    mila_free(v);
    v = nx;
  }
  mila_free(e);
}

Value *env_get(Env *e, const char *name) {
  for (Env *cur = e; cur; cur = cur->parent) {
    for (Var *v = cur->vars; v; v = v->next) {
      if (strcmp(v->name, name) == 0) {
        return v->value;
      }
    }
  }
  return NULL;
}

Value *env_get_contextual(Env *e, const char *name) {
  if (!(e && e->contextual_vars))
    return NULL;
  for (Env *cur = e; cur; cur = cur->parent) {
    for (Var *v = cur->contextual_vars; v; v = v->next) {
      if (strcmp(v->name, name) == 0) {
        return v->value;
      }
    }
  }
  return NULL;
}

void env_set_local_contextual(Env *e, const char *name, Value *val) {
  // set or create in current frame
  for (Var *v = e->contextual_vars; v; v = v->next) {
    if (strcmp(v->name, name) == 0) {
      v->value = val;
      return;
    }
  }
  Var *nv = mila_malloc(sizeof(Var));
  nv->name = mila_strdup(name);
  nv->next = e->contextual_vars;
  e->contextual_vars = nv;
}

int env_set_contextual(Env *e, const char *name, Value *val) {
  // assign to nearest visible frame that contains name, else set local
  for (Env *cur = e; cur; cur = cur->parent) {
    for (Var *v = cur->contextual_vars; v; v = v->next) {
      if (strcmp(v->name, name) == 0) {
        v->value = val;
        return 1;
      }
    }
  }
  // not found, set locally
  env_set_local_contextual(e, name, val);
  return 1;
}

void env_set_local_raw_contextual(Env *e, const char *name, Value *val) {
  // set or create in current frame
  for (Var *v = e->contextual_vars; v; v = v->next) {
    if (strcmp(v->name, name) == 0) {
      v->value = val;
      return;
    }
  }
  Var *nv = mila_malloc(sizeof(Var));
  nv->name = mila_strdup(name);
  nv->value = val;
  nv->next = e->contextual_vars;
  e->contextual_vars = nv;
}

int env_set_raw_contextual(Env *e, const char *name, Value *val) {
  // assign to nearest visible frame that contains name, else set local
  for (Env *cur = e; cur; cur = cur->parent) {
    for (Var *v = cur->contextual_vars; v; v = v->next) {
      if (strcmp(v->name, name) == 0) {
        v->value = val;
        return 1;
      }
    }
  }
  // not found, set locally
  env_set_local_raw_contextual(e, name, val);
  return 1;
}

void env_set_local(Env *e, const char *name, Value *val) {
  // set or create in current frame
  for (Var *v = e->vars; v; v = v->next) {
    if (strcmp(v->name, name) == 0) {
      val_release(v->value);
      v->value = val_retain(val);
      return;
    }
  }

  Var *nv = mila_malloc(sizeof(Var));
  nv->name = mila_strdup(name);
  nv->value = val_retain(val);
  nv->next = e->vars;
  e->vars = nv;
}

int env_set(Env *e, const char *name, Value *val) {
  // assign to nearest visible frame that contains name, else set local
  for (Env *cur = e; cur; cur = cur->parent) {
    for (Var *v = cur->vars; v; v = v->next) {
      if (strcmp(v->name, name) == 0) {
        val_release(v->value);
        v->value = val_retain(val);
        return 1;
      }
    }
  }
  // not found, set locally
  env_set_local(e, name, val);
  return 1;
}

void env_set_local_raw(Env *e, const char *name, Value *val) {
  // set or create in current frame
  for (Var *v = e->vars; v; v = v->next) {
    if (strcmp(v->name, name) == 0) {
      val_release(v->value);
      v->value = val;
      return;
    }
  }
  Var *nv = mila_malloc(sizeof(Var));
  nv->name = mila_strdup(name);
  nv->value = val;
  nv->next = e->vars;
  e->vars = nv;
}

int env_set_raw(Env *e, const char *name, Value *val) {
  // assign to nearest visible frame that contains name, else set local
  for (Env *cur = e; cur; cur = cur->parent) {
    for (Var *v = cur->vars; v; v = v->next) {
      if (strcmp(v->name, name) == 0) {
        val_release(v->value);
        v->value = val;
        return 1;
      }
    }
  }
  // not found, set locally
  env_set_local_raw(e, name, val);
  return 1;
}

void env_remove(Env *env, const char *name) {
  if (!env || !env->vars)
    return;

  Var *prev = NULL;
  Var *cur = env->vars;

  while (cur) {
    if (strcmp(cur->name, name) == 0) {
      if (prev)
        prev->next = cur->next;
      else
        env->vars = cur->next;
      mila_free(cur->name);
      mila_free(cur);
      return;
    }

    prev = cur;
    cur = cur->next;
  }
}

void env_remove_contextual(Env *env, const char *name) {
  if (!env || !env->contextual_vars)
    return;

  Var *prev = NULL;
  Var *cur = env->contextual_vars;

  while (cur) {
    if (strcmp(cur->name, name) == 0) {
      if (prev)
        prev->next = cur->next;
      else
        env->contextual_vars = cur->next;
      mila_free(cur->name);
      mila_free(cur);
      return;
    }

    prev = cur;
    cur = cur->next;
  }
}

#ifndef VMM_BUILD

int load_library(Env *env, const char *libpath) {
  int fail = 1;
#ifdef _WIN32
  HMODULE lib = LoadLibraryA(libpath);
  if (!lib) {
    fprintf(stderr, "LoadLibraryA('%s') failed (err=%lu)\n", libpath,
            GetLastError());
    return -1;
  }

  void (*init_func)(Env *) = void (*)(Env *)(lib, "_mila_lib_init");

  if (init_func) {
    fail = 0;
    init_func(env);
  }

  const char *const *names =
      (const char *const *)GetProcAddress(lib, "lib_functions");

  if (names) {
    for (size_t i = 0; names[i] != NULL; i++) {
      FARPROC f = GetProcAddress(lib, names[i]);
      if (!f) {
        fprintf(stderr, "Warning: function '%s' not found in '%s'\n", names[i],
                libpath);
        continue;
      }
      env_register_native(env, names[i], (void *)f);
    }
  }

  const NativeEntry *entries =
      (const NativeEntry *)GetProcAddress(lib, "lib_function_entries");

  if (entries) {
    char *name;
    NativeFn func;
    for (size_t i = 0; entries[i].name && entries[i].func; i++) {
      env_register_native(env, entries[i].name, entries[i].func);
    }
    return 0;
  }

  if (!fail)
    return 0;

  fprintf(stderr,
          "Must have a simple 'const char* lib_functions' and list the "
          "function names (last item must be NULL)\nor also 'const NativeEntry "
          "lib_function_entries[]' which takes in (NativeEntry){.name, .func} "
          "with the last item being (NativeEntry){NULL, NULL}\n");
  FreeLibrary(lib);
  return -2;

#else // POSIX
  void *lib = dlopen(libpath, RTLD_LAZY);
  if (!lib) {
    fprintf(stderr, "dlopen('%s') failed: %s\n", libpath, dlerror());
    return -1;
  }

  dlerror();
  void (*init_func)(Env *) = dlsym(lib, "_mila_lib_init");
  const char *err = dlerror();
  if (!err) {
    fail = 0;
    init_func(env);
  }

  const char *const *names = (const char *const *)dlsym(lib, "lib_functions");

  err = dlerror();
  if (!err) {
    for (size_t i = 0; names[i] != NULL; i++) {
      dlerror();
      void *f = dlsym(lib, names[i]);
      const char *err2 = dlerror();
      if (err2) {
        fprintf(stderr, "Warning: '%s' not found in '%s'\n", names[i], libpath);
        continue;
      }
      env_register_native(env, names[i], f);
    }
  }

  dlerror();
  const NativeEntry *entries =
      (const NativeEntry *)dlsym(lib, "lib_function_entries");

  err = dlerror();
  if (!err) {
    for (size_t i = 0; entries[i].name && entries[i].func; i++) {
      env_register_native(env, entries[i].name, entries[i].func);
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
#endif
}
int load_library_noisy(Env *env, const char *libpath) {
  int fail = 1;
#ifdef _WIN32
  HMODULE lib = LoadLibraryA(libpath);
  if (!lib) {
    fprintf(stderr, "LoadLibraryA('%s') failed (err=%lu)\n", libpath,
            GetLastError());
    return -1;
  }

  void (*init_func)(Env *) = void (*)(Env *)(lib, "_mila_lib_init");

  if (init_func) {
    fail = 0;
    init_func(env);
  }

  const char *const *names =
      (const char *const *)GetProcAddress(lib, "lib_functions");

  if (names) {
    for (size_t i = 0; names[i] != NULL; i++) {
      FARPROC f = GetProcAddress(lib, names[i]);
      if (!f) {
        fprintf(stderr, "Warning: function '%s' not found in '%s'\n", names[i],
                libpath);
        continue;
      }
      fprintf(stderr, "from lib_functions, Loaded symbol '%s'\n", names[i]);
      env_register_native(env, names[i], (void *)f);
    }
  }

  const NativeEntry *entries =
      (const NativeEntry *)GetProcAddress(lib, "lib_function_entries");

  if (entries) {
    char *name;
    NativeFn func;
    for (size_t i = 0; entries[i].name && entries[i].func; i++) {
      fprintf(stderr, "from lib_function_entries, Loaded symbol '%s'\n",
              entries[i].name);
      env_register_native(env, entries[i].name, entries[i].func);
    }
    return 0;
  }

  if (!fail)
    return 0;

  fprintf(stderr,
          "Must have a simple 'const char* lib_functions' and list the "
          "function names (last item must be NULL)\nor also 'const NativeEntry "
          "lib_function_entries[]' which takes in (NativeEntry){.name, .func} "
          "with the last item being (NativeEntry){NULL, NULL}\n");
  FreeLibrary(lib);
  return -2;

#else // POSIX
  void *lib = dlopen(libpath, RTLD_LAZY);
  if (!lib) {
    fprintf(stderr, "dlopen('%s') failed: %s\n", libpath, dlerror());
    return -1;
  }

  dlerror();
  void (*init_func)(Env *) = dlsym(lib, "_mila_lib_init");
  const char *err = dlerror();
  if (!err) {
    fail = 0;
    init_func(env);
  }

  const char *const *names = (const char *const *)dlsym(lib, "lib_functions");

  err = dlerror();
  if (!err) {
    for (size_t i = 0; names[i] != NULL; i++) {
      dlerror();
      void *f = dlsym(lib, names[i]);
      const char *err2 = dlerror();
      if (err2) {
        fprintf(stderr, "Warning: '%s' not found in '%s'\n", names[i], libpath);
        continue;
      }
      fprintf(stderr, "from lib_functions, Loaded symbol '%s'\n", names[i]);
      env_register_native(env, names[i], f);
    }
  }

  dlerror();
  const NativeEntry *entries =
      (const NativeEntry *)dlsym(lib, "lib_function_entries");

  err = dlerror();
  if (!err) {
    for (size_t i = 0; entries[i].name && entries[i].func; i++) {
      fprintf(stderr, "from lib_function_entries, Loaded symbol '%s'\n",
              entries[i].name);
      env_register_native(env, entries[i].name, entries[i].func);
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
#endif
}

#endif

// helper to bind native into environment with a name
void env_register_native(Env *env, const char *name, NativeFn fn) {
  Value *nv = vnative(fn, name);
  env_set_local_raw(env, name, nv);
}

// ---------- Parser/Evaluator that directly reads source and evaluates (no
// separate lexer) ----------

Src *src_new(const char *s) {
  Src *S = mila_malloc(sizeof(Src));
  S->len = strlen(s);
  S->src = mila_strdup(s);
  S->pos = 0;
  S->line = 0;
  return S;
}
void src_free(Src *s) {
  if (!s)
    return;
  mila_free(s->src);
  mila_free(s);
}

// helpers
char src_peek(Src *s) { return s->pos < s->len ? s->src[s->pos] : '\0'; }
char src_get(Src *s) { return s->pos < s->len ? s->src[s->pos++] : '\0'; }
int src_eof(Src *s) { return s->pos >= s->len; }

void skip_block(Src *s) {
  skip_ws(s);
  // body is block; extract substring from '{' to matching '}'
  if (src_peek(s) != '{') {
    return;
  }
  int depth = 0;
  int i = s->pos;
  for (; i < s->len; ++i) {
    char ch = s->src[i];
    if (ch == '\n')
      s->line++;
    if (ch == '{')
      depth++;
    else if (ch == '}') {
      depth--;
      if (depth == 0) {
        i++;
        break;
      }
    } else if (ch == '"') {
      // skip string literal
      i++;
      while (i < s->len && s->src[i] != '"') {
        if (s->src[i] == '\n')
          s->line++;
        if (s->src[i] == '\\' && i + 1 < s->len)
          i += 2;
        else
          i++;
      }
    } else if (ch == '/' && src_peek(s) == '/') {
      // skip comments
      i++;
      while (i < s->len && s->src[i] != '\n')
        i++;
    }
  }
  if (i > s->len)
    i = s->len;
  s->pos = i;
}

void skip_expr(Src *s) {
  skip_ws(s);
  // body is block; extract substring from '{' to matching '}'
  int depth = 1;
  int i = s->pos;
  for (; i < s->len; ++i) {
    char ch = s->src[i];
    if (ch == '(')
      depth++;
    else if (ch == ')') {
      depth--;
      if (depth == 0) {
        i++;
        break;
      }
    } else if (ch == '"') {
      // skip string literal
      i++;
      while (i < s->len && s->src[i] != '"') {
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

void skip_stmt(Src *s) {
  skip_ws(s);
  int i = s->pos;
  for (; i < s->len; ++i) {
    char ch = s->src[i];
    if (ch == ';') {
      i++;
      break;
    } else if (ch == '"') {
      // skip string literal
      i++;
      while (i < s->len && s->src[i] != '"') {
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

void skip_ws(Src *s) {
  for (;;) {
    char c = src_peek(s);
    if (c == '\0')
      return;
    if (isspace((unsigned char)c)) {
      src_get(s);
      continue;
    }
    if (c == '/' && s->pos + 1 < s->len && s->src[s->pos + 1] == '/') {
      // line comment
      src_get(s);
      src_get(s);
      while (!src_eof(s) && src_get(s) != '\n') {
      }
      continue;
    }
    if (c == '/' && s->pos + 1 < s->len && s->src[s->pos + 1] == '*') {
      // block comment
      src_get(s);
      src_get(s);
      while (!src_eof(s)) {
        char a = src_get(s);
        if (a == '*' && src_peek(s) == '/') {
          src_get(s);
          break;
        }
      }
      continue;
    }
    break;
  }
}

int match_char(Src *s, char c) {
  skip_ws(s);
  if (src_peek(s) == c) {
    src_get(s);
    return 1;
  }
  return 0;
}

int is_ident_start(char c) {
  return isalpha((unsigned char)c) || c == '_' || c == '.';
}

// parse identifier
char *parse_ident(Src *s) {
  skip_ws(s);
  int st = s->pos;
  char c = src_peek(s);
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

  if (res[0] == '.' && s->cur_namespace != NULL) {
    char *r = mila_strdup(s->cur_namespace);
    our_asprintf(&r, ".%s", res + 1);
    mila_free(res);
    return r;
  }

  return res;
}

// parse number (int or float)
Value *parse_number(Src *s) {
  skip_ws(s);
  int st = s->pos;
  _Bool seen_dot = 0;
  _Bool is_unsigned = 0;
  _Bool is_percent = 0;
  _Bool is_hex = 0;
  if (src_peek(s) == '-') {
    src_get(s);
  }
  // either normal number or hex
  while (isdigit((unsigned char)src_peek(s)) || src_peek(s) == '.' ||
         src_peek(s) == 'x' || src_peek(s) == 'X' ||
         isxdigit((unsigned char)src_peek(s))) {
    if (src_peek(s) == '.') {
      if (seen_dot || is_hex) // cant have hex literal have decimal points (that
                              // would be cool though)
      {
        break;
      }
      seen_dot = 1;
    } else if (src_peek(s) == 'x' || src_peek(s) == 'X') {
      if (is_hex || seen_dot) {
        break;
      }
      is_hex = 1;
    }
    src_get(s);
  }

  if (src_peek(s) == 'u' || src_peek(s) == 'U') {
    is_unsigned = 1;
    src_get(s);
  }
  if (src_peek(s) == '%') {
    is_percent = 1;
    src_get(s);
  }

  int en = s->pos;
  char tmp[MAX_NUMBER_DIGITS];
  int len = en - st;
  if (len <= 0)
    return NULL;

  if (len < (int)sizeof(tmp)) {
    memcpy(tmp, s->src + st, len);
    tmp[len] = 0;
    if (seen_dot) {
      double f = atof(tmp);
      if (is_percent)
        f /= 100.0;
      return vfloat(f);
    } else {
      long i = strtol(tmp, NULL, is_hex ? 16 : 10);
      if (is_percent)
        return vfloat((double)i / 100.0);
      return is_unsigned ? vuint(i > 0 ? i : -i) : vint(i);
    }
  } else {
    // implement arbritrarily sized integers in the future
    char *buf = mila_malloc(len + 1);
    memcpy(buf, s->src + st, len);
    buf[len] = 0;
    Value *r;
    if (seen_dot) {
      double f = atof(buf);
      if (is_percent)
        f /= 100.0;
      r = vfloat(f);
    } else {
      long tmp = strtol(buf, NULL, is_hex ? 16 : 10);
      if (is_percent)
        return vfloat((double)tmp / 100.0);
      r = is_unsigned ? vuint(tmp > 0 ? tmp : -tmp) : vint(tmp);
    }
    mila_free(buf);
    return r;
  }
}

// parse string literal (double quotes)
Value *parse_string(Src *s) {
  skip_ws(s);
  if (src_peek(s) != '"')
    return verror("String unterminated!");
  src_get(s); // consume opening "

  size_t cap = 256;
  size_t len = 0;
  char *buf = mila_malloc(cap);
  if (!buf)
    return verror("Allocation failure");

  while (!src_eof(s)) {
    char c = src_get(s);
    if (c == '"')
      break;

    if (c == '\\') {
      char n = src_get(s);
      switch (n) {
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
      case 'N': { // \Nxx (decimal) escape codes
        char code[3] = {src_get(s), src_get(s), 0};
        c = (char)atoi(code);
      } break;
      case 'x': { // \xhh (hex) escape codes
        char code[3] = {src_get(s), src_get(s), 0};
        c = (char)strtol(code, NULL, 16);
      } break;
      case '0': { // \0oo (octal) escape codes
        char code[3] = {src_get(s), src_get(s), 0};
        c = (char)strtol(code, NULL, 8);
      } break;
      default:
        c = n;
        break;
      }
    }

    // Grow buffer if needed
    if (len + 1 >= cap) {
      cap = cap + (int)(cap * 0.3); // grow by 30%
      char *tmp = mila_realloc(buf, cap);
      if (!tmp) {
        mila_free(buf);
        return verror("Allocation failure!");
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
  return vstring_take(res);
}

void src_advance_by(Src *s, size_t amount) { s->pos += amount; }

// parse function literal: fn(a,b){ ... }
int is_keyword_at(Src *s, const char *kw) {
  skip_ws(s);
  int p = s->pos;
  int klen = strlen(kw);
  if (p + klen > s->len)
    return 0;
  if (strncmp(s->src + p, kw, klen) == 0) {
    char after = s->src[p + klen];
    if (after != '_' &&
        (after == '\0' || isspace((unsigned char)after) || (!isalnum(after))))
      return 1;
  }
  return 0;
}

char *dup_substr(Src *s, int a, int b) {
  int n = b - a;
  char *r = mila_malloc(n + 1);
  memcpy(r, s->src + a, n);
  r[n] = 0;
  return r;
}

// parse comma-separated identifiers (for parameters)
char **parse_param_list(Src *s) {
  skip_ws(s);
  if (!match_char(s, '('))
    return NULL;
  skip_ws(s);
  // empty
  if (match_char(s, ')')) {
    char **p = mila_malloc(sizeof(char *));
    p[0] = NULL;
    return p;
  }
  // read identifiers
  char **arr = NULL;
  int cnt = 0;
  for (;;) {
    char *id = parse_ident(s);
    if (!id) {
      // error
      // cleanup
      for (int i = 0; i < cnt; i++)
        mila_free(arr[i]);
      mila_free(arr);
      return NULL;
    }
    if (match_char(s, ':')) {
      Value *type = parse_string(s);
      // ignore types for now
      val_kill(type);
    }
    arr = mila_realloc(arr, sizeof(char *) * (cnt + 2));
    arr[cnt++] = id;
    arr[cnt] = NULL;
    skip_ws(s);
    if (match_char(s, ','))
      continue;
    if (match_char(s, ')'))
      break;
    break;
  }
  return arr;
}

char **parse_context_list(Src *s) {
  skip_ws(s);
  if (!match_char(s, '['))
    return NULL;
  skip_ws(s);
  // empty
  if (match_char(s, ']')) {
    char **p = mila_malloc(sizeof(char *));
    p[0] = NULL;
    return p;
  }
  // read identifiers
  char **arr = NULL;
  int cnt = 0;
  for (;;) {
    char *id = parse_ident(s);
    if (!id) {
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
    skip_ws(s);
    if (match_char(s, ','))
      continue;
    if (match_char(s, ']'))
      break;
    // error -> break
    break;
  }
  return arr;
}

Value *parse_subscript(Src *s, Env *e) {
  if (!match_char(s, '[')) {
    return verror("Subscript was expected!");
  }

  Value *res = eval_expr(s, e);

  if (!match_char(s, ']')) {
    return verror("Closing square bracket was expected!");
  }

  return res;
}

// parse block: {...}
Value *eval_block(Src *s, Env *env) {
  skip_ws(s);
  if (!match_char(s, '{')) {
    return verror("Expected a block!");
  }
  // new local frame
  Env *frame = env_new(env);
  Value *last = vnull(); // last expression value
  while (1) {
    skip_ws(s);
    if (src_peek(s) == '\0')
      break;
    if (match_char(s, '}'))
      break;
    val_release(last);
    Value *st = eval_statement(s, frame);
    last = st;

    if (IS_ERROR(st)) {
      env_free(frame);
      return st;
    }
    if (IS_CONTROL(st)) {
      env_free(frame);
      if (st->type == T_RETURN)
        return st;
      HANDLE_CONTROL(st);
    }
  }
  env_free(frame);
  return last;
}

Value *eval_block_raw(Src *s, Env *frame) {
  skip_ws(s);
  if (!match_char(s, '{')) {
    return verror("Block was expected!");
  }
  // new local frame
  Value *last = vnull(); // last expression value
  while (1) {
    skip_ws(s);
    if (src_peek(s) == '\0')
      break;
    if (match_char(s, '}'))
      break;
    Value *st = eval_statement(s, frame);
    val_release(last);
    last = st;

    if (IS_ERROR(st)) {
      return st;
    }
    if (IS_CONTROL(st)) {
      if (st->type == T_RETURN)
        return st;
      HANDLE_CONTROL(st);
    }
  }
  return last;
}

// now we need eval_primary, function call, member? only call expressions and
// binary ops evaluate primary

Value *call_function_with(Env *env, Value *fnval, Value *first, ...) {
  va_list ap;
  size_t count = 0;

  /* First pass: count */
  va_start(ap, first);
  for (Value *v = first; v != NULL; v = va_arg(ap, Value *)) {
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
  for (Value *v = first; v != NULL; v = va_arg(ap, Value *)) {
    args[i++] = v;
  }
  va_end(ap);

  if (!fnval) {
    for (int i = 0; i < count; i++)
      val_release(args[i]);
    return verror("Function is NULL!");
  }

  Value *res = call_function(fnval, env, count, args);
  for (int i = 0; i < count; ++i)
    val_release(args[i]);
  mila_free(args);
  HANDLE_RETURN(res);
  return res;
}

Value *call_function_str(Env *env, const char *fnname, Value *first, ...) {
  va_list ap;
  size_t count = 0;

  /* First pass: count */
  va_start(ap, first);
  for (Value *v = first; v != NULL; v = va_arg(ap, Value *)) {
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
  for (Value *v = first; v != NULL; v = va_arg(ap, Value *)) {
    args[i++] = v;
  }
  va_end(ap);

  Value *fnval = env_get(env, fnname);
  if (!fnval) {
    for (int i = 0; i < count; i++)
      val_release(args[i]);
    return verror("Function is NULL!");
  }

  Value *res = call_function(fnval, env, count, args);
  for (int i = 0; i < count; ++i)
    val_release(args[i]);
  mila_free(args);
  HANDLE_RETURN(res);
  return res;
}

Value **make_args(Value *first, ...) {
  va_list ap;
  size_t count = 0;

  /* First pass: count */
  va_start(ap, first);
  for (Value *v = first; v != NULL; v = va_arg(ap, Value *)) {
    count++;
  }
  va_end(ap);

  Value **args = mila_malloc((count) * sizeof(Value *));
  if (!args)
    return NULL;

  /* Second pass: fill */
  va_start(ap, first);
  size_t i = 0;
  for (Value *v = first; v != NULL; v = va_arg(ap, Value *)) {
    args[i++] = v;
  }
  va_end(ap);
  return args;
}

Value *call_function(Value *fnval, Env *env, int argc, Value **argv) {
  if (!fnval)
    return verror("Function is NULL!");
  if (fnval->type == T_NATIVE) {
    for (int t = 0; t < argc; ++t)
      if (MILA_GET_TYPE(argv[t]) == T_ERROR)
        return argv[t];
    Value *result = fnval->v.native->fn(env, argc, argv);
    return result;
  } else if (fnval->type == T_FUNCTION) {
    // create new environment with closure as parent
    Env *frame = NULL;
    if (fnval->v.fn->closure) {
      fnval->v.fn->closure->parent = env;
      frame = env_new(fnval->v.fn->closure);
    } else {
      frame = env_new(env);
    }
    // bind params
    char **p = fnval->v.fn->params;
    int i = 0;
    for (; p && p[i]; ++i) {
      // if fewer args provided, bind null
      if (i < argc && MILA_GET_TYPE(argv[i]) == T_ERROR) {
        return argv[i];
      }
      Value *a = (i < argc) ? argv[i] : NULL;
      if (a == NULL) {
        i++;
        break;
      }
      env_set_local(frame, p[i], a);
    }
    // set contextual values
    p = fnval->v.fn->contextuals;
    i = 0;
    for (; p && p[i]; ++i) {
      char *name = mila_strdup(p[i]);
      int is_optional = 0;
      if (name[strlen(name) - 1] == '?') {
        is_optional = 1;
        name[strlen(name) - 1] = 0;
      }
      Value *a = env_get_contextual(env, name);
      if (is_optional == 0 && a == NULL) {
        env_free(frame);
        Value *res =
            verror("Function %s requires the contextual value `%s`",
                   fnval->v.fn->name ? fnval->v.fn->name : "(lambda)", name);
        mila_free(name);
        return res;
      } else if (a && !is_optional) {
        env_set_local(frame, name, a);
      } else if ((!a) && is_optional) {
        env_set_local_raw(frame, name, vnull());
      }
      mila_free(name);
    }
    // Evaluate body: note body_src contains the body text e.g., "{ ... }"
    Src *child = src_new(fnval->v.fn->body_src);
    // position should start at 0 for the body; body is a block (starts with
    // '{') Evaluate block using the new frame
    Value *res = eval_block(child, frame);
    src_free(child);
    env_free(frame);
    HANDLE_CONTROL(res);
    return res;
  } else {
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
Value *eval_primary(Src *s, Env *env) {
  skip_ws(s);
  char c = src_peek(s);
  if (c == '\0')
    return vnull();
  // number
  if (isdigit((unsigned char)c) ||
      ((c == '+' || c == '-') && isdigit((unsigned char)s->src[s->pos + 1])) ||
      (c == '0' && (s->src[s->pos + 1] == 'x' || s->src[s->pos + 1] == 'X') &&
       isxdigit((unsigned char)s->src[s->pos + 2]))) {
    return parse_number(s);
  }
  // string
  if (c == '"') {
    return parse_string(s);
  }
  if (c=='[') {
    src_get(s);
    char is_dict = match_char(s, '@');
    size_t start = s->pos;
    Value **args = NULL;
    Value* list = call_function_str(env, "list", NULL);
    int argc = 0;
    for (;;) {
      skip_ws(s);
      Value *a = eval_expr(s, env);
      if (IS_ERROR(a)) {
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
      while (k) {
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
          len, s->src + start);
    }
    if (is_dict) {
      Value* dict = native_new_dict(env, argc, args);
      val_release(list);
      free(args);
      return dict;
    }
    free(args);
    return list;
  }
  // parentheses
  if (c == '(') {
    size_t start = s->pos;
    src_get(s); // consume '('
    Value *expr = eval_expr(s, env);
    skip_ws(s);
    if (src_peek(s) == ')')
      src_get(s);

    if (src_peek(s) == '(') {
      // parse args
      src_get(s); // consume '('
      // parse comma separated expressions
      Value **args = NULL;
      int argc = 0;
      skip_ws(s);
      if (src_peek(s) != ')') {
        for (;;) {
          Value *a = eval_expr(s, env);
          if (IS_ERROR(a)) {
            mila_free(args);
            val_release(expr);
            for (int i = 0; i < argc; i++)
              val_release(args[i]);
            return a;
          }
          args = mila_realloc(args, sizeof(Value *) * (argc + 1));
          args[argc++] = a;
          skip_ws(s);
          if (match_char(s, ','))
            continue;
          if (match_char(s, ')'))
            break;
          val_release(expr);
          for (int i = 0; i < argc; i++)
            val_release(args[i]);
          mila_free(args);
          int k = 1;
          while (k) {
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
      } else {
        // empty
        src_get(s); // consume ')'
      }
      // callp
      Value *res = call_function(expr, env, argc, args);
      for (int i = 0; i < argc; i++)
        val_release(args[i]);
      mila_free(args);
      val_release(expr);
      if (MILA_GET_TYPE(res) == T_RETURN) {
        Value *tmp = (Value *)res->v.opaque;
        val_release(res);
        return tmp;
      }
      HANDLE_CONTROL(res);
      return res;
    } else if (src_peek(s) == '[') {
      Value *obj = expr;

      if (!obj) {
        Value *ret = verror("cannot be subscripted as it is cnull");
        return ret;
      }

      // Handle chained subscripts: (expr)[x][y][z]
      while (src_peek(s) == '[') {
        Value *index = parse_subscript(s, env);

        if (!obj) {
          val_release(index);
          Value *ret = verror("cannot be subscripted as it is cnull");
          return ret;
        }

        if (obj->method_table && obj->method_table[BMethodGetItem]) {
          Value *res =
              ((binary_method)obj->method_table[BMethodGetItem])(obj, index);
          val_release(index);
          Value *tmp = val_retain(res);
          val_release(obj);
          obj = tmp;
        } else {
          val_release(index);
          val_release(obj);
          return verror("Type %s does not support BMethodGetItem!",
                        MILA_GET_TYPENAME(obj));
        }
      }

      return obj;
    } else if (src_peek(s) == ':') {
      src_get(s); // skip the colon
      char *method = parse_ident(s);

      Value *obj = expr;
      Value *attr = vstring_take(method);
      Value *function = NULL;

      if (!obj) {
        val_release(attr);
        Value *ret = verror("cannot be subscripted as it is cnull");
        return ret;
      }

      if (obj->method_table && obj->method_table[BMethodGetItem]) {
        function =
            ((binary_method)obj->method_table[BMethodGetItem])(obj, attr);
        val_release(attr);
        if (!function) {
          Value *err =
              verror("Function %s didnt exist in expression", GET_STRING(attr));
          return err;
        }
      } else {
        val_release(attr);
        return verror("Type %s does not support BMethodGetItem!",
                      MILA_GET_TYPENAME(obj));
      }

      if (src_peek(s) == '(') {
        // parse args
        src_get(s); // consume '('
        // parse comma separated expressions
        Value **args = mila_malloc(sizeof(Value *));
        args[0] = val_retain(obj);
        int argc = 1;
        skip_ws(s);

        // handle (value)(...) calls
        if (src_peek(s) != ')') {
          for (;;) {
            Value *a = eval_expr(s, env);
            if (IS_ERROR(a)) {
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
            while (k) {
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
        } else {
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
    }
    return expr;
  }
  if (c == '{') {
    Value *v = eval_block(s, env);
    skip_ws(s);
    match_char(s, '}');

    HANDLE_RETURN(v);
    return v;
  }
  if (c == '!' && s->src[s->pos + 1] == '{') {
    src_get(s); // consume '!'
    size_t start = s->pos + 1;
    skip_block(s);
    size_t end = s->pos - 1; // avoid the closing }
    skip_ws(s);

    char *buffer = (char *)mila_malloc(sizeof(char) * (end - start) + 1);
    memcpy(buffer, s->src + start, end - start);

    return vstring_take(buffer);
  }
  // function literal
  if (is_keyword_at(s, "fn")) {
    // consume keyword
    s->pos += strlen("fn");
    // parse params
    char **params = parse_param_list(s);
    char **contextuals = parse_context_list(s);
    char **names;
    Env *closure = env_new(NULL);
    if (match_char(s, ':')) {
      // parse closure bindings
      names = parse_context_list(s); // reused parse_context_list
      for (int i = 0; names[i]; ++i) {
        env_set_local(closure, names[i], env_get(env, names[i]));
        mila_free(names[i]);
      }
      mila_free(names);
    }
    if (is_keyword_at(s, "->")) {
      s->pos += 2;
      skip_ws(s);
      if (src_peek(s) == '"') {
        Value *ret_type = parse_string(s);
        // ignore return type for now
        val_kill(ret_type);
      } else {
        env_free(closure);
        for (int i = 0; params[i]; ++i) {
          mila_free(params[i]);
        }
        mila_free(params);
        return vtagged_error(E_SYNTAX_ERROR,
                             "Expected a string literal for the return type.");
      }
    }
    skip_ws(s);
    // body is block; extract substring from '{' to matching '}'
    if (src_peek(s) != '{') {
      // error: expected body
      return verror("Body wasnt found.");
    }
    // find matching brace (we will copy out body)
    int depth = 0;
    int start = s->pos;
    int i = s->pos;
    for (; i < s->len; ++i) {
      char ch = s->src[i];
      if (ch == '{')
        depth++;
      else if (ch == '}') {
        depth--;
        if (depth == 0) {
          i++;
          break;
        }
      } else if (ch == '"') {
        // skip string literal
        i++;
        while (i < s->len && s->src[i] != '"') {
          if (s->src[i] == '\\' && i + 1 < s->len)
            i += 2;
          else
            i++;
        }
      }
    }
    if (i > s->len)
      i = s->len;
    int blen = i - start;
    char *body = mila_malloc(blen + 1);
    memcpy(body, s->src + start, blen);
    body[blen] = 0;
    s->pos = i;
    // create function value with closure get_line_pos(s) current env
    Value *fn = vfunction(params, contextuals, closure, body);
    return fn;
  }
  // identifier or keyword like 'null', 'true', 'false', or bare native name
  if (is_ident_start(c)) {
    size_t start = s->pos;
    char *id = parse_ident(s);
    if (!id)
      return vnull();
    // keywords
    if (strcmp(id, "null") == 0) {
      mila_free(id);
      return vnull();
    }
    if (strcmp(id, "none") == 0) {
      mila_free(id);
      return vnone();
    }
    if (strcmp(id, "true") == 0) {
      mila_free(id);
      return vbool(1);
    }
    if (strcmp(id, "false") == 0) {
      mila_free(id);
      return vbool(0);
    }
    if (strcmp(id, "break") == 0) {
      mila_free(id);
      return vbreak();
    }
    if (strcmp(id, "continue") == 0) {
      mila_free(id);
      return vcontinue();
    }
    // look ahead: function call? subscript?
    skip_ws(s);
    if (src_peek(s) == '(') {
      // parse args
      src_get(s); // consume '('
      // parse comma separated expressions
      Value **args = NULL;
      int argc = 0;
      skip_ws(s);

      // handle (value)(...) calls
      if (src_peek(s) != ')') {
        for (;;) {
          Value *a = eval_expr(s, env);
          if (IS_ERROR(a)) {
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
          while (k) {
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
      } else {
        // empty
        src_get(s); // consume ')'
      }
      // get callee (id)
      Value *callee = env_get(env, id);
      if (!callee) {
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
      Value *res = call_function(callee, env, argc, args);
      for (int i = 0; i < argc; i++)
        val_release(args[i]);
      mila_free(args);
      HANDLE_RETURN(res);
      return res;
    } else if (src_peek(s) == '[') {
      Value *obj = env_get(env, id);
      if (!obj) {
        Value *ret = verror("%s cannot be subscripted as it is cnull", id);
        mila_free(id);
        return ret;
      }
      mila_free(id);

      // Handle chained subscripts: (expr)[x][y][z]
      while (src_peek(s) == '[') {
        Value *index = parse_subscript(s, env);

        if (!obj) {
          val_release(index);
          Value *ret = verror("cannot be subscripted as it is cnull");
          return ret;
        }

        if (obj->method_table && obj->method_table[BMethodGetItem]) {
          Value *res =
              ((binary_method)obj->method_table[BMethodGetItem])(obj, index);
          val_release(index);
          Value *tmp = res;
          // val_release(obj);
          obj = tmp;
        } else {
          val_release(index);
          val_release(obj);
          return verror("Type %s does not support BMethodGetItem!",
                        MILA_GET_TYPENAME(obj));
        }
      }

      return val_retain(obj);
    } else if (src_peek(s) == ':') {
      src_get(s); // skip the colon
      char *method = parse_ident(s);

      Value *obj = env_get(env, id);
      Value *attr = vstring_take(method);
      Value *function = NULL;

      if (!obj) {
        val_release(attr);
        Value *ret = verror("cannot be subscripted as it is cnull");
        mila_free(id);
        return ret;
      }

      if (obj->method_table && obj->method_table[BMethodGetItem]) {
        function =
            ((binary_method)obj->method_table[BMethodGetItem])(obj, attr);
        if (!function) {
          Value *err = verror("Function %s didnt exist in value %s",
                              GET_STRING(attr), id);
          val_release(attr);
          mila_free(id);
          return err;
        }
        val_release(attr);
      } else {
        mila_free(id);
        val_release(attr);
        return verror("Type %s does not support BMethodGetItem!",
                      MILA_GET_TYPENAME(obj));
      }

      if (src_peek(s) == '(') {
        // parse args
        src_get(s); // consume '('
        // parse comma separated expressions
        Value **args = mila_malloc(sizeof(Value *));
        args[0] = val_retain(obj);
        int argc = 1;
        skip_ws(s);

        // handle (value)(...) calls
        if (src_peek(s) != ')') {
          for (;;) {
            Value *a = eval_expr(s, env);
            if (IS_ERROR(a)) {
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
            while (k) {
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
        } else {
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
    } else {
      // variable lookup
      Value *vv = env_get(env, id);
#ifdef MILA_DEBUG
      printf("    ?? read %s\n", id);
#endif
      mila_free(id);
      if (!vv) {
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
static int is_number(Value *v) {
  return v && (v->type == T_INT || v->type == T_FLOAT || v->type == T_UINT);
}

double to_double(Value *v) {
  if (!v)
    return 0.0;
  if (v->type == T_INT)
    return (double)v->v.i;
  if (v->type == T_FLOAT)
    return v->v.f;
  return 0.0;
}

// binary ops
Value *binary_op(Value *a, MethodType op, Value *b) {
  if (a->method_table && a->method_table[op]) {
    return ((binary_method)a->method_table[op])(a, b);
  } else if ((a->type == T_NONE || a->type == T_NULL) &&
             (b->type == T_NONE || b->type == T_NULL)) {
    if (BMethodEq == op)
      return vbool(a->type == b->type);
    if (BMethodNe == op)
      return vbool(a->type != b->type);
  } else if (op == BMethodDefault) {
    if (a->type == T_NONE || a->type == T_NULL || !is_truthy(a)) {
      return val_retain(b);
    } else {
      return val_retain(a);
    }
  } else if (BMethodOr == op) {
    int res = is_truthy(a) || is_truthy(b);
    return vbool(res);
  } else if (BMethodAnd == op) {
    int res = is_truthy(a) && is_truthy(b);
    return vbool(res);
  } else if (is_number(a) && is_number(b)) {
    if (a->type == T_UINT || b->type == T_UINT)
    // treat both numbers as unsigned.
    {
      unsigned long ia = a->v.ui, ib = b->v.ui;
      if (op == BMethodAdd)
        return vuint(ia + ib);
      if (op == BMethodSub) {
        if (a->type == T_INT) {
          long v = a->v.i - b->v.i;
          return vuint(v > 0 ? v : -v);
        }
        return vuint(ia - ib);
      }
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
      if (op == BMethodLshift)
        return vuint(ia << ib);
      if (op == BMethodRshift)
        return vuint(ia >> ib);
      if (op == BMethodMod)
        return vuint(ia % ib);
      return vnull();
    }
    // numeric arithmetic
    else if (a->type == T_FLOAT || b->type == T_FLOAT) {
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
    } else {
      long ia = a->v.i, ib = b->v.i;
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
      if (op == BMethodLshift)
        return vint(ia << ib);
      if (op == BMethodRshift)
        return vint(ia >> ib);
      if (op == BMethodMod)
        return vint(ia % ib);
      return vnull();
    }
  }
  // equality for strings
  else if (BMethodEq == op) {
    if (!a && !b)
      return vbool(1);
    if (!a || !b)
      return vbool(0);
    if (a->type == T_STRING && b->type == T_STRING)
      return vbool(strcmp(a->v.s, b->v.s) == 0);
    // fallback pointer equality
    return vbool(a == b);
  } else if (BMethodNe == op) {
    Value *eq = binary_op(a, BMethodEq, b);
    int res = (eq->type == T_BOOL && eq->v.b == 0);
    val_release(eq);
    return vbool(res);
  }
  // string concatenation for '+'
  else if (op == BMethodAdd && a->type == T_STRING && b->type == T_STRING) {
    size_t la = strlen(a->v.s), lb = strlen(b->v.s);
    char *buf = mila_malloc(la + lb + 1);
    memcpy(buf, a->v.s, la);
    memcpy(buf + la, b->v.s, lb);
    buf[la + lb] = 0;
    return vstring_take(buf);
  } else if (op == BMethodAdd && a->type == T_STRING) {
    size_t la = strlen(a->v.s);
    char *stringyfied = as_c_string(b);
    if (stringyfied) {
      char *buf = mila_malloc(la + strlen(stringyfied) + 1);
      if (!buf)
        return vnull();
      strcpy(buf, a->v.s);
      strcat(buf, stringyfied);
      mila_free(stringyfied);
      return vstring_take(buf);
    }
    return vnull();
  } else if (op == BMethodAdd && b->type == T_STRING) {
    size_t la = strlen(b->v.s);
    char *stringyfied = as_c_string(a);
    if (stringyfied) {
      char *buf = mila_malloc(la + strlen(stringyfied) + 1);
      if (!buf)
        return vnull();
      strcpy(buf, stringyfied);
      strcat(buf, b->v.s);
      mila_free(stringyfied);
      return vstring_take(buf);
    }
    return vnull();
  } else if (b->type == T_STRING && a->type == T_STRING && op == BMethodGlob) {
    char *string, *pattern;
    string = GET_STRING(a);
    pattern = GET_STRING(b);
    if (match(pattern, string))
      return vbool(1);
    else
      return vbool(0);
  }
  return vnull();
}

Value *binary_op_in_place(Value *a, MethodType op, Value *b) {
  if (is_number(a) && is_number(b)) {
    if (a->type == T_UINT || b->type == T_UINT)
    // treat both numbers as unsigned.
    {
      unsigned long ia = a->v.ui, ib = b->v.ui;
      a->type = T_UINT;
      if (op == BMethodAdd)
        a->v.ui = ia + ib;
      else if (op == BMethodSub) {
        if (a->type == T_INT) {
          long v = a->v.i - b->v.i;
          a->v.ui = v > 0 ? v : -v;
        } else {
          a->v.ui = ia - ib;
        }
      } else if (op == BMethodMul)
        a->v.ui = ia * ib;
      else if (op == BMethodDiv)
        a->v.ui = ia / ib;
      else if (op == BMethodLshift)
        a->v.ui = ia << ib;
      else if (op == BMethodRshift)
        a->v.ui = ia >> ib;
      else if (op == BMethodMod)
        a->v.ui = ia % ib;
      return a;
    }
    // numeric arithmetic
    else if (a->type == T_FLOAT || b->type == T_FLOAT) {
      double ra = to_double(a), rb = to_double(b);
      a->type = T_FLOAT;
      if (op == BMethodAdd)
        a->v.f = ra + rb;
      else if (op == BMethodSub)
        a->v.f = ra - rb;
      else if (op == BMethodMul)
        a->v.f = ra * rb;
      else if (op == BMethodDiv)
        a->v.f = ra / rb;
      return a;
    } else {
      a->type = T_INT;
      long ia = a->v.i, ib = b->v.i;
      if (op == BMethodAdd)
        a->v.i = ia + ib;
      else if (op == BMethodSub)
        a->v.i = ia - ib;
      else if (op == BMethodMul)
        a->v.i = ia * ib;
      else if (op == BMethodDiv) {
        a->type = T_FLOAT;
        a->v.f = (double)ia / (double)ib;
      } else if (op == BMethodLshift)
        a->v.i = ia << ib;
      else if (op == BMethodRshift)
        a->v.i = ia >> ib;
      else if (op == BMethodMod)
        a->v.i = ia % ib;
      return a;
    }
  }
  return verror("Only numeric types can do in place operations!");
}

// evaluate expression with precedence climbing
int precedence_of(MethodType op) {
  if (BMethodAnd == op)
    return 1;
  if (BMethodOr == op)
    return 2;
  if (BMethodDefault == op)
    return 3;
  if (BMethodEq == op || BMethodNe == op)
    return 4;
  if (BMethodLshift == op || BMethodRshift == op)
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

MethodType parse_op(Src *s) {
  skip_ws(s);
  char a = src_peek(s);
  if (a == '\0')
    return -1;
  char b = s->src[s->pos + 1];
  // two-char ops
  if (a == '|' && b == '|') {
    s->pos += 2;
    return BMethodOr;
  }
  if (a == '&' && b == '&') {
    s->pos += 2;
    return BMethodAnd;
  }
  if (a == '=' && b == '=') {
    s->pos += 2;
    return BMethodEq;
  }
  if (a == '!' && b == '=') {
    s->pos += 2;
    return BMethodNe;
  }
  if (a == '<' && b == '=') {
    s->pos += 2;
    return BMethodLE;
  }
  if (a == '>' && b == '=') {
    s->pos += 2;
    return BMethodGE;
  }
  if (a == '>' && b == '>') {
    s->pos += 2;
    return BMethodRshift;
  }
  if (a == '<' && b == '<') {
    s->pos += 2;
    return BMethodLshift;
  }
  if (a == '?' && b == '?') {
    s->pos += 2;
    return BMethodDefault;
  }
  if (a == '=' && b == '>') {
    s->pos += 2;
    return BMethodGlob;
  }
  // single-char ops
  s->pos++;
  switch (a) {
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

Value *eval_expr_prec(Src *s, Env *env, int min_prec) {
  skip_ws(s);
  Value *lhs = eval_primary(s, env);
  if (!lhs)
    return verror("Invalid expression!");
  for (;;) {
    int saved_pos = s->pos;
    MethodType op = parse_op(s);
    if (op == MethodNone)
      return lhs;
    int prec = precedence_of(op);
    if (prec < min_prec) {
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

void clean_elif_chain(Src *s) {
  skip_ws(s);
  while (is_keyword_at(s, "elif")) {
    s->pos += strlen("elif");
    if (match_char(s, '('))
      skip_expr(s);
    match_char(s, ')');
    skip_block(s);
  }
  if (is_keyword_at(s, "else")) {
    s->pos += strlen("else");
    skip_block(s);
  }
}

Value *eval_statement(Src *s, Env *env) {
  skip_ws(s);
  if (is_keyword_at(s, "set")) {
    s->pos += strlen("set");
    char *id = parse_ident(s);
    if (!id)
      return verror("Invalid set statement.");
    Value *v = NULL;
    MethodType mt = MethodNone;
    skip_ws(s);

    // Check if this is a subscripted assignment
    if (src_peek(s) == '[') {
      // Handle nested subscripts: set a[x][y][z] = value
      Value *obj = env_get(env, id);

      if (!obj) {
        Value *ret = verror("%s cannot be subscripted as it is cnull", id);
        mila_free(id);
        return ret;
      }

      val_retain(obj);

      // Collect all subscript indices
      Value **indices = NULL;
      int num_indices = 0;

      while (src_peek(s) == '[') {
        Value *index = parse_subscript(s, env);
        indices = mila_realloc(indices, sizeof(Value *) * (num_indices + 1));
        indices[num_indices++] = index;
      }

      skip_ws(s);

      // Parse the assignment or statement
      switch (src_peek(s)) {
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
      if (__builtin_expect(!!match_char(s, '='), 1)) {
        v = eval_expr(s, env);
        if (mt != MethodNone) {
          // Traverse to the parent object (all but the last index)
          Value *parent = obj;
          for (int i = 0; i < num_indices - 1; i++) {
            if (parent->method_table && parent->method_table[BMethodGetItem]) {
              Value *res =
                  ((binary_method)parent->method_table[BMethodGetItem])(
                      parent, indices[i]);
              // val_release(parent);
              parent = res;

              if (!parent) {
                Value *ret = verror("cannot be subscripted at level %d", i + 1);
                for (int j = 0; j < num_indices; j++)
                  val_release(indices[j]);
                mila_free(indices);
                val_release(obj);
                val_release(v);
                mila_free(id);
                return ret;
              }
            } else {
              Value *ret =
                  verror("Type %s does not support subscripting at level %d!",
                         MILA_GET_TYPENAME(parent), i + 1);
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
          if (parent->method_table && parent->method_table[TMethodSetItem]) {
            Value *inplace =
                ((binary_method)parent->method_table[BMethodGetItem])(
                    parent, last_index);

            for (int i = 0; i < num_indices; i++)
              val_release(indices[i]);
            mila_free(indices);
            val_release(obj);
            if (inplace)
              binary_op_in_place(inplace, mt, v);
            val_release(v);
            mila_free(id);
            match_char(s, ';');
            return val_retain(inplace);
          } else {
            Value *ret = verror("Type %s does not support item assignment!",
                                MILA_GET_TYPENAME(parent));
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
      } else if (match_char(s, '=')) {
        v = eval_expr(s, env);
        match_char(s, ';');
      } else if (match_char(s, ':')) {
        v = eval_statement(s, env);
      } else {
        for (int i = 0; i < num_indices; i++)
          val_release(indices[i]);
        mila_free(indices);
        val_release(obj);
        mila_free(id);
        return verror("Expected = or : after subscripts!");
      }

      if (v && v->type == T_RETURN) {
        Value *tmp = v;
        v = (Value *)tmp->v.opaque;
        val_release(tmp);
      }

      // Traverse to the parent object (all but the last index)
      Value *parent = obj;
      for (int i = 0; i < num_indices - 1; i++) {
        if (parent->method_table && parent->method_table[BMethodGetItem]) {
          Value *res = ((binary_method)parent->method_table[BMethodGetItem])(
              parent, indices[i]);
          // val_release(parent);
          parent = res;

          if (!parent) {
            Value *ret = verror("cannot be subscripted at level %d", i + 1);
            for (int j = 0; j < num_indices; j++)
              val_release(indices[j]);
            mila_free(indices);
            val_release(obj);
            val_release(v);
            mila_free(id);
            return ret;
          }
        } else {
          Value *ret =
              verror("Type %s does not support subscripting at level %d!",
                     MILA_GET_TYPENAME(parent), i + 1);
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
      if (parent->method_table && parent->method_table[TMethodSetItem]) {
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
      } else {
        Value *ret = verror("Type %s does not support item assignment!",
                            MILA_GET_TYPENAME(parent));

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

    switch (src_peek(s)) {
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
    if (__builtin_expect(!!match_char(s, '='), 1)) {
      v = eval_expr(s, env);
      if (mt != MethodNone) {
        Value *inplace = env_get(env, id);
        if (!inplace) {
          Value *err = verror(
              "Variable %s doesnt exist and yet inplace operator was used!",
              id);
          mila_free(id);
          val_release(v);
          return err;
        }
        if (inplace)
          binary_op_in_place(inplace, mt, v);
        mila_free(id);
        val_release(v);
        match_char(s, ';');
        return val_retain(inplace);
      }
    } else if (match_char(s, ':')) {
      v = eval_statement(s, env);
    } else {
      return verror("Expected a proper set statement!");
    }

    if (v && v->type == T_RETURN) {
      Value *tmp = v;
      v = (Value *)tmp->v.opaque;
      val_release(tmp);
    } else if (v && v->type == T_FUNCTION) {
      v->v.fn->name = mila_strdup(id);
    }
    skip_ws(s);
    env_set(env, id, v);
    mila_free(id);
    return v;
  }
  if (is_keyword_at(s, "var")) {
    s->pos += strlen("var");
    char *id = parse_ident(s);
    // check for type hints
    if (match_char(s, ':')) {
      Value *type = parse_string(s);
      val_release(type); // ignore for now
    }
    if (!id)
      return verror("Invalid var statement.");
    if (match_char(s, ';')) {
      // declare none
      env_set_raw(env, id, vnone());
      mila_free(id);
      return vnull();
    }
    Value *v = NULL;
    if (match_char(s, '=')) {
      v = eval_expr(s, env);
      match_char(s, ';');
    } else if (match_char(s, ':')) {
      v = eval_statement(s, env);
    } else {
      return verror("Expected a proper var statement!");
    }

    if (v && v->type == T_RETURN) {
      Value *tmp = v;
      v = (Value *)tmp->v.opaque;
      val_release(tmp);
    } else if (v && v->type == T_FUNCTION) {
      v->v.fn->name = mila_strdup(id);
    }
    env_set_local(env, id, v);
    mila_free(id);
    return v;
  }
  if (is_keyword_at(s, "contextual")) {
    s->pos += strlen("contextual");
    char *id = parse_ident(s);
    if (!id)
      return verror("Invalid contextual statement.");
    Value *a = env_get(env, id);
    if (!a) {
      Value *res = verror(
          "Variable `%s` cannot become contextual as it doesnt exist!", id);
      mila_free(id);
      return res;
    }
    if (is_keyword_at(s, "as")) {
      s->pos += 2;
      char *alias = parse_ident(s);
      if (!alias) {
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
  if (is_keyword_at(s, "sync")) {
    s->pos += strlen("sync");
    char *id = parse_ident(s);
    if (!id)
      return verror("Invalid sync statement.");
    Value *a = env_get(env, id);
    if (!a) {
      Value *res =
          verror("Variable `%s` cannot be synced as it doesnt exist!", id);
      mila_free(id);
      return res;
    }
    Value *val = NULL;
    if (match_char(s, '=')) {
      val = eval_expr(s, env);
      match_char(s, ';');
    }
    val_kill_incomplete(a);
    a->type = val->type;
    switch (MILA_GET_TYPE(val)) {
    case T_INT:
      a->v.i = GET_INTEGER(val);
      break;
    case T_UINT:
      a->v.ui = GET_UINTEGER(val);
      break;
    case T_OWNED_OPAQUE:
    case T_OPAQUE:
      a->v.opaque = GET_OPAQUE(val);
      break;
    case T_STRING:
      a->v.s = GET_STRING(val);
      break;
    case T_FUNCTION:
      a->v.fn = val->v.fn;
      break;
    default:
      return vtagged_error(E_FATAL, "Type %s cannot be synced!",
                           MILA_GET_TYPENAME(val));
    }
    mila_free(val);
    mila_free(id);
    return vnull();
  }
  if (is_keyword_at(s, "export")) {
    s->pos += strlen("export");
    char *id = parse_ident(s);
    if (!id)
      return verror("Invalid export statement.");
    Value *v = NULL;
    if (match_char(s, '=')) {
      v = eval_expr(s, env);
      match_char(s, ';');
    } else if (match_char(s, ':')) {
      v = eval_statement(s, env);
    } else {
      return verror("Expected a proper export statement!");
    }

    if (v && v->type == T_RETURN) {
      Value *tmp = v;
      v = (Value *)tmp->v.opaque;
      val_release(tmp);
    } else if (v && v->type == T_FUNCTION) {
      v->v.fn->name = mila_strdup(id);
    }
    if (env->parent) {
      env_set(env->parent, id, v);
      env_set_local(env, id, v);
    } else {
      env_set_local(env, id, v);
      fprintf(stderr, "= Warning: %s not binded to an outer scope!\n", id);
    }
    mila_free(id);
    return v;
  }
  if (is_keyword_at(s, "forget")) {
    s->pos += strlen("forget");
    skip_ws(s);
    if (src_peek(s) == '[') {
      char **names = parse_context_list(s);
      for (int i = 0; names[i]; ++i) {
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

    skip_ws(s);
    match_char(s, ';');
    return vnull();
  }
  if (is_keyword_at(s, "return")) {
    s->pos += strlen("return");
    Value *v = eval_expr(s, env);
    match_char(s, ';');
    // wrap as return value: create T_RETURN whose opaque pointer contains the
    // actual Value*
    Value *r = val_new(T_RETURN);
    r->v.opaque = v;
    return r;
  }
  if (is_keyword_at(s, "if")) {
    s->pos += strlen("if");
    skip_ws(s);
    if (match_char(s, '(')) {
      Value *cond = eval_expr(s, env);
      match_char(s, ')');
      int truth = is_truthy(cond);
      val_release(cond);
      skip_ws(s);
      if (truth) {
        Value *res = NULL;
        res = eval_block_raw(s, env);
        clean_elif_chain(s);
        HANDLE_CONTROL(res);
      } else {
        // skip then clause
        skip_block(s);
        // check elifs
        skip_ws(s);
        while (is_keyword_at(s, "elif")) {
          s->pos += strlen("elif");
          if (match_char(s, '(')) {
            Value *cond = eval_expr(s, env);
            match_char(s, ')');
            if (is_truthy(cond)) {
              Value *res = NULL;
              res = eval_block_raw(s, env);

              clean_elif_chain(s);
              val_release(cond);
              HANDLE_CONTROL(res);
            } else {
              // skip elif then clause
              val_release(cond);
              skip_block(s);
            }
          }
          skip_ws(s);
        }
        // run else if it exists
        if (is_keyword_at(s, "else")) {
          s->pos += strlen("else");
          Value *res = NULL;
          skip_ws(s);
          res = eval_block_raw(s, env);
          // check for return propagation
          HANDLE_CONTROL(res);
        }
      }
    }
    return vnull();
  }
  if (is_keyword_at(s, "while")) {
    s->pos += strlen("while");
    if (match_char(s, '(')) {
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
      while (1) {
        // Reset position to condition start for re-evaluation
        s->pos = cond_start_pos;

        // Re-evaluate condition
        Value *cond = eval_expr(s, env);
        match_char(s, ')');
        if (IS_ERROR(cond)) {
          val_release(bod);
          return cond;
        } else if (!is_truthy(cond)) {
          val_release(cond);
          if (MILA_GET_TYPE(bod) == T_RETURN) {
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
        if (bod->type == T_BREAK) {
          s->pos = body_end_pos;
          val_release(bod);
          return vnull();
        } else if (bod->type == T_CONTINUE) {
          s->pos = cond_start_pos;
          continue;
        } else if (bod->type == T_RETURN) {
          s->pos = body_end_pos;
          return bod;
        } else if (IS_ERROR(bod)) {
          s->pos = body_end_pos;
          return bod;
        }
      }
      return bod;
    }
    return verror("While loops condition must be wrapped in parenthesis!");
  }
  if (is_keyword_at(s, "foreach")) {
    s->pos += strlen("foreach");
    skip_ws(s);
    char *id = parse_ident(s);
    if (id) {
      if (!match_char(s, ':')) {
        mila_free(id);
        return verror("Foreach lacking ':'");
      }
      Value *iter_obj = eval_expr(s, env);

      if (IS_ERROR(iter_obj)) {
        mila_free(id);
        return iter_obj;
      }

      Value **value = NULL;

      if (iter_obj->method_table && iter_obj->method_table[UMethodToIter]) {
        Value *v =
            ((unary_method)iter_obj->method_table[UMethodToIter])(iter_obj);
        if (IS_ERROR(v)) {
          mila_free(id);
          return v;
        }
        if (!v) {
          mila_free(id);
          return verror("Iterable is cnull!");
        }
        value = v->v.opaque;
        if (!value) {
          mila_free(id);
          return verror("Value returned null!");
        }
        val_kill(v);
      } else {
        mila_free(id);
        Value *err = verror("Type %s does not implement UMethodToIter",
                            MILA_GET_TYPENAME(iter_obj));
        val_release(iter_obj);
        return err;
      }
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
      for (; v; v = value[i++]) {
        // reset the position to the start of the body for execution
        s->pos = body_start_pos;
        Env *frame = env_new(env);
        env_set_local_raw(frame, id, v);
        bod = eval_block_raw(s, frame);
        val_release(v);
        env_remove(frame, id);

        env_free(frame);
        // --- Handle body result ---
        if (bod) {
          if (bod->type == T_BREAK) {
            s->pos = body_end_pos;
            val_release(bod);
            mila_free(value);
            mila_free(id);
            return vnull();
          } else if (bod->type == T_CONTINUE) {
            s->pos = body_start_pos;
            if (bod)
              val_release(bod);
            continue;
          } else if (IS_ERROR(bod)) {
            s->pos = body_end_pos;
            return bod;
          } else if (bod->type == T_RETURN) {
            s->pos = body_end_pos;
            return bod;
          } else if (bod->type == T_ERROR) {
            s->pos = body_end_pos;
            return bod;
          }
        }

        val_release(bod);
      }
      s->pos = body_end_pos;
      mila_free(id);
      mila_free(value);
      return vnull();
    }
  }
  if (is_keyword_at(s, "block")) {
    s->pos += strlen("block");
    skip_ws(s);
    char *name = parse_ident(s);
    if (!name)
      return verror("Block needs a name!");
    skip_ws(s);
    Value *res = eval_block(s, env);
    if (IS_ERROR(res)) {
      Value *new_res =
          verror("Block %s reported an error: %s", name, res->v.message);
      val_release(res);
      mila_free(name);
      return new_res;
    }
    mila_free(name);
    return res;
  }
  if (is_keyword_at(s, "namespace")) {
    s->pos += strlen("namespace");
    skip_ws(s);
    char *old_name = s->cur_namespace;
    s->cur_namespace = parse_ident(s);
    if (!s->cur_namespace) {
      s->cur_namespace = old_name;
      return verror("Namespace needs a name!");
    }
    skip_ws(s);
    Value *res = eval_block_raw(s, env);
    mila_free(s->cur_namespace);
    s->cur_namespace = old_name;
    return res;
  }
  if (is_keyword_at(s, "catch")) {
    s->pos += strlen("catch");
    char *id = parse_ident(s);
    size_t start = s->pos;
    skip_block(s);
    size_t end = s->pos;
    s->pos = start;
    Value *res = eval_block_raw(s, env);
    if (IS_ERROR(res) && !IS_FATAL(res)) {
      if (id) {
        if (MILA_GET_TYPE(res) == T_ERROR) {
          Value *ret = vstring_fmt("%s", res->v.message);
          env_set_local(env, id, ret);
          val_release(res);
          mila_free(id);
          s->pos = end;
          return ret;
        } else {
          Value *ret =
              vstring_fmt("%s[%i]: %s", MILA_GET_ERRORNAME(res),
                          MILA_GET_ERROR(res), res->v.tagged_error.message);
          val_release(res);
          env_set_local(env, id, ret);
          mila_free(id);
          s->pos = end;
          return ret;
        }
      }
      return vnull();
    }
    if (IS_FATAL(res)) {
      fprintf(stderr, "\n\nTried to catch fatal error.\n");
      fprintf(stderr, "FATAL ERROR[%s]: %s\n", MILA_GET_ERRORNAME(res),
              GET_ERROR_TAGGED(res));
      abort();
    }
    s->pos = end;
    mila_free(id);
    return res;
  }
  if (is_keyword_at(s, "fn")) {
    // consume keyword
    s->pos += strlen("fn");
    char *name = parse_ident(s);
    if (!name)
      return verror("Function needs a name!");
    char **params = parse_param_list(s);
    char **contextuals = parse_context_list(s);
    char **names;
    Env *closure = env_new(NULL);
    if (match_char(s, ':')) {
      // parse closure bindings
      names = parse_context_list(s);
      for (int i = 0; names[i]; ++i) {
        env_set_local(closure, names[i], env_get(env, names[i]));
        mila_free(names[i]);
      }
      mila_free(names);
    }
    if (is_keyword_at(s, "->")) {
      s->pos += 2;
      skip_ws(s);
      if (src_peek(s) == '"') {
        Value *ret_type = parse_string(s);
        // ignore return type for now
        val_kill(ret_type);
      } else {
        env_free(closure);
        for (int i = 0; params[i]; ++i) {
          mila_free(params[i]);
        }
        mila_free(params);
        mila_free(name);
        return vtagged_error(E_SYNTAX_ERROR,
                             "Expected a string literal for the return type.");
      }
    }
    skip_ws(s);
    // body is block; extract substring from '{' to matching '}'
    if (src_peek(s) != '{') {
      env_free(closure);
      for (int i = 0; params[i]; ++i) {
        mila_free(params[i]);
      }
      mila_free(params);
      mila_free(name);
      // error: expected body
      return verror("Body wasnt found.");
    }
    // find matching brace (we will copy out body)
    int depth = 0;
    int start = s->pos;
    int i = s->pos;
    for (; i < s->len; ++i) {
      char ch = s->src[i];
      if (ch == '{')
        depth++;
      else if (ch == '}') {
        depth--;
        if (depth == 0) {
          i++;
          break;
        }
      } else if (ch == '"') {
        // skip string literal
        i++;
        while (i < s->len && s->src[i] != '"') {
          if (s->src[i] == '\\' && i + 1 < s->len)
            i += 2;
          else
            i++;
        }
      }
    }
    if (i > s->len)
      i = s->len;
    int blen = i - start;
    char *body = mila_malloc(blen + 1);
    memcpy(body, s->src + start, blen);
    body[blen] = 0;
    s->pos = i;
    // create function value with closure get_line_pos(s) current env
    Value *fn = vfunction(params, contextuals, closure, body);
    fn->v.fn->name = mila_strdup(name);
    env_set_local(env, name, fn);
    mila_free(name);
    return fn;
  }
  if (is_keyword_at(s, "object")) {
    s->pos += strlen("object");
    char *name = parse_ident(s);
    if (!name) {
      return vtagged_error(E_SYNTAX_ERROR, "Expected object to have a name!");
    }
    Value *obj;
    if (!is_keyword_at(s, "with"))
      obj = call_function_str(env, "dict", NULL);
    else {
      s->pos += strlen("with");
      char *obj_name = parse_ident(s);
      if (!name) {
        return vtagged_error(E_SYNTAX_ERROR, "Expected a name after `from`");
      }
      obj = call_function_str(env, "dict", NULL);
      Value *with_obj = env_get(env, obj_name);
      if (!obj) {
        Value *res = vtagged_error(
            E_RUNTIME,
            "Cannot build on top of variable `%s` as it doesnt exist!",
            obj_name);
        mila_free(obj_name);
        mila_free(name);
        return res;
      }

      KVPair *entries = NULL;
      size_t count = 0, capacity = 16;
      entries = (KVPair *)mila_malloc(capacity * sizeof(KVPair));
      if (!entries)
        return NULL;

      // Collect all entries
      for (size_t i = 0; i < ((Dict *)with_obj->v.opaque)->capacity; i++) {
        DictEntry *entry = ((Dict *)with_obj->v.opaque)->buckets[i];
        while (entry) {
          if (count >= capacity) {
            capacity *= 2;
            KVPair *tmp = (KVPair *)realloc(entries, capacity * sizeof(KVPair));
            if (!tmp) {
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

      for (size_t i = count; i > 0; i--) {
        dict_set_raw((Dict *)obj->v.opaque, entries[i - 1].key,
                     entries[i - 1].value);
      }
      mila_free(entries);
      mila_free(obj_name);
    }
    Env *class_env = env_new(env);
    Value *res = eval_block_raw(s, class_env);
    if (IS_ERROR(res)) {
      env_free(class_env);
      mila_free(name);
      val_release(obj);
      return res;
    } else
      val_release(res);

    for (Var *v = class_env->vars; v; v = v->next) {
      Value *name = vstring_dup(v->name);
      dict_set((Dict *)obj->v.opaque, name, v->value);
      val_release(name);
    }

    env_set_local_raw(env, name, obj);
    mila_free(name);
    env_free(class_env);
    return val_retain(obj);
  }
  skip_ws(s);
  // block
  if (src_peek(s) == '{') {
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

int match_types(Value **args, ...) {
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

// vfunction creation
Value *vfunction(char **params, char **contextuals, Env *closure,
                 char *body_src) {
  Value *v = val_new(T_FUNCTION);
  v->v.fn = (FunctionV *)mila_malloc(sizeof(FunctionV));
  v->v.fn->params = params;
  v->v.fn->contextuals = contextuals;
  v->v.fn->body_src = body_src;
  v->v.fn->closure = closure;
  return v;
}

// top-level eval of source - runs sequential statements in global env
Value *eval_source(Src *s, Env *env) {
  Value *last = vnull();
  while (!src_eof(s)) {
    if (src_eof(s))
      break;
    Value *st = eval_statement(s, env);

    val_release(last);
    last = st;
    if (last) {
      if (last->type == T_ERROR) {
        printf("\n= Error: %s\n", last->v.message);
        return last;
      }
      if (last->type == T_TAGGED_ERROR) {
        if (IS_FATAL(last))
          printf("\n= FATAL ERROR[%s]: %s\n", MILA_GET_ERRORNAME(last),
                 last->v.tagged_error.message);
        else
          printf("\n= Error[%s]: %s\n", MILA_GET_ERRORNAME(last),
                 last->v.tagged_error.message);
        return last;
      } else if (last->type == T_RETURN) {
        Value *res = (Value *)last->v.opaque;
        val_release(last);
        return res;
      }
    }
  }
  return last;
}

Value *eval_str(char *src, Env *env) {
  Src *S = src_new(src);
  Value *res = eval_source(S, env);
  src_free(S);
  return res;
}

int run_file(char *name, Env *env) {
  char out_pwd[MAX_PATH_LENGTH] = {0};
  char out[MAX_PATH_LENGTH] = {0};
  path_dirname(name, out, sizeof(out));
  path_list_add(search_path, out);

  char basename[MAX_PATH_LENGTH] = {0};
  path_basename(name, basename, sizeof(basename));
  char *src_text = NULL;
  FILE *f = fopen(name, "rb");
  if (!f) {
    fprintf(stderr, "Cannot open %s\n", name);
    return 1;
  }
  env_set_local_raw(env, "__name__", vstring_dup(basename));
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
  return 0;
}

Value *run_file_keep_res(char *name, Env *env) {
  char out_pwd[MAX_PATH_LENGTH] = {0};
  char out[MAX_PATH_LENGTH] = {0};
  path_dirname(name, out, sizeof(out));
  path_list_add(search_path, out);

  char basename[MAX_PATH_LENGTH] = {0};
  path_basename(name, basename, sizeof(basename));
  char *src_text = NULL;
  FILE *f = fopen(name, "rb");
  if (!f) {
    return verror("Cannot open %s\n", name);
  }
  env_set_local_raw(env, "__name__", vstring_dup(basename));
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
  return res;
}

int needs_more(const char *src) {
  int parens = 0, braces = 0;
  int in_string = 0;

  for (const char *p = src; *p; p++) {
    if (*p == '"' && (p == src || *(p - 1) != '\\')) {
      in_string = !in_string; // toggle string state
    } else if (!in_string) {
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

Env *mila_init(void) {
  Env *g = env_new(NULL);
  env_register_builtins(g);
  return g;
}

void mila_deinit(Env *g) {
  // we might handle stuff here

  env_free(g);
}

#if !(defined(VMM_BUILD) || defined(ML_LIB))
int main(int argc, char **argv) {
  // read file if provided or use built-in demo
  char *src_text = NULL;
  if (argc == 2) {
    if (strcmp(argv[1], "--info") == 0) {
      printf("MiLa - Info\n"
             "Version: 1.0\n\n"
             "Variable size:\n"
             "  %lu Bytes\n"
             "  %lu Bytes for worst case (boxed value is not counted)\n"
             "Estimated memory:\n"
             "  t * %lu + n * 40 Bytes\n"
             "  n = # of vars\n"
             "  t = # of types\n"
             "Max num digits:\n"
             "  %i\n",
             sizeof(Value),
             sizeof(Value) + sizeof(MethodTable) * MethodTotalCount,
             sizeof(MethodTable) * MethodTotalCount, MAX_NUMBER_DIGITS);
      return 0;
    } else if (strcmp(argv[1], "--version") == 0 ||
               strcmp(argv[1], "-v") == 0) {
      printf("MiLa Specification v1.0\n"
             "CLI v1.0\n"
             "API v1.0\n");
      return 0;
    } else if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
      printf("MiLa v1.0\n"
             "  --info         = For internal info as well as version info\n"
             "  --version | -v = Prints version\n"
             "  --help    | -h = Prints this list\n");
      return 0;
    }
  }
  Value *array = NULL;

  Env *g = mila_init();

  // Check if built ins is the canonical
  Value *builtins_flag = env_get(g, "__mila_canonical_builtins");
  int is_builtins = builtins_flag != NULL && builtins_flag->type == T_INT &&
                    builtins_flag->v.i == 202603L;

  search_path = path_list_new();
  char *cwd = path_get_cwd();
  if (!cwd)
    fprintf(stderr, "current working directory was not determined.");
  else
    path_list_add(search_path, cwd);

  path_list_add(search_path, "~/.local/lib/mila");

  char out_pwd[MAX_PATH_LENGTH] = {0};
  char out[MAX_PATH_LENGTH] = {0};
  path_dirname(argv[1], out, sizeof(out));
  path_join(out_pwd, sizeof(out_pwd), 2, cwd, out);
  path_list_add(search_path, out_pwd);
  mila_free(cwd);

  if (argc >= 2 && strcmp(argv[1], "--") != 0) {
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
      fprintf(stderr, "Cannot open %s: Missing or not a file.\n", argv[1]);
      env_kill(g);
      return 1;
    }

    // argv handling is the only part that touches the builtins.

    // make sure we are using the bundled canonical builtins
    // otherwise set argv as __argv with the type opaque
    if (is_builtins) {
      array = call_function_str(g, "array", vint(argc - 1), NULL);
      for (int i = 1; i < argc; i++) {
        Value *str = vstring_dup(argv[i]);
        val_release(call_function_str(g, "array.set", val_retain(array),
                                      vint(i - 1), str, NULL));
      }
      env_set_raw(g, "argv", array);
    } else {
      env_set_raw(g, "__argc", vint(argc));
      env_set_raw(g, "__argv", vopaque(argv));
      env_set_raw(g, "argv", vnone());
    }

    env_set_local_raw(g, "__name__", vstring_dup("__main__"));

    // read file
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    src_text = mila_malloc(size + 1);
    fread(src_text, 1, size, f);
    src_text[size] = 0;
    fclose(f);

    Src *S = src_new(src_text);
    Value *res = eval_source(S, g);

    path_list_free(search_path);

    // cleanup
    val_release(res);
    src_free(S);

    mila_free(src_text);

    mila_deinit(g);
    env_free_builtins();
  } else {
    if (is_builtins && argc > 1 && strcmp(argv[1], "--") == 0) {
      array = call_function_str(g, "array", vint(argc - 2), NULL);
      for (int i = 2; i < argc; i++) {
        Value *str = vstring_dup(argv[i]);
        val_release(call_function_str(g, "array.set", val_retain(array),
                                      vint(i - 2), str, NULL));
      }
      env_set_raw(g, "argv", array);
    }

    printf("MiLa REPL\n");
    printf("Running MiLa '%s'\n", env_get(g, "__mila_codename")
                                      ? env_get(g, "__mila_codename")->v.s
                                      : "???");

    // Notify users when MiLa is built using the canonical builtins
    if (is_builtins) {
      printf("Cannonical Builtins (%ld) version %ld\n",
             env_get(g, "__mila_canonical_builtins")->v.i,
             env_get(g, "__mila_canonical_builtins_version")->v.i);
    }

    char line[256];
    char buffer[2048]; // accumulated snippet
    buffer[0] = 0;

    printf(">>> ");
    fflush(stdout);

    while (fgets(line, sizeof(line), stdin)) {
      // append line to buffer
      strcat(buffer, line);
      fflush(stdout);

      // debugging
      if (strncmp(buffer, ".mempro", 7) == 0) {
        FILE *f = fopen("/proc/self/status", "r");
        char line[256];

        while (fgets(line, sizeof(line), f)) {
          if (strncmp(line, "VmRSS:", 6) == 0 ||
              strncmp(line, "VmData:", 7) == 0 ||
              strncmp(line, "VmStack:", 8) == 0 ||
              strncmp(line, "VmExe:", 6) == 0 ||
              strncmp(line, "VmStk:", 6) == 0) {
            printf("%s", line);
          }
        }
        fclose(f);
        printf(">>> ");
        fflush(stdout);
        buffer[0] = 0;
        continue;
      } else if (strncmp(buffer, ".load", 5) == 0) {
        *strchr(buffer, '\n') = 0;
        if (load_library_noisy(g, buffer + 6))
          printf("Library loading went wrong!\n");
        printf(">>> ");
        buffer[0] = 0;
        continue;
      } else if (strncmp(buffer, ".mem", 4) == 0) {
        print_memory_usage();
        buffer[0] = 0;
        printf(">>> ");
        fflush(stdout);
        continue;
      } else if (strncmp(buffer, ".quit", 4) == 0) {
        break;
      }

      // check if expression is syntactically complete
      if (!needs_more(buffer)) {
        // evaluate accumulated buffer
        Src *S = src_new(buffer);
        Value *res = eval_source(S, g);

        if (MILA_GET_TYPE(res) != T_NULL && !IS_ERROR(res)) {
          printf("  : ");
          print_value_repr(res);
          putchar('\n');
        }

        val_release(res);
        src_free(S);

        // clear buffer
        buffer[0] = 0;

        printf(">>> ");
        fflush(stdout);
      } else {
        // prompt for continuation
        printf("... ");
        fflush(stdout);
      }
    }

    mila_deinit(g);
  }
  return 0;
}

#else

int repr() {
  // read file if provided or use built-in demo
  char *src_text = NULL;

  // prepare global env
  // register native functions
  Env *g = mila_init();

  // Check if built ins is the canonical
  Value *builtins_flag = env_get(g, "__mila_canonical_builtins");
  int is_builtins = builtins_flag != NULL && builtins_flag->type == T_INT &&
                    builtins_flag->v.i == 202603L;

  printf("MiLa REPL\n");
  printf("Running MiLa '%s'\n", env_get(g, "__mila_codename")
                                    ? env_get(g, "__mila_codename")->v.s
                                    : "???");

  // Notify users when MiLa is built using the canonical builtins
  if (is_builtins) {
    printf("Cannonical Builtins (%ld) version %ld\n",
           env_get(g, "__mila_canonical_builtins")->v.i,
           env_get(g, "__mila_canonical_builtins_version")->v.i);
  }

  char line[256];
  char buffer[2048]; // accumulated snippet
  buffer[0] = 0;

  printf(">>> ");
  fflush(stdout);

  while (fgets(line, sizeof(line), stdin)) {
    // append line to buffer
    strcat(buffer, line);
    fflush(stdout);

    // debugging
    if (strncmp(buffer, ".mempro", 7) == 0) {
      FILE *f = fopen("/proc/self/status", "r");
      char line[256];

      while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "VmRSS:", 6) == 0 ||
            strncmp(line, "VmData:", 7) == 0 ||
            strncmp(line, "VmStack:", 8) == 0 ||
            strncmp(line, "VmExe:", 6) == 0 ||
            strncmp(line, "VmStk:", 6) == 0) {
          printf("%s", line);
        }
      }
      fclose(f);
      printf(">>> ");
      fflush(stdout);
      buffer[0] = 0;
      continue;
    } else if (strncmp(buffer, ".mem", 4) == 0) {
      print_memory_usage();
      buffer[0] = 0;
      printf(">>> ");
      fflush(stdout);
      continue;
    } else if (strncmp(buffer, ".quit", 4) == 0) {
      break;
    }

    // check if expression is syntactically complete
    if (!needs_more(buffer)) {
      // evaluate accumulated buffer
      Src *S = src_new(buffer);
      Value *res = eval_source(S, g);

      if (MILA_GET_TYPE(res) != T_NULL && !IS_ERROR(res)) {
        printf("  : ");
        print_value_repr(res);
        putchar('\n');
      }

      val_release(res);
      src_free(S);

      // clear buffer
      buffer[0] = 0;

      printf(">>> ");
      fflush(stdout);
    } else {
      // prompt for continuation
      printf("... ");
      fflush(stdout);
    }
  }

  mila_deinit(g);
  return 0;
}

#endif // VMM_BUILD