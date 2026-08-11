#define ML_NO_MAIN
#include "mila.c"
#include "ml_commons.h"

typedef struct {
    size_t line, col;
} Pos;

static Pos _get_pos(Src *s) {
    size_t line = 1, col = 0;
    size_t line_start = 0;

    for (size_t i = 0; i < s->pos && i < s->len; ++i) {
        if (s->src[i] == '\n') {
            line++;
            line_start = i + 1;
            col = 0;
        } else {
            col++;
        }
    }
    return (Pos){line, col};
}

const char *_mtags(Src *s, char *file_name, char **buffer, int level);
const char *_process_block(Src *s, char *file_name, char **buffer, int level) {
    if (!match_char(s, '{'))
        return ERR_EXPECTED_BRACE;

    while (!src_eof(s)) {
        skip_ws(s);
        if (match_char(s, '}'))
            return ERR_SUCCESS;
        const char *err = _mtags(s, file_name, buffer, level + 1);
        if (err)
            return err;
    }
    return ERR_BLOCK_UNCLOSED;
}

const char *_process_body(Src *s, char *file_name, char **buffer, int level) {
    while (!src_eof(s)) {
        const char *err = _mtags(s, file_name, buffer, level);
    }
    return ERR_SUCCESS;
}
const char *_mtags(Src *s, char *file_name, char **buffer, int level) {
    skip_ws(s);
    if (is_keyword_at(s, "var")) {
        s->pos += 3;
        Pos pos = _get_pos(s);
        char *id = parse_ident(s);
        if (!id)
            return ERR_INVALID_IDENT;
        char *type_str = NULL;
        size_t len = 0;
        char *assign = NULL;
        if (match_char(s, ':')) {
            skip_ws(s);
            if (src_peek(s) != '"')
                return ERR_EXPECTED_TYPE_ANNOTATION;
            Value *tmp = parse_string(s);
            type_str = mila_strdup(GET_STRING(tmp));
            val_release(tmp);
        }
        size_t start = s->pos;
        assign = s->src + s->pos;
        if (match_char(s, '=')) {
            const char *err = skip_parse_expr_prec(s, 1);
            if (err)
                return err;
            len = s->pos - start;
        } else {
            assign = "";
            len = 0;
        }
        malloc_sprintf(
            buffer, "%s:%zu:%zu %i var \"%s\" %s var %s: \"%s\" %.*s;\n",
            file_name, pos.line, pos.col, level, type_str ? type_str : "any",
            id, id, type_str ? type_str : "any", len, assign);
        mila_free(id);
        return match_char(s, ';') ? ERR_SUCCESS : ERR_EXPECTED_SEMICOLON;
    }

    if (is_keyword_at(s, "set")) {
        s->pos += 3;
        Pos pos = _get_pos(s);
        skip_ws(s);
        size_t id_start = s->pos;
        char *id = parse_ident(s);
        if (!id)
            return ERR_INVALID_IDENT;
        while (match_char(s, '[')) {
            const char *err = skip_parse_expr_prec(s, 1);
            if (err)
                return err;
            if (!match_char(s, ']'))
                return ERR_BRACKET_UNCLOSED;
        }
        size_t id_len = s->pos - id_start;
        skip_ws(s);
        size_t start = s->pos;
        char *assign = s->src + s->pos;
        if (src_peek(s) == '+' || src_peek(s) == '-' || src_peek(s) == '*' ||
            src_peek(s) == '/' || src_peek(s) == '%')
            src_get(s);
        if (!match_char(s, '='))
            return ERR_EXPECTED_EQUALS;
        const char *err = skip_parse_expr_prec(s, 1);
        if (err)
            return err;
        match_char(s, ';');
        size_t len = s->pos - start;
        malloc_sprintf(buffer,
                       "%s:%zu:%zu %i set \"any\" %s set %.*s: \"any\" %.*s\n",
                       file_name, pos.line, pos.col, level, id, id_len,
                       id_start + s->src, len, assign);
        mila_free(id);
        return ERR_SUCCESS;
    }

    if (is_keyword_at(s, "return")) {
        s->pos += 6;
        const char *err = skip_parse_expr_prec(s, 1);
        if (err)
            return err;
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
            err = _process_block(s, file_name, buffer, level);
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
                err = _process_block(s, file_name, buffer, level);
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
                err = _process_block(s, file_name, buffer, level);
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
            return _process_block(s, file_name, buffer, level);
        }
        return skip_parse_statement(s);
    }

    if (is_keyword_at(s, "foreach")) {
        s->pos += 7;
        char *id = parse_ident(s);
        if (!id)
            return ERR_INVALID_IDENT;
        mila_free(id);
        if (!match_char(s, ':'))
            return ERR_EXPECTED_COLON;
        const char *err = skip_parse_expr_prec(s, 1);
        if (err)
            return err;
        if (match_char(s, '{')) {
            s->pos--;
            return _process_block(s, file_name, buffer, level);
        }
        return skip_parse_statement(s);
    }

    if (is_keyword_at(s, "fn")) {
        s->pos += 2;
        Pos pos = _get_pos(s);
        char *name = parse_ident(s);
        if (!name)
            return ERR_INVALID_IDENT;
        FunctionParameters *params = parse_param_list(s);
        if (!params)
            return ERR_INVALID_PARAM_LIST;
        char *p_str = NULL;
        for (int i = 0; params->params[i]; i++) {
            malloc_sprintf(&p_str, "%s", params->params[i]);
            mila_free(params->params[i]);
            if (params->types[i]) {
                malloc_sprintf(&p_str, ": \"%s\"", params->types[i]);
                mila_free(params->types[i]);
            } else {
                malloc_sprintf(&p_str, ": \"any\"");
            }
            if (params->defaults[i]) {
                mila_free(params->defaults[i]);
            }
            if (params->params[i + 1])
                malloc_sprintf(&p_str, ", ");
        }
        mila_free(params->params);
        mila_free(params->defaults);
        mila_free(params->types);
        mila_free(params);

        char **ctx = parse_context_list(s);
        if (ctx) {
            for (int i = 0; ctx[i]; i++)
                mila_free(ctx[i]);
            mila_free(ctx);
        }

        if (match_char(s, ':')) {
            char **closure = parse_context_list(s);
            if (closure) {
                for (int i = 0; closure[i]; i++)
                    mila_free(closure[i]);
                mila_free(closure);
            }
        }

        char *ret_type = NULL;
        if (is_keyword_at(s, "->")) {
            s->pos += 2;
            skip_ws(s);
            if (src_peek(s) == '"') {
                Value *tmp = parse_string(s);
                ret_type = mila_strdup(GET_STRING(tmp));
                val_release(tmp);
            }
        }

        malloc_sprintf(
            buffer, "%s:%zu:%zu %i fn \"%s\" %s fn %s(%s) -> \"%s\"\n",
            file_name, pos.line, pos.col, level, ret_type ? ret_type : "any",
            name, name, p_str, ret_type ? ret_type : "any");
        mila_free(name);
        mila_free(ret_type);
        mila_free(p_str);

        if (match_char(s, '{')) {
            s->pos--;
            return _process_block(s, file_name, buffer, level);
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
        return _process_block(s, file_name, buffer, level);
    }

    if (is_keyword_at(s, "catch")) {
        s->pos += 5;
        char *cid = parse_ident(s);
        if (!cid)
            return ERR_INVALID_IDENT;
        mila_free(cid);
        return _process_block(s, file_name, buffer, level);
    }
    if (is_keyword_at(s, "alias")) {
        s->pos += 5;
        char *cid = parse_ident(s);
        if (!cid)
            return ERR_INVALID_IDENT;
        match_char(s, ':');
        const char *expr_id = skip_parse_expr_prec(s, 1);
        mila_free(cid);
        return match_char(s, ';') ? expr_id : ERR_EXPECTED_SEMICOLON;
    }
    if (src_peek(s) == '{') {
        return _process_block(s, file_name, buffer, level);
    }

    if (src_peek(s) == '@') {
        src_get(s); // consume '@'

        // Parse the function identifier
        char *id = parse_ident(s);
        if (!id) {
            return ERR_INVALID_IDENT;
        }
        mila_free(id);

        // Parse arguments until semicolon
        const char *err = ERR_SUCCESS;
        while ((err == ERR_SUCCESS) && !match_char(s, ';')) {
            skip_ws(s);
            if (src_peek(s) == '\0') {
                return ERR_EXPECTED_SEMICOLON;
            }
            if (src_peek(s) == '{') {
                err = _process_block(s, file_name, buffer, level);
            } else {
                err = skip_parse_expr(s);
            }
        }
        return ERR_SUCCESS;
    }

    const char *err = skip_parse_expr_prec(s, 1);
    if (err)
        return err;
    match_char(s, ';');
    return ERR_SUCCESS;
}

