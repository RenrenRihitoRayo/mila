# Welcome to MiLa

A small embeddable scripting language.
MiLa is if C and Lua made a child,
small simple and fast.

## What MiLa is

* A scaffold

MiLa is the build-a-bear of scripting.
Directly add bash commands as builtin functions?
Why not?

* A kernel

MiLa already has the bare minimum to work.
You can even make a programming language using
MiLa.

* The "builtins" is not mandatory.

Building MiLa with "make bare" creates a compiled version of MiLa
that loads `mila_builtins.so` from anywhere in `LD_PATH`.
This allows MiLa as a CLI to be dynamic and patchable.
You can check MiLa can "function" with the bear minimum by doing the follwing in
the REPL.

```Plaintext
...
>>> // returns the edition (example 202603) of the canonical built ins
>>> // (the one packaged with MiLa by default, other implementations must skip setting this)
>>> __mila_canonical_builtins
  : 202603
>>> __mila_canonical_builtins_version // returns the version of the current loaded builtins
  : 1
>>> __mila_builtins_dynamic // if MiLa is built using 'make bare' this should be true
  : true
>>> __mila_builtins_dynamic_fail // if loading failed this should be set otherwise its null
>>> __mila_codename // code name for the loaded builtins
  : canon
```

## What MiLa is not

* A serious language

This is a side project.
Do not treat MiLa as an actual production
ready language, simply because it isnt.

* Community Driven

This Repo of MiLa will never accept
pull requests pertaining to adding features.
MiLa is a kernel, not a language.

## Rules to Forking MiLa

Credit me as its owner,
you cannot earn momey from selling copies of MiLa
or you will be held acountable for it.
<br><br>
Make sure your fork of MiLa still
follows this specs defined bellow.
<br><br>
You may add features but never modify syntax.
Such as changing syntax of while loops.

## Keywords (19 total)

* if
* elif
* else
* fn (5 variants)
* return
* catch (2 variants)
* block
* set (3 variants)
* var
* foreach
* while
* break
* continue
* true
* false
* null
* none
* export
* contextual

Total of 25 different constructs.

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

## Examples

* Assignment

```Plaintext
// Declaration
var name = value;
{
    // This is a different variable
    // from the outer name.
    var name = value;
    
    // Assignment
    set name = value;
}
```

* Functions

```Plaintext
fn greet(name) {
    println("Hello " + name + "!");
}

greet("Gene");

fn greet_ctx()[name] {
    println("Hello " + name + "!");
}

{
    var name = "Gene";
    contextual name;
    greet_ctx();
}

fn make_greet(name) {
    fn greet_closure():[name] {
        println("Hello " + name + "!");
    }
}

var greet_closure = make_greet("Gene");
greet_closure();
```

* Loops

```Plaintext
let i = 0;
let sum = 0;
while (i < 10) {
    let sum = sum + i;
    let i = i + 1;
}

var l = array(5);
set l[0]= "where";
set l[1] = "what";
set l[2] = "when";
set l[3] = "who";
set l[4] = "why";

foreach item : l {
    println(item);
}

foreach n : range(10) {
    println(n);
}
```

* Implicit and Explicit return

```Plaintext
let res = fn{
    90;
};

// or

let res = fn{
    /* this is preferred for readability */
    return 90;
};
```

* Block var/set statements

```Plaintext
// notice we use a colon, not an equal sign
let res : {
    return 90;
} // no semi colon
```

* If chains

```Plaintext
if (cond) {
    ...
} elif (cond) {
    ...
} else {
    ...
}
```

* Subscripting

