/*
 * example.c - simple BLR demo shell
 *
 * Compile:
 *   gcc -o example example.c blr.c
 */

#include "blr.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HISTORY_FILE ".blr_example_history"
#define HISTORY_MAX  200

int main(void)
{
    BlrHistory hist;
    if (blr_history_init(&hist, HISTORY_MAX) != 0) {
        perror("blr_history_init");
        return 1;
    }

    /* Load persisted history from a previous session (if any). */
    blr_history_load(&hist, HISTORY_FILE);

    printf("BLR example shell. Type 'quit' to exit.\n");
    printf("Try arrow keys, Ctrl-A/E, Alt-B/F, Ctrl-K/U/W.\n\n");

    char *line;
    while ((line = blr_rec_read("blr> ", &hist)) != NULL) {
        if (strcmp(line, "quit") == 0) {
            free(line);
            break;
        }

        /* Demo: show history with 'history' command. */
        if (strcmp(line, "history") == 0) {
            for (size_t i = 0; i < hist.count; i++)
                printf("  %3zu  %s\n", i + 1, hist.entries[i]);
        } else {
            printf(">> %s\n", line);
        }

        free(line);
    }

    /* Persist history. */
    blr_history_save(&hist, HISTORY_FILE);
    blr_history_free(&hist);

    printf("\nGoodbye!\n");
    return 0;
}