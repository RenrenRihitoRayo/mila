/* blr.c - Basic Line Read (implementation) */

#pragma once

#include "blr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32) || defined(_WIN64)
#  define BLR_WINDOWS 1
#else
#  define BLR_POSIX   1
#endif

#ifdef BLR_WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <io.h>
#else
#  include <termios.h>
#  include <unistd.h>
#  include <sys/ioctl.h>
#endif

/* Returns 1 if stdin is an interactive terminal. */
static int blr_is_tty(void)
{
#ifdef BLR_WINDOWS
    return _isatty(_fileno(stdin));
#else
    return isatty(STDIN_FILENO);
#endif
}

/* ---- LineBuf: fixed-size editing buffer with cursor tracking ---- */

typedef struct {
    char   buf[BLR_MAX_LINE + 1];
    size_t len;
    size_t pos;
} LineBuf;

static void lb_init(LineBuf *lb)
{
    memset(lb->buf, 0, sizeof(lb->buf));
    lb->len = 0;
    lb->pos = 0;
}

/* Insert ch at cursor; returns -1 if buffer is full. */
static int lb_insert(LineBuf *lb, char ch)
{
    if (lb->len >= BLR_MAX_LINE) return -1;
    memmove(lb->buf + lb->pos + 1, lb->buf + lb->pos, lb->len - lb->pos + 1);
    lb->buf[lb->pos] = ch;
    lb->len++;
    lb->pos++;
    return 0;
}

/* Delete char before cursor (Backspace). */
static void lb_backspace(LineBuf *lb)
{
    if (lb->pos == 0) return;
    memmove(lb->buf + lb->pos - 1, lb->buf + lb->pos, lb->len - lb->pos + 1);
    lb->len--;
    lb->pos--;
}

/* Delete char under cursor (Delete key). */
static void lb_delete(LineBuf *lb)
{
    if (lb->pos >= lb->len) return;
    memmove(lb->buf + lb->pos, lb->buf + lb->pos + 1, lb->len - lb->pos);
    lb->len--;
}

/* Kill from cursor to end of line (Ctrl-K). */
static void lb_kill_eol(LineBuf *lb)
{
    lb->buf[lb->pos] = '\0';
    lb->len = lb->pos;
}

/* Kill entire line (Ctrl-U). */
static void lb_kill_line(LineBuf *lb)
{
    lb->buf[0] = '\0';
    lb->len = 0;
    lb->pos = 0;
}

/* Kill word before cursor (Ctrl-W): skip spaces then non-space chars. */
static void lb_kill_word(LineBuf *lb)
{
    size_t p = lb->pos;
    while (p > 0 && lb->buf[p - 1] == ' ') p--;
    while (p > 0 && lb->buf[p - 1] != ' ') p--;
    size_t killed = lb->pos - p;
    memmove(lb->buf + p, lb->buf + lb->pos, lb->len - lb->pos + 1);
    lb->len -= killed;
    lb->pos  = p;
}

/* Move cursor one word left (Alt-B / Ctrl-Left). */
static void lb_word_left(LineBuf *lb)
{
    size_t p = lb->pos;
    while (p > 0 && lb->buf[p - 1] == ' ') p--;
    while (p > 0 && lb->buf[p - 1] != ' ') p--;
    lb->pos = p;
}

/* Move cursor one word right (Alt-F / Ctrl-Right). */
static void lb_word_right(LineBuf *lb)
{
    size_t p = lb->pos;
    while (p < lb->len && lb->buf[p] == ' ') p++;
    while (p < lb->len && lb->buf[p] != ' ') p++;
    lb->pos = p;
}

/* Replace buffer contents with str; cursor moves to end. */
static void lb_set(LineBuf *lb, const char *str)
{
    size_t n = strlen(str);
    if (n > BLR_MAX_LINE) n = BLR_MAX_LINE;
    memcpy(lb->buf, str, n);
    lb->buf[n] = '\0';
    lb->len = n;
    lb->pos = n;
}

/* ---- Terminal: redraw line using ANSI VT100 (works on Win10+ conhost too) ---- */

