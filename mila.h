// This project is licensed under the GNU Affero General Public License
#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define MILA_LPREFIX "mila:"
#define ML(x) MILA_LPREFIX x

#include "ml_maths.h"
#include "ml_paths.h"

#define MAX_NUMBER_DIGITS 19
#define MILA_N_ESCAPE_DIGITS 10
#define MAX_PATH_LENGTH 1028

// MiLa produces an alternative string representation
// when collections' item amounts exceed this number
#define MAX_ITEMS_DISPLAYED 1000

// ====== PUBLIC API

// Error constants (for syntax checking)
#define ERR_SUCCESS NULL
#define ERR_EOF "Unexpected end of file"
#define ERR_BLOCK_UNCLOSED "Block not closed with '}'"
#define ERR_PAREN_UNCLOSED "Parenthesis not closed with ')'"
#define ERR_BRACKET_UNCLOSED "Bracket not closed with ']'"
#define ERR_INVALID_IDENT "Invalid identifier"
#define ERR_INVALID_EXPR "Invalid expression"
#define ERR_INVALID_STMT "Invalid statement"
#define ERR_INVALID_FN_CALL "Invalid function call"
#define ERR_INVALID_FN_ARGS "Invalid function arguments"
#define ERR_INVALID_PARAM_LIST "Invalid parameter list"
#define ERR_INVALID_CONTEXT_LIST "Invalid context list"
#define ERR_EXPECTED_PAREN "Expected '('"
#define ERR_EXPECTED_BRACKET "Expected '['"
#define ERR_EXPECTED_BRACE "Expected '{'"
#define ERR_EXPECTED_COMMA "Expected ','"
#define ERR_EXPECTED_SEMICOLON "Expected ';'"
#define ERR_EXPECTED_EQUALS "Expected '='"
#define ERR_EXPECTED_COLON "Expected ':'"
#define ERR_INVALID_CONDITION "Invalid if condition"
#define ERR_INVALID_LOOP "Invalid loop"
#define ERR_INVALID_FOREACH "Invalid foreach"
#define ERR_INVALID_FN_DEF "Invalid function definition"
#define ERR_INVALID_OBJECT "Invalid object definition"
#define ERR_STRING_UNCLOSED "String not terminated"
#define ERR_EXPECTED_TYPE_ANNOTATION "Expected type annotation (string)"

// Public getters for types
#define IS_ERROR(v) (GET_TYPE(v) == T_ERROR || GET_TYPE(v) == T_TAGGED_ERROR)
#define IS_ERROR_TAGGED(v) (GET_TYPE(v) == T_TAGGED_ERROR)
#define IS_FATAL(v) ((GET_TAGGED_ERROR_TYPE(v) == E_FATAL || GET_TAGGED_ERROR_TYPE(v) == E_SYNTAX_ERROR || GET_TAGGED_ERROR_TYPE(v) == E_THREAD_HALT))
#define GET_STRING(val) (val ? val->v.s : NULL)
#define GET_INTEGER(val) (val ? val->v.i : 0)
#define GET_BINTEGER(val) (val ? val->v.bi : 0)
#define GET_UINTEGER(val) (val ? val->v.ui : 0)
#define GET_FLOAT(val) (val ? val->v.f : 0.0)
#define GET_BFLOAT(val) (val ? val->v.bf : (mila_float128_internal){0.0, 0.0})
#define GET_BOOL(val) (val ? val->v.b : 0)
#define GET_OPAQUE(val) (val ? val->v.opaque : NULL)
#define GET_FUNCTION(val) (val ? val->v.fn : NULL)
#define GET_NATIVE(val) (val ? val->v.native : NULL)
#define GET_TAGGED_ERROR_TYPE_MESSAGE(val) (val ? val->v.message : NULL)
#define GET_TAGGED_ERROR_MESSAGE(val) (val ? val->v.tagged_error.message : NULL)
#define OWNED(val) (val->type = T_OWNED_OPAQUE)
#define UNOWNED(val) (val->type = T_OPAQUE)

#define GET_METHOD(v, m) ((v->method_table && v->method_table[m - v->table_offset]) ? v->method_table[m - v->table_offset] : NULL)

