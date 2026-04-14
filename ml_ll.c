// This project is licensed under the GNU Affero General Public License
#pragma once
#include "mila.h"
#include <stddef.h>
#include <stdlib.h>

typedef struct LLNode {
  Value *value;
  struct LLNode *next;
} LLNode;

typedef struct LinkedList {
  LLNode *head;
  LLNode *tail;
  size_t size;
} LinkedList;

LinkedList *ll_create() {
  LinkedList *list = malloc(sizeof(LinkedList));
  if (!list)
    return NULL;
  list->head = NULL;
  list->tail = NULL;
  list->size = 0;
  return list;
}

void ll_free(LinkedList *list) {
  if (!list)
    return;
  LLNode *node = list->head;
  while (node) {
    LLNode *next = node->next;
    free(node);
    node = next;
  }
  free(list);
}

void ll_append(LinkedList *list, Value *val) {
  if (!list)
    return;
  LLNode *node = malloc(sizeof(LLNode));
  if (!node)
    return;
  node->value = val;
  node->next = NULL;

  if (!list->head) {
    list->head = node;
    list->tail = node;
  } else {
    list->tail->next = node;
    list->tail = node;
  }
  list->size++;
}

void ll_insert(LinkedList *list, size_t index, Value *val) {
  if (!list)
    return;

  if (index >= list->size) {
    ll_append(list, val);
    return;
  }

  LLNode *node = malloc(sizeof(LLNode));
  if (!node)
    return;
  node->value = val;

  if (index == 0) {
    node->next = list->head;
    list->head = node;
    if (!list->tail)
      list->tail = node;
  } else {
    LLNode *cur = list->head;
    for (size_t i = 0; i < index - 1; i++)
      cur = cur->next;
    node->next = cur->next;
    cur->next = node;
  }

  list->size++;
}

Value *ll_get(LinkedList *list, size_t index) {
  if (!list || index >= list->size)
    return NULL;
  LLNode *cur = list->head;
  for (size_t i = 0; i < index; i++)
    cur = cur->next;
  return cur->value;
}

void ll_set(LinkedList *list, size_t index, Value *val) {
  if (!list || index >= list->size)
    return;
  LLNode *cur = list->head;
  for (size_t i = 0; i < index; i++)
    cur = cur->next;
  val_release(cur->value); // free previous tenant
  cur->value = val;
}

Value *ll_pop(LinkedList *list, long index) {
  if (!list)
    return verror("ll_pop: list data is null.");
  if (index < 0)
    index = (~index);
  size_t t_index = (size_t)index;
  if (t_index >= list->size)
    return verror("ll_pop: index out of bounds.");

  LLNode *cur = list->head;
  LLNode *prev = NULL;
  for (size_t i = 0; i < t_index; i++) {
    prev = cur;
    cur = cur->next;
  }

  if (prev)
    prev->next = cur->next;
  else
    list->head = cur->next;

  if (cur == list->tail)
    list->tail = prev;

  Value *val = cur->value;
  free(cur);
  list->size--;
  return val;
}

Value **ll_to_iter(LinkedList *list) {
  if (!list)
    return NULL;
  Value **arr = malloc((list->size + 1) * sizeof(Value *));
  if (!arr)
    return NULL;
  LLNode *cur = list->head;
  size_t i = 0;
  while (cur) {
    arr[i++] = val_retain(cur->value);
    cur = cur->next;
  }
  arr[i] = NULL;
  return arr;
}