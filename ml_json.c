// This project is licensed under the GNU Affero General Public License
#pragma once

#include "mila.h"
#include "ml_dict.h"
#include "ml_ll.c"
#include "ml_primitives.h"
#include "ml_string.c"
#include <ctype.h>

Value *native_list_append(Env *, int, Value **);
Value *parse_dict_unified(Src *json, int parse_fn);

// turn \xXX to \uXXXX for JSON valid escapes
// because why should JSON respect hex escapes?
void ascii_to_unicode(char **buffer, const char *input) {
    const uint8_t *text = (const uint8_t *)input;

    for (size_t i = 0; i < strlen(input); ++i) {
        uint8_t byte = text[i];

        if (byte < 0x80) {
            switch (byte) {
            case '\b':
                malloc_sprintf(buffer, "\\b");
                break;
            case '\f':
                malloc_sprintf(buffer, "\\f");
                break;
            case '\n':
                malloc_sprintf(buffer, "\\n");
                break;
            case '\r':
                malloc_sprintf(buffer, "\\r");
                break;
            case '\t':
                malloc_sprintf(buffer, "\\t");
                break;
            case '"':
                malloc_sprintf(buffer, "\\\"");
                break;
            case '\\':
                malloc_sprintf(buffer, "\\\\");
                break;
            default:
                if (isprint(byte))
                    malloc_sprintf(buffer, "%c", byte);
                else
                    malloc_sprintf(buffer, "\\u%04X", byte);
            }
        } else if ((byte & 0xE0) == 0xC0 && i + 1 < strlen(input)) {
            uint32_t codepoint = ((byte & 0x1F) << 6) | (text[1 + i] & 0x3F);
            malloc_sprintf(buffer, "\\u%04X", codepoint);
            i++;
        } else if ((byte & 0xF0) == 0xE0 && i + 2 < strlen(input)) {
            uint32_t codepoint = ((byte & 0x0F) << 12) |
                                 ((text[1 + i] & 0x3F) << 6) |
                                 (text[2 + i] & 0x3F);
            malloc_sprintf(buffer, "\\u%04X", codepoint);
            i += 2;
        } else if ((byte & 0xF8) == 0xF0 && i + 3 < strlen(input)) {
            uint32_t codepoint =
                ((byte & 0x07) << 18) | ((text[1 + i] & 0x3F) << 12) |
                ((text[2 + i] & 0x3F) << 6) | (text[3 + i] & 0x3F);
            malloc_sprintf(buffer, "\\u%04X", codepoint);
            i += 3;
        } else {
            malloc_sprintf(buffer, "\\u%04X", byte);
        }
    }
}

