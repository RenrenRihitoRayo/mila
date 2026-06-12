// This project is licensed under the GNU Affero General Public License
#pragma once
#include <stddef.h>

char *mila_strdup(const char *s);
char *mila_strndup(const char *s, size_t n);
char *mila_strcat_alloc(const char *a, const char *b);
char *mila_strtok(char *str, const char *delim);