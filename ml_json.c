// This project is licensed under the GNU Affero General Public License
#pragma once

#include "mila.h"
#include "ml_dict.h"
#include "ml_ll.c"
#include "ml_primitives.h"
#include <ctype.h>

typedef struct {
    char *data;
    size_t capacity;
    size_t len;
} StringBuffer;

static inline StringBuffer* sb_new(size_t initial_capacity) {
    StringBuffer *sb = mila_malloc(sizeof(StringBuffer));
    sb->capacity = initial_capacity > 256 ? initial_capacity : 256;
    sb->data = mila_malloc(sb->capacity);
    sb->len = 0;
    return sb;
}

static inline void sb_append_char(StringBuffer *sb, char c) {
    if (sb->len + 1 >= sb->capacity) {
        sb->capacity *= 2;
        sb->data = mila_realloc(sb->data, sb->capacity);
    }
    sb->data[sb->len++] = c;
}

static inline void sb_append_str(StringBuffer *sb, const char *str) {
    size_t str_len = strlen(str);
    if (sb->len + str_len >= sb->capacity) {
        sb->capacity = (sb->len + str_len) * 1.7;
        sb->data = mila_realloc(sb->data, sb->capacity);
    }
    memcpy(sb->data + sb->len, str, str_len);
    sb->len += str_len;
}

static inline void sb_append_fmt(StringBuffer *sb, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    
    if (sb->len + needed >= sb->capacity) {
        sb->capacity = (sb->len + needed) * 1.7;
        sb->data = mila_realloc(sb->data, sb->capacity);
    }
    
    va_start(args, fmt);
    vsnprintf(sb->data + sb->len, needed + 1, fmt, args);
    va_end(args);
    sb->len += needed;
}

static inline char* sb_free_get(StringBuffer *sb) {
    sb->data[sb->len] = '\0';
    char *result = sb->data;
    mila_free(sb);
    return result;
}

static inline void sb_free(StringBuffer *sb) {
    mila_free(sb->data);
    mila_free(sb);
}

typedef struct {
    FILE *f;
    char *data;
    size_t capacity;
    size_t len;
} FileBuffer;

static inline FileBuffer* fb_new(FILE *f, size_t initial_capacity) {
    FileBuffer *fb = mila_malloc(sizeof(FileBuffer));
    fb->f = f;
    fb->capacity = initial_capacity;
    fb->data = mila_malloc(fb->capacity);
    fb->len = 0;
    return fb;
}

static inline void fb_flush(FileBuffer *fb) {
    if (fb->len > 0) {
        fwrite(fb->data, 1, fb->len, fb->f);
        fb->len = 0;
    }
}

static inline void fb_append_char(FileBuffer *fb, char c) {
    if (fb->len + 1 >= fb->capacity) {
        fb_flush(fb);
    }
    fb->data[fb->len++] = c;
}

static inline void fb_append_str(FileBuffer *fb, const char *str) {
    size_t str_len = strlen(str);
    if (fb->len + str_len >= fb->capacity) {
        fb_flush(fb);
        if (str_len >= fb->capacity) {
            fwrite(str, 1, str_len, fb->f);
            return;
        }
    }
    memcpy(fb->data + fb->len, str, str_len);
    fb->len += str_len;
}

static inline void fb_append_fmt(FileBuffer *fb, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    
    if (fb->len + needed >= fb->capacity) {
        fb_flush(fb);
        if ((size_t)needed >= fb->capacity) {
            va_start(args, fmt);
            vfprintf(fb->f, fmt, args);
            va_end(args);
            return;
        }
    }
    
    va_start(args, fmt);
    vsnprintf(fb->data + fb->len, needed + 1, fmt, args);
    va_end(args);
    fb->len += needed;
}

static inline void fb_free_flush(FileBuffer *fb) {
    fb_flush(fb);
    mila_free(fb->data);
    mila_free(fb);
}

Value *native_list_append(Env *, int, Value **);
Value *parse_dict_unified(Src *json, int parse_fn);