// each of these methods may be reffered to as
// type{method name}
// like array{UMethodToIter}
// or simply UMethodToIter
// This is used for Value Instance Operator Overloading (good for speed, bad for flexibility)
typedef enum __attribute__((packed))
{
    MethodNone = -1,

    TMethodBinop,
    BMethodGetItem, // name[...] syntax
    TMethodSetItem, // set name[...] syntax

    // when converting objects into strings
    UMethodToString,
    UMethodToRepr,

    // foreach syntax
    UMethodToIter,
    UMethodStepIter, // step update
    UMethodStepIterInit, // initializes state for iterator
    UMethodStepIterClean, // initializes state for iterator
    UMethodToGen,  // Method to turn collections into generators

    UMethodFree,
    UMethodKill,

    UMethodCopy, // Deep copy by default
    UMethodCopyShallow,

    MethodTotalCount
} MethodType;

typedef enum __attribute__((packed))
{

    // value op value syntax
    BMethodAdd = BMethodGetItem,
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
    BMethodGlob,
    BMethodDefault,
} MethodType_Internal; // not used by VIOO instances

// == Environment

// When writing your own mila kernel
// these functions bellow (env and value related) might be the only
// part of mila youll ever touch.

typedef enum
{
    T_WHAT = -1,
    T_NULL,
    T_INT,
    T_UINT,
    T_FLOAT,
    T_BINT,
    T_BFLOAT,
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

#ifndef MILA_PROTO

// OVERLOAD_* can be used by users, ensures users dont use constant strings and maintains
// consistency accross possible updates
// You see object operator overloading isnt really MiLas... priority
// so updates might change so fast developers might get whiplash

// Left Operators
const char* OVERLOAD_ADD = ":+";
const char* OVERLOAD_SUB = ":-";
const char* OVERLOAD_MUL = ":*";
const char* OVERLOAD_DIV = ":/";
const char* OVERLOAD_MOD = ":%";
const char* OVERLOAD_RSHIFT = ":>>";
const char* OVERLOAD_LSHIFT = ":<<";
const char* OVERLOAD_EQ = ":==";
const char* OVERLOAD_NE = ":!=";
const char* OVERLOAD_GT = ":>";
const char* OVERLOAD_LT = ":<";
const char* OVERLOAD_GE = ":>=";
const char* OVERLOAD_LE = ":<=";

// Right Operators
const char* OVERLOAD_R_ADD = "+:";
const char* OVERLOAD_R_SUB = "-:";
const char* OVERLOAD_R_MUL = "*:";
const char* OVERLOAD_R_DIV = "/:";
const char* OVERLOAD_R_MOD = "%:";
const char* OVERLOAD_R_RSHIFT = ">>:";
const char* OVERLOAD_R_LSHIFT = "<<:";
const char* OVERLOAD_R_DEFAULT = "??:";
const char* OVERLOAD_R_EQ = "==:";
const char* OVERLOAD_R_NE = "!=:";
const char* OVERLOAD_R_GT = ">:";
const char* OVERLOAD_R_LT = "<:";
const char* OVERLOAD_R_GE = ">=:";
const char* OVERLOAD_R_LE = "<=:";

// More complex overloads
const char* OVERLOAD_DISPLAY = ":display";
const char* OVERLOAD_COPY = ":copy";
const char* OVERLOAD_COPYSHALLOW = ":copyshallow";
const char* OVERLOAD_TO_BOOL = ":to_bool";

#endif // MILA_PROTO

typedef enum
{
    E_NO_ERROR = -1, // Default
    E_SYNTAX_ERROR,  // Self explanatory
    E_PRE_RUNTIME,   // Must always be fatal!
    E_RUNTIME,       // Errors such as undefined variables
    E_TYPE_ERROR,    // Errors when doing a type cannot do (impossible in core mila, invalid op == null)
    E_FATAL,         // Errors that should be fatal, like syntax errors
    E_GENERIC,       // Errors that cannot be classified as ones above
    E_ASSERT,        // Errors triggered by a faulty assert
    E_THREAD_HALT,   // Signal threads to halt (propagates like an error)
    E_EXIT,          // When user calls exit
} ErrorType;

#ifndef MILA_PROTO
// Use (GET_TAGGED_ERROR_TYPENAME than this)
const char *MILA_ERROR_NAMES[] = {
    "SyntaxError",
    "PreRuntime",
    "Runtime",
    "TypeError",
    "Fatal",
    "Generic",
    "AssertionError",
    "ThreadHalt",
    "Exit",
};
#endif // MILA_PROTO

typedef struct Value Value;
typedef struct Env Env;
typedef Value *(*NativeFn)(Env *env, int argc, Value **argv);
typedef void* MethodTable;

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
// Copy an environment
void env_copy(Env* dest, Env* src);
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

// Convert a 128 bit into a string
char *i128toa(__int128 value);
// Return an int if a MiLa value is truthy
int is_truthy(Value *value);
// Make a new value with a type
Value *val_new(ValueType t);
// Copy a value
Value* val_copy(Value *src);
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
extern Value *val_retain(Value *v);
// Disown a value
extern void val_release(Value *v);
// Free a value regardless of refcount
void val_kill(Value *v);
// Integer contructor
extern Value *vint(long i);
// Uint constructor
extern Value *vuint(unsigned long i);
// Float constructor
extern Value *vfloat(double f);
// Bool constructor
extern Value *vbint(__int128 i);
// Float constructor
extern Value *vbfloat(mila_float128_internal f);
// Bool constructor
extern Value *vbool(int b);
// Duplicate a string
extern Value *vstring_dup(const char *s);
// Take a string (assuming MiLa can free it)
extern Value *vstring_take(char *s);
// Generates a string from an fmt
__attribute__((format(printf, 1, 2)))
extern Value *vstring_fmt(char *fmt, ...);
// Slice a string
extern Value *vstring_slice(const char *src, size_t start, size_t len);
// Index a string
extern Value *vstring_index(const char *src, size_t index);
// Replace some path of a string (used string.patch)
extern Value *vstring_replace(const char *src,
                       const char *needle,
                       const char *repl);
// Opaque pointer constructor
extern Value *vopaque(void *p);
// Owned opaque pointer constructor (MiLa takes ownership)
extern Value *vowned_opaque(void *p);
// Create a native function
extern Value *vnative(NativeFn fn, const char *name);
// Create a bool if a value is truthy
extern Value *vtruthy(Value *value);
extern Value *vbreak();
extern Value *vcontinue();
// Null
extern Value *vnull();
// None
extern Value *vnone();
// Error
__attribute__((format(printf, 1, 2)))
Value *verror(char *message, ...);
// Tagged error
__attribute__((format(printf, 2, 3)))
Value *vtagged_error(ErrorType type, char *message, ...);
// Tagged error with a return code
__attribute__((format(printf, 3, 4)))
Value *vtagged_coded_error(ErrorType type, int ret_code, char *message, ...);
// Create a function
Value *vfunction(char **params, char** defaults, char** contextuals, Env* closure, char *body_src);
// Check if a value is any numeric type
extern int is_number(Value *v);
// Turn any numeric type to a double
extern double to_double(Value *v);
extern unsigned long to_uint(Value *v);
// Turn a value into its string equivalent
Value *to_string(Value *v);
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
// Debug print
int print_value_debug(Value *v);
// Print a values representation
int print_value_repr(Value *v);
// Call a function
Value *call_function_with(Env *env, Value *fnval, Value *first, ...);
// Call a function from within an environment using its name representation
Value *call_function_str(Env *env, const char *fnname, Value *first, ...);
// Create an opaque
Value *vopaque_extra(void *p, Value *(*dis)(Value *), const char *type);
// Create an owned opaque
Value *vowned_opaque_extra(void *p, Value *(*dis)(Value *), const char *type);
#ifndef EXT_WEB
__int128 atoi128(char* num);
char* i128toa(__int128 num);
#endif

typedef struct {
    char* name;
    void(*fn)(Env*);
} CleanupRegistryEntry;

typedef struct {
    CleanupRegistryEntry** registry;
    size_t size, count;
} CleanupRegistry;

CleanupRegistry* cleanup_registry;

CleanupRegistry* make_cleanup_registry();
CleanupRegistryEntry* make_cleanup_entry(char* name, void(*fn)(Env*));
void free_cleanup_registry(CleanupRegistry* registry);

// ================= NOT SO PUBLIC APIS (or spicy api stuff, depends on your mood)

// THESE ARE INTERNAL
#define GET_TAGGED_ERROR_TYPENAME(val) (val ? (val->type == T_TAGGED_ERROR ? MILA_ERROR_NAMES[val->v.tagged_error.type] : "???" ) : "???")
#define GET_TYPENAME(v) (v ? (v->type_name ? v->type_name : MILA_TYPE_NAMES[v->type] ) : "???")
#define GET_TAGGED_ERROR_TYPE(val) (IS_ERROR_TAGGED(val) ? val->v.tagged_error.type : E_GENERIC)
#define GET_TYPE(v) (v ? v->type : T_WHAT )

#define HANDLE_RETURN(val)  { if (val && val->type == T_RETURN) {Value* tmp = val->v.opaque; val_release(val); return tmp; } }

#define HANDLE_CONTROL(val) \
    {\
        if (val->type == T_BREAK)\
            return val;\
        if (val->type == T_CONTINUE)\
            return val;\
        return val;\
    }

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
    }