// Parse unified mjson flag enables fn parsing
Value *parse_expr_unified(Src *s, int parse_fn) {
    skip_ws(s);
    char c = src_peek(s);
    if (c == '\0')
        return vnull();

    if (isdigit((unsigned char)c) ||
        ((c == '+' || c == '-') &&
         isdigit((unsigned char)s->src[s->pos + 1])) ||
        (c == '0' && (s->src[s->pos + 1] == 'x' || s->src[s->pos + 1] == 'X') &&
         isxdigit((unsigned char)s->src[s->pos + 2])))
        return parse_number(s);

    if (c == '"')
        return parse_string(s);

    if (c == '[') {
        src_get(s);
        size_t start = s->pos;
        Value **args = NULL;
        Value *list = call_native_with(NULL, native_list_new, NULL);
        int argc = 0;
        skip_ws(s);

        if (src_peek(s) != ']') {
            for (;;) {
                Value *a = parse_expr_unified(s, parse_fn);
                if (IS_ERROR(a)) {
                    mila_free(args);
                    val_release(list);
                    return a;
                }
                args = mila_realloc(args, sizeof(Value *) * (argc + 1));
                args[argc++] = a;
                val_release(call_native_with(NULL, native_list_append,
                                             val_retain(list), a, NULL));
                skip_ws(s);

                if (match_char(s, ','))
                    continue;
                if (match_char(s, ']'))
                    break;

                val_release(list);
                mila_free(args);
                int k = 1;
                while (k) {
                    if (src_peek(s) == '[')
                        k++;
                    if (src_peek(s) == ']')
                        k--;
                    s->pos++;
                }
                size_t end = s->pos;
                return vtagged_error(
                    E_SYNTAX_ERROR,
                    "Expected a %s or closing bracket!\nAt list `%.*s`",
                    "comma", (int)(end - start + 1), s->src + start - 1);
            }
        } else {
            src_get(s);
        }
        free(args);
        return list;
    }

    if (c == '{')
        return parse_dict_unified(s, parse_fn);

    if (is_ident_start(c)) {
        char *id = parse_ident(s);
        if (!id)
            return vnull();

        if (strcmp(id, "null") == 0) {
            mila_free(id);
            return vnull();
        }
        if (strcmp(id, "true") == 0) {
            mila_free(id);
            return vbool(1);
        }
        if (strcmp(id, "false") == 0) {
            mila_free(id);
            return vbool(0);
        }

        if (parse_fn && strcmp(id, "fn") == 0) {
            free(id);
            FunctionParameters *params = parse_param_list(s);
            char **contextuals = parse_context_list(s);
            Env *closure = env_new(NULL);
            char *ret = NULL;

            if (match_char(s, ':')) {
                char **names = parse_context_list(s);
                for (int i = 0; names[i]; ++i)
                    mila_free(names[i]);
                mila_free(names);
            }

            if (is_keyword_at(s, "->")) {
                s->pos += 2;
                skip_ws(s);
                if (src_peek(s) == '"') {
                    Value *ret_type = parse_string(s);
                    ret = mila_strdup(GET_STRING(ret_type));
                    val_kill(ret_type);
                } else {
                    env_free(closure);
                    for (int i = 0; params->params[i]; ++i) {
                        mila_free(params->params[i]);
                        mila_free(params->defaults[i]);
                        mila_free(params->types[i]);
                    }
                    mila_free(params->params);
                    mila_free(params->defaults);
                    mila_free(params->types);
                    mila_free(params);
                    return vtagged_error(
                        E_SYNTAX_ERROR,
                        "Expected a string literal for the return type.");
                }
            }

            skip_ws(s);
            size_t start = s->pos, i = s->pos;

            if (src_peek(s) == '{') {
                int depth = 0;
                for (; i < s->len; ++i) {
                    char ch = s->src[i];
                    if (ch == '{')
                        depth++;
                    else if (ch == '}') {
                        depth--;
                        if (depth == 0) {
                            i++;
                            break;
                        }
                    } else if (ch == '"') {
                        i++;
                        while (i < s->len && s->src[i] != '"') {
                            if (s->src[i] == '\\' && i + 1 < s->len)
                                i += 2;
                            else
                                i++;
                        }
                    }
                }
            } else {
                skip_parse_statement(s);
                i = s->pos;
            }

            if (i > s->len)
                i = s->len;
            int blen = i - start;
            char *body = mila_malloc(blen + 1);
            memcpy(body, s->src + start, blen);
            body[blen] = 0;
            s->pos = i;

            Value *fn = vfunction(params, ret, contextuals, closure, body);
            free(params);
            GET_FUNCTION(fn)->name = mila_strdup("[lambda]");
            return fn;
        }

        mila_free(id);
    }

    return vnull();
}

