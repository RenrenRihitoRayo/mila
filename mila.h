#pragma once

#include "ml_paths.c"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_METHODS 100
#define MAX_NUMBER_DIGITS 1000
#define MAX_PATH_LENGTH 10000

#define HANDLE_CONTROL(val) \
    {\
        if (!val) return val;\
        if (val->type == T_BREAK)\
            return val;\
        if (val->type == T_CONTINUE)\
            return val;\
        if (val->type == T_RETURN)\
        {\
            Value *res = (Value*)val->v.opaque;\
            val_release(val);\
            return res;\
        }\
        return val;\
    }\

#define IS_CONTROL(v) (v && (v->type == T_BREAK || v->type == T_CONTINUE || v->type == T_RETURN))

path_list *search_path;

typedef struct Value Value;
typedef struct Env Env;
typedef Value *(*NativeFn)(Env *env, int argc, Value **argv);
typedef char *(*Printer)(Value *self);

typedef struct
{
    char *name;
    NativeFn func;
} NativeEntry;

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
    T_NONE,
    T_ERROR,
    T_BREAK,
    T_CONTINUE,
    T_ARG_END
} ValueType;

const char *MILA_TYPE_NAMES[] = {
    "null",
    "int",
    "float",
    "string",
    "bool",
    "function",
    "native",
    "opaque",
    "return",
    "none",
    "error",
    "break",
    "continue",
    "arg_end",
};

typedef enum __attribute__((packed))
{
    BMethodAdd,
    BMethodSub,
    BMethodMul,
    BMethodDiv,
    BMethodMod,
    BMethodLshift,
    BMethodRshift,
    BMethodLE,
    BMethodGE,
    BMethodLess,
    BMethodGreat,
    BMethodEq,
    BMethodNe,
    BMethodAnd,
    BMethodOr,
    BMethodDefault,
    BMethodGetItem,
    BMethodSetItem,
    BMethodTypeCount
} BMethodType;

typedef enum __attribute__((packed))
{
    UMethodFree = BMethodTypeCount,
    UMethodKill,
    UMethodToString,
    UMethodToRepr,
    UMethodToIter,
    MethodTotalCount
} UMethodType;

typedef Value *(*bin_op_method)(Value *self, Value *other);
typedef Value *(*unary_op_method)(Value *self);

typedef struct
{
    _Bool is_binop;
    union {
        bin_op_method binop;
        unary_op_method unary;
    };
} MethodTable;

typedef struct
{
    char **params;  // NULL-terminated
    char *body_src; // pointer to function body source (we'll keep a copy)
    // For evaluation we keep source pointer and we need the position. We'll parse/eval at call-time.
    char *name;
    Env *closure; // closure environment
} FunctionV;

typedef struct
{
    NativeFn fn;
    void *userdata;
    char *name;
} NativeFunctionV;

// Primitives are <50 bytes gauranteed.
// worst case is 300+ Bytes (especially variables with methods
struct Value
{
    MethodTable *method_table; // 8 bytes ptr
    char *type_name;           // 8 bytes ptr
    ValueType type;            // 4 bytes
    int refcount;              // simple refcount (4 bytes)
    char owns_table;           // check if table can be freed or not (1 bytes)
    union {
        char *s;
        char *message;
        _Bool b;
        double f;
        void *opaque;
        long i;
        // function
        FunctionV *fn;
        NativeFunctionV *native;
    } v; // around 8 bytes
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
void env_kill(Env *e);
Value *env_get(Env *e, const char *name);
void env_set_local(Env *e, const char *name, Value *val);
int env_set(Env *e, const char *name, Value *val);
void env_set_local_raw(Env *e, const char *name, Value *val);
int env_set_raw(Env *e, const char *name, Value *val);
void env_register_native(Env *env, const char *name, NativeFn fn);
void env_register_builtins(Env *g);

// == Value Related

// When writing your own mila kernel
// these functions might be the only
// part of mila youll ever touch.

int is_truthy(Value *value);
Value *val_new(ValueType t);
void val_allocate_table(Value *v);
MethodTable *val_make_table(void);
void val_set_table(Value *v, MethodTable *t);
void val_set_bmethod(Value *v, BMethodType t, bin_op_method func);
void val_set_umethod(Value *v, UMethodType t, unary_op_method func);
void val_set_bmethod_table(MethodTable *v, BMethodType t, bin_op_method func);
void val_set_umethod_table(MethodTable *v, UMethodType t, unary_op_method func);
void val_unset_bmethod_table(MethodTable *v, BMethodType t);
void val_unset_umethod_table(MethodTable *v, UMethodType t);
void val_unset_method(Value *v, BMethodType t);
Value *val_retain(Value *v);
void val_release(Value *v);
void val_kill(Value *v);
Value *vint(long x);
Value *vfloat(double f);
Value *vbool(int b);
Value *vstring_dup(const char *s);
Value *vstring_take(char *s);
Value *vopaque(void *p);
Value *vnative(NativeFn fn, const char *name);
Value *vtruthy(Value *value);
Value *vnull();
Value *vnone();
Value *verror(char *message, ...);
Value *vfunction(char **params, char *body_src, Env *closure);
int is_number(Value *v);
double to_double(Value *v);
char *as_c_string(Value *v);
Value *to_c_string(Value *v);
char *as_c_string_repr(Value *v);
void print_value(Value *v);
Value *call_function_with(Env *env, Value *fnval, Value *first, ...);
Value *vopaque_extra(void *p, Value *(*dis)(Value *), const char *type);

// == Parsing

/*
 * I suggest you also look in mila.c
 * in how these functions are used
 */

typedef struct Src
{
    char *src;    // full source string (null-terminated)
    uint64_t pos; // current position
    int line;
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
Value *binary_op(Value *a, BMethodType op, Value *b);
int precedence_of(BMethodType op);
BMethodType parse_op(Src *s);
Value *eval_expr_prec(Src *s, Env *env, int min_prec);
Value *eval_expr(Src *s, Env *env);
Value *eval_statement_fn(Src *s, Env *env);
Value *eval_statement(Src *s, Env *env);

// == Helpers
int our_asprintf(char **strp, const char *fmt, ...);
Value *call_function(Value *fnval, Env *env, int argc, Value **argv);
int match_types(Value **args, ...);
Value *eval_source(Src *s, Env *env);
Value *eval_str(char *src, Env *env);
int run_file(char *name, Env *env);
double get_unix_timestamp();
char *read_input(void);
int load_library(Env *env, const char *libpath);