#define GET_OVERLOAD(obj, method) ((obj)->type_name && strcmp((obj)->type_name, MILA_LPREFIX "dict") == 0) ? dict_get_str((Dict*)(obj)->v.opaque, method) : NULL

#define FN_UNUSED __attribute__((unused))

#define IS_CONTROL(v) (v && (v->type == T_BREAK || v->type == T_CONTINUE || v->type == T_RETURN))

typedef char *(*Printer)(Value *self);

typedef struct
{
    char *name;
    NativeFn func;
} NativeEntry;

#ifndef MILA_PROTO
// Simple trick (use GET_TYPENAME rather than use this directly)
const char *MILA_TYPE_NAMES[] = {
    "null",
    "int",
    "uint",
    "float",
    "bint",
    "bfloat",
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

const int MILA_TYPE_COUNT = T_ARG_END;

const int MILA_ERROR_COUNT = E_THREAD_HALT;

path_list *search_path;

#else
extern path_list *search_path;
extern char **MILA_TYPE_NAMES;
extern char **MILA_ERROR_NAMES;
extern int MILA_ERROR_COUNT;
extern int MILA_TYPE_COUNT;

// Left Operatorshttps
extern char* OVERLOAD_ADD;
extern char* OVERLOAD_SUB;
extern char* OVERLOAD_MUL;
extern char* OVERLOAD_DIV;
extern char* OVERLOAD_MOD;
extern char* OVERLOAD_RSHIFT;
extern char* OVERLOAD_LSHIFT;
extern char* OVERLOAD_EQ;
extern char* OVERLOAD_NE;
extern char* OVERLOAD_GT;
extern char* OVERLOAD_LT;
extern char* OVERLOAD_GE;
extern char* OVERLOAD_LE;

// Right Operators
extern char* OVERLOAD_R_ADD;
extern char* OVERLOAD_R_SUB;
extern char* OVERLOAD_R_MUL;
extern char* OVERLOAD_R_DIV;
extern char* OVERLOAD_R_MOD;
extern char* OVERLOAD_R_RSHIFT;
extern char* OVERLOAD_R_LSHIFT;
extern char* OVERLOAD_R_EQ;
extern char* OVERLOAD_R_NE;
extern char* OVERLOAD_R_GT;
extern char* OVERLOAD_R_LT;
extern char* OVERLOAD_R_GE;
extern char* OVERLOAD_R_LE;

// More complex overloads
extern char* OVERLOAD_DISPLAY;
extern char* OVERLOAD_COPY;
extern char* OVERLOAD_COPYSHALLOW;
extern char* OVERLOAD_TO_BOOL;
#endif // MILA_PROTO

typedef Value *(*trinary_method)(Value *self, Value *b, Value* c);
typedef Value *(*binary_method)(Value *self, Value *other);
typedef Value *(*unary_method)(Value *self);

typedef struct
{
    char **params;  // NULL-terminated
    char **defaults;  // NULL-terminated
    char **contextuals; // NULL_terminated
    char *body_src; // pointer to function body source (we'll keep a copy)
    // For evaluation we keep source pointer and we need the position. We'll parse/eval at call-time.
    char *name;
    Env* closure;
} FunctionV;

typedef struct {
    char** params;
    char** defaults;
    size_t count;
} FunctionParameters;

typedef struct
{
    NativeFn fn;
    void *userdata;
    char *name;
} NativeFunctionV;

// Primitives are <50 bytes gauranteed.
// worst case is 300+ Bytes (especially if VIOO)
struct Value
{
    ValueType type;            // 4 bytes
    unsigned short refcount;   // simple refcount (4 bytes)
    unsigned char table_offset;
    char owns_table;           // check if table can be freed or not (1 bytes)
    MethodTable *method_table; // 8 bytes ptr
    char *type_name;           // 8 bytes ptr
    union {
        char * s; // string
        char * message; // for errors
        void *opaque; // pointers (weak, owned, unowned)
        _Bool b;
        // function
        FunctionV *fn;
        NativeFunctionV *native;
        double f;
        mila_float128_internal bf;
        __int128 bi;
        long i;
        unsigned long ui;
        struct {
            char* message;
            ErrorType type;
            int return_code; // -1 by default, if it remains -1 the error type is the error code.
        } tagged_error;
    } v; // around 16 bytes
};

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
    uint64_t len;
} Src;

