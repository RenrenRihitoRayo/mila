/*
 * MiLa
 * A modern programming language
 * the smallest it can get.
 * Welcome to the MiLa Language Kernel.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <psapi.h>
#else
#include <sys/time.h>
#include <dlfcn.h>
#include <unistd.h>
#include <limits.h>
#include <sys/resource.h>
#endif

#include "ml_paths.c"

#ifndef MILA_USE_SHARED
#include "ml_builtins.c"
_Bool mila_is_builtins_dynamic = 0;
#else
_Bool mila_is_builtins_dynamic = 1;
#endif

#include "mila.h"

path_list *search_path = NULL;

void print_memory_usage()
{
    size_t memory_usage = 0;

#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    memory_usage = pmc.WorkingSetSize;
#else
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    memory_usage = usage.ru_maxrss * 1024;
#endif

    // Convert to appropriate unit
    double memory_usage_d = (double)memory_usage;
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    while (memory_usage_d >= 1024 && unit_index < 4)
    {
        memory_usage_d /= 1024;
        unit_index++;
    }

    printf("Memory usage: %.2f %s\n", memory_usage_d, units[unit_index]);
}

// ---------- Value representation ----------

void float_to_string(float f, char *buf, size_t bufsize)
{
    // Step 1: try %g with max precision
    snprintf(buf, bufsize, "%.10g", f); // 9 digits for float

    // Step 2: check if there's a decimal point or exponent
    if (strchr(buf, '.') == NULL && strchr(buf, 'e') == NULL && strchr(buf, 'E') == NULL)
    {
        // integer-looking float, force .0
        size_t len = strlen(buf);
        if (len + 2 < bufsize)
        { // enough room for ".0\0"
            buf[len] = '.';
            buf[len + 1] = '0';
            buf[len + 2] = '\0';
        }
    }
}

Value *val_new(ValueType t)
{
    Value *p = malloc(sizeof(Value));
    p->type = t;
    p->refcount = 1;
    p->type_name = NULL;
    p->method_table = NULL;
    p->owns_table = 1;
    return p;
}

void val_allocate_table(Value *v)
{
    v->method_table = (MethodTable *)malloc(sizeof(MethodTable) * MethodTotalCount);
    memset(v->method_table, 0, sizeof(MethodTable) * MethodTotalCount);
}

MethodTable *val_make_table(void)
{
    MethodTable *t = (MethodTable *)malloc(sizeof(MethodTable) * MethodTotalCount);
    memset(t, 0, sizeof(MethodTable) * MethodTotalCount);
    return t;
}

void val_set_table(Value *v, MethodTable *t)
{
    v->owns_table = 0;
    v->method_table = t;
}

void val_set_bmethod(Value *v, BMethodType t, bin_op_method func)
{
    v->method_table[t].is_binop = 1;
    v->method_table[t].binop = func;
}

void val_set_umethod(Value *v, UMethodType t, unary_op_method func)
{
    v->method_table[t].is_binop = 0;
    v->method_table[t].unary = func;
}

void val_unset_bmethod(Value *v, BMethodType t)
{
    v->method_table[t].binop = NULL;
    v->method_table[t].is_binop = 0;
}

void val_unset_umethod(Value *v, UMethodType t)
{
    v->method_table[t].unary = NULL;
    v->method_table[t].is_binop = 0;
}

void val_set_bmethod_table(MethodTable *v, BMethodType t, bin_op_method func)
{
    v[t].is_binop = 1;
    v[t].binop = func;
}

void val_set_umethod_table(MethodTable *v, UMethodType t, unary_op_method func)
{
    v[t].is_binop = 0;
    v[t].unary = func;
}

void val_unset_bmethod_table(MethodTable *v, BMethodType t)
{
    v[t].binop = NULL;
    v[t].is_binop = 0;
}

void val_unset_umethod_table(MethodTable *v, UMethodType t)
{
    v[t].unary = NULL;
    v[t].is_binop = 0;
}

Value *val_retain(Value *v)
{
    if (!v)
        return NULL;
    v->refcount++;
    return v;
}

// Helpers to create typed values or check their truthiness

int is_truthy(Value *value)
{
    _Bool truth = 0;
    if (value->type == T_BOOL)
        truth = value->v.b;
    else if (value->type == T_INT)
        truth = value->v.i != 0;
    else if (value->type == T_FLOAT)
        truth = value->v.f != 0.0;
    else if (value->type == T_STRING)
        truth = value->v.s != NULL && strlen(value->v.s);
    else if (value->type == T_OPAQUE)
        truth = value->v.opaque;
    else
        truth = (value->type != T_NULL && value->type != T_NONE);
    return truth;
}

Value *vnull() { return val_new(T_NULL); }
Value *vnone() { return val_new(T_NONE); }
Value *vbreak() { return val_new(T_BREAK); }
Value *vcontinue() { return val_new(T_CONTINUE); }
Value *verror(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    // First pass: find length
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int len = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (len < 0)
    {
        va_end(ap);
        return NULL;
    }

    char *buf = malloc(len + 1);
    if (!buf)
    {
        va_end(ap);
        Value *v = val_new(T_ERROR);
        v->v.message = strdup("verror could not allocate memory!");
        return v;
    }

    vsnprintf(buf, len + 1, fmt, ap);
    va_end(ap);
    Value *v = val_new(T_ERROR);
    v->v.message = buf;
    return v;
}
Value *vint(long x)
{
    Value *v = val_new(T_INT);
    v->v.i = x;
    return v;
}
Value *vfloat(double f)
{
    Value *v = val_new(T_FLOAT);
    v->v.f = f;
    return v;
}
Value *vbool(int b)
{
    Value *v = val_new(T_BOOL);
    v->v.b = b ? 1 : 0;
    return v;
}
Value *vstring_dup(const char *s)
{
    Value *v = val_new(T_STRING);
    v->v.s = strdup(s ? s : "");
    return v;
}
Value *vstring_take(char *s)
{
    Value *v = val_new(T_STRING);
    v->v.s = s;
    return v;
}
Value *vopaque(void *p)
{
    Value *v = val_new(T_OPAQUE);
    v->v.opaque = p;
    if (!p) {
        val_kill(v);
        return NULL;
    }
    return v;
}
Value *vnative(NativeFn fn, const char *name)
{
    Value *v = val_new(T_NATIVE);
    v->v.native = (NativeFunctionV *)malloc(sizeof(NativeFunctionV));
    v->v.native->fn = fn;
    v->v.native->userdata = NULL;
    v->v.native->name = name ? strdup(name) : NULL;
    return v;
}
Value *vtruthy(Value *value)
{
    return vbool(is_truthy(value));
}

int our_asprintf(char **strp, const char *fmt, ...)
{
    if (!strp)
        return -1;

    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);
    int add_size = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);

    if (add_size < 0)
    {
        va_end(args);
        return -1;
    }

    // If *strp is NULL, treat it as empty string
    size_t old_len = 0;
    if (*strp)
    {
        // We trust the caller has either a valid string or NULL.
        old_len = strlen(*strp);
    }

    char *newbuf = realloc(*strp ? *strp : NULL, old_len + add_size + 1);
    if (!newbuf)
    {
        va_end(args);
        return -1;
    }

    *strp = newbuf;

    if (vsnprintf(*strp + old_len, add_size + 1, fmt, args) < 0)
    {
        va_end(args);
        return -1;
    }

    va_end(args);
    return (int)(old_len + add_size);
}

Value *vopaque_extra(void *p, Value *(*dis)(Value *), const char *type)
{
    Value *v = vopaque(p);
    if (dis)
    {
        val_allocate_table(v);
        val_set_umethod(v, UMethodToString, dis);
    }
    v->type_name = strdup(type);
    return v;
}

char *as_c_string(Value *v)
{
    char *buffer = NULL;
    if (!v)
    {
        return strdup("cnull");
    }
    if (v->method_table && (!v->method_table[UMethodToString].is_binop) && v->method_table[UMethodToString].unary)
    {
        Value *str = v->method_table[UMethodToString].unary(v);
        char *res = strdup(str->v.s);
        val_kill(str);
        return res;
    }
    if (v->method_table && (!v->method_table[UMethodToRepr].is_binop) && v->method_table[UMethodFree].unary)
    {
        Value *str = v->method_table[UMethodToRepr].unary(v);
        char *res = strdup(str->v.s);
        val_kill(str);
        return res;
    }
    switch (v->type)
    {
    case T_NULL:
        our_asprintf(&buffer, "null");
        break;
    case T_NONE:
        our_asprintf(&buffer, "none");
        break;
    case T_ERROR:
        our_asprintf(&buffer, "<error:%s>", v->v.message);
        break;
    case T_INT:
        our_asprintf(&buffer, "%ld", v->v.i);
        break;
    case T_FLOAT:
    {
        char buf[MAX_NUMBER_DIGITS] = {0};
        float_to_string(v->v.f, buf, sizeof(buf));
        our_asprintf(&buffer, "%s", buf);
        break;
    }
    case T_STRING:
        our_asprintf(&buffer, "%s", v->v.s ? v->v.s : "");
        break;
    case T_BOOL:
        our_asprintf(&buffer, "%s", v->v.b ? "true" : "false");
        break;
    case T_FUNCTION:
        our_asprintf(&buffer, "<function:%s at %p>", v->v.fn->name ? v->v.fn->name : "(lambda)", v);
        break;
    case T_NATIVE:
        our_asprintf(&buffer, "<native:%s at %p>", v->v.native->name ? v->v.native->name : "???", v->v.native->fn);
        break;
    case T_OPAQUE:
        if (*v->type_name)
            our_asprintf(&buffer, "<opaque:%p %s>", v->v.opaque, v->type_name);
        else
            our_asprintf(&buffer, "<opaque:%p>", v->v.opaque);
        break;
    case T_RETURN:
    {
        char *str = as_c_string_repr(v->v.opaque);
        our_asprintf(&buffer, "<return:%s>", str);
        free(str);
    }
    break;
    default:
        our_asprintf(&buffer, "???");
    }
    return buffer;
}

Value *to_c_string(Value *v)
{
    char *s = as_c_string(v);
    return vstring_take(s);
}

char *as_c_string_repr(Value *v)
{
    char *buffer = NULL;
    if (!v)
    {
        return strdup("cnull");
    }
    if (v->method_table && (!v->method_table[UMethodToRepr].is_binop) && v->method_table[UMethodFree].unary)
    {
        Value *str = v->method_table[UMethodToRepr].unary(v);
        char *res = strdup(str->v.s);
        val_kill(str);
        return res;
    }
    if (v->method_table && (!v->method_table[UMethodToString].is_binop) && v->method_table[UMethodToString].unary)
    {
        Value *str = v->method_table[UMethodToString].unary(v);
        char *res = strdup(str->v.s);
        val_kill(str);
        return res;
    }
    switch (v->type)
    {
    case T_NULL:
        our_asprintf(&buffer, "null");
        break;
    case T_NONE:
        our_asprintf(&buffer, "none");
        break;
    case T_ERROR:
        our_asprintf(&buffer, "<error:%s>", v->v.message);
        break;
    case T_INT:
        our_asprintf(&buffer, "%ld", v->v.i);
        break;
    case T_FLOAT:
        our_asprintf(&buffer, "%f", v->v.f);
        break;
    case T_STRING:
        our_asprintf(&buffer, "\"");
        char* temp = v->v.s ? v->v.s : "";
        for (size_t i=0; i<strlen(temp); ++i) {
            switch (temp[i]) {
                case  7: our_asprintf(&buffer, "\\a"); break;
                case  9: our_asprintf(&buffer, "\\t"); break;
                case 10: our_asprintf(&buffer, "\\n"); break;
                case 11: our_asprintf(&buffer, "\\v"); break;
                case 12: our_asprintf(&buffer, "\\f"); break;
                default: our_asprintf(&buffer, "%c", temp[i]);
            }
        }
        our_asprintf(&buffer, "\"");
        break;
    case T_BOOL:
        our_asprintf(&buffer, "%s", v->v.b ? "true" : "false");
        break;
    case T_FUNCTION:
        our_asprintf(&buffer, "<function:%s at %p>", v->v.fn->name ? v->v.fn->name : "(lambda)", v);
        break;
    case T_NATIVE:
        our_asprintf(&buffer, "<native:%s at %p>", v->v.native->name ? v->v.native->name : "???", v->v.native->fn);
        break;
    case T_OPAQUE:
        if (*v->type_name)
            our_asprintf(&buffer, "<opaque:%p %s>", v->v.opaque, v->type_name);
        else
            our_asprintf(&buffer, "<opaque:%p>", v->v.opaque);
        break;
    case T_RETURN:
    {
        char *str = as_c_string_repr(v->v.opaque);
        our_asprintf(&buffer, "<return:%s>", str);
        free(str);
    }
    break;
    default:
        our_asprintf(&buffer, "???");
    }
    return buffer;
}

// print value (for debug / native print)
void print_value(Value *v)
{
    char *txt = as_c_string(v);
    printf("%s", txt);
    free(txt);
}

void print_value_repr(Value *v)
{
    char *txt = as_c_string_repr(v);
    printf("%s", txt);
    free(txt);
}

// release
void val_release(Value *v)
{
    if (!v)
        return;
    v->refcount--;
    if (v->refcount <= 0)
    {
        if (v->method_table && !v->method_table[UMethodFree].is_binop && v->method_table[UMethodFree].unary)
        {
            v->method_table[UMethodFree].unary(v);
            free(v);
            return;
        }
        // free internals
        if (v->type == T_STRING && v->v.s)
            free(v->v.s);
        if (v->type == T_ERROR && v->v.message)
            free(v->v.message);
        if (v->type == T_FUNCTION)
        {
            if (v->v.fn->params)
            {
                char **p = v->v.fn->params;
                for (int i = 0; p[i]; ++i)
                    free(p[i]);
                free(p);
            }
            if (v->v.fn->body_src)
                free(v->v.fn->body_src);
            if (v->v.fn->name)
                free(v->v.fn->name);
            free(v->v.fn);
            // closure env not freed here (env owns values)
        }
        if (v->type == T_NATIVE)
        {
            if (v->v.native->name)
                free(v->v.native->name);
        }
        if (v->type_name)
            free(v->type_name);
        if (v->method_table && v->owns_table)
            free(v->method_table);
        free(v);
    }
}

void val_kill(Value *v)
{
    if (!v)
        return;
    if (v->method_table && !v->method_table[UMethodKill].is_binop && v->method_table[UMethodKill].unary)
    {
        v->method_table[UMethodKill].unary(v);
        free(v);
        return;
    }
    if (v->method_table && !v->method_table[UMethodFree].is_binop && v->method_table[UMethodFree].unary)
    {
        v->method_table[UMethodFree].unary(v);
        free(v);
        return;
    }
    // free internals
    if (v->type == T_STRING && v->v.s)
        free(v->v.s);
    if (v->type == T_ERROR && v->v.message)
        free(v->v.message);
    if (v->type == T_FUNCTION)
    {
        if (v->v.fn->params)
        {
            char **p = v->v.fn->params;
            for (int i = 0; p[i]; ++i)
                free(p[i]);
            free(p);
        }
        if (v->v.fn->body_src)
            free(v->v.fn->body_src);
        if (v->v.fn->name)
            free(v->v.fn->name);
        free(v->v.fn);
        // closure env not freed here (env owns values)
    }
    if (v->type == T_NATIVE)
    {
        if (v->v.native->name)
            free(v->v.native->name);
    }
    free(v->type_name);
    if (v->method_table && v->owns_table)
        free(v->method_table);
    free(v);
}

// ---------- Environment (simple linked list of frames + variables) ----------

Env *env_new(Env *parent)
{
    Env *e = malloc(sizeof(Env));
    e->vars = NULL;
    e->parent = parent;
    return e;
}

void env_free(Env *e)
{
    if (!e)
        return;
    Var *v = e->vars;
    while (v)
    {
        Var *nx = v->next;
        free(v->name);
        val_release(v->value);
        free(v);
        v = nx;
    }
    free(e);
}

void env_dump(Env *e)
{
    if (!e)
        return;
    Var *v = e->vars;
    while (v)
    {
        Var *nx = v->next;
        if (!v->value)
        {
            printf("%s dangling\n", v->name);
            v = nx;
            continue;
        }
        if (v->value)
        {
            char *res = as_c_string_repr(v->value);
            printf("%s = %s (%i)\n", v->name, res, v->value->refcount);
            free(res);
        }
        v = nx;
    }
}

void env_kill(Env *e)
{
    if (!e)
        return;
    Var *v = e->vars;
    while (v)
    {
        Var *nx = v->next;
        free(v->name);
        val_kill(v->value);
        free(v);
        v = nx;
    }
    free(e);
}

Value *env_get(Env *e, const char *name)
{
    for (Env *cur = e; cur; cur = cur->parent)
    {
        for (Var *v = cur->vars; v; v = v->next)
        {
            if (strcmp(v->name, name) == 0)
            {
                return v->value;
            }
        }
    }
    return NULL;
}

void env_set_local(Env *e, const char *name, Value *val)
{
    // set or create in current frame
    for (Var *v = e->vars; v; v = v->next)
    {
        if (strcmp(v->name, name) == 0)
        {
            val_release(v->value);
            v->value = val_retain(val);
            return;
        }
    }
    /* try to set the name variable, makes debugging easier */
    if (val->type == T_FUNCTION && val->v.fn->name == NULL)
    {
        val->v.fn->name = strdup(name);
    }
    Var *nv = malloc(sizeof(Var));
    nv->name = strdup(name);
    nv->value = val_retain(val);
    nv->next = e->vars;
    e->vars = nv;
}