const char *mtags(Src *s, char *file_name, char **buffer) {
    skip_ws(s);
    // Skip shebang: #!/path/to/mila
    if (src_peek(s) == '#') {
        src_get(s);
        if (src_get(s) != '!')
            return "Invalid shebang";
        while (!src_eof(s) && src_get(s) != '\n')
            ;
    }

    skip_ws(s);

    // Parse optional main function: !fn(args) -> "type" { body }
    if (src_peek(s) == '!') {
        src_get(s);
        if (is_keyword_at(s, "fn")) {
            s->pos += 2;
            FunctionParameters *fnp = parse_param_list(s);
            if (!fnp)
                return ERR_INVALID_PARAM_LIST;
            for (int i = 0; fnp->params[i]; i++) {
                mila_free(fnp->params[i]);
                if (fnp->defaults[i])
                    mila_free(fnp->defaults[i]);
                if (fnp->types[i])
                    mila_free(fnp->types[i]);
            }
            mila_free(fnp->params);
            mila_free(fnp->defaults);
            mila_free(fnp->types);
            mila_free(fnp);

            // Optional return type annotation
            if (is_keyword_at(s, "->")) {
                s->pos += 2;
                skip_ws(s);
                if (src_peek(s) != '"')
                    return ERR_EXPECTED_TYPE_ANNOTATION;
                val_release(parse_string(s));
            }
        }
    }
    return _process_body(s, file_name, buffer, 0);
}

