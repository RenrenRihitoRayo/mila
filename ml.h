#pragma once

typedef struct Value Value;
typedef struct Env Env;
typedef Value *(*NativeFn)(Env *env, uint64_t line_pos, int argc, Value **argv);
typedef char *(*Printer)(Value *self);

typedef enum
{
    T_NULL,
    T_INT,
    T_FLOAT,
    T_STRING,
    T_BOOL,
    T_FUNCTION,
    T_NATIVE,
    T_OPAQUE,
    T_RETURN,
    T_ARG_END,
    T_NONE
} ValueType;

struct Value
{
    char type_name[50];
    ValueType type;
    int refcount; // simple refcount
    union {
        long i;
        double f;
        char *s;
        _Bool b;
        // function
        struct
        {
            char **params;  // NULL-terminated
            char *body_src; // pointer to function body source (we'll keep a copy)
            // For evaluation we keep source pointer and we need the position. We'll parse/eval at call-time.
            char *name;
            Env *closure; // closure environment
        } fn;
        struct
        {
            NativeFn fn;
            void *userdata;
            char *name;
        } native;
        void *opaque;
    } v;
    Printer display;
};

// == Environment
typedef struct Var
{
    char *name;
    Value *value;
    struct Var *next;
} Var;

struct Env
{
    Var *vars;
    Env *parent;
};

Env *env_new(Env *parent);
void env_free(Env *e);
Value *env_get(Env *e, const char *name);
void env_set_local(Env *e, const char *name, Value *val);
int env_set(Env *e, const char *name, Value *val);
void env_register_native(Env *env, const char *name, NativeFn fn);
void env_register_builtins(Env *g);

// == Parsing

/*
 * I suggest you also look in ml.c
 * in how these functions are used
 */

typedef struct Src
{
    char *src;    // full source string (null-terminated)
    uint64_t pos; // current position
    int len;
} Src;

Src *src_new(const char *s);
void src_free(Src *s);
void skip_ws(Src *s);
char src_peek(Src *s);
char src_get(Src *s);
int src_eof(Src *s);
int is_ident_start(char c);
uint64_t get_line_pos(Src *src);
int report(Src *src, FILE *fp, const char *fmt, ...);
int match_char(Src *s, char c);
char *parse_ident(Src *s);
Value *parse_number(Src *s);
Value *parse_string(Src *s);
int is_keyword_at(Src *s, const char *kw);
char *dup_substr(Src *s, int a, int b);
char **parse_param_list(Src *s);
Value *eval_block(Src *s, Env *env);
Value *eval_primary(Src *s, Env *env);
Value *binary_op(Value *a, const char *op, Value *b);
int precedence_of(const char *op);
char *parse_op(Src *s);
Value *eval_expr_prec(Src *s, Env *env, int min_prec);
Value *eval_expr(Src *s, Env *env);
Value *eval_statement(Src *s, Env *env);

// == Value Related
int is_truthy(Value *value);
Value *val_new(ValueType t);
Value *val_retain(Value *v);
void val_release(Value *v);
Value *vint(long x);
Value *vfloat(double f);
Value *vbool(int b);
Value *vstring_dup(const char *s);
Value *vstring_take(char *s);
Value *vopaque(void *p);
Value* vopaque_extra(void* p, Printer dis, const char* type);
Value *vnative(NativeFn fn, const char *name);
Value *vtruthy(Value *value);
Value *vnull();
Value *vnone();
Value *vfunction(char **params, char *body_src, Env *closure);

// == Helpers
int our_asprintf(char **strp, const char *fmt, ...);
char *as_c_string(Value *v);
Value *to_c_string(Value *v);
char *as_c_string_repr(Value *v);
void print_value(Value *v);
Value *call_function(Value *fnval, uint64_t line_pos, Env *env, int argc, Value **argv);
int is_number(Value *v);
double to_double(Value *v);
int match_types(Value **args, ...);
Value *eval_source(Src *s, Env *env);
Value *eval_str(char *src, Env *env);
int run_file(char *name, Env *env);
double get_unix_timestamp();
char *read_input(void);
int load_library(Env* env, const char *libpath);

// Gaurd to avoid duplication of main
#ifndef ML_LIB
int main(int argc, char **argv);
#endif