/* CR, write prompt+buf, erase-to-EOL, reposition cursor via CUF. */
static void blr_refresh(const char *prompt, const LineBuf *lb)
{
    size_t prompt_len = strlen(prompt);
    fwrite("\r",      1, 1,          stdout);
    fwrite(prompt,    1, prompt_len, stdout);
    fwrite(lb->buf,   1, lb->len,    stdout);
    fwrite("\x1b[0K", 1, 4,          stdout);
    fwrite("\r",      1, 1,          stdout);
    size_t target = prompt_len + lb->pos;
    if (target > 0) {
        char seq[32];
        int n = snprintf(seq, sizeof(seq), "\x1b[%zuC", target);
        if (n > 0) fwrite(seq, 1, (size_t)n, stdout);
    }
    fflush(stdout);
}

/* ---- Platform-independent key event ---- */

typedef enum {
    BLR_KEY_NONE = 0,
    BLR_KEY_CHAR,
    BLR_KEY_ENTER,
    BLR_KEY_CTRL_C,
    BLR_KEY_CTRL_D,
    BLR_KEY_BACKSPACE,
    BLR_KEY_DELETE,
    BLR_KEY_LEFT,
    BLR_KEY_RIGHT,
    BLR_KEY_UP,
    BLR_KEY_DOWN,
    BLR_KEY_HOME,
    BLR_KEY_END,
    BLR_KEY_CTRL_A,
    BLR_KEY_CTRL_E,
    BLR_KEY_CTRL_B,
    BLR_KEY_CTRL_F,
    BLR_KEY_CTRL_K,
    BLR_KEY_CTRL_U,
    BLR_KEY_CTRL_W,
    BLR_KEY_CTRL_P,
    BLR_KEY_CTRL_N,
    BLR_KEY_WORD_LEFT,
    BLR_KEY_WORD_RIGHT,
    BLR_KEY_IGNORE,
    BLR_KEY_EOF_OR_ERROR,
} BlrKeyType;

typedef struct {
    BlrKeyType type;
    char       ch; /* valid when type == BLR_KEY_CHAR */
} BlrKey;

/* ---- POSIX raw-mode + key reader ---- */

#ifdef BLR_POSIX

typedef struct termios BlrTermState;

/* Switch stdin to raw mode; save prior state in *saved. Returns 0/-1. */
static int blr_term_raw(BlrTermState *saved)
{
    if (tcgetattr(STDIN_FILENO, saved) == -1) return -1;
    struct termios raw = *saved;
    raw.c_iflag &= ~(unsigned)(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(unsigned)(OPOST);
    raw.c_cflag |=  (unsigned)(CS8);
    raw.c_lflag &= ~(unsigned)(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    return tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

/* Restore terminal state saved by blr_term_raw. */
static void blr_term_restore(const BlrTermState *saved)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, saved);
}

/* Read one raw byte from stdin; returns byte value or -1 on EOF/error. */
static int blr_read_byte(void)
{
    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n <= 0) return -1;
    return (int)c;
}

