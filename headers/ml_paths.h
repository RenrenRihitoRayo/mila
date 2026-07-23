// This project is licensed under the GNU Affero General Public License
#pragma once
#include <stddef.h>

typedef struct path_list path_list;
struct path_list {
  char **items;
  int count;
  int capacity;
};


int file_exists(const char *p);
void normalize_slashes(char *buf);
const char *get_env(const char *name, size_t len);
void expand_env(char **bufptr);
void path_join(char *out, size_t outsize, int count, ...);
void expand_home(char **bufptr);
char *transform_path(const char *input);
path_list *path_list_new(void);
void path_dirname(const char *path, char *out, size_t outsize);
void path_basename(const char *path, char *out, size_t outsize);
char* path_dirname_alloc(const char *path);
char* path_basename_alloc(const char *path);
char* path_basename_id_alloc(const char *path);
void path_list_free(path_list *pl);
int path_list_add(path_list *pl, const char *path);
int path_list_remove(path_list *pl, const char *path);
char *path_list_find(path_list *pl, const char *file);
char *path_list_find_alternative(path_list *pl, const char *file);
char *path_get_cwd(void);
char* path_join_alloc(char* path, ...);
int path_list_add_top(path_list *pl, const char *path);