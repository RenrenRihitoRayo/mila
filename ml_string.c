// This project is licensed under the GNU Affero General Public License
#pragma once

#include <string.h>
void *mila_malloc(size_t);

#ifndef MILA_PROTO
char *mila_strdup(const char *s) {
  if (!s)
    return NULL;

  size_t len = strlen(s);
  char *out = (char *)mila_malloc(len + 1);
  if (!out)
    return NULL;

  for (size_t i = 0; i <= len; i++) {
    out[i] = s[i];
  }
  return out;
}

char *mila_strndup(const char *s, size_t n) {
  if (!s)
    return NULL;

  size_t len = 0;
  while (len < n && s[len])
    len++;

  char *out = (char *)mila_malloc(len + 1);
  if (!out)
    return NULL;

  for (size_t i = 0; i < len; i++) {
    out[i] = s[i];
  }
  out[len] = '\0';

  return out;
}

char *mila_strcat_alloc(const char *a, const char *b) {
  if (!a || !b)
    return NULL;

  size_t len_a = strlen(a);
  size_t len_b = strlen(b);

  char *out = (char *)mila_malloc(len_a + len_b + 1);
  if (!out)
    return NULL;

  for (size_t i = 0; i < len_a; i++) {
    out[i] = a[i];
  }
  for (size_t i = 0; i < len_b; i++) {
    out[len_a + i] = b[i];
  }

  out[len_a + len_b] = '\0';
  return out;
}

#else

char *mila_strdup(const char *s);
char *mila_strndup(const char *s, size_t n);
char *mila_strcat_alloc(const char *a, const char *b);
char *mila_strtok(char *str, const char *delim);

#endif