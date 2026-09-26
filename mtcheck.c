#define ML_NO_MAIN
#define RESTRICTED_BUILD
#include "mila.c"

typedef struct {
    char *name;
    char *type;
    int scope_level;
} Symbol;

typedef struct {
    Symbol *symbols;
    size_t count;
    size_t capacity;
} SymbolTable;

static SymbolTable *symtable_new(void) {
    SymbolTable *st = malloc(sizeof(SymbolTable));
    st->symbols = malloc(sizeof(Symbol) * 64);
    st->count = 0;
    st->capacity = 64;
    return st;
}

static void symtable_add(SymbolTable *st, const char *name, const char *type, int scope) {
    if (st->count >= st->capacity) {
        st->capacity *= 2;
        st->symbols = realloc(st->symbols, sizeof(Symbol) * st->capacity);
    }
    st->symbols[st->count].name = mila_strdup(name);
    st->symbols[st->count].type = mila_strdup(type);
    st->symbols[st->count].scope_level = scope;
    st->count++;
}

static Symbol *symtable_lookup(SymbolTable *st, const char *name) {
    for (int i = st->count - 1; i >= 0; i--) {
        if (strcmp(st->symbols[i].name, name) == 0)
            return &st->symbols[i];
    }
    return NULL;
}

static void symtable_pop_scope(SymbolTable *st, int scope) {
    for (size_t i = 0; i < st->count; i++) {
        if (st->symbols[i].scope_level > scope) {
            mila_free(st->symbols[i].name);
            mila_free(st->symbols[i].type);
            memmove(&st->symbols[i], &st->symbols[i + 1],
                    sizeof(Symbol) * (st->count - i - 1));
            st->count--;
            i--;
        }
    }
}

static char *_infer_type(const char *text) {
    if (!text || strlen(text) == 0)
        return mila_strdup("any");
    
    const char *p = text;
    while (*p && isspace(*p)) p++;
    
    if (!*p)
        return mila_strdup("any");
    
    if (*p == '"')
        return mila_strdup("string");
    if (*p == '[')
        return mila_strdup("list");
    if (*p == '{')
        return mila_strdup(*(p+1) == '@' ? "dict" : "any");
    if (isdigit(*p) || (*p == '-' && isdigit(*(p+1)))) {
        int has_dot = 0;
        if (*p == '-') p++;
        while (*p && (isdigit(*p) || *p == '.')) {
            if (*p == '.') {
                if (has_dot) break;
                has_dot = 1;
            }
            p++;
        }
        return mila_strdup(has_dot ? "float" : "int");
    }
    if (strncmp(p, "true", 4) == 0 || strncmp(p, "false", 5) == 0)
        return mila_strdup("bool");
    
    return mila_strdup("any");
}

static int types_compatible(const char *actual, const char *expected) {
    if (!actual || !expected) return 1;
    if (strcmp(expected, "any") == 0) return 1;
    if (strcmp(actual, expected) == 0) return 1;
    if (strcmp(actual, "any") == 0) return 1;
    return 0;
}

int sibling = 0;

const char *_mtags(Src *s, char *file_name, int level, int sibling, SymbolTable *st);

const char *_process_block(Src *s, char *file_name, int level, SymbolTable *st) {
    int sibling_cur = sibling++;
    if (!match_char(s, '{'))
        return ERR_EXPECTED_BRACE;

    while (!src_eof(s)) {
        skip_ws(s);
        if (match_char(s, '}')) {
            symtable_pop_scope(st, level);
            return ERR_SUCCESS;
        }
        const char *err = _mtags(s, file_name, level + 1, sibling_cur, st);
        if (err)
            return err;
    }
    return ERR_BLOCK_UNCLOSED;
}

const char *_process_body(Src *s, char *file_name, int level, SymbolTable *st) {
    while (!src_eof(s)) {
        const char *err = _mtags(s, file_name, level, sibling, st);
        if (err)
            return err;
    }
    return ERR_SUCCESS;
}