/* Parse one keypress from raw stdin, including multi-byte escape sequences. */
static BlrKey posix_next_key(void)
{
    BlrKey k; k.type = BLR_KEY_NONE; k.ch = 0;

    int c = blr_read_byte();
    if (c < 0) { k.type = BLR_KEY_EOF_OR_ERROR; return k; }

    switch (c) {
        case  1: k.type = BLR_KEY_CTRL_A;    return k;
        case  2: k.type = BLR_KEY_CTRL_B;    return k;
        case  3: k.type = BLR_KEY_CTRL_C;    return k;
        case  4: k.type = BLR_KEY_CTRL_D;    return k;
        case  5: k.type = BLR_KEY_CTRL_E;    return k;
        case  6: k.type = BLR_KEY_CTRL_F;    return k;
        case  8: k.type = BLR_KEY_BACKSPACE; return k;
        case 10: /* fall-through */
        case 13: k.type = BLR_KEY_ENTER;     return k;
        case 11: k.type = BLR_KEY_CTRL_K;    return k;
        case 14: k.type = BLR_KEY_CTRL_N;    return k;
        case 16: k.type = BLR_KEY_CTRL_P;    return k;
        case 21: k.type = BLR_KEY_CTRL_U;    return k;
        case 23: k.type = BLR_KEY_CTRL_W;    return k;
        case 127: k.type = BLR_KEY_BACKSPACE; return k;

        case 27: {
            int seq0 = blr_read_byte();
            if (seq0 < 0) { k.type = BLR_KEY_IGNORE; return k; }

            if (seq0 == '[') {
                int seq1 = blr_read_byte();
                if (seq1 < 0) { k.type = BLR_KEY_IGNORE; return k; }
                switch (seq1) {
                    case 'A': k.type = BLR_KEY_UP;    return k;
                    case 'B': k.type = BLR_KEY_DOWN;  return k;
                    case 'C': k.type = BLR_KEY_RIGHT; return k;
                    case 'D': k.type = BLR_KEY_LEFT;  return k;
                    case 'H': k.type = BLR_KEY_HOME;  return k;
                    case 'F': k.type = BLR_KEY_END;   return k;
                    case '1': {
                        int s2 = blr_read_byte();
                        if (s2 < 0) { k.type = BLR_KEY_IGNORE; return k; }
                        if (s2 == '~') { k.type = BLR_KEY_HOME; return k; }
                        if (s2 == ';') {
                            int s3 = blr_read_byte();
                            int s4 = blr_read_byte();
                            if (s3 < 0 || s4 < 0) { k.type = BLR_KEY_IGNORE; return k; }
                            if (s4 == 'C') { k.type = BLR_KEY_WORD_RIGHT; return k; }
                            if (s4 == 'D') { k.type = BLR_KEY_WORD_LEFT;  return k; }
                        }
                        k.type = BLR_KEY_IGNORE; return k;
                    }
                    case '3': { int s2=blr_read_byte(); if(s2=='~'){k.type=BLR_KEY_DELETE;return k;} k.type=BLR_KEY_IGNORE; return k; }
                    case '4': { int s2=blr_read_byte(); if(s2=='~'){k.type=BLR_KEY_END;   return k;} k.type=BLR_KEY_IGNORE; return k; }
                    case '7': { int s2=blr_read_byte(); if(s2=='~'){k.type=BLR_KEY_HOME;  return k;} k.type=BLR_KEY_IGNORE; return k; }
                    case '8': { int s2=blr_read_byte(); if(s2=='~'){k.type=BLR_KEY_END;   return k;} k.type=BLR_KEY_IGNORE; return k; }
                    default:   k.type = BLR_KEY_IGNORE; return k;
                }
            } else if (seq0 == 'O') {
                int seq1 = blr_read_byte();
                if (seq1 < 0) { k.type = BLR_KEY_IGNORE; return k; }
                switch (seq1) {
                    case 'A': k.type = BLR_KEY_UP;    return k;
                    case 'B': k.type = BLR_KEY_DOWN;  return k;
                    case 'C': k.type = BLR_KEY_RIGHT; return k;
                    case 'D': k.type = BLR_KEY_LEFT;  return k;
                    case 'H': k.type = BLR_KEY_HOME;  return k;
                    case 'F': k.type = BLR_KEY_END;   return k;
                    default:  k.type = BLR_KEY_IGNORE; return k;
                }
            } else if (seq0 == 'b') { k.type = BLR_KEY_WORD_LEFT;  return k; }
              else if (seq0 == 'f') { k.type = BLR_KEY_WORD_RIGHT; return k; }
            k.type = BLR_KEY_IGNORE; return k;
        }

        default:
            if ((unsigned char)c >= 0x20 || (unsigned char)c == '\t') {
                k.type = BLR_KEY_CHAR; k.ch = (char)c;
            } else {
                k.type = BLR_KEY_IGNORE;
            }
            return k;
    }
}

#endif /* BLR_POSIX */

/* ---- Windows Console API implementation ---- */

#ifdef BLR_WINDOWS

typedef DWORD BlrTermState;

static HANDLE blr_hin  = INVALID_HANDLE_VALUE;
static HANDLE blr_hout = INVALID_HANDLE_VALUE;

