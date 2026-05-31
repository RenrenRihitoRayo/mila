# Welcome to MiLa

A small embeddable scripting language framework.
Canonical MiLa is small simple and fast for general computation.

## What MiLa is

* A scaffold

MiLa is the build-a-bear of scripting.
Directly add bash commands as builtin functions?
Why not?

* A kernel

MiLa already has the bare minimum to work.
You can even make a programming language using
MiLa.

## Keywords (22 total)

* if
* elif
* else
* fn (5 variants)
* return
* catch (2 variants)
* block
* set (3 variants)
* var
* foreach (2 variants)
* while
* break
* continue
* true
* false
* null
* none
* export
* contextual
* sync
* yield (from `foreach yield`)
* object(2 variants)

Total of 30 different constructs.
Less than most programming languages' number of keywords!
(Python3.12 has 35 keywords)

## Operators

|      Method      | Operation/Statement |
|-----------------:|:-------------------:|
|    BMethodAdd    |        a +  b       |
|    BMethodSub    |        a -  b       |
|    BMethodMul    |        a *  b       |
|    BMethodDiv    |        a /  b       |
|  BMethodRshift   |        a >> b       |
|  BMethodLshift   |        a << b       |
|    BMethodOr     |        a \|\| b       |
|    BMethodAnd    |        a && b       |
|  BMethodDefault  |        a ?? b       |
|  BMethodGetItem  |         a[b]        |
|  BMethodSetItem  |   set a[b] = c;     |

## Escape Sequences

| Character | C Equivalent |
|----------:|:-------------|
|    \\n    |     \\n      |
|    \\f    |     \\f      |
|    \\v    |     \\v      |
|    \\t    |     \\t      |
|    \\b    |     \\b      |
|    \\r    |     \\r      |
|    \\a    |     \\a      |
|    \\xhh    |     \\xhh      |
|    \\0oo    |     \\0oo      |
|    \\Ndd    |     \\Ndd      |

`\N` accepts decimal digits.

## Example

### Hello world
```MiLa
println("Hello, world!");
```

### Fibbonacci
```MiLa
println("Hello, world!");
```


## Example of using MiLa (in C)

```C
// for impl otherwise use mila.h
#define ML_LIB
#include "mila/mila.c"

int main(void) {
    // set up env
    Env* e = env_new(NULL);
    env_register_builtins(e);
    
    // set variables
    env_set_raw(e, "var", vint(42));
    
    // set local
    env_set_local_raw(e, "var2", vfloat(84.0));
    
    // run code
    Value* v = eval_str("println(\"Hello, world!\", var);", e);
    
    // delete a variable
    Value* tmp = env_get(e, "var");
    val_kill(tmp);
    env_set(e, "var", NULL);

    // Updated way
    // env_remove(e, "var");
    
    // Clean up
    val_release(v); 
    env_free(e);
    return 0;
}
```

### Or with a file

```C
// for impl otherwise use mila.h
#define ML_LIB
#include "mila/mila.c"

int main(void) {
    // set up env
    Env* e = env_new(NULL);
    env_register_builtins(e);
    
    run_file("some_file.mila", e);

    // Below keeps last statements value

    Value* res = run_file_keep_res("some_file.mila", e);
    val_release(res);
    
    val_release(v); 
    env_free(e);
    return 0;
}

```

### Writing a shared library for MiLa

```C
#include <stdio.h>
#include "mila.c"

Value* hello(Env* e, int argc, Value** argv) {
    (void)e; // no unused warning
    // keep in mind MiLa doesnt check argument counts
    // you the author does.
    printf("Hello from C!"):
    return vnull();
}

// Optional init function
void _mila_lib_init(Env* e) {
    (void)e;
    printf("Lib init!");
}

// You can choose between the two bellow to export functions

// Simple way (recomeded for C codebases, prioritized)
const char* lib_functions[] = {
    "hello",
    NULL
};

// More stable way (for C++ code bases where name mangling may take place)
const NativeEntry lib_function_entries[] = {
    {.name = "hello", .func = hello},
    {NULL, NULL}
};
```