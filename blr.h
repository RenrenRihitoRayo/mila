/*
 * blr.h - Basic Line Read: portable readline-like line editing with history.
 * Windows uses the Console API; POSIX uses termios raw mode.
 * Non-interactive input (pipes) falls back to fgets automatically.
 *
 * Key bindings (both read functions):
 *   Ctrl-A/Home  Ctrl-E/End  Ctrl-B/Left  Ctrl-F/Right
 *   Alt-B/Ctrl-Left (word left)  Alt-F/Ctrl-Right (word right)
 *   Backspace/Ctrl-H  Delete/Ctrl-D  Ctrl-K  Ctrl-U  Ctrl-W
 *   Enter/Ctrl-J confirm,  Ctrl-C discard (returns NULL)
 * History navigation (blr_rec_read only): Up/Ctrl-P, Down/Ctrl-N
 *
 * Both read functions return a heap string the caller must free().
 * NULL means EOF or Ctrl-C. BlrHistory must be init'd and free'd by caller.
 * Not thread-safe; use external locking for shared history.
 *
 * Example:
 *   BlrHistory h; blr_history_init(&h, 100);
 *   char *line;
 *   while ((line = blr_rec_read("> ", &h))) { puts(line); free(line); }
 *   blr_history_save(&h, ".hist"); blr_history_free(&h);
 */

#ifndef BLR_H
#define BLR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Bounded FIFO history store. Oldest entry evicted when max_entries is reached. */
typedef struct {
    char   **entries;
    size_t   count;
    size_t   max_entries;
} BlrHistory;

/* Allocate entries array for max_entries lines. Must be called first. Returns 0/-1. */
int blr_history_init(BlrHistory *hist, size_t max_entries);

/* Free all entries and zero the struct. The struct itself is caller-owned. */
void blr_history_free(BlrHistory *hist);

/* Append a copy of line; evicts oldest on overflow. Empty strings are ignored. Returns 0/-1. */
int blr_history_add(BlrHistory *hist, const char *line);

/* Write history to path (one entry per line, file is created/truncated). Returns 0/-1. */
int blr_history_save(const BlrHistory *hist, const char *path);

/* Load entries from path into hist. Missing file is treated as empty (not an error). Returns 0/-1.
   Lines longer than BLR_MAX_LINE are silently truncated. */
int blr_history_load(BlrHistory *hist, const char *path);

/* Normal read: show prompt, edit a line, return heap string the caller must free(). No history nav.
   Returns NULL on EOF (Ctrl-D on empty line) or Ctrl-C. Trailing newline is stripped. */
char *blr_read(const char *prompt);

/* Recorded read: like blr_read() but Up/Down history nav is active and confirmed lines are saved to hist.
   Returns NULL on EOF or Ctrl-C. Non-empty lines are appended to hist before being returned. */
char *blr_rec_read(const char *prompt, BlrHistory *hist);

/* Maximum bytes per input line (override before including or via -DBLR_MAX_LINE=N). */
#ifndef BLR_MAX_LINE
#  define BLR_MAX_LINE 4096
#endif

#ifdef __cplusplus
}
#endif

#endif /* BLR_H */