int env_set(Env *e, const char *name, Value *val)
{
    // assign to nearest visible frame that contains name, else set local
    /* try to set the name variable, makes debugging easier */
    if (val->type == T_FUNCTION && val->v.fn->name == NULL)
    {
        val->v.fn->name = strdup(name);
    }
    for (Env *cur = e; cur; cur = cur->parent)
    {
        for (Var *v = cur->vars; v; v = v->next)
        {
            if (strcmp(v->name, name) == 0)
            {
                val_release(v->value);
                v->value = val_retain(val);
                return 1;
            }
        }
    }
    // not found, set locally
    env_set_local(e, name, val);
    return 1;
}

void env_set_local_raw(Env *e, const char *name, Value *val)
{
    // set or create in current frame
    for (Var *v = e->vars; v; v = v->next)
    {
        if (strcmp(v->name, name) == 0)
        {
            val_release(v->value);
            v->value = val;
            return;
        }
    }
    /* try to set the name variable, makes debugging easier */
    if (val->type == T_FUNCTION && val->v.fn->name == NULL)
    {
        val->v.fn->name = strdup(name);
    }
    Var *nv = malloc(sizeof(Var));
    nv->name = strdup(name);
    nv->value = val;
    nv->next = e->vars;
    e->vars = nv;
}

