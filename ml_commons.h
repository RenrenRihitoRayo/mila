#pragma once
#include <stdlib.h>
#include <stdio.h>

// Got infected by tsoding

#define da_growth_rate 0.75
#define da_grow(x) x+(x*da_growth_rate)

#define da_debug(arr)\
    printf("Debug for Dynamic Array " #arr "\nSize: %zu\nCount: %zu\n", arr->size, arr->count);

#define da_append(arr, item) \
    if (arr->count == 0) {\
        typeof(arr->size) new_size = 10;\
        arr->items = calloc(sizeof(arr->items[0]), new_size);\
        arr->size = new_size;\
    }\
    else if (arr->count >= arr->size || arr->items == NULL) {\
        typeof(arr->size) new_size = da_grow(arr->size);\
        arr->items = realloc(arr->items, new_size);\
        arr->size = new_size;\
    }\
    arr->items[arr->count++] = item;
