// This project is licensed under the GNU Affero General Public License
#pragma once

#include "mila.h"
#include "ml_string.c"
Value* native_list_append(Env*, int, Value**);
#include "ml_dict.c"
#include "ml_ll.c"

char* mila_to_json(Value* s);

Value* parse_dict(Src *src);
Value* parse_json(Src* s);
Value *parse_expr(Src *s)
{
    skip_ws(s);
    char c = src_peek(s);
    if (c == '\0')
        return vnull();
    // number
    if (isdigit((unsigned char)c) ||
        ((c == '+' || c == '-') && isdigit((unsigned char)s->src[s->pos + 1])) ||
        (c == '0' && (s->src[s->pos + 1] == 'x' || s->src[s->pos + 1] == 'X') &&
         isxdigit((unsigned char)s->src[s->pos + 2])))
    {
        return parse_number(s);
    }
    // string
    if (c == '"')
    {
        return parse_string(s);
    }
    if (c == '[')
    {
        src_get(s);
        size_t start = s->pos;
        Value **args = NULL;
        Value *list = call_native_with(NULL, native_list_new, NULL);
        int argc = 0;
        skip_ws(s);
        if (src_peek(s) != ']') {
            for (;;)
            {
                Value *a = parse_expr(s);
                if (IS_ERROR(a))
                {
                    mila_free(args);
                    val_release(list);
                    return a;
                }
                args = mila_realloc(args, sizeof(Value *) * (argc + 1));
                args[argc++] = a;
                val_release(call_native_with(NULL, native_list_append, val_retain(list), a, NULL));
                skip_ws(s);
                if (match_char(s, ','))
                    continue;
                if (match_char(s, ']'))
                    break;
                val_release(list);
                mila_free(args);
                int k = 1;
                while (k)
                {
                    if (src_peek(s) == '[')
                        k++;
                    if (src_peek(s) == ']')
                        k--;
                    s->pos++;
                }
                size_t end = s->pos;
                int len = end - start + 1;
                return vtagged_error(
                    E_SYNTAX_ERROR,
                    "Expected a %s or closing bracket!\nAt list `%.*s`",
                    "comma",
                    len, s->src + start - 1);
            }
        } else {
            src_get(s);
        }
        free(args);
        return list;
    }
    if (c == '{')
    {
        return parse_dict(s);
    }
    // identifier or keyword like 'null', 'true', 'false', or bare native name
    if (is_ident_start(c))
    {
        char *id = parse_ident(s);
        if (!id)
            return vnull();
        // keywords
        if (strcmp(id, "null") == 0)
        {
            mila_free(id);
            return vnull();
        }
        if (strcmp(id, "true") == 0)
        {
            mila_free(id);
            return vbool(1);
        }
        if (strcmp(id, "false") == 0)
        {
            mila_free(id);
            return vbool(0);
        }
        mila_free(id);
    }
    // fallback
    return vnull();
}

Value* parse_dict(Src* json) {
    if (!match_char(json, '{')) return verror("invalid dict");
    Value* dict = call_native_with(NULL, native_new_dict, NULL);
    skip_ws(json);
    while (src_peek(json) != '}') {
        if (is_ident_start(src_peek(json))) {
            char* id = parse_ident(json);
            if (!match_char(json, ':'))
                return verror("Expected colon!");
            Value* value = parse_expr(json);
            val_release(call_native_with(NULL, native_set_dict, val_retain(dict), vstring_take(id), value, NULL));
            val_release(value);
            skip_ws(json);
            if (match_char(json, ',')) {
                skip_ws(json);
                if (src_peek(json) == '}') // trailing comma
                    break;
            }
        } else if (src_peek(json) == '"') {
            Value* id = parse_string(json);
            if (!match_char(json, ':'))
                return verror("Expected colon!");
            Value* value = parse_expr(json);
            val_release(call_native_with(NULL, native_set_dict, val_retain(dict), id, value, NULL));
            val_release(value);
            skip_ws(json);
            if (match_char(json, ',')) {
                skip_ws(json);
                if (src_peek(json) == '}') // trailing comma
                    break;
            }
        }
    }
    src_get(json);
    return dict;
}