// Dict parser unified
Value *parse_dict_unified(Src *json, int parse_fn) {
    if (!match_char(json, '{'))
        return verror("invalid dict");
    Value *dict = call_native_with(NULL, native_new_dict, NULL);
    skip_ws(json);

    while (src_peek(json) != '}') {
        Value *id = NULL;

        if (is_ident_start(src_peek(json))) {
            id = vstring_take(parse_ident(json));
        } else if (src_peek(json) == '"') {
            id = parse_string(json);
        } else {
            break;
        }

        if (!match_char(json, ':'))
            return verror("Expected colon!");

        Value *value = parse_expr_unified(json, parse_fn);
        if (parse_fn && GET_TYPE(value) == T_FUNCTION) {
            free(GET_FUNCTION(value)->name);
            GET_FUNCTION(value)->name = mila_strdup(GET_STRING(id));
        }

        val_release(call_native_with(NULL, native_set_dict, val_retain(dict),
                                     id, value, NULL));
        val_release(value);
        skip_ws(json);

        if (match_char(json, ',')) {
            skip_ws(json);
            if (src_peek(json) == '}')
                break;
        }
    }

    src_get(json);
    return dict;
}

Value *parse_json(Src *s) { return parse_expr_unified(s, 0); }

Value *parse_mjson(Src *s) { return parse_expr_unified(s, 1); }

// Serialize unified include_fn flag
// TODO: swap result from string to a string builder to avoid contineous alloc
char *_mila_to_json_unified(Value *v, int level, int include_fn) {
    char *result = NULL;

    if (!v) {
        malloc_sprintf(&result, "null");
        return result;
    }

    switch (GET_TYPE(v)) {
    case T_NULL:
        malloc_sprintf(&result, "null");
        break;
    case T_BOOL:
        malloc_sprintf(&result, GET_BOOL(v) ? "true" : "false");
        break;
    case T_INT:
    case T_UINT:
        malloc_sprintf(&result, "%ld", GET_INTEGER(v));
        break;
    case T_FLOAT: {
        double d = GET_FLOAT(v);
        malloc_sprintf(&result, d == (long long)d ? "%.1f" : "%.17g", d);
        break;
    }
    case T_STRING: {
        malloc_sprintf(&result, "\"");
        ascii_to_unicode(&result, GET_STRING(v));
        malloc_sprintf(&result, "\"");
        break;
    }
    case T_OPAQUE:
    case T_OWNED_OPAQUE: {
        if (v->type_name && strcmp(v->type_name, MILA_LPREFIX "list") == 0) {
            LinkedList *list = (LinkedList *)GET_OPAQUE(v);
            malloc_sprintf(&result, "[\n");
            for (size_t i = 0; i < list->size; ++i) {
                char *item_json = _mila_to_json_unified(ll_get(list, i),
                                                        level + 1, include_fn);
                malloc_sprintf(&result, "%*s%s%s", level * 2, "", item_json,
                               i < list->size - 1 ? ",\n" : "");
                mila_free(item_json);
            }
            malloc_sprintf(&result, "\n%*s]", (level - 1) * 2, "");
        } else if (v->type_name &&
                   strcmp(v->type_name, MILA_LPREFIX "dict") == 0) {
            Dict *dict = (Dict *)GET_OPAQUE(v);
            malloc_sprintf(&result, "{\n");
            int first = 1;
            for (size_t i = 0; i < dict->capacity; ++i) {
                for (DictEntry *entry = dict->buckets[i]; entry;
                     entry = entry->next) {
                    if (!first)
                        malloc_sprintf(&result, ",\n");
                    first = 0;
                    char *val_json = _mila_to_json_unified(
                        entry->value, level + 1, include_fn);
                    malloc_sprintf(&result, "%*s%s: %s", level * 2, "",
                                   entry->key, val_json);
                    mila_free(val_json);
                }
            }
            malloc_sprintf(&result, "\n%*s}", (level - 1) * 2, "");
        } else {
            malloc_sprintf(&result, "null");
        }
        break;
    }
    case T_FUNCTION: {
        if (include_fn) {
            FunctionV *fn = GET_FUNCTION(v);
            char *args = mila_strdup("");
            for (int i = 0; fn->params[i]; ++i) {
                malloc_sprintf(&args, "%s%s%s", args, fn->params[i],
                               fn->defaults[i] ? fn->defaults[i] : "");
                if (fn->params[i + 1])
                    malloc_sprintf(&args, "%s,", args);
            }
            malloc_sprintf(&result, "fn(%s) %s", args, fn->body_src);
            free(args);
        } else {
            malloc_sprintf(&result, "null");
        }
        break;
    }
    default:
        malloc_sprintf(&result, "null");
    }

    return result;
}