Value *parse_expr_unified(Src *s, int parse_fn) {
    skip_ws(s);
    char c = src_peek(s);
    if (c == '\0') return vnull();
    if (isdigit((unsigned char)c) || ((c == '+' || c == '-') && isdigit((unsigned char)s->src[s->pos + 1])) || (c == '0' && (s->src[s->pos + 1] == 'x' || s->src[s->pos + 1] == 'X') && isxdigit((unsigned char)s->src[s->pos + 2])))
        return parse_number(s);
    if (c == '"') return parse_string(s);
    if (c == '[') {
        src_get(s);
        Value *list = call_native_with(NULL, native_list_new, NULL);
        skip_ws(s);
        if (src_peek(s) != ']') {
            for (;;) {
                Value *a = parse_expr_unified(s, parse_fn);
                if (IS_ERROR(a)) { val_release(list); return a; }
                val_release(call_native_with(NULL, native_list_append, val_retain(list), a, NULL));
                skip_ws(s);
                if (match_char(s, ',')) continue;
                if (match_char(s, ']')) break;
                val_release(list);
                return verror("Expected comma or bracket");
            }
        } else src_get(s);
        return list;
    }
    if (c == '{') return parse_dict_unified(s, parse_fn);
    if (is_ident_start(c)) {
        char *id = parse_ident(s);
        if (!id) return vnull();
        if (strcmp(id, "null") == 0) { mila_free(id); return vnull(); }
        if (strcmp(id, "true") == 0) { mila_free(id); return vbool(1); }
        if (strcmp(id, "false") == 0) { mila_free(id); return vbool(0); }
        mila_free(id);
    }
    return vnull();
}

Value *parse_dict_unified(Src *json, int parse_fn) {
    if (!match_char(json, '{')) return verror("invalid dict");
    Value *dict = call_native_with(NULL, native_new_dict, NULL);
    skip_ws(json);
    while (src_peek(json) != '}') {
        Value *id = NULL;
        if (is_ident_start(src_peek(json))) id = vstring_take(parse_ident(json));
        else if (src_peek(json) == '"') id = parse_string(json);
        else break;
        if (!match_char(json, ':')) return verror("Expected colon!");
        Value *value = parse_expr_unified(json, parse_fn);
        val_release(call_native_with(NULL, native_set_dict, val_retain(dict), id, value, NULL));
        val_release(value);
        skip_ws(json);
        if (match_char(json, ',')) { skip_ws(json); if (src_peek(json) == '}') break; }
    }
    src_get(json);
    return dict;
}

void ascii_to_unicode(StringBuffer *sb, const char *input) {
    const uint8_t *text = (const uint8_t *)input;
    size_t len = strlen(input);

    for (size_t i = 0; i < len; ++i) {
        uint8_t byte = text[i];
        if (byte < 0x80) {
            switch (byte) {
            case '\b': sb_append_str(sb, "\\b"); break;
            case '\f': sb_append_str(sb, "\\f"); break;
            case '\n': sb_append_str(sb, "\\n"); break;
            case '\r': sb_append_str(sb, "\\r"); break;
            case '\t': sb_append_str(sb, "\\t"); break;
            case '"':  sb_append_str(sb, "\\\""); break;
            case '\\': sb_append_str(sb, "\\\\"); break;
            default:
                if (byte >= 0x20 && byte < 0x7F)
                    sb_append_char(sb, byte);
                else
                    sb_append_fmt(sb, "\\u%04X", byte);
            }
        } else {
            sb_append_fmt(sb, "\\u%04X", byte);
        }
    }
}

Value *parse_json(Src *s) { return parse_expr_unified(s, 0); }
Value *parse_mjson(Src *s) { return parse_expr_unified(s, 1); }

static void _mila_to_json_impl(StringBuffer *sb, Value *v, int level, int include_fn);

static inline void serialize_escaped_string(StringBuffer *sb, const char *str) {
    sb_append_char(sb, '"');
    ascii_to_unicode(sb, str);
    sb_append_char(sb, '"');
}

static inline void serialize_list(StringBuffer *sb, LinkedList *list, int level, int include_fn) {
    if (list->size == 0) {
        sb_append_str(sb, "[]");
        return;
    }
    
    sb_append_str(sb, "[\n");
    for (size_t i = 0; i < list->size; ++i) {
        for (int j = 0; j < level; ++j) sb_append_str(sb, "  ");
        _mila_to_json_impl(sb, ll_get(list, i), level + 1, include_fn);
        if (i < list->size - 1) sb_append_char(sb, ',');
        sb_append_char(sb, '\n');
    }
    for (int j = 0; j < level - 2; ++j) sb_append_str(sb, "  ");
    sb_append_char(sb, ']');
}