const char *_mtags(Src *s, char *file_name, int level, int sibling_id, SymbolTable *st) {
    skip_ws(s);
    
    if (is_keyword_at(s, "var")) {
        s->pos += 3;
        Pos pos = get_pos(s);
        char *id = parse_ident(s);
        if (!id)
            return ERR_INVALID_IDENT;
        
        char *type_str = NULL;
        if (match_char(s, ':')) {
            skip_ws(s);
            if (src_peek(s) != '"')
                return ERR_EXPECTED_TYPE_ANNOTATION;
            Value *tmp = parse_string(s);
            type_str = mila_strdup(GET_STRING(tmp));
            val_release(tmp);
        } else {
            type_str = mila_strdup("any");
        }
        
        if (match_char(s, '=')) {
            size_t start = s->pos;
            const char *err = skip_parse_expr_prec(s, 1);
            if (err)
                return err;
            size_t len = s->pos - start;
            
            char *expr_text = malloc(len + 1);
            strncpy(expr_text, s->src + start, len);
            expr_text[len] = '\0';
            
            char *inferred = _infer_type(expr_text);
            
            if (!types_compatible(inferred, type_str)) {
                fprintf(stdout, "%s:%zu:%zu TYPE_ERROR var %s: expected %s got %s\n",
                        file_name, pos.line, pos.column, id, type_str, inferred);
            }
            
            mila_free(expr_text);
            mila_free(inferred);
        }
        
        symtable_add(st, id, type_str, level);
        mila_free(id);
        mila_free(type_str);
        return match_char(s, ';') ? ERR_SUCCESS : ERR_EXPECTED_SEMICOLON;
    }

    if (is_keyword_at(s, "const")) {
        s->pos += 5;
        Pos pos = get_pos(s);
        char *id = parse_ident(s);
        if (!id)
            return ERR_INVALID_IDENT;
        
        char *type_str = NULL;
        if (match_char(s, ':')) {
            skip_ws(s);
            if (src_peek(s) != '"')
                return ERR_EXPECTED_TYPE_ANNOTATION;
            Value *tmp = parse_string(s);
            type_str = mila_strdup(GET_STRING(tmp));
            val_release(tmp);
        } else {
            type_str = mila_strdup("any");
        }
        
        if (!match_char(s, '='))
            return "const requires assignment";
        
        size_t start = s->pos;
        const char *err = skip_parse_expr_prec(s, 1);
        if (err)
            return err;
        size_t len = s->pos - start;
        
        char *expr_text = malloc(len + 1);
        strncpy(expr_text, s->src + start, len);
        expr_text[len] = '\0';
        
        char *inferred = _infer_type(expr_text);
        
        if (!types_compatible(inferred, type_str)) {
            fprintf(stdout, "%s:%zu:%zu TYPE_ERROR const %s: expected %s got %s\n",
                    file_name, pos.line, pos.column, id, type_str, inferred);
        }
        
        symtable_add(st, id, type_str, level);
        mila_free(id);
        mila_free(type_str);
        mila_free(expr_text);
        mila_free(inferred);
        return match_char(s, ';') ? ERR_SUCCESS : ERR_EXPECTED_SEMICOLON;
    }

    if (is_keyword_at(s, "set")) {
        s->pos += 3;
        Pos pos = get_pos(s);
        skip_ws(s);
        
        char *id = parse_ident(s);
        if (!id)
            return ERR_INVALID_IDENT;
        
        Symbol *sym = symtable_lookup(st, id);
        if (!sym) {
            fprintf(stdout, "%s:%zu:%zu ERROR: undefined variable %s\n",
                    file_name, pos.line, pos.column, id);
            mila_free(id);
            return "undefined variable";
        }
        
        while (match_char(s, '[')) {
            skip_parse_expr_prec(s, 1);
            if (!match_char(s, ']'))
                return ERR_BRACKET_UNCLOSED;
        }
        
        skip_ws(s);
        
        if (src_peek(s) == '+' || src_peek(s) == '-' || src_peek(s) == '*' ||
            src_peek(s) == '/' || src_peek(s) == '%')
            src_get(s);
        
        if (!match_char(s, '='))
            return ERR_EXPECTED_EQUALS;
        
        size_t start = s->pos;
        const char *err = skip_parse_expr_prec(s, 1);
        if (err)
            return err;
        size_t len = s->pos - start;
        
        char *expr_text = malloc(len + 1);
        strncpy(expr_text, s->src + start, len);
        expr_text[len] = '\0';
        
        char *inferred = _infer_type(expr_text);
        
        if (!types_compatible(inferred, sym->type)) {
            fprintf(stdout, "%s:%zu:%zu TYPE_ERROR set %s: expected %s got %s\n",
                    file_name, pos.line, pos.column, id, sym->type, inferred);
        }
        
        match_char(s, ';');
        mila_free(id);
        mila_free(expr_text);
        mila_free(inferred);
        return ERR_SUCCESS;
    }

    if (is_keyword_at(s, "return")) {
        s->pos += 6;
        size_t start = s->pos;
        const char *err = skip_parse_expr_prec(s, 1);
        if (err)
            return err;
        size_t len = s->pos - start;
        
        char *expr_text = malloc(len + 1);
        strncpy(expr_text, s->src + start, len);
        expr_text[len] = '\0';
        
        char *inferred = _infer_type(expr_text);
        mila_free(expr_text);
        mila_free(inferred);
        
        return match_char(s, ';') ? ERR_SUCCESS : ERR_EXPECTED_SEMICOLON;
    }

    if (is_keyword_at(s, "if")) {
        s->pos += 2;
        if (!match_char(s, '('))
            return ERR_EXPECTED_PAREN;
        const char *err = skip_parse_expr_prec(s, 1);
        if (err)
            return err;
        if (!match_char(s, ')'))
            return ERR_PAREN_UNCLOSED;
        
        if (match_char(s, '{')) {
            s->pos--;
            err = _process_block(s, file_name, level, st);
            if (err)
                return err;
        } else {
            err = skip_parse_statement(s);
            if (err)
                return err;
        }
        
        while (is_keyword_at(s, "elif")) {
            s->pos += 4;
            if (!match_char(s, '('))
                return ERR_EXPECTED_PAREN;
            err = skip_parse_expr_prec(s, 1);
            if (err)
                return err;
            if (!match_char(s, ')'))
                return ERR_PAREN_UNCLOSED;
            if (match_char(s, '{')) {
                s->pos--;
                err = _process_block(s, file_name, level, st);
                if (err)
                    return err;
            } else {
                err = skip_parse_statement(s);
                if (err)
                    return err;
            }
        }
        
        if (is_keyword_at(s, "else")) {
            s->pos += 4;
            if (match_char(s, '{')) {
                s->pos--;
                err = _process_block(s, file_name, level, st);
                if (err)
                    return err;
            } else {
                err = skip_parse_statement(s);
                if (err)
                    return err;
            }
        }
        return ERR_SUCCESS;
    }

    if (is_keyword_at(s, "while")) {
        s->pos += 5;
        if (!match_char(s, '('))
            return ERR_EXPECTED_PAREN;
        const char *err = skip_parse_expr_prec(s, 1);
        if (err)
            return err;
        if (!match_char(s, ')'))
            return ERR_PAREN_UNCLOSED;
        
        if (match_char(s, '{')) {
            s->pos--;
            return _process_block(s, file_name, level, st);
        }
        return skip_parse_statement(s);
    }

    if (is_keyword_at(s, "foreach")) {
        s->pos += 7;
        char *id = parse_ident(s);
        if (!id)
            return ERR_INVALID_IDENT;
        
        if (!match_char(s, ':'))
            return ERR_EXPECTED_COLON;
        
        const char *err = skip_parse_expr_prec(s, 1);
        if (err)
            return err;
        
        symtable_add(st, id, "any", level + 1);
        mila_free(id);
        
        if (match_char(s, '{')) {
            s->pos--;
            return _process_block(s, file_name, level, st);
        }
        return skip_parse_statement(s);
    }

    if (is_keyword_at(s, "fn")) {
        s->pos += 2;
        char *name = parse_ident(s);
        if (!name)
            return ERR_INVALID_IDENT;
        mila_free(name);
        
        FunctionParameters *params = parse_param_list(s);
        if (!params)
            return ERR_INVALID_PARAM_LIST;
        
        for (int i = 0; params->params[i]; i++) {
            mila_free(params->params[i]);
            if (params->defaults[i])
                mila_free(params->defaults[i]);
            if (params->types[i])
                mila_free(params->types[i]);
        }
        
        mila_free(params->params);
        mila_free(params->defaults);
        mila_free(params->types);
        mila_free(params);
        
        if (is_keyword_at(s, "->")) {
            s->pos += 2;
            skip_ws(s);
            if (src_peek(s) == '"')
                val_release(parse_string(s));
        }
        
        if (match_char(s, '{')) {
            s->pos--;
            return _process_block(s, file_name, level, st);
        }
        return skip_parse_statement(s);
    }

    if (is_keyword_at(s, "object")) {
        s->pos += 6;
        char *name = parse_ident(s);
        if (!name)
            return ERR_INVALID_IDENT;
        mila_free(name);
        
        if (is_keyword_at(s, "with")) {
            s->pos += 4;
            char *obj = parse_ident(s);
            if (!obj)
                return ERR_INVALID_IDENT;
            mila_free(obj);
        }
        return _process_block(s, file_name, level, st);
    }

    if (is_keyword_at(s, "catch")) {
        s->pos += 5;
        char *cid = parse_ident(s);
        if (!cid)
            return ERR_INVALID_IDENT;
        mila_free(cid);
        return _process_block(s, file_name, level, st);
    }
    
    if (src_peek(s) == '{') {
        return _process_block(s, file_name, level, st);
    }

    const char *err = skip_parse_expr_prec(s, 1);
    if (err)
        return err;
    match_char(s, ';');
    return ERR_SUCCESS;
}