Src *src_new(const char *s);
void src_free(Src *s);
extern void skip_ws(Src *s);
extern char src_peek(Src *s);
extern void skip_expr(Src *s);
extern void skip_block(Src *s);
extern char src_get(Src *s);
extern int src_eof(Src *s);
void src_advance_by(Src* s, size_t amount);
int is_ident_start(char c);
uint64_t get_line_pos(Src *src);
int report(Src *src, FILE *fp, const char *fmt, ...);
int match_char(Src *s, char c);
extern char *parse_ident_string(Src *s);
extern char *parse_ident(Src *s);
extern Value *parse_number(Src *s);
extern Value *parse_string(Src *s);
extern int is_keyword_at(Src *s, const char *kw);
char *dup_substr(Src *s, int a, int b);
FunctionParameters *parse_param_list(Src *s);
char** parse_context_list(Src *s);
Value *eval_block(Src *s, Env *env);
extern Value *eval_primary(Src *s, Env *env);
Value *binary_op(Value *a, MethodType op, Value *b);
Value *binary_op_objects(Env* env, char right, Value* a, MethodType op, Value* b);
int precedence_of(MethodType op);
MethodType parse_op(Src *s);
Value *eval_expr_prec(Src *s, Env *env, int min_prec);
Value *eval_expr(Src *s, Env *env);
Value *eval_statement_fn(Src *s, Env *env);
Value *eval_statement(Src *s, Env *env);
extern double to_double(Value *v);
extern mila_float128_internal to_bdouble(Value *v);
extern __int128 to_bint(Value *v);

