#pragma once

#include "ml_paths.c"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_METHODS 100
#define MAX_NUMBER_DIGITS 250
#define MAX_PATH_LENGTH 1028

// MiLa produces an alternative string representation
// when collections' item amounts exceed this number
#define MAX_ITEMS_DISPLAYED 1000

#define IS_ERROR(v) (MILA_GET_TYPE(v) == T_ERROR || MILA_GET_TYPE(v) == T_TAGGED_ERROR)
#define IS_FATAL(v) (IS_ERROR(v) && MILA_GET_ERROR(v) == E_FATAL)
#define GET_STRING(val) (val ? val->v.s : NULL)
#define GET_INTEGER(val) (val ? val->v.i : 0)
#define GET_UINTEGER(val) (val ? val->v.ui : 0)
#define GET_FLOAT(val) (val ? val->v.f : 0.0)
#define GET_OPAQUE(val) (val ? val->v.opaque : NULL)
#define GET_FUNCTION(val) (val ? val->v.fn : NULL)
#define GET_NATIVE(val) (val ? val->v.native : NULL)
#define GET_ERROR(val) (val ? val->v.message : NULL)
#define GET_ERROR_TAGGED(val) (val ? val->v.tagged_error.message : NULL)
#define OWNED(val) (val->type = T_OWNED_OPAQUE)
#define UNOWNED(val) (val->type = T_OPAQUE)
#define WEAK(val) (val->type = T_WEAK_OPAQUE)

#define MILA_GET_ERRORNAME(val) (val ? (val->type == T_TAGGED_ERROR ? MILA_ERROR_NAMES[val->v.tagged_error.type] : "???" ) : "???")
#define MILA_GET_TYPENAME(v) (v ? (v->type_name ? v->type_name : MILA_TYPE_NAMES[v->type] ) : "???")
#define MILA_GET_ERROR(val) (IS_ERROR(val) ? val->v.tagged_error.type : E_GENERIC)
#define MILA_GET_TYPE(v) (v ? v->type : -1 )

#define HANDLE_RETURN(val)  { if (val && val->type == T_RETURN) {Value* tmp = val->v.opaque; val_release(val); return tmp; } }

#define HANDLE_CONTROL(val) \
    {\
        if (!val) return val;\
        if (val->type == T_BREAK)\
            return val;\
        if (val->type == T_CONTINUE)\
            return val;\
        return val;\
    }\

#define HANDLE_CONTROL_LOOP(val) \
    {\
        if (!val) return val;\
        if (val->type == T_BREAK)\
            return val;\
        if (val->type == T_CONTINUE)\
            return val;\
        if (val->type == T_RETURN)\
        {\
            Value *res = (Value*)val->v.opaque;\
            val_kill(val);\
            return res;\
        }\
    }\

#define IS_CONTROL(v) (v && (v->type == T_BREAK || v->type == T_CONTINUE || v->type == T_RETURN))

typedef struct path_list path_list;
path_list *search_path = NULL;

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
    E_PRE_RUNTIME, // Must always be fatal!
    E_RUNTIME,     // Errors such as undefined variables
    E_TYPE_ERROR,  // Errors when doing a type cannot do (impossible in core mila, invalid op == null)
    E_FATAL,       // Errors that should be fatal, like syntax errors
    E_GENERIC      // Errors that cannot be classified as ones above
} ErrorType;

const char *MILA_ERROR_NAMES[] = {
    "PreRuntime",
    "Runtime",
    "TypeError",
    "Fatal",
    "Generic"
};

typedef enum
{
    T_NULL = 0,
    T_INT,
    T_UINT,
    T_FLOAT,
    T_STRING,
    T_BOOL,
    T_FUNCTION,
    T_NATIVE,
    T_OPAQUE,
    T_OWNED_OPAQUE,
    T_WEAK_OPAQUE,
    T_RETURN,
    T_NONE,
    T_ERROR,
    T_BREAK,
    T_CONTINUE,
    T_TAGGED_ERROR,
    T_ARG_END
} ValueType;

