// This project is licensed under the GNU Affero General Public License
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ml_string.c"

#ifdef ML_LIB
#define ML_ALREADY
#endif

#define ML_LIB
#include "mila.h"

#ifndef ML_ALREADY
#undef ML_LIB
#endif

typedef struct DictEntry {
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

// Simple djb2 hash function
static unsigned long hash_string(const char *str) {
  unsigned long hash = 5381;
  int c;
  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;
  return hash;
}

FN_UNUSED static unsigned long hash_value(Value *val) {
  char *og = as_c_string_repr(val);
  char *str = og;
  unsigned long hash = 5381;
  int c;
  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;
  mila_free(og);
  return hash;
}

static DictEntry *dict_entry_create(const char *key, Value *value) {
  DictEntry *entry = (DictEntry *)mila_malloc(sizeof(DictEntry));
  if (!entry)
    return NULL;
  entry->key = mila_strdup(key);
  entry->value = val_retain(value);
  entry->next = NULL;
  return entry;
}

static void dict_entry_free(DictEntry *entry) {
  if (!entry)
    return;
  mila_free(entry->key);
  val_release(entry->value);
  mila_free(entry);
}

Dict *dict_create() {
  Dict *dict = (Dict *)mila_malloc(sizeof(Dict));
  if (!dict)
    return NULL;
  dict->capacity = INITIAL_CAPACITY;
  dict->size = 0;
  dict->buckets =
      (DictEntry **)mila_malloc(dict->capacity * sizeof(DictEntry *));
  if (!dict->buckets) {
    mila_free(dict);
    return NULL;
  }
  return dict;
}

static void dict_resize(Dict *dict) {
  size_t new_capacity = dict->capacity * 2;
  DictEntry **new_buckets =
      (DictEntry **)mila_malloc(new_capacity * sizeof(DictEntry *));
  if (!new_buckets)
    return;

  for (size_t i = 0; i < dict->capacity; i++) {
    DictEntry *entry = dict->buckets[i];
    while (entry) {
      DictEntry *next = entry->next;
      unsigned long hash = hash_string(entry->key) % new_capacity;
      entry->next = new_buckets[hash];
      new_buckets[hash] = entry;
      entry = next;
    }
  }

  mila_free(dict->buckets);
  dict->buckets = new_buckets;
  dict->capacity = new_capacity;
}

int dict_set(Dict *dict, Value *key, Value *value) {
  if (!dict || !key)
    return 0;

  if ((double)dict->size / dict->capacity > LOAD_FACTOR) {
    dict_resize(dict);
  }

  char *key_str = as_c_string_repr(key);

  unsigned long index = hash_string(key_str) % dict->capacity;
  DictEntry *entry = dict->buckets[index];

  while (entry) {
    if (strcmp(entry->key, key_str) == 0) {
      val_release(entry->value);
      entry->value = val_retain(value);
      mila_free(key_str);
      return 1; // updated existing
    }
    entry = entry->next;
  }

  DictEntry *new_entry = dict_entry_create(key_str, value);
  if (!new_entry) {
    mila_free(key_str);
    return 0;
  }
  new_entry->next = dict->buckets[index];
  dict->buckets[index] = new_entry;
  dict->size++;
  mila_free(key_str);
  return 1; // new insertion
}

int dict_set_raw(Dict *dict, char *key, Value *value) {
  if (!dict || !key)
    return 0;

  if ((double)dict->size / dict->capacity > LOAD_FACTOR) {
    dict_resize(dict);
  }

  unsigned long index = hash_string(key) % dict->capacity;
  DictEntry *entry = dict->buckets[index];

  while (entry) {
    if (strcmp(entry->key, key) == 0) {
      val_release(entry->value);
      entry->value = val_retain(value);
      return 1; // updated existing
    }
    entry = entry->next;
  }

  DictEntry *new_entry = dict_entry_create(key, value);
  if (!new_entry) {
    mila_free(key);
    return 0;
  }
  new_entry->next = dict->buckets[index];
  dict->buckets[index] = new_entry;
  dict->size++;
  return 1; // new insertion
}

Value *dict_get(Dict *dict, Value *key) {
  if (!dict || !key)
    return NULL;
  char *key_str = as_c_string_repr(key);
  unsigned long index = hash_string(key_str) % dict->capacity;
  DictEntry *entry = dict->buckets[index];
  while (entry) {
    if (strcmp(entry->key, key_str) == 0) {
      mila_free(key_str);
      return entry->value;
    }
    entry = entry->next;
  }
  mila_free(key_str);
  return NULL;
}

int dict_remove(Dict *dict, Value *key) {
  if (!dict || !key)
    return 0;

  char *key_str = as_c_string_repr(key);
  unsigned long index = hash_string(key_str) % dict->capacity;
  DictEntry *entry = dict->buckets[index];
  DictEntry *prev = NULL;

  while (entry) {
    if (strcmp(entry->key, key_str) == 0) {
      if (prev)
        prev->next = entry->next;
      else
        dict->buckets[index] = entry->next;
      dict_entry_free(entry);
      dict->size--;
      mila_free(key_str);
      return 1;
    }
    prev = entry;
    entry = entry->next;
  }
  mila_free(key_str);
  return 0;
}

void dict_free(Dict *dict) {
  if (!dict)
    return;
  for (size_t i = 0; i < dict->capacity; i++) {
    DictEntry *entry = dict->buckets[i];
    while (entry) {
      DictEntry *next = entry->next;
      dict_entry_free(entry);
      entry = next;
    }
  }
  mila_free(dict->buckets);
  mila_free(dict);
}

Value *dict_display(Value *self) {
  Dict *dict = (Dict *)self->v.opaque;

  if (dict->size > MAX_ITEMS_DISPLAYED)
    return vstring_fmt("dict(%zu pairs)", dict->size);

  if (!dict || !dict->buckets)
    return vstring_dup("[@]");

  char *buffer = NULL;
  our_asprintf(&buffer, "[@ ");

  KVPair *entries = NULL;
  size_t count = 0, capacity = 16;
  entries = (KVPair *)mila_malloc(capacity * sizeof(KVPair));
  if (!entries)
    return NULL;

  // Collect all entries
  for (size_t i = 0; i < dict->capacity; i++) {
    DictEntry *entry = dict->buckets[i];
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
    char *val_str = as_c_string_repr(entries[i - 1].value);
    if (i > 1)
      our_asprintf(&buffer, "%s = %s, ", entries[i - 1].key, val_str);
    else
      our_asprintf(&buffer, "%s = %s", entries[i - 1].key, val_str);
    mila_free(val_str);
  }

  our_asprintf(&buffer, "]");
  mila_free(entries);
  return vstring_take(buffer);
}