// == Helpers
void sleep_ms(uint64_t ms);
int malloc_sprintf(char **strp, const char *fmt, ...);
Value *call_function(Value *fnval, Env *env, int argc, Value **argv);
Value *call_native_with(Env *env, NativeFn fnval, Value *first, ...);
int match_types(Value **args, ...);
Value *eval_source(Src *s, Env *env);
Value *eval_str(char *src, Env *env);
int run_file(char *name, Env *env);
Value* run_file_keep_res(char *name, Env *env);
int invoke_file(char *name, Env *env);
Value* invoke_file_keep_res(char *name, Env *env);
double get_unix_timestamp();
char *read_input(void);
int load_library(Env *env, const char *libpath);
void mila_add_atexit(Value* fn);
const char* skip_parse_block(Src* s);
const char* skip_parse_statement(Src* s);
const char* skip_parse_expr(Src* s);

// Initialize a minimal environment for a MiLa script
// This will automatically inject built ins
// Global version
Env* mila_global_init(void);
void mila_global_deinit(Env* env);
// Instance
Env* mila_init(void);
void mila_deinit(Env* env);

extern void* mila_malloc(size_t size);
extern void* mila_realloc(void* ptr, size_t size);
extern void mila_free(void* ptr);

// Misc
unsigned long get_process_id(void);
