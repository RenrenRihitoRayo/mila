#pragma once

#include "ml_paths.c"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_METHODS 100
#define MAX_NUMBER_DIGITS 250
#define MAX_PATH_LENGTH 1028

// MiLa produces an alternative string representation
// when collections' item amounts exceed this number
#define MAX_ITEMS_DISPLAYED 1000

#define IS_ERROR(v) (MILA_GET_TYPE(v) == T_ERROR || MILA_GET_TYPE(v) == T_TAGGED_ERROR)
#define IS_ERROR_TAGGED(v) (MILA_GET_TYPE(v) == T_TAGGED_ERROR)
#define IS_FATAL(v) ((MILA_GET_ERROR(v) == E_FATAL || MILA_GET_ERROR(v) == E_SYNTAX_ERROR))
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
#define MILA_GET_ERROR(val) (IS_ERROR_TAGGED(val) ? val->v.tagged_error.type : E_GENERIC)
#define MILA_GET_TYPE(v) (v ? v->type : -1 )

#define HANDLE_RETURN(val)  { if (val && val->type == T_RETURN) {Value* tmp = val->v.opaque; val_release(val); return tmp; } }

#define HANDLE_CONTROL(val) \
    {\
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
    E_SYNTAX_ERROR, // Self explanatory
    E_PRE_RUNTIME,  // Must always be fatal!
    E_RUNTIME,      // Errors such as undefined variables
    E_TYPE_ERROR,   // Errors when doing a type cannot do (impossible in core mila, invalid op == null)
    E_FATAL,        // Errors that should be fatal, like syntax errors
    E_GENERIC       // Errors that cannot be classified as ones above
} ErrorType;

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

typedef struct path_list path_list;
#ifndef MILA_PROTO
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

const char *MILA_ERROR_NAMES[] = {
    "SyntaxError",
    "PreRuntime",
    "Runtime",
    "TypeError",
    "Fatal",
    "Generic"
};

path_list *search_path;

#else
extern path_list *search_path;
extern char **MILA_TYPE_NAMES;
extern char **MILA_ERROR_NAMES;

#endif

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

    BMethodGlob,
    MethodTotalCount
} MethodType;

typedef Value *(*trinary_method)(Value *self, Value *b, Value* c);
typedef Value *(*binary_method)(Value *self, Value *other);
typedef Value *(*unary_method)(Value *self);

typedef void* MethodTable;

typedef struct
{
    char **params;  // NULL-terminated
    char **contextuals; // NULL_terminated
    char *body_src; // pointer to function body source (we'll keep a copy)
    // For evaluation we keep source pointer and we need the position. We'll parse/eval at call-time.
    char *name;
    Env* closure;
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
    Var *contextual_vars;
    Env *parent;
};

// Make an environment
Env *env_new(Env *parent);
// Print environment info
void env_dump(Env *e);
// Free an environment and disown variables
void env_free(Env *e);
// Free an environment and ensure vriables are freed
void env_kill(Env *e);
// Get a variable
Value *env_get(Env *e, const char *name);
// Set a variable in the local scope (and own it)
void env_set_local(Env *e, const char *name, Value *val);
// Set a variable, if no outer bindings are found, set it in the local scope (and own it)
int env_set(Env *e, const char *name, Value *val);
// Like env_set_local but doesnt let env own the variable
void env_set_local_raw(Env *e, const char *name, Value *val);
// Like env_set but doesnt let env own the variable
int env_set_raw(Env *e, const char *name, Value *val);
// Register a native
void env_register_native(Env *env, const char *name, NativeFn fn);
// Register built ins
void env_register_builtins(Env *g);

// == Value Related

// When writing your own mila kernel
// these functions might be the only
// part of mila youll ever touch.

// Return an int if a MiLa value is truthy
int is_truthy(Value *value);
// Make a new value with a type
Value *val_new(ValueType t);
// Allocate a method table for a value
void val_allocate_table(Value *v);
// Make a standalone method table
MethodTable *val_make_table(void);
// Set a values method table
void val_set_table(Value *v, MethodTable *t);
// Set the method of a value
void val_set_method(Value *v, MethodType t, void* func);
// Set the method of a method table
void val_set_method_table(MethodTable *v, MethodType t, void* func);
//  Unset the method of a method table
void val_unset_method_table(MethodTable *v, MethodType t);
// Unset the method of a value
void val_unset_method(Value *v, MethodType t);
// Own a value
Value *val_retain(Value *v);
// Disown a value
void val_release(Value *v);
// Disown a value, doesnt free value instance, only the inner data
void val_release_incomplete(Value *v);
// Printf but support `%?` to print values.
int mila_printf(char *fmt, ...);
// Generates a string from an fmt
__attribute__((format(printf, 1, 2)))
Value *vstring_fmt(char *fmt, ...);
// Slice a string
Value *vstring_slice(const char *src, size_t start, size_t len);
// Index a string
Value *vstring_index(const char *src, size_t index);
// Replace some path of a string (used string.patch)
Value *vstring_replace(const char *src,
                       const char *needle,
                       const char *repl);
// Free a value regardless of refcount
void val_kill(Value *v);
// Integer contructor
Value *vint(long i);
// Uint constructor
Value *vuint(unsigned long i);
// Float constructor
Value *vfloat(double f);
// Bool constructor
Value *vbool(int b);
// Duplicate a string
Value *vstring_dup(const char *s);
// Take a string (assuming MiLa can free it)
Value *vstring_take(char *s);
// Opaque pointer constructor
Value *vopaque(void *p);
// Owned opaque pointer constructor (MiLa takes ownership)
Value *vowned_opaque(void *p);
// Create a native function
Value *vnative(NativeFn fn, const char *name);
// Create a bool if a value is truthy
Value *vtruthy(Value *value);
// Null
Value *vnull();
// None
Value *vnone();
// Error
__attribute__((format(printf, 1, 2)))
Value *verror(char *message, ...);
// Tagged error
__attribute__((format(printf, 2, 3)))
Value *vtagged_error(ErrorType type, char *message, ...);
// Create a function
Value *vfunction(char **params, char** contextuals, Env* closure, char *body_src);
// Check if a value is any numeric type
static int is_number(Value *v);
// Turn any numeric type to a double
double to_double(Value *v);
// Turn a value into its c string equivalent
char *as_c_string(Value *v);
// Turn a value into its c string representation equivalent
char *as_c_string_repr(Value *v);
// Turn a value into its c string equivalent (do not use its overload of UMethodToString)
char *as_c_string_raw(Value *v);
// Turn a value into its c string representation equivalent (do not use its overload of UMethodToRepr)
char *as_c_string_repr_raw(Value *v);
// Print a value
int print_value(Value *v);
// Print a value (numeric types get a thousands operator)
int print_value_fancy(Value *v);
// Print a values representation
int print_value_repr(Value *v);
// Call a function
Value *call_function_with(Env *env, Value *fnval, Value *first, ...);
// Create an opaque
Value *vopaque_extra(void *p, Value *(*dis)(Value *), const char *type);
// Create an owned opaque
Value *vowned_opaque_extra(void *p, Value *(*dis)(Value *), const char *type);

// == Parsing

/*
 * I suggest you also look in mila.c
 * in how these functions are used
 */

typedef struct Src
{
    char *src;    // full source string (null-terminated)
    char *cur_namespace; // current namespace
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

void* mila_malloc(size_t size);
void* mila_realloc(void* ptr, size_t size);
void mila_free(void* ptr);