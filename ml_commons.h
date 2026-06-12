// This project is licensed under the GNU Affero General Public License
#pragma once
#include "./mila.h"
#include <stdlib.h>
#include <stdio.h>

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

typedef struct {
    void** items;
    size_t size, count;
} __dynamic_array_mask;

void da_append(void* arr, void* item) {
    if (!arr) {
        fprintf(stderr, "Given dynamic array was NULL! <%p>\n", arr);
        abort();
    }
    __dynamic_array_mask* array = (__dynamic_array_mask*)arr;

    if (!array->items) {
        array->items = (void**)mila_malloc(sizeof(void*));
        if (!array->items) abort();
        array->items[array->count++] = item;
        array->size = 1;
    } else if (array->size <= array->count + 1) {
        size_t new_size = (array->size + (size_t)(array->size * 0.75) + 1);
        array->items = (void**)mila_realloc(array->items, sizeof(void*) * new_size);
        if (!array->items) abort();
        array->items[array->count++] = item;
        array->size = new_size;
    }
}