/* Disable line/echo input; enable VT input on Win10+. Returns 0/-1. */
static int blr_term_raw(BlrTermState *saved)
{
    blr_hin  = GetStdHandle(STD_INPUT_HANDLE);
    blr_hout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (blr_hin == INVALID_HANDLE_VALUE || blr_hout == INVALID_HANDLE_VALUE) return -1;
    if (!GetConsoleMode(blr_hin, saved)) return -1;
    DWORD raw = *saved & ~(DWORD)(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
    raw |= ENABLE_VIRTUAL_TERMINAL_INPUT;
    if (!SetConsoleMode(blr_hin, raw)) {
        raw &= ~(DWORD)ENABLE_VIRTUAL_TERMINAL_INPUT;
        if (!SetConsoleMode(blr_hin, raw)) return -1;
    }
    DWORD out_mode;
    if (GetConsoleMode(blr_hout, &out_mode))
        SetConsoleMode(blr_hout, out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);
    return 0;
}

/* Restore console mode saved by blr_term_raw. */
static void blr_term_restore(const BlrTermState *saved)
{
    if (blr_hin != INVALID_HANDLE_VALUE) SetConsoleMode(blr_hin, *saved);
}

/* Translate a Windows virtual-key event into a platform-independent BlrKey. */
static BlrKey win_map_key(WORD vk, CHAR ch, DWORD ctrl)
{
    BlrKey k; k.type = BLR_KEY_NONE; k.ch = 0;
    int ctrl_pressed = (ctrl & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;
    int alt_pressed  = (ctrl & (LEFT_ALT_PRESSED  | RIGHT_ALT_PRESSED))  != 0;

    switch (vk) {
        case VK_RETURN: k.type = BLR_KEY_ENTER;     return k;
        case VK_BACK:   k.type = BLR_KEY_BACKSPACE;  return k;
        case VK_DELETE: k.type = BLR_KEY_DELETE;     return k;
        case VK_LEFT:   k.type = ctrl_pressed ? BLR_KEY_WORD_LEFT  : BLR_KEY_LEFT;  return k;
        case VK_RIGHT:  k.type = ctrl_pressed ? BLR_KEY_WORD_RIGHT : BLR_KEY_RIGHT; return k;
        case VK_UP:     k.type = BLR_KEY_UP;         return k;
        case VK_DOWN:   k.type = BLR_KEY_DOWN;       return k;
        case VK_HOME:   k.type = BLR_KEY_HOME;       return k;
        case VK_END:    k.type = BLR_KEY_END;        return k;
    }
    if (ch >= 1 && ch <= 26 && ctrl_pressed) {
        switch (ch) {
            case  1: k.type = BLR_KEY_CTRL_A; return k;
            case  2: k.type = BLR_KEY_CTRL_B; return k;
            case  3: k.type = BLR_KEY_CTRL_C; return k;
            case  4: k.type = BLR_KEY_CTRL_D; return k;
            case  5: k.type = BLR_KEY_CTRL_E; return k;
            case  6: k.type = BLR_KEY_CTRL_F; return k;
            case 11: k.type = BLR_KEY_CTRL_K; return k;
            case 14: k.type = BLR_KEY_CTRL_N; return k;
            case 16: k.type = BLR_KEY_CTRL_P; return k;
            case 21: k.type = BLR_KEY_CTRL_U; return k;
            case 23: k.type = BLR_KEY_CTRL_W; return k;
            default: k.type = BLR_KEY_IGNORE; return k;
        }
    }
    if (alt_pressed) {
        if (ch == 'b' || ch == 'B') { k.type = BLR_KEY_WORD_LEFT;  return k; }
        if (ch == 'f' || ch == 'F') { k.type = BLR_KEY_WORD_RIGHT; return k; }
        k.type = BLR_KEY_IGNORE; return k;
    }
    if (ch == '\n' || ch == '\r') { k.type = BLR_KEY_ENTER; return k; }
    if (ch >= 0x20) { k.type = BLR_KEY_CHAR; k.ch = ch; return k; }
    k.type = BLR_KEY_IGNORE;
    return k;
}

/* Read one key event from the Windows console input queue. */
static BlrKey posix_next_key(void) /* unified name used by blr_edit_loop */
{
    BlrKey k; k.type = BLR_KEY_NONE; k.ch = 0;
    INPUT_RECORD ir; DWORD nread;
    while (1) {
        if (!ReadConsoleInputA(blr_hin, &ir, 1, &nread) || nread == 0) {
            k.type = BLR_KEY_EOF_OR_ERROR; return k;
        }
        if (ir.EventType != KEY_EVENT || !ir.Event.KeyEvent.bKeyDown) continue;
        k = win_map_key(ir.Event.KeyEvent.wVirtualKeyCode,
                        ir.Event.KeyEvent.uChar.AsciiChar,
                        ir.Event.KeyEvent.dwControlKeyState);
        if (k.type != BLR_KEY_NONE) return k;
    }
}

#endif /* BLR_WINDOWS */

/* ---- Core editing engine ---- */

/* Per-call state shared by blr_read and blr_rec_read; hist is NULL for the former. */
typedef struct {
    const char  *prompt;
    LineBuf      lb;
    BlrHistory  *hist;
    int          hist_pos;
    char         saved[BLR_MAX_LINE + 1];
} EditState;

/* Move through history by dir (-1 = older, +1 = newer); saves the live line on first nav. */
static void blr_nav_history(EditState *es, int dir)
{
    if (!es->hist || es->hist->count == 0) return;
    int new_pos = es->hist_pos + dir;
    if (new_pos < 0) new_pos = 0;
    if (new_pos > (int)es->hist->count) new_pos = (int)es->hist->count;
    if (new_pos == es->hist_pos) return;
    if (es->hist_pos == (int)es->hist->count)
        memcpy(es->saved, es->lb.buf, es->lb.len + 1);
    es->hist_pos = new_pos;
    lb_set(&es->lb, new_pos == (int)es->hist->count ? es->saved
                                                     : es->hist->entries[new_pos]);
}

/* Main event loop: raw-mode editing on TTY, fgets fallback otherwise. Returns heap string or NULL. */
static char *blr_edit_loop(EditState *es)
{
    BlrTermState saved_term;
    int raw_ok = 0;
    if (blr_is_tty()) raw_ok = (blr_term_raw(&saved_term) == 0);

    fwrite(es->prompt, 1, strlen(es->prompt), stdout);
    fflush(stdout);

    if (!raw_ok) {
        char *line = malloc(BLR_MAX_LINE + 1);
        if (!line) return NULL;
        if (!fgets(line, BLR_MAX_LINE, stdin)) { free(line); return NULL; }
        size_t n = strlen(line);
        if (n > 0 && line[n-1] == '\n') line[--n] = '\0';
        if (n > 0 && line[n-1] == '\r') line[--n] = '\0';
        return line;
    }

    char *result = NULL;
    for (;;) {
        blr_refresh(es->prompt, &es->lb);
        BlrKey key = posix_next_key();
        switch (key.type) {
            case BLR_KEY_CHAR:        lb_insert(&es->lb, key.ch); break;
            case BLR_KEY_ENTER:
                fwrite("\r\n", 1, 2, stdout); fflush(stdout);
                result = malloc(es->lb.len + 1);
                if (result) memcpy(result, es->lb.buf, es->lb.len + 1);
                goto done;
            case BLR_KEY_CTRL_C:
                fwrite("\r\n", 1, 2, stdout); fflush(stdout);
                result = NULL; goto done;
            case BLR_KEY_CTRL_D:
                if (es->lb.len == 0) { fwrite("\r\n",1,2,stdout); fflush(stdout); result=NULL; goto done; }
                lb_delete(&es->lb); break;
            case BLR_KEY_BACKSPACE:   lb_backspace(&es->lb);  break;
            case BLR_KEY_DELETE:      lb_delete(&es->lb);     break;
            case BLR_KEY_LEFT:
            case BLR_KEY_CTRL_B:      if (es->lb.pos > 0) es->lb.pos--;          break;
            case BLR_KEY_RIGHT:
            case BLR_KEY_CTRL_F:      if (es->lb.pos < es->lb.len) es->lb.pos++; break;
            case BLR_KEY_WORD_LEFT:   lb_word_left(&es->lb);  break;
            case BLR_KEY_WORD_RIGHT:  lb_word_right(&es->lb); break;
            case BLR_KEY_HOME:
            case BLR_KEY_CTRL_A:      es->lb.pos = 0;          break;
            case BLR_KEY_END:
            case BLR_KEY_CTRL_E:      es->lb.pos = es->lb.len; break;
            case BLR_KEY_CTRL_K:      lb_kill_eol(&es->lb);    break;
            case BLR_KEY_CTRL_U:      lb_kill_line(&es->lb);   break;
            case BLR_KEY_CTRL_W:      lb_kill_word(&es->lb);   break;
            case BLR_KEY_UP:
            case BLR_KEY_CTRL_P:      blr_nav_history(es, -1); break;
            case BLR_KEY_DOWN:
            case BLR_KEY_CTRL_N:      blr_nav_history(es, +1); break;
            case BLR_KEY_EOF_OR_ERROR:
                fwrite("\r\n",1,2,stdout); fflush(stdout); result=NULL; goto done;
            default: break;
        }
    }
done:
    blr_term_restore(&saved_term);
    return result;
}

/* ---- Public API ---- */

char *blr_read(const char *prompt)
{
    EditState es;
    memset(&es, 0, sizeof(es));
    es.prompt   = prompt ? prompt : "";
    es.hist     = NULL;
    es.hist_pos = 0;
    lb_init(&es.lb);
    return blr_edit_loop(&es);
}

char *blr_rec_read(const char *prompt, BlrHistory *hist)
{
    EditState es;
    memset(&es, 0, sizeof(es));
    es.prompt   = prompt ? prompt : "";
    es.hist     = hist;
    es.hist_pos = (int)(hist ? hist->count : 0);
    lb_init(&es.lb);
    char *line = blr_edit_loop(&es);
    if (line && line[0] != '\0' && hist) blr_history_add(hist, line);
    return line;
}

int blr_history_init(BlrHistory *hist, size_t max_entries)
{
    if (!hist || max_entries == 0) { errno = EINVAL; return -1; }
    hist->entries = (char **)calloc(max_entries, sizeof(char *));
    if (!hist->entries) return -1;
    hist->count       = 0;
    hist->max_entries = max_entries;
    return 0;
}

void blr_history_free(BlrHistory *hist)
{
    if (!hist || !hist->entries) return;
    for (size_t i = 0; i < hist->count; i++) { free(hist->entries[i]); hist->entries[i] = NULL; }
    free(hist->entries);
    hist->entries = NULL; hist->count = 0; hist->max_entries = 0;
}

int blr_history_add(BlrHistory *hist, const char *line)
{
    if (!hist || !line || line[0] == '\0') return 0;
    if (hist->count == hist->max_entries) {
        free(hist->entries[0]);
        memmove(hist->entries, hist->entries + 1, (hist->max_entries - 1) * sizeof(char *));
        hist->entries[hist->max_entries - 1] = NULL;
        hist->count--;
    }
    size_t n = strlen(line);
    hist->entries[hist->count] = (char *)malloc(n + 1);
    if (!hist->entries[hist->count]) return -1;
    memcpy(hist->entries[hist->count], line, n + 1);
    hist->count++;
    return 0;
}

int blr_history_save(const BlrHistory *hist, const char *path)
{
    if (!hist || !path) { errno = EINVAL; return -1; }
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    for (size_t i = 0; i < hist->count; i++) {
        if (fputs(hist->entries[i], f) == EOF || fputc('\n', f) == EOF)
            { fclose(f); return -1; }
    }
    return fclose(f) == 0 ? 0 : -1;
}

int blr_history_load(BlrHistory *hist, const char *path)
{
    if (!hist || !path) { errno = EINVAL; return -1; }
    FILE *f = fopen(path, "r");
    if (!f) return errno == ENOENT ? 0 : -1;
    char line[BLR_MAX_LINE + 2];
    while (fgets(line, (int)sizeof(line), f)) {
        size_t n = strlen(line);
        while (n > 0 && (line[n-1] == '\n' || line[n-1] == '\r')) line[--n] = '\0';
        if (n == 0) continue;
        if (blr_history_add(hist, line) == -1) { fclose(f); return -1; }
    }
    fclose(f);
    return 0;
}