const char *mtags(Src *s, char *file_name, SymbolTable *st) {
    skip_ws(s);
    if (src_peek(s) == '#') {
        src_get(s);
        if (src_get(s) != '!')
            return "Invalid shebang";
        while (!src_eof(s) && src_get(s) != '\n')
            ;
    }
    return _process_body(s, file_name, 0, st);
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Usage: %s [FILE]...\n", argv[0]);
        return 0;
    }
    
    SymbolTable *st = symtable_new();
    
    for (int i = 1; i < argc; ++i) {
        char *file = read_file(argv[i]);
        if (!file) {
            fprintf(stderr, "File %s does not exist\n", argv[i]);
            return 1;
        }
        Src *src = src_new(file);
        const char *err = mtags(src, argv[i], st);
        if (err) {
            size_t line = 1, col = 0, line_start = 0;
            for (size_t i = 0; i < src->pos && i < src->len; ++i) {
                if (src->src[i] == '\n') {
                    line++;
                    line_start = i + 1;
                    col = 0;
                } else {
                    col++;
                }
            }
            size_t line_end = line_start;
            while (line_end < src->len && src->src[line_end] != '\n')
                line_end++;

            fprintf(stderr, "In %s at line %zu, col %zu: %s\n", argv[i], line, col, err);
            fprintf(stderr, "  `");
            for (size_t i = line_start; i < line_end && i < 60; ++i)
                fprintf(stderr, "%c", src->src[i]);
            fprintf(stderr, "`\n");
            return 1;
        }
        src_free(src);
        mila_free(file);
    }
    
    return 0;
}