static inline void serialize_dict(StringBuffer *sb, Dict *dict, int level, int include_fn) {
    if (dict->size == 0) {
        sb_append_str(sb, "{}");
        return;
    }
    
    sb_append_str(sb, "{\n");
    int first = 1;
    
    for (size_t i = 0; i < dict->capacity; ++i) {
        for (DictEntry *entry = dict->buckets[i]; entry; entry = entry->next) {
            if (!first) sb_append_str(sb, ",\n");
            first = 0;
            for (int j = 0; j < level; ++j) sb_append_str(sb, "  ");
            sb_append_str(sb, entry->key);
            sb_append_str(sb, ": ");
            _mila_to_json_impl(sb, entry->value, level + 1, include_fn);
        }
    }
    
    sb_append_char(sb, '\n');
    for (int j = 0; j < level - 2; ++j) sb_append_str(sb, "  ");
    sb_append_char(sb, '}');
}

static void _mila_to_json_impl(StringBuffer *sb, Value *v, int level, int include_fn) {
    if (!v) {
        sb_append_str(sb, "null");
        return;
    }

    switch (GET_TYPE(v)) {
    case T_NULL:
        sb_append_str(sb, "null");
        break;
    case T_BOOL:
        sb_append_str(sb, GET_BOOL(v) ? "true" : "false");
        break;
    case T_INT:
    case T_UINT:
        sb_append_fmt(sb, "%ld", GET_INTEGER(v));
        break;
    case T_FLOAT: {
        double d = GET_FLOAT(v);
        sb_append_fmt(sb, d == (long long)d ? "%.1f" : "%.17g", d);
        break;
    }
    case T_STRING: {
        serialize_escaped_string(sb, GET_STRING(v));
        break;
    }
    case T_OPAQUE:
    case T_OWNED_OPAQUE: {
        if (!v->type_name) {
            sb_append_str(sb, "null");
            break;
        }
        
        if (strcmp(v->type_name, "list") == 0) {
            serialize_list(sb, (LinkedList *)GET_OPAQUE(v), level + 1, include_fn);
        } else if (strcmp(v->type_name, "dict") == 0) {
            serialize_dict(sb, (Dict *)GET_OPAQUE(v), level + 1, include_fn);
        } else {
            sb_append_str(sb, "null");
        }
        break;
    }
    case T_FUNCTION: {
        if (include_fn) {
            FunctionV *fn = GET_FUNCTION(v);
            sb_append_str(sb, "fn(");
            
            for (int i = 0; fn->params[i]; ++i) {
                sb_append_str(sb, fn->params[i]);
                if (fn->defaults[i])
                    sb_append_str(sb, fn->defaults[i]);
                if (fn->params[i + 1])
                    sb_append_char(sb, ',');
            }
            
            sb_append_str(sb, ") {\n");
            char *indented = indent(fn->body_src, level + 1);
            sb_append_str(sb, indented);
            mila_free(indented);
            sb_append_char(sb, '\n');
            sb_append_char(sb, '}');
        } else {
            sb_append_str(sb, "null");
        }
        break;
    }
    default:
        sb_append_str(sb, "null");
    }
}

char *mila_to_json(Value *v) {
    StringBuffer *sb = sb_new(500);
    _mila_to_json_impl(sb, v, 1, 0);
    return sb_free_get(sb);
}

char *mila_to_mjson(Value *v) {
    StringBuffer *sb = sb_new(500);
    _mila_to_json_impl(sb, v, 1, 1);
    return sb_free_get(sb);
}

static void _mila_to_json_file_impl(FileBuffer *fb, Value *v, int level, int include_fn);

static inline void serialize_list_file(FileBuffer *fb, LinkedList *list, int level, int include_fn) {
    if (list->size == 0) {
        fb_append_str(fb, "[]");
        return;
    }
    
    fb_append_str(fb, "[\n");
    for (size_t i = 0; i < list->size; ++i) {
        for (int j = 0; j < level; ++j) fb_append_str(fb, "  ");
        _mila_to_json_file_impl(fb, ll_get(list, i), level + 1, include_fn);
        if (i < list->size - 1) fb_append_char(fb, ',');
        fb_append_char(fb, '\n');
    }
    for (int j = 0; j < level - 2; ++j) fb_append_str(fb, "  ");
    fb_append_char(fb, ']');
}

