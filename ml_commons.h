// This project is licensed under the GNU Affero General Public License
#pragma once
#include "./mila.h"
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

// Read entire file in one go.
char* read_file(const char* name) {
    FILE* fhandle = fopen(name, "r");
    if (!fhandle) return NULL;
    fseek(fhandle, 0, SEEK_END);
    size_t size = ftell(fhandle);
    char* file = (char*)mila_malloc(sizeof(char)*size+1);
    if (!file) {
        fclose(fhandle);
        return NULL;
    }
    fseek(fhandle, 0, SEEK_SET);
    fread(file, sizeof(char), size, fhandle);
    fclose(fhandle);
    return file;
}

void noise(
    long start,
    long count,
    long min,
    long max,
    long magnitude,
    long* out
) {
    long current = start;

    out[0] = current;

    for (long i = 1; i < count; i++) {

        long dir;

        if (current <= min) {
            dir = 1;
        }
        else if (current >= max) {
            dir = -1;
        }
        else {
            /* -1, 0, or 1 */
            dir = (rand() % 3) - 1;
        }

        /* random jump from 0 to magnitude */
        long jump = rand() % (magnitude + 1);

        current += dir * jump;

        /* clamp */
        if (current < min) {
            current = min;
        }

        if (current > max) {
            current = max;
        }

        out[i] = current;
    }
}