int main(int argc, char *argv[]) {
    char *buffer = NULL;
    for (int i = 1; i < argc; ++i) {
        char *tmp_buffer = NULL;
        malloc_sprintf(&tmp_buffer, "# currently in %s\n", argv[i]);
        char *file = read_file(argv[i]);
        if (!file) {
            fprintf(stderr, "File %s does not exist\n", argv[i]);
            return 1;
        }
        Src *src = src_new(file);
        const char *err = mtags(src, argv[i], &tmp_buffer);
        if (err) {
            size_t line = 1, col = 0;
            size_t line_start = 0;

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

            fprintf(stderr, "In %s\nSyntax Error at line %zu, column %zu:\n",
                    argv[i], line, col);
            fprintf(stderr, "  %s\n", err);
            fprintf(stderr, "  `");
            for (size_t i = line_start; i < line_end && i < 60; ++i)
                fprintf(stderr, "%c", src->src[i]);
            fprintf(stderr, "`\n  ");
            for (size_t i = 1; i < col - 1 && i < 60; ++i)
                fprintf(stderr, " ");
            fprintf(stderr, "^\n");
            return 1;
        }
        src_free(src);
        mila_free(file);
        if (!tmp_buffer)
            malloc_sprintf(&tmp_buffer, "# nothing notable in %s\n", argv[i]);
        malloc_sprintf(&buffer, "%s", tmp_buffer);
    }
    printf("%s", buffer);
}