// Simple trick
const char *MILA_TYPE_NAMES[] = {
    "null",
    "int",
    "uint",
    "float",
    "string",
    "bool",
    "function",
    "native",
    "opaque",
    "owned_opaque",
    "weak_opaque",
    "return",
    "none",
    "error",
    "break",
    "continue",
    "tagged_error",
    "arg_end",
};

// each of these methods may be reffered to as
// type<method name>
// like array<UMethodToIter>
// or simple UMethodToIter
typedef enum __attribute__((packed))
{
    MethodNone = -1,
    // value op value syntax
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

    BMethodGetItem, // name[...] syntax
    TMethodSetItem, // var name[...] or set name[...] syntax

    // when converting objects into strings
    UMethodToString,
    UMethodToRepr,

    // foreach syntax
    UMethodToIter,

    UMethodFree,
    UMethodKill,

    MethodTotalCount
} MethodType;

typedef Value *(*trinary_method)(Value *self, Value *b, Value* c);
typedef Value *(*binary_method)(Value *self, Value *other);
typedef Value *(*unary_method)(Value *self);

typedef void* MethodTable;

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
        unsigned long ui;
        // function
        FunctionV *fn;
        NativeFunctionV *native;
        struct {
            char* message;
            ErrorType type;
        } tagged_error;
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
void env_dump(Env *e);
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
void val_set_method(Value *v, MethodType t, void* func);
void val_set_method_table(MethodTable *v, MethodType t, void* func);
void val_unset_method_table(MethodTable *v, MethodType t);
void val_unset_method(Value *v, MethodType t);
Value *val_retain(Value *v);
void val_release(Value *v);
__attribute__((format(printf, 1, 2)))
Value *vstring_fmt(char *fmt, ...);
Value *vstring_slice(const char *src, size_t start, size_t len);
Value *vstring_index(const char *src, size_t index);
Value *vstring_replace(const char *src,
                       const char *needle,
                       const char *repl);
void val_kill(Value *v);
Value *vint(long i);
Value *vuint(unsigned long i);
Value *vfloat(double f);
Value *vbool(int b);
Value *vstring_dup(const char *s);
Value *vstring_take(char *s);
Value *vopaque(void *p);
Value *vweak_opaque(void *p);
Value *vowned_opaque(void *p);
Value *vnative(NativeFn fn, const char *name);
Value *vtruthy(Value *value);
Value *vnull();
Value *vnone();
__attribute__((format(printf, 1, 2)))
Value *verror(char *message, ...);
Value *vtagged_error(ErrorType type, char *message, ...);
Value *vfunction(char **params, char *body_src, Env *closure);
static int is_number(Value *v);
double to_double(Value *v);
Value *to_c_string(Value *v);
char *as_c_string(Value *v);
char *as_c_string_repr(Value *v);
char *as_c_string_raw(Value *v);
char *as_c_string_repr_raw(Value *v);
int print_value(Value *v);
int print_value_fancy(Value *v);
int print_value_repr(Value *v);
Value *call_function_with(Env *env, Value *fnval, Value *first, ...);
Value *vopaque_extra(void *p, Value *(*dis)(Value *), const char *type);
Value *vowned_opaque_extra(void *p, Value *(*dis)(Value *), const char *type);

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
void src_advance_by(Src* s, size_t amount);
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
Value *binary_op(Value *a, MethodType op, Value *b);
int precedence_of(MethodType op);
MethodType parse_op(Src *s);
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
Value* run_file_keep_res(char *name, Env *env);
double get_unix_timestamp();
char *read_input(void);
int load_library(Env *env, const char *libpath);
void mila_add_atexit(Value* fn);

// Initialize a minimal environment for a MiLa script
// This will automatically inject built ins
Env* mila_init(void);
void mila_deinit(Env* env);

#ifndef MILA_CUSTOM
void* mila_malloc(size_t size) {
    void* ptr = malloc(size);
    memset(ptr, 0, size);
    return ptr;
}
void* mila_realloc(void* ptr, size_t size) {
    return realloc(ptr, size);
}
void mila_free(void* ptr) {
    free(ptr);
}
#else
#include "ml_alloc.c"
#endif