```
var a = array.from("hello", 42, 138);
// hello 42 138
println(a[0], a[1], a[2]);

var d = dict(
    "name", "Ezha",
    "age", 17,
    "addresses", dict(
        0, "city 1",
        1, "city 2"
    )
);

// no support for nested subscription when setting (yet)
set tmp = d["addresses"];
set tmp[2] = "city 3";

println(d["name"], d["age"]);
println((d["addresses"])[0]);
println((d["addresses"])[1]);
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
    
    run_file(e, "some_file.mila");
    
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

## Creating types

#### char*

* vstring\_take(char*)

If the pointer is already owned

* vstring\_dup(const char*)

Used with const char* also duplicating strings.

#### Numbers

* vint(long)

* vfloat(double)

#### MiLa Type Map

* MiLa int    = C long
* MiLa float  = C double
* MiLa string = C char*
* MiLa opaque = C void*
* MiLa native = C Value*(*NativeFn)(Env* e, int argc, Value** argv)

##### Types that dont have direct C equivalents

The types bellow requires structured data types
to implement.

* MiLa Function

```C
struct {
    char *name;
    char **params;
    char *body_src;             
    Env *closure;
}
```

## How MiLa Manages Memory

MiLa uses a refcount system for managing memory.
Particularly the functions `val_retain`, `val_release`, and
`val_kill`.

* val\_retain

Increament ref count.

* val\_release

Decreament ref count and free if zero.

* val\_kill

Free memory no matter if ref count isnt zero.

Types that depends on these is

* strings
* functions

# Everything beyong this point may be implementation specific.

## Parameter Notation and Return Notation

This is only for docs.

* ??

Opaque type

* id?

Optional parameter

* id!

`id` is mutated in that function!

* id

Required parameter

* fun() -> type

`fun` returns type `type`

* fun(...)

`...` means any argument

* !!

`@` or `id@` means any single type

## Built in functions

### Text IO

* println(...)

Prints given arguments, adds a trailing newline,
each argument separated with a space.

* printr(...)

Prints given arguments with no space in between
and no trailing newline.

* print(...)

Prints given arguments with a space in between
but with no trailing newline.

* input(prompt?) -> str

Prompts user to enter a string.

### File IO

* open(file, mode) -> ??

Like fopen, returns opaque.

* fread(file, bytes) -> str

Like fread.

* fprint(file, text) -> int

Like fprintf.

* fseek(file, pos, whence) -> int

Like fseek.

* ftell(file) -> int

Like ftell.

* fclose(file)

Like fclose.

### Casting

* cast.*

Types are `int`, `float`, and `string`.

* typeof(@)

### Math

* pow(base, exp) -> int

* ceil(flt) -> flt

* floor(flt) -> flt

* tan(i) -> int

* sin(i) -> int

* cos(i) -> int

* atan2(i) -> int

* sqrt(i) -> float

### Bitwise

* and(i, i)

* or(i, i)

* xor(i, i)

### Boolean

* not(i)

boolean "and" and "or" is "&&" and "||" respectively

### str.*

* slice(str, index, size) -> str

str.slice("hello", 2, 3) == "llo"

* index(str, index) -> str

str.index("hello", 1) == "e"

* patch(str, needle, replacement) -> str

str.patch("hello", "e", "wow") == "hwowllo"

* pop\_f(str!) -> str

let str = "hello";
str.pop\_f(str) == "h"
str == "ello"

* pop\_b(str!) -> str

let str = "hello";
str.pop\_b(str) == "o"
str == "hell"

* length(str) -> int

str.length("this") == 4

### ascii.*

* ascii.to(code) -> str

ascii.to(65) == "A"

* ascii.from(str) -> int

ascii.from("A") == 65

### Error handling

* exit(code?) -> never

Exits with the given exit code, 0 otherwise

* report(message) -> error

Raises an error

### OS

* system(cmd) -> int

Runs cmd on the systems console, returns exit code
of the command.

### Module Related

* eval(str) -> @

Evaluates the given string.

* run(path) -> int

Runs the file given via its path.

* load(abs\_path) -> null

Loads the given .so or .dll file given via
its path.

### Array

Protected access.

* array(size) -> array??

Allocates `size` elements.
All elements are null by default.

* array.set(array, index, value@)

Set the `index`th value to `value

* array.get(array, index) -> @

Get the element in that index.

* array.len(array) -> index

Returns number of allocated elements
not the current set elements.
(array isnt a linked array, fragmentation
may occur and skew actual count)

* array.free(array)

Free the opaque type (array obj itself).

### Dict

* dict() -> dict??

* dict.set(d, name, value@)

Sets the key `name` to `value`.

* dict.get(d, name)

Gets the key `name`, null if doesnt exist.

* dict.rem(d, name)

Remove that key.

* dict.free(d)

Free the opaque type (dict object itself).

### Misc

* get_time() -> float

Returns the current unix timestamp.