// File write unified
long _io_mila_to_json_unified(FILE *file, Value *v, int level, int include_fn) {
    long result = 0;

    if (!v)
        return fprintf(file, "null");

    switch (GET_TYPE(v)) {
    case T_NULL:
        result += fprintf(file, "null");
        break;
    case T_BOOL:
        result += fprintf(file, GET_BOOL(v) ? "true" : "false");
        break;
    case T_INT:
    case T_UINT:
        result += fprintf(file, "%ld", GET_INTEGER(v));
        break;
    case T_FLOAT: {
        double d = GET_FLOAT(v);
        result += fprintf(file, d == (long long)d ? "%.1f" : "%.17g", d);
        break;
    }
    case T_STRING: {
        char *escaped = NULL;
        ascii_to_unicode(&escaped, GET_STRING(v));
        result += fprintf(file, "\"%s\"", escaped);
        mila_free(escaped);
        break;
    }
    case T_OPAQUE:
    case T_OWNED_OPAQUE: {
        if (v->type_name && strcmp(v->type_name, MILA_LPREFIX "list") == 0) {
            LinkedList *list = (LinkedList *)GET_OPAQUE(v);
            result += fprintf(file, "[\n");
            for (size_t i = 0; i < list->size; ++i) {
                result += fprintf(file, "%*s", level * 2, "");
                result += _io_mila_to_json_unified(file, ll_get(list, i),
                                                   level + 1, include_fn);
                if (i < list->size - 1)
                    result += fprintf(file, ",\n");
            }
            result += fprintf(file, "\n%*s]", (level - 1) * 2, "");
        } else if (v->type_name &&
                   strcmp(v->type_name, MILA_LPREFIX "dict") == 0) {
            Dict *dict = (Dict *)GET_OPAQUE(v);
            result += fprintf(file, "{\n");
            int first = 1;
            for (size_t i = 0; i < dict->capacity; ++i) {
                for (DictEntry *entry = dict->buckets[i]; entry;
                     entry = entry->next) {
                    if (!first)
                        result += fprintf(file, ",\n");
                    first = 0;
                    result +=
                        fprintf(file, "%*s%s: ", level * 2, "", entry->key);
                    result += _io_mila_to_json_unified(file, entry->value,
                                                       level + 1, include_fn);
                }
            }
            result += fprintf(file, "\n%*s}", (level - 1) * 2, "");
        } else {
            result += fprintf(file, "null");
        }
        break;
    }
    case T_FUNCTION: {
        if (include_fn) {
            FunctionV *fn = GET_FUNCTION(v);
            result += fprintf(file, "fn(");
            for (int i = 0; fn->params[i]; ++i) {
                result += fprintf(file, "%s%s", fn->params[i],
                                  fn->defaults[i] ? fn->defaults[i] : "");
                if (fn->params[i + 1])
                    result += fprintf(file, ",");
            }
            result += fprintf(file, ") %s", fn->body_src);
        } else {
            result += fprintf(file, "null");
        }
        break;
    }
    default:
        result += fprintf(file, "null");
    }

    return result;
}

char *mila_to_json(Value *v) { return _mila_to_json_unified(v, 1, 0); }

char *mila_to_mjson(Value *v) { return _mila_to_json_unified(v, 1, 1); }

long mila_to_json_io(FILE *file, Value *v) {
    return _io_mila_to_json_unified(file, v, 1, 0);
}

long mila_to_mjson_io(FILE *file, Value *v) {
    return _io_mila_to_json_unified(file, v, 1, 1);
}