static inline void serialize_dict_file(FileBuffer *fb, Dict *dict, int level, int include_fn) {
    if (dict->size == 0) {
        fb_append_str(fb, "{}");
        return;
    }
    
    fb_append_str(fb, "{\n");
    int first = 1;
    
    for (size_t i = 0; i < dict->capacity; ++i) {
        for (DictEntry *entry = dict->buckets[i]; entry; entry = entry->next) {
            if (!first) fb_append_str(fb, ",\n");
            first = 0;
            for (int j = 0; j < level; ++j) fb_append_str(fb, "  ");
            fb_append_str(fb, entry->key);
            fb_append_str(fb, ": ");
            _mila_to_json_file_impl(fb, entry->value, level + 1, include_fn);
        }
    }
    
    fb_append_char(fb, '\n');
    for (int j = 0; j < level - 2; ++j) fb_append_str(fb, "  ");
    fb_append_char(fb, '}');
}

static void _mila_to_json_file_impl(FileBuffer *fb, Value *v, int level, int include_fn) {
    if (!v) {
        fb_append_str(fb, "null");
        return;
    }

    switch (GET_TYPE(v)) {
    case T_NULL:
        fb_append_str(fb, "null");
        break;
    case T_BOOL:
        fb_append_str(fb, GET_BOOL(v) ? "true" : "false");
        break;
    case T_INT:
    case T_UINT:
        fb_append_fmt(fb, "%ld", GET_INTEGER(v));
        break;
    case T_FLOAT: {
        double d = GET_FLOAT(v);
        fb_append_fmt(fb, d == (long long)d ? "%.1f" : "%.17g", d);
        break;
    }
    case T_STRING: {
        fb_append_char(fb, '"');
        const uint8_t *text = (const uint8_t *)GET_STRING(v);
        size_t len = strlen(GET_STRING(v));
        for (size_t i = 0; i < len; ++i) {
            uint8_t byte = text[i];
            if (byte < 0x80) {
                switch (byte) {
                case '\b': fb_append_str(fb, "\\b"); break;
                case '\f': fb_append_str(fb, "\\f"); break;
                case '\n': fb_append_str(fb, "\\n"); break;
                case '\r': fb_append_str(fb, "\\r"); break;
                case '\t': fb_append_str(fb, "\\t"); break;
                case '"':  fb_append_str(fb, "\\\""); break;
                case '\\': fb_append_str(fb, "\\\\"); break;
                default:
                    if (byte >= 0x20 && byte < 0x7F)
                        fb_append_char(fb, byte);
                    else
                        fb_append_fmt(fb, "\\u%04X", byte);
                }
            } else {
                fb_append_fmt(fb, "\\u%04X", byte);
            }
        }
        fb_append_char(fb, '"');
        break;
    }
    case T_OPAQUE:
    case T_OWNED_OPAQUE: {
        if (!v->type_name) {
            fb_append_str(fb, "null");
            break;
        }
        
        if (strcmp(v->type_name, "list") == 0) {
            serialize_list_file(fb, (LinkedList *)GET_OPAQUE(v), level, include_fn);
        } else if (strcmp(v->type_name, "dict") == 0) {
            serialize_dict_file(fb, (Dict *)GET_OPAQUE(v), level, include_fn);
        } else {
            fb_append_str(fb, "null");
        }
        break;
    }
    case T_FUNCTION: {
        if (include_fn) {
            FunctionV *fn = GET_FUNCTION(v);
            fb_append_str(fb, "fn(");
            
            for (int i = 0; fn->params[i]; ++i) {
                fb_append_str(fb, fn->params[i]);
                if (fn->defaults[i])
                    fb_append_str(fb, fn->defaults[i]);
                if (fn->params[i + 1])
                    fb_append_char(fb, ',');
            }
            
            fb_append_str(fb, ") {\n");
            char *indented = indent(fn->body_src, level);
            fb_append_str(fb, indented);
            mila_free(indented);
            fb_append_char(fb, '\n');
            fb_append_char(fb, '}');
        } else {
            fb_append_str(fb, "null");
        }
        break;
    }
    default:
        fb_append_str(fb, "null");
    }
}

long mila_to_json_io(FILE *file, Value *v) {
    FileBuffer *fb = fb_new(file, 4096);
    _mila_to_json_file_impl(fb, v, 1, 0);
    fb_free_flush(fb);
    return 0;
}

long mila_to_mjson_io(FILE *file, Value *v) {
    FileBuffer *fb = fb_new(file, 4096);
    _mila_to_json_file_impl(fb, v, 1, 1);
    fb_free_flush(fb);
    return 0;
}