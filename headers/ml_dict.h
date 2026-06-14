// This project is licensed under the GNU Affero General Public License
#pragma once
#include "../mila.h"
#include <stddef.h>

typedef struct DictEntry {
  ValueType key_type;
  char *key;
  Value *value;
  struct DictEntry *next;
} DictEntry;

typedef struct {
  DictEntry **buckets;
  size_t capacity;
  size_t size;
} Dict;

typedef struct {
  char *key;
  Value *value;
} KVPair;

#define INITIAL_CAPACITY 16
#define LOAD_FACTOR 0.75

void hash_set_seed(unsigned long seed);
static unsigned long hash_string(const char *str);
FN_UNUSED static unsigned long hash_value(Value *val);
static DictEntry *dict_entry_create(const char *key, Value *value);
static void dict_entry_free(DictEntry *entry);
Dict *dict_create();
static void dict_resize(Dict *dict);
int dict_set(Dict *dict, Value *key, Value *value);
int dict_set_raw(Dict *dict, char *key, Value *value);
Value *dict_get_str(Dict *dict, const char *key);
int dict_set_str(Dict *dict, char *str_key, Value *value);
Value *dict_get(Dict *dict, Value *key);
int dict_remove(Dict *dict, Value *key);
void dict_free(Dict *dict);
Value *dict_display(Value *self);
Value *dict_copy(Value *self);
Value** dict_keys(Dict* dict);
void dict_free_keys(char** entries);