int env_set_raw(Env *e, const char *name, Value *val)
{
    // assign to nearest visible frame that contains name, else set local
    /* try to set the name variable, makes debugging easier */
    if (val->type == T_FUNCTION && val->v.fn->name == NULL)
    {
        val->v.fn->name = strdup(name);
    }
    for (Env *cur = e; cur; cur = cur->parent)
    {
        for (Var *v = cur->vars; v; v = v->next)
        {
            if (strcmp(v->name, name) == 0)
            {
                val_release(v->value);
                v->value = val;
                return 1;
            }
        }
    }
    // not found, set locally
    env_set_local_raw(e, name, val);
    return 1;
}

int load_library(Env *env, const char *libpath)
{
    int fail = 1;
#ifdef _WIN32
    HMODULE lib = LoadLibraryA(libpath);
    if (!lib)
    {
        fprintf(stderr, "LoadLibraryA('%s') failed (err=%lu)\n", libpath, GetLastError());
        return -1;
    }

    void (*init_func)(Env *) = void(*)(Env *)(lib, "_mila_lib_init");

    if (init_func)
    {
        fail = 0;
        init_func(env);
    }

    const char *const *names =
        (const char *const *)GetProcAddress(lib, "lib_functions");

    if (names)
    {
        for (size_t i = 0; names[i] != NULL; i++)
        {
            FARPROC f = GetProcAddress(lib, names[i]);
            if (!f)
            {
                fprintf(stderr, "Warning: function '%s' not found in '%s'\n", names[i], libpath);
                continue;
            }
            env_register_native(env, names[i], (void *)f);
        }
        return 0;
    }

    const NativeEntry *entries =
        (const NativeEntry *)GetProcAddress(lib, "lib_function_entries");

    if (entries)
    {
        char *name;
        NativeFn func;
        for (size_t i = 0; entries[i].name && entries[i].func; i++)
        {
            env_register_native(env, entries[i].name, entries[i].func);
        }
        return 0;
    }

    if (!fail)
        return 0;

    fprintf(stderr, "Must have a simple 'const char* lib_functions' and list the function names (last item must be NULL)\nor also 'const NativeEntry lib_function_entries[]' which takes in (NativeEntry){.name, .func} with the last item being (NativeEntry){NULL, NULL}\n");
    FreeLibrary(lib);
    return -2;

#else // POSIX
    void *lib = dlopen(libpath, RTLD_LAZY);
    if (!lib)
    {
        fprintf(stderr, "dlopen('%s') failed: %s\n", libpath, dlerror());
        return -1;
    }

    dlerror();
    void (*init_func)(Env *) = dlsym(lib, "_mila_lib_init");
    const char *err = dlerror();
    if (!err)
    {
        fail = 0;
        init_func(env);
    }

    const char *const *names =
        (const char *const *)dlsym(lib, "lib_functions");

    err = dlerror();
    if (!err)
    {
        for (size_t i = 0; names[i] != NULL; i++)
        {
            dlerror();
            void *f = dlsym(lib, names[i]);
            const char *err2 = dlerror();
            if (err2)
            {
                fprintf(stderr, "Warning: '%s' not found in '%s'\n",
                        names[i], libpath);
                continue;
            }
            env_register_native(env, names[i], f);
        }
        return 0;
    }

    dlerror();
    const NativeEntry *entries =
        (const NativeEntry *)dlsym(lib, "lib_function_entries");

    err = dlerror();
    if (!err)
    {
        for (size_t i = 0; entries[i].name && entries[i].func; i++)
        {
            env_register_native(env, entries[i].name, entries[i].func);
        }
        return 0;
    }

    if (!fail)
        return 0;

    fprintf(stderr, "dlsym 'lib_functions' error: %s\nMust have a simple 'const char* lib_functions' and list the function names (last item must be NULL)\nor also 'const NativeEntry lib_function_entries[]' which takes in (NativeEntry){.name, .func} with the last item being (NativeEntry){NULL, NULL}\n", err);
    dlclose(lib);
    return -2;
#endif
}

// helper to bind native into environment with a name
void env_register_native(Env *env, const char *name, NativeFn fn)
{
    Value *nv = vnative(fn, name);
    env_set_local(env, name, nv);
    val_release(nv);
}

// ---------- Parser/Evaluator that directly reads source and evaluates (no separate lexer) ----------

Src *src_new(const char *s)
{
    Src *S = malloc(sizeof(Src));
    S->len = strlen(s);
    S->src = strdup(s);
    S->pos = 0;
    S->line = 0;
    return S;
}
void src_free(Src *s)
{
    if (!s)
        return;
    free(s->src);
    free(s);
}

// helpers
char src_peek(Src *s) { return s->pos < s->len ? s->src[s->pos] : '\0'; }
char src_get(Src *s) { return s->pos < s->len ? s->src[s->pos++] : '\0'; }
int src_eof(Src *s) { return s->pos >= s->len; }

void skip_block(Src *s)
{
    skip_ws(s);
    // body is block; extract substring from '{' to matching '}'
    if (src_peek(s) != '{')
    {
        return;
    }
    int depth = 0;
    int i = s->pos;
    for (; i < s->len; ++i)
    {
        char ch = s->src[i];
        if (ch == '\n')
            s->line++;
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
                if (s->src[i] == '\n')
                    s->line++;
                if (s->src[i] == '\\' && i + 1 < s->len)
                    i += 2;
                else
                    i++;
            }
        }
        else if (ch == '/' && src_peek(s) == '/')
        {
            // skip comments
            i++;
            while (i < s->len && s->src[i] != '\n')
                i++;
        }
    }
    if (i > s->len)
        i = s->len;
    s->pos = i;
}

