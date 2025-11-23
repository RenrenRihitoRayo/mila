# Welcome to MiLa

A small embeddable scripting language.
MiLa is if C and Lua made a child,
small simple and fast.

## Keywords (11)

* while
* if
* elif
* else
* return
* fn
* catch
* block
* let
* break
* continue

## Examples

* Assignment

```plaintext
// Assignment not declaration
let name = value;
{
    // This is not a different variable
    // from the outer name.
    let name = value;
    
    /*
     * Scopes are only created with
     * function calls.
     */
}
```
...

* Functions (all lambdas)

```plaintext
let greet = fn(name) {
    println("Hello " + name "!");
}

greet("Name");
```

* Loops (only while loops)

```plaintext
let i = 0;
let sum = 0;
while (i < 10) {
    let sum = sum + i;
    let i = i + 1;
}
```

* Implicit and Explicit return

```plaintext
let res = fn{
    90;
};

// or

let res = fn{
    /* this is preferred for readability */
    return 90;
};
```

* Block let statements

```plaintext
// notice we use a colon, not an equal sign
let res : {
    return 90;
} // no semi colon
```

* If chains

```plaintext
if (cond) {
    ...
} elif (cond) {
    ...
} else {
    ...
}

// below is also valid
if (cond)
   ...;
elif (cond)
   ...;
else ...;
```

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

* typeof(*)

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

* pop_f(str!) -> str

let str = "hello";
str.pop_f(str) == "h"
str == "ello"

* pop_b(str!) -> str

let str = "hello";
str.pop_f(str) == "o"
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

* load(abs_path) -> null

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
(array isnt a linked list, fragmentation
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