Value* parse_json(Src* json) {
    return parse_expr(json);
}

Value* parse_mjson_dict(Src *src);
Value* parse_mjson(Src* s);
Value *parse_mjson_expr(Src *s)
{
    skip_ws(s);
    char c = src_peek(s);
    if (c == '\0')
        return vnull();
    // number
    if (isdigit((unsigned char)c) ||
        ((c == '+' || c == '-') && isdigit((unsigned char)s->src[s->pos + 1])) ||
        (c == '0' && (s->src[s->pos + 1] == 'x' || s->src[s->pos + 1] == 'X') &&
         isxdigit((unsigned char)s->src[s->pos + 2])))
    {
        return parse_number(s);
    }
    // string
    if (c == '"')
    {
        return parse_string(s);
    }
    if (c == '[')
    {
        src_get(s);
        size_t start = s->pos;
        Value **args = NULL;
        Value *list = call_native_with(NULL, native_list_new, NULL);
        int argc = 0;
        skip_ws(s);
        if (src_peek(s) != ']') {
            for (;;)
            {
                Value *a = parse_expr(s);
                if (IS_ERROR(a))
                {
                    mila_free(args);
                    val_release(list);
                    return a;
                }
                args = mila_realloc(args, sizeof(Value *) * (argc + 1));
                args[argc++] = a;
                val_release(call_native_with(NULL, native_list_append, val_retain(list), a, NULL));
                skip_ws(s);
                if (match_char(s, ','))
                    continue;
                if (match_char(s, ']'))
                    break;
                val_release(list);
                mila_free(args);
                int k = 1;
                while (k)
                {
                    if (src_peek(s) == '[')
                        k++;
                    if (src_peek(s) == ']')
                        k--;
                    s->pos++;
                }
                size_t end = s->pos;
                int len = end - start + 1;
                return vtagged_error(
                    E_SYNTAX_ERROR,
                    "Expected a %s or closing bracket!\nAt list `%.*s`",
                    "comma",
                    len, s->src + start - 1);
            }
        } else {
            src_get(s);
        }
        free(args);
        return list;
    }
    if (c == '{')
    {
        return parse_mjson_dict(s);
    }
    // identifier or keyword like 'null', 'true', 'false', or bare native name
    if (is_ident_start(c))
    {
        char *id = parse_ident(s);
        if (!id)
            return vnull();
        // keywords
        if (strcmp(id, "null") == 0)
        {
            mila_free(id);
            return vnull();
        }
        if (strcmp(id, "true") == 0)
        {
            mila_free(id);
            return vbool(1);
        }
        if (strcmp(id, "false") == 0)
        {
            mila_free(id);
            return vbool(0);
        }
        if (strcmp(id, "fn") == 0) {
            free(id);
            // parse params
            FunctionParameters *params = parse_param_list(s);
            char **contextuals = parse_context_list(s);
            char **names;
            Env *closure = env_new(NULL);
            if (match_char(s, ':'))
            {
                // parse closure bindings
                names = parse_context_list(s); // reused parse_context_list
                for (int i = 0; names[i]; ++i)
                {
                    mila_free(names[i]);
                }
                mila_free(names);
            }
            if (is_keyword_at(s, "->"))
            {
                s->pos += 2;
                skip_ws(s);
                if (src_peek(s) == '"')
                {
                    Value *ret_type = parse_string(s);
                    // ignore return type for now
                    val_kill(ret_type);
                }
                else
                {
                    env_free(closure);
                    for (int i = 0; params->params[i]; ++i)
                    {
                        mila_free(params->params[i]);
                        mila_free(params->defaults[i]);
                    }
                    mila_free(params);
                    return vtagged_error(E_SYNTAX_ERROR,
                                         "Expected a string literal for the return type.");
                }
            }
            skip_ws(s);
            // body is block; extract substring from '{' to matching '}'
            size_t start = s->pos;
            size_t i = s->pos;
            if (src_peek(s) == '{')
            {
                int depth = 0;
                // find matching brace (we will copy out body)
                for (; i < s->len; ++i)
                {
                    char ch = s->src[i];
                    if (ch == '{')
                        depth++;
                    else if (ch == '}')
                    {
                        depth--;
                        if (depth == 0)
                        {
                            i++;
                            break;
                        }
                    }
                    else if (ch == '"')
                    {
                        // skip string literal
                        i++;
                        while (i < s->len && s->src[i] != '"')
                        {
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
            // create function value with closure get_line_pos(s) current env
            Value *fn = vfunction(params->params, params->defaults, contextuals, closure, body);
            free(params);
            fn->v.fn->name = mila_strdup("[lambda]");
            return fn;
        }
        mila_free(id);
    }
    // fallback
    return vnull();
}

Value* parse_mjson_dict(Src* json) {
    if (!match_char(json, '{')) return verror("invalid dict");
    Value* dict = call_native_with(NULL, native_new_dict, NULL);
    skip_ws(json);
    while (src_peek(json) != '}') {
        if (is_ident_start(src_peek(json))) {
            char* id = parse_ident(json);
            if (!match_char(json, ':'))
                return verror("Expected colon!");
            Value* value = parse_mjson_expr(json);
            val_release(call_native_with(NULL, native_set_dict, val_retain(dict), vstring_take(id), value, NULL));
            val_release(value);
            skip_ws(json);
            if (match_char(json, ',')) {
                skip_ws(json);
                if (src_peek(json) == '}') // trailing comma
                    break;
            }
        } else if (src_peek(json) == '"') {
            Value* id = parse_string(json);
            if (!match_char(json, ':'))
                return verror("Expected colon!");
            Value* value = parse_mjson_expr(json);
            if (GET_TYPE(value) == T_FUNCTION) {
                free(GET_FUNCTION(value)->name);
                GET_FUNCTION(value)->name = mila_strdup(GET_STRING(id));
            }
            val_release(call_native_with(NULL, native_set_dict, val_retain(dict), id, value, NULL));
            val_release(value);
            skip_ws(json);
            if (match_char(json, ',')) {
                skip_ws(json);
                if (src_peek(json) == '}') // trailing comma
                    break;
            }
        }
    }
    src_get(json);
    return dict;
}

Value* parse_mjson(Src* json) {
    return parse_mjson_expr(json);
}

// turn \xXX to \uXXXX for JSON valid escapes
// because why should JSON respect hex escapes?
char* ascii_to_unicode(const char* input) {
    char* buffer = NULL;
    const uint8_t *text = (const uint8_t*)input;
    
    for (size_t i = 0; i < strlen(input); ++i) {
        uint8_t byte = text[i];
        
        if (byte < 0x80) {
            switch (byte) {
                case '\a': our_asprintf(&buffer, "\\a"); break;
                case '\t': our_asprintf(&buffer, "\\t"); break;
                case '\n': our_asprintf(&buffer, "\\n"); break;
                case '\v': our_asprintf(&buffer, "\\v"); break;
                case '\f': our_asprintf(&buffer, "\\f"); break;
                case '\r': our_asprintf(&buffer, "\\r"); break;
                case '"': our_asprintf(&buffer, "\\\""); break;
                case '\\': our_asprintf(&buffer, "\\\\"); break;
                default:
                    if (isprint(byte))
                        our_asprintf(&buffer, "%c", byte);
                    else
                        our_asprintf(&buffer, "\\u%04X", byte);
            }
        } else if ((byte & 0xE0) == 0xC0 && i + 1 < strlen(input)) {
            uint32_t codepoint = ((byte & 0x1F) << 6) | (text[1+i] & 0x3F);
            our_asprintf(&buffer, "\\u%04X", codepoint);
            i++;
        } else if ((byte & 0xF0) == 0xE0 && i + 2 < strlen(input)) {
            uint32_t codepoint = ((byte & 0x0F) << 12) | ((text[1+i] & 0x3F) << 6) | (text[2+i] & 0x3F);
            our_asprintf(&buffer, "\\u%04X", codepoint);
            i+=2;
        } else if ((byte & 0xF8) == 0xF0 && i + 3 < strlen(input)) {
            uint32_t codepoint = ((byte & 0x07) << 18) | ((text[1+i] & 0x3F) << 12) 
                               | ((text[2+i] & 0x3F) << 6) | (text[3+i] & 0x3F);
            our_asprintf(&buffer, "\\u%04X", codepoint);
            i+=3;
        } else {
            our_asprintf(&buffer, "\\u%04X", byte);
        }
    }
    
    return buffer;
}

char* _mila_to_json(Value* v, int level) {
    char* result = NULL;
    
    if (!v) {
        our_asprintf(&result, "null");
        return result;
    }
    
    switch (GET_TYPE(v)) {
        case T_NULL:
            our_asprintf(&result, "null");
            break;
        case T_BOOL:
            our_asprintf(&result, GET_BOOL(v) ? "true" : "false");
            break;
        case T_INT:
        case T_UINT:
            our_asprintf(&result, "%ld", GET_INTEGER(v));
            break;
        case T_BINT:
            {
                char* num = i128toa(GET_BINTEGER(v));
                our_asprintf(&result, "%s", num);
                mila_free(num);
            }
            break;
        case T_FLOAT:
        case T_BFLOAT:
            {
                double d = GET_FLOAT(v);
                if (d == (long long)d)
                    our_asprintf(&result, "%.1f", d);
                else
                    our_asprintf(&result, "%.17g", d);
            }
            break;
        case T_STRING:
            {
                our_asprintf(&result, "\"");
                char* escaped = ascii_to_unicode(GET_STRING(v));
                our_asprintf(&result, "%s", escaped);
                mila_free(escaped);
                our_asprintf(&result, "\"");
            }
            break;
        case T_OPAQUE:
        case T_OWNED_OPAQUE:
        case T_WEAK_OPAQUE:
            {
                if (v->type_name && strcmp(v->type_name, MILA_LPREFIX "list") == 0) {
                    LinkedList* list = (LinkedList*)GET_OPAQUE(v);
                    our_asprintf(&result, "[\n");
                    for (size_t i = 0; i < list->size; ++i) {
                        Value* item = ll_get(list, i);
                        char* item_json = _mila_to_json(item, level+1);
                        our_asprintf(&result, "%*s%s", level*2, "", item_json);
                        mila_free(item_json);
                        if (i < list->size - 1)
                            our_asprintf(&result, ",\n");
                    }
                    our_asprintf(&result, "\n%*s]", (level-1)*2, "");
                } else if (v->type_name && strcmp(v->type_name, MILA_LPREFIX "dict") == 0) {
                    Dict* dict = (Dict*)GET_OPAQUE(v);
                    our_asprintf(&result, "{\n");
                    int first = 1;
                    for (size_t i = 0; i < dict->capacity; ++i) {
                        DictEntry* entry = dict->buckets[i];
                        while (entry) {
                            if (!first)
                                our_asprintf(&result, ",\n");
                            first = 0;
                            our_asprintf(&result, "%*s%s: ", level*2, "", entry->key);
                            char* val_json = _mila_to_json(entry->value, level+1);
                            our_asprintf(&result, "%s", val_json);
                            mila_free(val_json);
                            entry = entry->next;
                        }
                    }
                    our_asprintf(&result, "\n%*s}", (level-1)*2, "");
                } else {
                    our_asprintf(&result, "null");
                }
            }
            break;
        default:
            our_asprintf(&result, "null");
    }
    
    return result;
}


char* _mila_to_mjson(Value* v, int level) {
    char* result = NULL;
    
    if (!v) {
        our_asprintf(&result, "null");
        return result;
    }
    
    switch (GET_TYPE(v)) {
        case T_NULL:
            our_asprintf(&result, "null");
            break;
        case T_BOOL:
            our_asprintf(&result, GET_BOOL(v) ? "true" : "false");
            break;
        case T_INT:
        case T_UINT:
            our_asprintf(&result, "%ld", GET_INTEGER(v));
            break;
        case T_BINT:
            {
                char* num = i128toa(GET_BINTEGER(v));
                our_asprintf(&result, "%s", num);
                mila_free(num);
            }
            break;
        case T_FLOAT:
        case T_BFLOAT:
            {
                double d = GET_FLOAT(v);
                if (d == (long long)d)
                    our_asprintf(&result, "%.1f", d);
                else
                    our_asprintf(&result, "%.17g", d);
            }
            break;
        case T_STRING:
            {
                our_asprintf(&result, "\"");
                char* escaped = ascii_to_unicode(GET_STRING(v));
                our_asprintf(&result, "%s", escaped);
                mila_free(escaped);
                our_asprintf(&result, "\"");
            }
            break;
        case T_OPAQUE:
        case T_OWNED_OPAQUE:
        case T_WEAK_OPAQUE:
            {
                if (v->type_name && strcmp(v->type_name, MILA_LPREFIX "list") == 0) {
                    LinkedList* list = (LinkedList*)GET_OPAQUE(v);
                    our_asprintf(&result, "[\n");
                    for (size_t i = 0; i < list->size; ++i) {
                        Value* item = ll_get(list, i);
                        char* item_json = _mila_to_mjson(item, level+1);
                        our_asprintf(&result, "%*s%s", level*2, "", item_json);
                        mila_free(item_json);
                        if (i < list->size - 1)
                            our_asprintf(&result, ",\n");
                    }
                    our_asprintf(&result, "\n%*s]", (level-1)*2, "");
                } else if (v->type_name && strcmp(v->type_name, MILA_LPREFIX "dict") == 0) {
                    Dict* dict = (Dict*)GET_OPAQUE(v);
                    our_asprintf(&result, "{\n");
                    int first = 1;
                    for (size_t i = 0; i < dict->capacity; ++i) {
                        DictEntry* entry = dict->buckets[i];
                        while (entry) {
                            if (!first)
                                our_asprintf(&result, ",\n");
                            first = 0;
                            our_asprintf(&result, "%*s%s: ", level*2, "", entry->key);
                            char* val_json = _mila_to_mjson(entry->value, level+1);
                            our_asprintf(&result, "%s", val_json);
                            mila_free(val_json);
                            entry = entry->next;
                        }
                    }
                    our_asprintf(&result, "\n%*s}", (level-1)*2, "");
                } else {
                    our_asprintf(&result, "null");
                }
            }
            break;
        case T_FUNCTION: {
            FunctionV* fn = GET_FUNCTION(v);
            char* args = mila_strdup("");
            for (int i=0; fn->params[i]; ++i) {
                our_asprintf(&args, "%s", fn->params[i]);
                if (fn->defaults[i]) {
                    our_asprintf(&args, "=%s", fn->defaults[i]);
                }
                if (fn->params[i+1]) {
                    our_asprintf(&args, ",");
                }
            }
            our_asprintf(&result, "fn(%s) %s", args, fn->body_src);
            free(args);
        } break;
        default:
            our_asprintf(&result, "null");
    }
    
    return result;
}

char* mila_to_json(Value* v) {
    return _mila_to_json(v, 1);
}

char* mila_to_mjson(Value* v) {
    // return _mila_to_mjson(v, 1);
    return _mila_to_mjson(v, 1);
}