void skip_expr(Src *s)
{
    skip_ws(s);
    // body is block; extract substring from '{' to matching '}'
    int depth = 1;
    int i = s->pos;
    for (; i < s->len; ++i)
    {
        char ch = s->src[i];
        if (ch == '(')
            depth++;
        else if (ch == ')')
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
    if (i > s->len)
        i = s->len;
    s->pos = i;
}

void skip_stmt(Src *s)
{
    skip_ws(s);
    int i = s->pos;
    for (; i < s->len; ++i)
    {
        char ch = s->src[i];
        if (ch == ';')
        {
            i++;
            break;
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
    if (i > s->len)
        i = s->len;
    s->pos = i;
}

void skip_ws(Src *s)
{
    for (;;)
    {
        char c = src_peek(s);
        if (c == '\0')
            return;
        if (isspace((unsigned char)c))
        {
            src_get(s);
            continue;
        }
        if (c == '/' && s->pos + 1 < s->len && s->src[s->pos + 1] == '/')
        {
            // line comment
            src_get(s);
            src_get(s);
            while (!src_eof(s) && src_get(s) != '\n')
            {
            }
            continue;
        }
        if (c == '/' && s->pos + 1 < s->len && s->src[s->pos + 1] == '*')
        {
            // block comment
            src_get(s);
            src_get(s);
            while (!src_eof(s))
            {
                char a = src_get(s);
                if (a == '*' && src_peek(s) == '/')
                {
                    src_get(s);
                    break;
                }
            }
            continue;
        }
        break;
    }
}

int match_char(Src *s, char c)
{
    skip_ws(s);
    if (src_peek(s) == c)
    {
        src_get(s);
        return 1;
    }
    return 0;
}

// parse identifier
char *parse_ident(Src *s)
{
    skip_ws(s);
    int st = s->pos;
    char c = src_peek(s);
    if (!(isalpha((unsigned char)c) || c == '_'))
        return NULL;
    src_get(s);
    while (isalnum((unsigned char)src_peek(s)) || src_peek(s) == '_' || src_peek(s) == '.')
        src_get(s);
    int en = s->pos;
    int n = en - st;
    char *res = malloc(n + 1);
    memcpy(res, s->src + st, n);
    res[n] = 0;
    return res;
}

// parse number (int or float)
Value *parse_number(Src *s)
{
    skip_ws(s);
    int st = s->pos;
    int seen_dot = 0;
    int neg = 0;
    if (src_peek(s) == '+' || src_peek(s) == '-')
    {
        neg = 1;
        src_get(s);
    }
    while (isdigit((unsigned char)src_peek(s)) || src_peek(s) == '.')
    {
        if (src_peek(s) == '.')
        {
            if (seen_dot)
            {
                break;
            }
            seen_dot = 1;
        }
        src_get(s);
    }
    int en = s->pos;
    char tmp[MAX_NUMBER_DIGITS];
    int len = en - st;
    if (len <= 0)
        return NULL;

    if (len < (int)sizeof(tmp))
    {
        memcpy(tmp, s->src + st, len);
        tmp[len] = 0;
        if (seen_dot)
        {
            double f = atof(tmp);
            return vfloat(neg ? f * -1 : f);
        }
        else
        {
            long i = atol(tmp);
            return vint(neg ? i * -1 : i);
        }
    }
    else
    {
        char *buf = malloc(len + 1);
        memcpy(buf, s->src + st, len);
        buf[len] = 0;
        Value *r;
        if (seen_dot)
            r = vfloat(neg ? atof(buf) * -1 : atof(buf));
        else
            r = vint(neg ? atol(buf) * -1 : atol(buf));
        free(buf);
        return r;
    }
}

// parse string literal (double quotes)
Value *parse_string(Src *s)
{
    skip_ws(s);
    if (src_peek(s) != '"')
        return NULL;
    src_get(s); // consume opening "

    size_t cap = 256;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf)
        return NULL; // check allocation

    while (!src_eof(s))
    {
        char c = src_get(s);
        if (c == '"')
            break;

        if (c == '\\')
        {
            char n = src_get(s);
            switch (n)
            {
            case 'n':
                c = '\n';
                break;
            case 't':
                c = '\t';
                break;
            case 'r':
                c = '\r';
                break;
            case 'f':
                c = '\f';
                break;
            case 'v':
                c = '\v';
                break;
            case 'a':
                c = '\a';
                break;
            case '\\':
                c = '\\';
                break;
            default:
                c = n;
                break;
            }
        }

        // Grow buffer if needed
        if (len + 1 >= cap)
        {
            cap = cap + (int)(cap * 0.3); // grow by 30%
            char *tmp = realloc(buf, cap);
            if (!tmp)
            {
                free(buf);
                return NULL; // allocation failed
            }
            buf = tmp;
        }

        buf[len++] = c;
    }

    // shrink to exact size
    char *res = realloc(buf, len + 1);
    if (!res)
        res = buf; // if realloc fails, keep original buffer

    res[len] = '\0';
    return vstring_take(res);
}

// parse function literal: fn(a,b){ ... }
int is_keyword_at(Src *s, const char *kw)
{
    skip_ws(s);
    int p = s->pos;
    int klen = strlen(kw);
    if (p + klen > s->len)
        return 0;
    if (strncmp(s->src + p, kw, klen) == 0)
    {
        char after = s->src[p + klen];
        if (after == '\0' || isspace((unsigned char)after) || after == '(' || after == '{')
            return 1;
    }
    return 0;
}

char *dup_substr(Src *s, int a, int b)
{
    int n = b - a;
    char *r = malloc(n + 1);
    memcpy(r, s->src + a, n);
    r[n] = 0;
    return r;
}

// parse comma-separated identifiers (for parameters)
char **parse_param_list(Src *s)
{
    skip_ws(s);
    if (!match_char(s, '('))
        return NULL;
    skip_ws(s);
    // empty
    if (match_char(s, ')'))
    {
        char **p = malloc(sizeof(char *));
        p[0] = NULL;
        return p;
    }
    // read identifiers
    char **arr = NULL;
    int cnt = 0;
    for (;;)
    {
        char *id = parse_ident(s);
        if (!id)
        {
            // error
            // cleanup
            for (int i = 0; i < cnt; i++)
                free(arr[i]);
            free(arr);
            return NULL;
        }
        arr = realloc(arr, sizeof(char *) * (cnt + 2));
        arr[cnt++] = id;
        arr[cnt] = NULL;
        skip_ws(s);
        if (match_char(s, ','))
            continue;
        if (match_char(s, ')'))
            break;
        // error -> break
        break;
    }
    return arr;
}

// parse block: {...}
Value *eval_block(Src *s, Env *env)
{
    skip_ws(s);
    if (!match_char(s, '{'))
    {
        return vnull();
    }
    // new local frame
    Env *frame = env_new(env);
    Value *last = vnull(); // last expression value
    while (1)
    {
        skip_ws(s);
        if (src_peek(s) == '\0')
            break;
        if (match_char(s, '}'))
            break;

        Value *st = eval_statement(s, frame);

        if (last)
            val_release(last);
        last = st;

        if (IS_CONTROL(st))
        {
            env_free(frame);
            HANDLE_CONTROL(st);
        }
    }
    env_free(frame);
    return last;
}

Value* parse_subscript(Src* s, Env* e)
{
    if (!match_char(s, '['))
    {
        return verror("Subscript was expected!");
    }
    
    Value* res = parse_expr(s, e);
    
    if (!match_char(s, ']'))
    {
        return verror("Closing square bracket was expected!");
    }
    
    return res;
}

Value *eval_block_raw(Src *s, Env *frame)
{
    skip_ws(s);
    if (!match_char(s, '{'))
    {
        return verror("Block was expected!");
    }
    // new local frame
    Value *last = vnull(); // last expression value
    while (1)
    {
        skip_ws(s);
        if (src_peek(s) == '\0')
            break;
        if (match_char(s, '}'))
            break;
        Value *st = eval_statement(s, frame);

        if (last)
            val_release(last);
        last = st;

        if (IS_CONTROL(st))
        {
            HANDLE_CONTROL(st);
        }
    }
    return last;
}

// now we need eval_primary, function call, member? only call expressions and binary ops
// evaluate primary

Value *call_function_with(Env *env, Value *fnval, Value *first, ...)
{
    va_list ap;
    size_t count = 0;

    /* First pass: count */
    va_start(ap, first);
    for (Value *v = first; v != NULL; v = va_arg(ap, Value *))
    {
        count++;
    }
    va_end(ap);

    /* Allocate array (+1 if you want NULL terminator preserved) */
    Value **args = malloc((count + 1) * sizeof(Value *));
    if (!args)
        return NULL;

    /* Second pass: fill */
    va_start(ap, first);
    size_t i = 0;
    for (Value *v = first; v != NULL; v = va_arg(ap, Value *))
    {
        args[i++] = v;
    }
    va_end(ap);

    if (!fnval)
    {
        for (int i = 0; i < count; i++)
            val_release(args[i]);
        return verror("Function is NULL!");
    }

    if (fnval->type == T_NATIVE)
    {
        Value *result = fnval->v.native->fn(env, count, args);
        for (int i = 0; i < count; i++)
            val_release(args[i]);
        free(args);
        return result;
    }
    else if (fnval->type == T_FUNCTION)
    {
        // create new environment with closure as parent
        Env *frame = env_new(fnval->v.fn->closure);
        // bind params
        char **p = fnval->v.fn->params;
        int i = 0;
        for (; p && p[i]; ++i)
        {
            // if fewer args provided, bind null
            Value *a = (i < count) ? args[i] : vnull();
            env_set_local(frame, p[i], a);
        }
        // Evaluate body: note body_src contains the body text e.g., "{ ... }"
        Src *child = src_new(fnval->v.fn->body_src);
        // position should start at 0 for the body; body is a block (starts with '{')
        // Evaluate block using the new frame
        Value *res = eval_block(child, frame);

        src_free(child);
        env_free(frame);
        // If res is return value (T_RETURN) it will have been unwrapped earlier in eval_block but to be safe:
        if (res && res->type == T_RETURN)
        {
            Value *rv = (Value *)res->v.opaque;
            val_kill(res);
            for (int i = 0; i < count; i++)
                val_release(args[i]);
            free(args);
            return rv;
        }
        for (int i = 0; i < count; i++)
            val_release(args[i]);
        free(args);
        return res;
    }
    else
    {
        for (int i = 0; i < count; i++)
            val_release(args[i]);
        free(args);
        // not callable
        return verror("Attempt to call non-callable value.");
    }
}

Value *call_function_str(Env *env, const char *fnname, Value *first, ...)
{
    va_list ap;
    size_t count = 0;

    /* First pass: count */
    va_start(ap, first);
    for (Value *v = first; v != NULL; v = va_arg(ap, Value *))
    {
        count++;
    }
    va_end(ap);

    /* Allocate array (+1 if you want NULL terminator preserved) */
    Value **args = malloc((count + 1) * sizeof(Value *));
    if (!args)
        return NULL;

    /* Second pass: fill */
    va_start(ap, first);
    size_t i = 0;
    for (Value *v = first; v != NULL; v = va_arg(ap, Value *))
    {
        args[i++] = v;
    }
    va_end(ap);

    Value *fnval = env_get(env, fnname);
    if (!fnval)
    {
        for (int i = 0; i < count; i++)
            val_release(args[i]);
        return verror("Function is NULL!");
    }

    if (fnval->type == T_NATIVE)
    {
        Value *result = fnval->v.native->fn(env, count, args);
        for (int i = 0; i < count; i++)
            val_release(args[i]);
        free(args);
        return result;
    }
    else if (fnval->type == T_FUNCTION)
    {
        // create new environment with closure as parent
        Env *frame = env_new(fnval->v.fn->closure);
        // bind params
        char **p = fnval->v.fn->params;
        int i = 0;
        for (; p && p[i]; ++i)
        {
            // if fewer args provided, bind null
            Value *a = (i < count) ? args[i] : vnull();
            env_set_local(frame, p[i], a);
        }
        // Evaluate body: note body_src contains the body text e.g., "{ ... }"
        Src *child = src_new(fnval->v.fn->body_src);
        // position should start at 0 for the body; body is a block (starts with '{')
        // Evaluate block using the new frame
        Value *res = eval_block(child, frame);

        src_free(child);
        env_free(frame);
        // If res is return value (T_RETURN) it will have been unwrapped earlier in eval_block but to be safe:
        if (res && res->type == T_RETURN)
        {
            Value *rv = (Value *)res->v.opaque;
            val_kill(res);
            for (int i = 0; i < count; i++)
                val_release(args[i]);
            free(args);
            return rv;
        }
        for (int i = 0; i < count; i++)
            val_release(args[i]);
        free(args);
        return res;
    }
    else
    {
        for (int i = 0; i < count; i++)
            val_release(args[i]);
        free(args);
        // not callable
        return verror("Attempt to call non-callable value.");
    }
}

Value **make_args(Value *first, ...)
{
    va_list ap;
    size_t count = 0;

    /* First pass: count */
    va_start(ap, first);
    for (Value *v = first; v != NULL; v = va_arg(ap, Value *))
    {
        count++;
    }
    va_end(ap);

    Value **args = malloc((count) * sizeof(Value *));
    if (!args)
        return NULL;

    /* Second pass: fill */
    va_start(ap, first);
    size_t i = 0;
    for (Value *v = first; v != NULL; v = va_arg(ap, Value *))
    {
        args[i++] = v;
    }
    va_end(ap);
    return args;
}

Value *call_function(Value *fnval, Env *env, int argc, Value **argv)
{
    if (!fnval)
        return verror("Function is NULL!");
    if (fnval->type == T_NATIVE)
    {
        Value *result = fnval->v.native->fn(env, argc, argv);
        return result;
    }
    else if (fnval->type == T_FUNCTION)
    {
        // create new environment with closure as parent
        Env *frame = env_new(fnval->v.fn->closure);
        // bind params
        char **p = fnval->v.fn->params;
        int i = 0;
        for (; p && p[i]; ++i)
        {
            // if fewer args provided, bind null
            Value *a = (i < argc) ? argv[i] : vnull();
            env_set_local(frame, p[i], a);
        }
        // Evaluate body: note body_src contains the body text e.g., "{ ... }"
        Src *child = src_new(fnval->v.fn->body_src);
        // position should start at 0 for the body; body is a block (starts with '{')
        // Evaluate block using the new frame
        Value *res = eval_block(child, frame);

        src_free(child);
        env_free(frame);
        if (IS_CONTROL(res))
            HANDLE_CONTROL(res);
    }
    else
    {
        // not callable
        return verror("Attempt to call non-callable value.");
    }
    return vnull();
}

// parse expression with precedence:
// We'll implement: primary, unary, multiplicative(*,/), additive(+,-), comparison (<,>,<=,>=,==,!=), logical (&&,||)

int is_ident_start(char c) { return isalpha((unsigned char)c) || c == '_'; }

// parse primary: numbers, strings, identifiers (variables or function calls), parentheses, function literal
Value *eval_primary(Src *s, Env *env)
{
    skip_ws(s);
    char c = src_peek(s);
    if (c == '\0')
        return vnull();
    // number
    if ((isdigit((unsigned char)c) || (c == '+' || c == '-')) && isdigit((unsigned char)s->src[s->pos + 1]))
    {
        return parse_number(s);
    }
    if (isdigit((unsigned char)c) && (unsigned char)s->src[s->pos + 1] == '.')
    {
        return parse_number(s);
    }
    if ((isdigit((unsigned char)c) || (c == '+' || c == '-')) && isdigit((unsigned char)s->src[s->pos + 1]) && (unsigned char)s->src[s->pos + 2] == '.')
    {
        return parse_number(s);
    }
    if (isdigit((unsigned char)c))
    {
        return vint(src_get(s) - 48);
    }
    // string
    if (c == '"')
        return parse_string(s);
    // parentheses
    if (c == '(')
    {
        src_get(s); // consume '('
        Value *expr = eval_expr(s, env);
        skip_ws(s);
        if (src_peek(s) == ')')
            src_get(s);

        if (src_peek(s) == '(')
        {
            // parse args
            src_get(s); // consume '('
            // parse comma separated expressions
            Value **args = NULL;
            int argc = 0;
            skip_ws(s);
            if (src_peek(s) != ')')
            {
                for (;;)
                {
                    Value *a = eval_expr(s, env);
                    args = realloc(args, sizeof(Value *) * (argc + 1));
                    args[argc++] = a;
                    skip_ws(s);
                    if (match_char(s, ','))
                        continue;
                    if (match_char(s, ')'))
                        break;
                    break;
                }
            }
            else
            {
                // empty
                src_get(s); // consume ')'
            }
            // callp
            Value *res = call_function(expr, env, argc, args);
            for (int i = 0; i < argc; i++)
                val_release(args[i]);
            free(args);
            val_release(expr);
            return res;
        }
        return expr;
    }
    if (c == '{')
    {
        Value *v = eval_block(s, env);
        skip_ws(s);

        if (IS_CONTROL(v))
            HANDLE_CONTROL(v);

        return v;
    }
    if (c == '!' && s->src[s->pos + 1] == '{')
    {
        src_get(s); // consume '!'
        size_t start = s->pos + 1;
        skip_block(s);
        size_t end = s->pos - 1; // avoid the closing }
        skip_ws(s);

        char *buffer = (char *)malloc(sizeof(char) * (end - start) + 1);
        memcpy(buffer, s->src + start, end - start);

        return vstring_take(buffer);
    }
    // function literal
    if (is_keyword_at(s, "fn"))
    {
        // consume keyword
        s->pos += strlen("fn");
        // parse params
        char **params = parse_param_list(s);
        skip_ws(s);
        // body is block; extract substring from '{' to matching '}'
        if (src_peek(s) != '{')
        {
            // error: expected body
            return verror("Body wasnt found.");
        }
        // find matching brace (we will copy out body)
        int depth = 0;
        int start = s->pos;
        int i = s->pos;
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
        if (i > s->len)
            i = s->len;
        int blen = i - start;
        char *body = malloc(blen + 1);
        memcpy(body, s->src + start, blen);
        body[blen] = 0;
        s->pos = i;
        // create function value with closure get_line_pos(s) current env
        Value *fn = vfunction(params, body, env);
        return fn;
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
            free(id);
            return vnull();
        }
        if (strcmp(id, "cnull") == 0)
        {
            free(id);
            return NULL;
        }
        if (strcmp(id, "none") == 0)
        {
            free(id);
            return vnone();
        }
        if (strcmp(id, "true") == 0)
        {
            free(id);
            return vbool(1);
        }
        if (strcmp(id, "false") == 0)
        {
            free(id);
            return vbool(0);
        }
        if (strcmp(id, "break") == 0)
        {
            free(id);
            return vbreak();
        }
        if (strcmp(id, "continue") == 0)
        {
            free(id);
            return vcontinue();
        }
        // look ahead: function call?
        skip_ws(s);
        if (src_peek(s) == '(')
        {
            // parse args
            src_get(s); // consume '('
            // parse comma separated expressions
            Value **args = NULL;
            int argc = 0;
            skip_ws(s);

            // handle (value)(...) calls
            if (src_peek(s) != ')')
            {
                for (;;)
                {
                    Value *a = eval_expr(s, env);
                    args = realloc(args, sizeof(Value *) * (argc + 1));
                    args[argc++] = a;
                    skip_ws(s);
                    if (match_char(s, ','))
                        continue;
                    if (match_char(s, ')'))
                        break;
                    break;
                }
            }
            else
            {
                // empty
                src_get(s); // consume ')'
            }
            // get callee (id)
            Value *callee = env_get(env, id);
            if (!callee)
            {
                Value *res = verror("Undefined function '%s'", id);
                free(id);
                // release args
                for (int i = 0; i < argc; i++)
                    val_release(args[i]);
                free(args);
                return res;
            }
            // callp
            Value *res = call_function(callee, env, argc, args);
            free(id);
            for (int i = 0; i < argc; i++)
                val_release(args[i]);
            free(args);

            HANDLE_CONTROL(res);
        }
        else
        {
            // variable lookup
            Value *vv = env_get(env, id);
            free(id);
            if (!vv)
            {
                // undefined variable -> null
                return vnull();
            }
            val_retain(vv);
            return vv;
        }
        free(id);
    }
    // fallback
    return vnull();
}

// helper to convert numeric types and do arithmetic
int is_number(Value *v)
{
    return v && (v->type == T_INT || v->type == T_FLOAT);
}

double to_double(Value *v)
{
    if (!v)
        return 0.0;
    if (v->type == T_INT)
        return (double)v->v.i;
    if (v->type == T_FLOAT)
        return v->v.f;
    return 0.0;
}

// binary ops
Value *binary_op(Value *a, BMethodType op, Value *b)
{
    if (a->method_table && a->method_table[op].is_binop)
    {
        return a->method_table[op].binop(a, b);
    }
    if (op == BMethodDefault)
    {
        if (a->type == T_NONE || a->type == T_NULL || !is_truthy(a))
        {
            return val_retain(b);
        }
        else
        {
            return val_retain(a);
        }
    }
    if ((a->type == T_NONE || a->type == T_NULL) && (b->type == T_NONE || b->type == T_NULL))
    {
        if (BMethodEq == op)
            return vbool(a->type == b->type);
        if (BMethodNe == op)
            return vbool(a->type != b->type);
    }
    // string concatenation for '+'
    if (op == BMethodAdd && a->type == T_STRING && b->type == T_STRING)
    {
        size_t la = strlen(a->v.s), lb = strlen(b->v.s);
        char *buf = malloc(la + lb + 1);
        memcpy(buf, a->v.s, la);
        memcpy(buf + la, b->v.s, lb);
        buf[la + lb] = 0;
        return vstring_take(buf);
    }
    if (op == BMethodAdd && a->type == T_STRING)
    {
        size_t la = strlen(a->v.s);
        char *stringyfied = as_c_string(b);
        if (stringyfied)
        {
            char *buf = malloc(la + strlen(stringyfied) + 1);
            if (!buf)
                return vnull();
            strcpy(buf, a->v.s);
            strcat(buf, stringyfied);
            free(stringyfied);
            return vstring_take(buf);
        }
        return vnull();
    }
    if (op == BMethodAdd && b->type == T_STRING)
    {
        size_t la = strlen(b->v.s);
        char *stringyfied = as_c_string(a);
        if (stringyfied)
        {
            char *buf = malloc(la + strlen(stringyfied) + 1);
            if (!buf)
                return vnull();
            strcpy(buf, stringyfied);
            strcat(buf, b->v.s);
            free(stringyfied);
            return vstring_take(buf);
        }
        return vnull();
    }
    // numeric arithmetic
    if (is_number(a) && is_number(b))
    {
        if (a->type == T_FLOAT || b->type == T_FLOAT)
        {
            double ra = to_double(a), rb = to_double(b);
            if (op == BMethodAdd)
                return vfloat(ra + rb);
            if (op == BMethodSub)
                return vfloat(ra - rb);
            if (op == BMethodMul)
                return vfloat(ra * rb);
            if (op == BMethodDiv)
                return vfloat(ra / rb);
            if (op == BMethodLess)
                return vbool(ra < rb);
            if (op == BMethodGreat)
                return vbool(ra > rb);
            if (op == BMethodLE)
                return vbool(ra <= rb);
            if (op == BMethodGE)
                return vbool(ra >= rb);
            if (op == BMethodEq)
                return vbool(ra == rb);
            if (op == BMethodNe)
                return vbool(ra != rb);
        }
        else
        {
            long ia = a->v.i, ib = b->v.i;
            if (op == BMethodAdd)
                return vint(ia + ib);
            if (op == BMethodSub)
                return vint(ia - ib);
            if (op == BMethodMul)
                return vint(ia * ib);
            if (op == BMethodDiv)
                return vint(ia / ib);
            if (op == BMethodLess)
                return vbool(ia < ib);
            if (op == BMethodGreat)
                return vbool(ia > ib);
            if (op == BMethodLE)
                return vbool(ia <= ib);
            if (op == BMethodGE)
                return vbool(ia >= ib);
            if (op == BMethodEq)
                return vbool(ia == ib);
            if (op == BMethodNe)
                return vbool(ia != ib);
            if (op == BMethodLshift)
                return vint(ia << ib);
            if (op == BMethodRshift)
                return vint(ia >> ib);
        }
        return vnull();
    }
    // equality for strings
    if (BMethodEq == op)
    {
        if (!a && !b)
            return vbool(1);
        if (!a || !b)
            return vbool(0);
        if (a->type == T_STRING && b->type == T_STRING)
            return vbool(strcmp(a->v.s, b->v.s) == 0);
        // fallback pointer equality
        return vbool(a == b);
    }
    if (BMethodNe == op)
    {
        Value *eq = binary_op(a, BMethodEq, b);
        int res = (eq->type == T_BOOL && eq->v.b == 0);
        val_release(eq);
        return vbool(res);
    }
    if (BMethodAnd == op)
    {
        int res = is_truthy(a) && is_truthy(b);
        return vbool(res);
    }
    if (BMethodOr == op)
    {
        int res = is_truthy(a) || is_truthy(b);
        return vbool(res);
    }
    return vnull();
}

// evaluate expression with precedence climbing
int precedence_of(BMethodType op)
{
    if (BMethodOr == op)
        return 1;
    if (BMethodAnd == op)
        return 2;
    if (BMethodDefault == op)
        return 3;
    if (BMethodEq == op || BMethodNe == op)
        return 4;
    if (BMethodLshift == op || BMethodRshift == op)
        return 5;
    if (BMethodLE == op || BMethodGE == op || BMethodLess == op || BMethodGreat == op)
        return 6;
    if (BMethodAdd == op || BMethodSub == op)
        return 7;
    if (BMethodMod == op || BMethodMul == op || BMethodDiv == op)
        return 8;
    return 0;
}

BMethodType parse_op(Src *s)
{
    skip_ws(s);
    char a = src_peek(s);
    char b = (s->pos + 1 < s->len) ? s->src[s->pos + 1] : '\0';
    // two-char ops
    if (a == '|' && b == '|')
    {
        s->pos += 2;
        return BMethodOr;
    }
    if (a == '&' && b == '&')
    {
        s->pos += 2;
        return BMethodAnd;
    }
    if (a == '=' && b == '=')
    {
        s->pos += 2;
        return BMethodEq;
    }
    if (a == '!' && b == '=')
    {
        s->pos += 2;
        return BMethodNe;
    }
    if (a == '<' && b == '=')
    {
        s->pos += 2;
        return BMethodLE;
    }
    if (a == '>' && b == '=')
    {
        s->pos += 2;
        return BMethodGE;
    }
    if (a == '>' && b == '>')
    {
        s->pos += 2;
        return BMethodRshift;
    }
    if (a == '<' && b == '<')
    {
        s->pos += 2;
        return BMethodLshift;
    }
    if (a == '?' && b == '?')
    {
        s->pos += 2;
        return BMethodDefault;
    }
    // single-char ops
    s->pos++;
    switch (a)
    {
    case '+':
        return BMethodAdd;
    case '-':
        return BMethodSub;
    case '*':
        return BMethodMul;
    case '/':
        return BMethodDiv;
    case '%':
        return BMethodMod;
    case '<':
        return BMethodLess;
    case '>':
        return BMethodGreat;
    }
    return -1;
}

Value *eval_expr_prec(Src *s, Env *env, int min_prec)
{
    skip_ws(s);
    // unary + - not implemented separately; parse primary then handle binary ops
    Value *lhs = eval_primary(s, env);
    if (!lhs)
        return NULL;
    for (;;)
    {
        int saved_pos = s->pos;
        BMethodType op = parse_op(s);
        if (op < 0)
            break;
        int prec = precedence_of(op);
        if (prec < min_prec)
        {
            s->pos = saved_pos;
            break;
        }
        // handle right associativity? none needed
        int next_min = prec + 1;
        Value *rhs = eval_expr_prec(s, env, next_min);
        Value *newlhs = binary_op(lhs, op, rhs);
        val_release(lhs);
        val_release(rhs);
        lhs = newlhs;
    }
    return lhs;
}

Value *eval_expr(Src *s, Env *env)
{
    return eval_expr_prec(s, env, 1);
}

void clean_elif_chain(Src *s)
{
    skip_ws(s);
    while (is_keyword_at(s, "elif"))
    {
        s->pos += strlen("elif");
        skip_ws(s);
        if (match_char(s, '('))
            skip_expr(s);
        match_char(s, ')');
        if (src_peek(s) == '{')
            skip_block(s);
        else
            skip_stmt(s);
    }
    if (is_keyword_at(s, "else"))
    {
        s->pos += strlen("else");
        if (src_peek(s) == '{')
            skip_block(s);
        else
            skip_stmt(s);
    }
}

Value *eval_statement(Src *s, Env *env)
{
    skip_ws(s);
    if (is_keyword_at(s, "set"))
    {
        s->pos += strlen("set");
        char *id = parse_ident(s);
        if (!id)
            return verror("Invalid let statement.");
        skip_ws(s);
        if (match_char(s, '='))
        {
            Value *v = eval_expr(s, env);
            // unwrap return
            if (v->type == T_RETURN)
            {
                skip_ws(s);
                match_char(s, ';');
                Value *tmp = v->v.opaque;
                env_set_raw(env, id, tmp);
                free(id);
                val_release(v);
                return tmp;
            }
            else if (v->type == T_FUNCTION)
            {
                v->v.fn->name = strdup(id);
            }
            skip_ws(s);
            match_char(s, ';');
            if (v->type != T_STRING)
                env_set(env, id, v);
            else
                env_set_raw(env, id, val_retain(v));
            free(id);
            return v;
        }
        else if (match_char(s, ':'))
        {
            Value *v = eval_statement(s, env);
            // unwrap return
            if (v->type == T_RETURN)
            {
                skip_ws(s);
                match_char(s, ';');
                Value *tmp = v->v.opaque;
                env_set_raw(env, id, tmp);
                free(id);
                val_release(v);
                return tmp;
            }
            else if (v->type == T_FUNCTION)
            {
                v->v.fn->name = strdup(id);
            }
            skip_ws(s);
            match_char(s, ';');
            if (v->type != T_STRING)
                env_set(env, id, v);
            else
                env_set_raw(env, id, val_retain(v));
            free(id);
            return v;
        }
        else
        {
            // declare none
            env_set_raw(env, id, vnone());
            free(id);
            match_char(s, ';');
            return vnull();
        }
    }
    if (is_keyword_at(s, "var"))
    {
        s->pos += strlen("var");
        char *id = parse_ident(s);
        if (!id)
            return verror("Invalid let statement.");
        skip_ws(s);
        if (match_char(s, '='))
        {
            Value *v = eval_expr(s, env);
            // unwrap return
            if (v->type == T_RETURN)
            {
                skip_ws(s);
                match_char(s, ';');
                Value *tmp = v->v.opaque;
                env_set_local_raw(env, id, tmp);
                free(id);
                val_release(v);
                return tmp;
            }
            else if (v->type == T_FUNCTION)
            {
                v->v.fn->name = strdup(id);
            }
            skip_ws(s);
            match_char(s, ';');
            if (v->type != T_STRING)
                env_set_local(env, id, v);
            else
                env_set_local_raw(env, id, val_retain(v));
            free(id);
            return v;
        }
        else if (match_char(s, ':'))
        {
            Value *v = eval_statement(s, env);

            // unwrap return
            if (v->type == T_RETURN)
            {
                skip_ws(s);
                match_char(s, ';');
                Value *tmp = v->v.opaque;
                env_set_local_raw(env, id, tmp);
                free(id);
                val_release(v);
                return tmp;
            }
            else if (v->type == T_FUNCTION)
            {
                v->v.fn->name = strdup(id);
            }

            skip_ws(s);
            match_char(s, ';');
            if (v->type != T_STRING)
                env_set_local(env, id, v);
            else
                env_set_local_raw(env, id, val_retain(v));
            free(id);
            return v;
        }
        else
        {
            // declare none
            env_set_raw(env, id, vnone());
            free(id);
            match_char(s, ';');
            return vnull();
        }
    }
    if (is_keyword_at(s, "return"))
    {
        s->pos += strlen("return");
        Value *v = eval_expr(s, env);
        match_char(s, ';');
        // wrap as return value: create T_RETURN whose opaque pointer contains the actual Value*
        Value *r = val_new(T_RETURN);
        r->v.opaque = v;
        return r;
    }
    if (is_keyword_at(s, "if"))
    {
        s->pos += strlen("if");
        skip_ws(s);
        if (match_char(s, '('))
        {
            Value *cond = eval_expr(s, env);
            match_char(s, ')');
            int truth = is_truthy(cond);
            val_release(cond);
            skip_ws(s);
            if (truth)
            {
                Value *res = NULL;
                if (src_peek(s) == '{')
                    res = eval_block(s, env);
                else
                    res = eval_statement(s, env);

                clean_elif_chain(s);

                HANDLE_CONTROL(res);
            }
            else
            {
                // skip then clause
                if (src_peek(s) == '{')
                    skip_block(s);
                else
                    skip_stmt(s);
                // check elifs
                skip_ws(s);
                while (is_keyword_at(s, "elif"))
                {
                    s->pos += strlen("elif");
                    skip_ws(s);
                    if (match_char(s, '('))
                    {
                        Value *cond = eval_expr(s, env);
                        match_char(s, ')');
                        if (is_truthy(cond))
                        {
                            Value *res = NULL;
                            if (src_peek(s) == '{')
                                res = eval_block(s, env);
                            else
                                res = eval_statement(s, env);

                            clean_elif_chain(s);
                            val_release(cond);
                            HANDLE_CONTROL(res);
                        }
                        else
                        {
                            val_release(cond);
                            if (src_peek(s) == '{')
                                skip_block(s);
                            else
                                skip_stmt(s);
                        }
                    }
                    skip_ws(s);
                }
                // run else if it exists
                if (is_keyword_at(s, "else"))
                {
                    s->pos += strlen("else");
                    Value *res = NULL;
                    skip_ws(s);
                    if (src_peek(s) == '{')
                        res = eval_block(s, env);
                    else
                        res = eval_statement(s, env);
                    // check for return propagation
                    HANDLE_CONTROL(res);
                }
            }
        }
        return vnull();
    }
    if (is_keyword_at(s, "while"))
    {
        s->pos += strlen("while");
        skip_ws(s);
        if (match_char(s, '('))
        {
            uint64_t cond_start_pos = s->pos;
            skip_expr(s);
            match_char(s, ')');
            uint64_t body_start_pos = s->pos;
            skip_block(s);
            uint64_t body_end_pos = s->pos;
            Value *bod = vnull();

            // we execute the loop until the condition is false or a control-flow statement is hit
            while (1)
            {
                // Reset position to condition start for re-evaluation
                s->pos = cond_start_pos;

                // Re-evaluate condition
                Value *cond = eval_expr(s, env);
                skip_ws(s);
                match_char(s, ')');

                if (!is_truthy(cond))
                {
                    val_release(cond);
                    skip_block(s);
                    return vnull();
                }
                val_release(cond);
                // reset the position to the start of the body for execution
                s->pos = body_start_pos;
                if (bod)
                    val_release(bod);
                bod = eval_block(s, env);

                // --- Handle body result ---
                if (bod->type == T_BREAK)
                {
                    s->pos = body_end_pos;
                    return vnull();
                }
                else if (bod->type == T_CONTINUE)
                {
                    s->pos = cond_start_pos;
                    continue;
                }
                else if (bod->type == T_RETURN)
                {
                    Value *res = bod->v.opaque;
                    val_release(bod);
                    s->pos = body_end_pos;
                    return res;
                }
            }
            return bod;
        }
    }
    if (is_keyword_at(s, "foreach"))
    {
        s->pos += strlen("foreach");
        skip_ws(s);
        char *id = parse_ident(s);
        if (id)
        {
            skip_ws(s);
            match_char(s, ':');
            Value *iter_obj = eval_expr(s, env);
            Value **value = NULL;

            if (iter_obj->method_table && (!iter_obj->method_table[UMethodToIter].is_binop) && iter_obj->method_table[UMethodToIter].unary)
            {
                Value *v = iter_obj->method_table[UMethodToIter].unary(iter_obj);
                value = v->v.opaque;
                if (!value) {
                    free(id);
                    return verror("Value returned by ");
                }
                val_kill(v);
            }
            else
            {
                free(id);
                return verror("Type %s does not implement UMethodToIter", iter_obj->type_name ? iter_obj->type_name : MILA_TYPE_NAMES[iter_obj->type]);
            }
            skip_ws(s);

            uint64_t body_start_pos = s->pos;
            skip_block(s);
            uint64_t body_end_pos = s->pos;
            Value *bod = NULL;

            // we execute the loop until the condition is false or a control-flow statement is hit
            size_t i = 0;
            Value *v = value[i];
            i++;
            for (; v; v = value[i++])
            {
                // reset the position to the start of the body for execution
                s->pos = body_start_pos;
                Env *frame = env_new(env);
                env_set_local_raw(env, id, v);
                bod = eval_block_raw(s, frame);
                env_free(frame);
                // --- Handle body result ---
                if (bod->type == T_BREAK)
                {
                    s->pos = body_end_pos;
                    val_release(bod);
                    return vnull();
                }
                else if (bod->type == T_CONTINUE)
                {
                    s->pos = body_start_pos;
                    if (bod)
                        val_release(bod);
                    continue;
                }
                else if (bod->type == T_RETURN)
                {
                    Value *res = bod->v.opaque;
                    val_release(bod);
                    s->pos = body_end_pos;
                    return res;
                }

                val_release(bod);
            }
            s->pos = body_end_pos;
            free(id);
            return bod;
        }
    }
    if (is_keyword_at(s, "block"))
    {
        s->pos += strlen("block");
        skip_ws(s);
        char *name = parse_ident(s);
        if (!name)
            return verror("Block needs a name!");
        skip_ws(s);
        Value *res = NULL;
        res = eval_block(s, env);
        if (res->type == T_ERROR)
        {
            Value *new_res = verror("Block %s reported an error: %s", name, res->v.message);
            val_release(res);
            free(name);
            return new_res;
        }
        free(name);
        return res;
    }
    if (is_keyword_at(s, "catch"))
    {
        s->pos += strlen("catch");
        skip_ws(s);
        Value *res = NULL;
        res = eval_block(s, env);
        if (res->type == T_ERROR)
        {
            val_release(res);
            return vnull();
        }
        return res;
    }
    skip_ws(s);
    // block
    if (src_peek(s) == '{')
    {
        Env *frame = env_new(env);
        Value *res = eval_block(s, frame);
        env_free(frame);
        return res;
    }
    // expression statement
    Value *e = eval_expr(s, env);
    match_char(s, ';');
    if (e && e->type == T_FUNCTION || e->type == T_NATIVE) {
        Value* res = call_function_with(env, e, NULL);
        return res;
    }
    return e;
}

int match_types(Value **args, ...)
{
    va_list types;
    va_start(types, args);
    ValueType current;
    for (int i = 0; (current = va_arg(types, ValueType)) != T_ARG_END; i++)
        if (current != args[i]->type)
            goto f;

    va_end(types);
    return 1;
f:
    va_end(types);
    return 0;
}

// vfunction creation
Value *vfunction(char **params, char *body_src, Env *closure)
{
    Value *v = val_new(T_FUNCTION);
    v->v.fn = (FunctionV *)malloc(sizeof(FunctionV));
    v->v.fn->params = params;
    v->v.fn->body_src = body_src;
    v->v.fn->closure = closure;
    return v;
}

// top-level eval of source - runs sequential statements in global env
Value *eval_source(Src *s, Env *env)
{
    Value *last = vnull();
    while (!src_eof(s))
    {
        skip_ws(s);
        if (src_eof(s))
            break;
        Value *st = eval_statement(s, env);
        
        val_release(last);
        last = st;
        if (last && last->type == T_ERROR)
        {
            printf("\n= Error: %s\n", last->v.message);
            return last;
        }
    }
    return last;
}

Value *eval_str(char *src, Env *env)
{
    Src *S = src_new(src);
    Value *res = eval_source(S, env);
    src_free(S);
    return res;
}

int run_file(char *name, Env *env)
{
    char *src_text = NULL;
    FILE *f = fopen(name, "rb");
    if (!f)
    {
        fprintf(stderr, "Cannot open %s\n", name);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    src_text = malloc(size + 1);
    fread(src_text, 1, size, f);
    src_text[size] = 0;
    fclose(f);
    Src *S = src_new(src_text);
    Value *res = eval_source(S, env);
    val_release(res);
    src_free(S);
    free(src_text);
    return 0;
}

int needs_more(const char *src)
{
    int parens = 0, braces = 0;
    int in_string = 0;

    for (const char *p = src; *p; p++)
    {
        if (*p == '"' && (p == src || *(p - 1) != '\\'))
        {
            in_string = !in_string; // toggle string state
        }
        else if (!in_string)
        {
            if (*p == '(')
                parens++;
            else if (*p == ')')
                parens--;
            else if (*p == '{')
                braces++;
            else if (*p == '}')
                braces--;
        }
    }

    // if inside a string or any unbalanced delimiter, we need more input
    return in_string || parens > 0 || braces > 0;
}

#ifndef ML_LIB
int main(int argc, char **argv)
{
    // read file if provided or use built-in demo
    char *src_text = NULL;
    if (argc == 2)
    {
        if (strcmp(argv[1], "--info") == 0)
        {
            printf(
                "MiLa - Info\n"
                "Version: 1.0\n\n"
                "Variable size:\n"
                "  %lu Bytes\n"
                "  %lu Bytes for worst case (boxed value is not counted)\n"
                "Estimated memory:\n"
                "  t * %lu + n * 40 Bytes\n"
                "  n = # of vars\n"
                "  t = # of types\n"
                "Max num digits:\n"
                "  %i\n",
                sizeof(Value),
                sizeof(Value) + sizeof(MethodTable),
                sizeof(MethodTable),
                MAX_NUMBER_DIGITS);
            return 0;
        }
        else if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)
        {
            printf(
                "MiLa Specification v1.0\n"
                "CLI v1.0\n"
                "API v1.0\n");
            return 0;
        }
        else if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
        {
            printf(
                "MiLa v1.0\n"
                "  --info         = For internal info as well as version info\n"
                "  --version | -v = Prints version\n"
                "  --help    | -h = Prints this list\n");
            return 0;
        }
    }

    // prepare global env
    Env *g = env_new(NULL);
#ifndef MILA_USE_SHARED
    // register native functions
    env_register_builtins(g);
    env_set_raw(g, "__mila_builtins_dynamic", vbool(0));
#else
    // allows users to use so files instead.
    // Must be on LD_PATH
    if (load_library(g, "mila_builtins.so"))
        env_set_raw(g, "__mila_builtins_dynamic_failed", vbool(1));
    env_set_raw(g, "__mila_builtins_dynamic", vbool(1));
#endif

    // Check if built ins is the canonical
    Value *builtins_flag = env_get(g, "__mila_canonical_builtins");
    int is_builtins = builtins_flag != NULL &&
                      builtins_flag->type == T_INT &&
                      builtins_flag->v.i == 202603L;

    search_path = path_list_new();

    if (argc >= 2)
    {
        FILE *f = fopen(argv[1], "rb");
        if (!f)
        {
            fprintf(stderr, "Cannot open %s: Missing or not a file.\n", argv[1]);
            env_kill(g);
            return 1;
        }

        char out[MAX_PATH_LENGTH] = {0};
        path_dirname(argv[1], out, sizeof(out));
        path_list_add(search_path, out);

        char cwd[MAX_PATH_LENGTH] = {0};
        path_get_cwd(cwd, 1024);
        path_list_add(search_path, cwd);
        path_list_add(search_path, "~/.local/lib/mila");

        // argv handling is the only part that touches the builtins.

        // make sure we are using the bundled canonical builtins
        // otherwise set argv as __argv with the type opaque
        Value *array = NULL;
        if (is_builtins)
        {
            array = call_function_str(g, "array", vint(argc - 1), NULL);
            for (int i = 1; i < argc; i++)
                val_release(call_function_str(g, "array.set", val_retain(array), vint(i - 1), vstring_dup(argv[i]), NULL));
            env_set(g, "argv", array);
        }
        else
        {
            env_set_raw(g, "__argc", vint(argc));
            env_set_raw(g, "__argv", vopaque(argv));
            env_set_raw(g, "argv", vnone());
        }

        // read file
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        src_text = malloc(size + 1);
        fread(src_text, 1, size, f);
        src_text[size] = 0;
        fclose(f);

        Src *S = src_new(src_text);
        Value *res = eval_source(S, g);

        // cleanup
        val_release(res);
        src_free(S);

        free(src_text);

        if (is_builtins)
        {
            val_release(call_function_str(g, "array.free", array, NULL));
            call_function_str(g, "__mila_canonical_builtins_free", NULL);
        }

        env_free(g);
    }
    else
    {
        char cwd[MAX_PATH_LENGTH] = {0};
        path_get_cwd(cwd, 1024);
        path_list_add(search_path, cwd);
        path_list_add(search_path, "~/.local/lib/mila");

        printf("MiLa REPL\n");
        printf("Running MiLa '%s'\n", env_get(g, "__mila_codename") ? env_get(g, "__mila_codename")->v.s : "???");

        // Notify users when MiLa is built using the canonical builtins
        if (is_builtins)
        {
            printf("Cannonical Builtins (%ld) version %ld\n",
                   env_get(g, "__mila_canonical_builtins")->v.i,
                   env_get(g, "__mila_canonical_builtins_version")->v.i);
        }

        if (mila_is_builtins_dynamic)
        {
            printf("Builtins loaded via shared object.\n");
            if (env_get(g, "__mila_builtins_dynamic_failed"))
                printf("INFO: Loading mightve failed!\n");
        }
        else
        {
            printf("Builtins embedded.\n");
        }

        char line[2048];
        char buffer[8192]; // accumulated snippet
        buffer[0] = 0;

        printf(">>> ");

        while (fgets(line, sizeof(line), stdin))
        {
            // append line to buffer
            strcat(buffer, line);
            fflush(stdout);

            if (strncmp(buffer, ".mem", 4) == 0)
            {
                print_memory_usage();
                buffer[0] = 0;
                printf(">>> ");
                continue;
            }

            // check if expression is syntactically complete
            if (!needs_more(buffer))
            {
                // evaluate accumulated buffer
                Src *S = src_new(buffer);
                Value *res = eval_source(S, g);
                
                if (res && res->type != T_NULL){
                    printf("  : ");
                    print_value_repr(res);
                    putchar('\n');
                } else if (!res) {
                    printf("  : cnull\n");
                }

                val_release(res);
                src_free(S);

                // clear buffer
                buffer[0] = 0;

                printf(">>> ");
            }
            else
            {
                // prompt for continuation
                printf("... ");
            }
        }

        env_free(g);
    }
    return 0;
}
#endif