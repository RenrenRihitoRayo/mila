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
* namespace
* set (3 variants)
* var
* contextual
* sync
* export
* yield (from `foreach yield`)
* object (2 variants)
* foreach (2 variants)
* while
* break
* continue
* true
* false
* null
* none

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

|    Character    | C Equivalent |
|----------------:|:-------------|
|    \\n          |     \\n      |
|    \\f          |     \\f      |
|    \\v          |     \\v      |
|    \\t          |     \\t      |
|    \\b          |     \\b      |
|    \\r          |     \\r      |
|    \\a          |     \\a      |
|    \\xhh        |     \\xhh    |
|    \\0oo        |     \\0oo    |
|    \\Ndd        |     None     |
|    \\N{integer} |     None     |
|    \\uxxxx      |     None     |
|    \\Uxxxxxxxx  |     None     |

`\N` and `\N{integer}` accepts decimal digits.

## Primitives

MiLa defines primitives as values that are supported by the language directly.
This includes having operations that are hardcoded into the language (being supported
on the syntax level is not enough to be considered a primitive in MiLa.), or being
part of the implementation that is critical.

True Primitives in MiLa are (lowest possible level of abstraction):
* Strings
* Integer variants
* Float variants
* none
* null
* booleans
* Functions
* Opaque variants
True Primitives that are hidden from the user:
* Error values
* Control values

False Primitives (part of the runtime but not really supported in a critical level):
* Dictionaries
* Lists (this is a half exception as it does have behavior the runtime itself dictates)
* Arrays

False primitives are implemented using the `opaque` type and use operator overloading via
Value Instance Operator Overloading (VIOO) which is faster than Object Instance Operator Overloading (OOIO).

None Primitves:
* User defined objects

## VIOO vs OIOO

VIOO gives the implementor as much control as possible.
OIOO gives the implementor as much convenience as possible.

VIOO is a static array of function methods which cut the dispatch to O(1).
OIOO is a dynamic object which has O(n) amortized dispatch.

VIOO tries to minimies the amount of context switches (runtime to C then back).
OIOO minimizes low level mental overhead (you still write in MiLa).

VIOO gives you access to deallocation logic.
OIOO gives you basic arithmetic overloading and boolean coercion.

VIOO is perfect for wrapping C types.
OIOO is perfect for convenient abstraction.

VIOO for speed and control.
OIOO for convenience and abstraction.

## Example

### Hello world
```MiLa
println("Hello, world!");
```

### Fibbonacci
```MiLa
fn fib(n) {
    var a = 1;
    var b = 0;
    var c = 0;
    while (n > 0) {
        set c = a + b;
        set b = a;
        set a = c;
        set n -= 1;
    }
    return b;
}

println(fib(10)); // 55
```


## Example of using MiLa (in C)

```C
// for impl otherwise use mila.h
#define ML_LIB
#include "mila/mila.c"

int main(void) {
    // set up env
    Env* e = mila_global_init();
    
    // set variables
    env_set_raw(e, "var", vint(42));
    
    // set local
    env_set_local_raw(e, "var2", vfloat(84.0));
    
    // run code
    Value* v = eval_str("println(\"Hello, world!\", var);", e);
    
    // delete a variable
    env_remove(e, "var");
    
    // Clean up
    val_release(v); 
    mila_global_deinit(e);
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
    Env* e = mila_global_init();
    
    run_file("some_file.mila", e);

    // Below keeps last statements value

    Value* res = run_file_keep_res("some_file.mila", e);
    val_release(res);
    
    val_release(v); 
    mila_global_deinit(e);
    return 0;
}

```

### Writing a shared library for MiLa

```C
#include <stdio.h>
#include "mila.c"

Value* hello(Env* e, int argc, Value** argv) {
    // keep in mind MiLa doesnt check argument counts
    // you the author does.
    printf("Hello from C!\n");
    return vnull();
}

// Optional init function
void _mila_lib_init(Env* e) {
    printf("Lib init!\n");
}

// Optional init function
void _mila_lib_deinit(Env* e) {
    printf("Lib cleanup!\n");
}

// You can choose between the two bellow to export functions

// Simple way (recomeded for C codebases)
const char* lib_functions[] = {
    "hello",
    NULL
};

// More stable way (for C++ code bases where name mangling may take place, prioritized over lib_functions)
const NativeEntry lib_function_entries[] = {
    {.name = "hello", .func = hello},
    {NULL, NULL}
};
```