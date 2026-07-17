# Built-in Functions and Variables

* [Text IO](#io-text)
* [File IO](#io-file)
* [Lists](#list)
* [Dictionaries](#dict)
* [Arrays](#arr)
* [Strings](#str)
* [Math](#math)
    * [Bitwise Logic](#math-bit)
* [Types](#cast)
* [Time](#time)
* [System](#system)
* [Running and Loading](#run)
* [Error Handling](#error)
* [Others](#other)

NOTE: Not all functions are monkey patch safe!!!
Implementations may vary in how they are registered
and thus may not be guaranteed as safe for monkey patching.

## <a id="io-text"></a>Text IO

* `println(...values)`

    Print the given values, each separated with a space,
    and print a newline.

* `print(...values)`

    Print the given values, each separated with a space,
    does not print a newline.

* `input(prompt: "string"="") -> "string"`

    Print the prompt and wait for user input.

## <a id="io-file"></a>File IO

* `open(file: "string", mode: "string") -> "opaque:fd"`

    Open a file, uses C `fopen`.

* `fread(fd: "opaque:fd", num: "int") -> "string"`

    Read a certain amount of characters.

* `fread_all(fd: "opaque:fd") -> "string"`

    Read the entire file.

* `fprint(fd: "opaque:fd", value)`

    Print the value into the file.

* `ftell(fd: "opaque:fd") -> "int"`

    Return the current cursor position.

* `fseek(fd: "opaque:fd", offset: "int", whence: "int") -> "int"`

    Exactly like C fseek.
    For whence use the values `SEEK_CUR`, `SEEK_SET`, and `SEEK_END`

* `fclose(fd: "opaque:fd")`

    Close a file.

### Variables related to File IO

* `SEEK_CUR`, `SEEK_END`, and `SEEK_SET`

    Integer values used for the `whence` paramater in `fseek`.

* `stderr`

    Standard error file descriptor.

* `stdout`

    Standard out file descriptor.

* `stdin`

    Standard in file descriptor. (safe for piping data in)

## <a id="list"></a>Lists

* `[item1, item2, item3, ..., itemN]`

    Standard list syntax in MiLa.

* `list(item1, item2, item3, ..., itemN) -> "opaque:list"`

    List constructor. Same as using the list syntax.
    (calls the same underlying constructor)

* `list.append(list: "opaque:list", item)`

    Append an item to the given list.

* `list.pop(list: "opaque:list") -> "any"`

    Pop an item, returns `null` if no items are in the list.

* `list.len(list: "opaque:list") -> "int"`

    Get the length of a list.

* `list.contains(list: "opaque:list", item) -> "bool"`

    Check if an item exists in the list (equality check)

* `list.deconstruct(pattern: "opaque:list", list: "opaque:list") -> "opaque:dict"`

    Deconstruct a list into a dict.<br>
    Example: `list.deconstruct([...a, b], [90, 70, 80])`
    would become `[@ "a"=[90, 70], "b"=80]`

* `some_list[index]`

    Syntax to index a list item.
    Returns `null` when it isn't found.

* `set some_list[index] = value;`

    Set a lists item.

## <a id="dict"></a>Dictionaries

Dictionaries in MiLa uses a djb4 hash function,
this functions seed value can be customized if needed.

* `[@ key1=val1, key2=val2, key3=val3, ..., keyN=valN]`

    Standard dict syntax.

* `dict(key1, val1, key2, val2, key3, val3, ..., keyN, valN) -> "opaque:dict"`

    Dictionary constructor.

* `dict.rem(d: "opaque:dict", key: "str")`

    Remove a key-value pair from a dict.
    Does nothing when the key isn't found.

* `some_dict[key]`

    Reading a key-value pair from a dict.
    Returns `null` when it isn't found.

* `set some_dict[key] = value;`

    Set a dicts item.

## <a id="arr"></a>Arrays

Internally stored as

```C
typedef struct {
    Value** items; // actual array (when lists are too slow)
    // other fields...
} Array;
```

* `array(slots: "int") -> "opaque:array"`

    Array constructor.
    Allocates memory for the given number of slots.

* `array.from(item1, item2, item3, ..., itemN) -> "opaque:array"`

    Array constructor.
    Allocates memory for the number of arguments passed.

* `array.len(list: "opaque:array") -> "int"`

    Get the length of a list.

* `some_array[index]`

    Syntax to index am array item.
    Returns `null` when it isn't found.

* `set some_array[index] = value;`

    Set an arrays item.

## <a id="str"></a>Strings

* `str.slice(str: "index", index: "int", len: "int") -> "string"`

    Slice a string. Note MiLa strings are immutable.

* `str.index(str: "index", index: "int") -> "string"`

    Index a character in a string.

* `str.patch(str: "string", needle: "string", replacement: "string") -> "string"`

    Replace every occurrence of the needle with the given
    replacement.

* `str.copy(str: "string") -> "string"`

    Copyba string.

* `str.len(str: "string") -> "int"`

    Get the length of a string.

* `str.pop_f(str: "string") -> "string"`

    Pop the first character of a string.

* `str.pop_b(str: "string") -> "string"`

    Pop the back of the string.

* `str.split(str: "string", delim: "string") -> "opaque:list[string]"`

    Split the string into a list of strings.

* `str.join(delim: "string", items: "opaque:list") -> "string"`

    Join a list of items into a string joined by the delim string.

* `str.startswith(str: "string", prefix: "string") -> "bool"`

    Check if a string has a prefix of the given prefix string.

* `str.endswith(str: "string", suffix: "string") -> "bool"`

    Check if a string has a suffix of the given suffix string.

* `str.contains(str: "string", needle: "string") -> "bool"`

    Check if a substring exists in string.

* `str.caseless_contains(str: "string", needle: "string") -> "bool"`

    Check if a substrinf exists in string
    regardless of the character's cases.

* `str.find(str: "string", needle: "string") -> "int"`

    Find the occurrence of the needle in string
    and return the index of the first character.

* `str.caseless_find(str: "string", needle: "string") -> "int"`

    Find the occurrence of the needle in string
    and return the index of the first character,
    disregarding the character's cases.

* `str.match_replace(str: "string", pattern: "string", replacement: "string") -> "string"`

    Find every match of `pattern` in `str` and replace it with `replacement`.

* `str.match_find(str: "string", pattern: "string") -> "int"`

    Find the first match from the left of `pattern` in `str` and return the index to the first character of the match.

* `istring(str: "string") -> "opaque:istring"`

    Turn the string into an iterable string.

* `ascii.from_int(char: "int") -> "string"`

    Self explanatory name.

* `ascii.from_string(char: "string") -> "int"`

    Self explanatory name.

## <a id="math"></a>Math

Self explanatory names.

* `floor(f: "float") -> "float"`
* `ceil(f: "float") -> "float"`
* `sqrt(f: "float") -> "float"`
* `sin(f: "float") -> "float"`
* `cos(f: "float") -> "float"`
* `tan(f: "float") -> "float"`
* `atan2(f: "float") -> "float"`
* `pow(f: "float") -> "float"`
* `fabs(f: "float") -> "float"`
* `abs(i: "int") -> "int"`

### <a id="math-bit"></a>Bitwise Logic

* `and(a: "int", b: "int") -> "int"`
* `xor(a: "int", b: "int") -> "int"`
* `not(a: "int") -> "int"`

## <a id="cast"></a>Types

Self explanatory names.

* `cast.int(s: "string") -> "int"`
* `cast.float(s: "string") -> "float"`
* `cast.str(a: "any") -> "string"`

    Most useless here....

* `cast.i2u(i: "int") -> "uint"`
* `cast.u2i(u: "uint") -> "int"`
* `cast.i2f(i: "int") -> "float"`
* `cast.f2i(f: "float") -> "int"`
* `typeof(a: "any") -> "string"`

    Arguably the most useful function in MiLa

## <a id="time"></a>Time

Theres no date object shenanigans if theres no date object.

* `get_time() -> "float"`

    Returns unix timestamp.

* `time_sleep(sec: "int")`

    Sleep for a specified amount of seconds.

* `time_sleep_ms(sec: "int")`

    Sleep for a specified amount of miliseconds.

* `strftime(fmt: "string", tm: "opaque:tm") -> "string"`

    C strftime wrapper.

* `get_tm_local(time: "float") -> "opaque:tm`

    Get a `tm` struct for local time.
    Accepts unix timestamp.

* `get_tm_gmt(time: "float") -> "opaque:tm`

    Get a `tm` struct for gmt timezone.
    Accepts unix timestamp.

## <a id="system"></a>System

* `system(cmd: "string") -> "int"`

    Run a command.
    Returns the commands error code.

* `sys.get_platform() -> "string"`

    May return the following strings:
    * "win" (not tested yet)
    * "android" (tested)
    * "linux" (works on archlinux)
    * "web" (tested)
    * "mach" (not tested yet)
    * "ios" (not tested yet)
    * "tvOS" (not tested yet)
    
        Added as a joke
    
    * "apple" (not tested yet)
    
        Fall back if not on Mach, iOS, and tvOS
    
    * "unix" (not tested yet)
    
         Plausible if ported properly.
    
    * "unknown" (highly unstable)

* `sys.get_arch() -> "string"`

    May return the following strings:
    * "x86_64"
    * "x86" (not tested yet)
    * "arm64"
    * "arm" (not tested yet)
    * "riscv" (not tested yet)
    * "ppc" (not tested yet)
    * "mips" (not tested yet)
    * "unknown" (not tested yet)

* `sys.get_pid() -> "int"`

    Returns the interpreters PID.

## <a id="run"></a>Execution and Loading

* `run(file: "string") -> "any"`

    May return the scripts final statements value.
    Files may use the `return` keyword to return
    a value.

* `invoke(file: "string") -> "any"`

    Similar to `run` ***BUT*** runs the file as it
    was invoked from the CLI hence the function name.
    This triggers the execution of "init.setup-mila" files
    when present.
    <br><br>
    If you dont want to invoke files, using `run` instead
    is highly advisable.

* `eval(code: "string") -> "any"`

    Run a string.

* `load(file: "string")`

    Load a native library for MiLa.

## <a id="error"></a>Error Handling

* `exit(code: "int"=0) -> "tagged_error!"`

    Exit with the error code `code`.

* `abort() -> "!"`

    Call C abort.

* `report(message: "string") -> "error!"`

    Raise an error with the provided message.

* `report_tagged(tag: "int", message: "message") -> "tagged_error!"`

    Raise a tagged error with the provided tag and message.

* `assert(cond: "bool", message: "string") -> "tagged_error?"`

    Raise an E_ASSERT error with the given message if the condition
    `cond` is false.

### Variables for errors

* E_EXIT

    Exit.

* E_GENERIC

    Generic error. (this is the error type for untagged errors)

* E_SYNTAX_ERROR

    A kind of fatal error for syntax.

* E_FATAL

    Generic fatal error.

* E_PRE_RUNTIME

    Generic error for preruntime logic.
    
* E_RUNTIME

    Generic error for runtime logic.

* E_ASSERT

    Generic error for failed asserts.

* E_THREAD_HALT

    A force propagated non fatal error.
    Simply stops the thread that this is raised in,
    if used in the main interprter this just exits.

## <a id="other"></a>Miscellaneous

* `is(a, b) -> "bool"`

    Yay pointer equality.
    Do not use this for everything.
    These are also not true `is(none, none)` and
    `is(null, null)`

* `crandom() -> "int"`

    Returns random integers.

* `random(min: "int", max: "int") -> "int"`

    Returns a random number from the specified range
    (exclusive)

* `srandom(seed: "int")`

    Sets the seed for random numbers.
    This is important for getting actual random numbers.

* `noise(start: "int", min: "int", max: "int", mag: "int"=5) -> "opaque:list[int]"`

    Return a list of random integers.
    Magnitude is the maximum value the next integer can be far from the previous number.
    Think of this as a 1d perlin noise generator.

* `range(start: "int", stop: "int", step: "int"=1) -> "opaque:list[int]"`

    Just like in python, exlusive.

### Miscellaneous of the Miscellaneous

These two are the most unsafe but also safe.
Opaque pointers cant just be modified

* `as_opaque(any) -> "opaue"`

    Cast any type into an opaque.<br>
    May only work for certain primitives like:
    * int
    * float
    * uint
    * bfloat
    * bint
    * string

* `from_opaque(type: "string", opaque: "opaque") -> "any"`

    Cast an opaque into a type.<br>
    Supports these strings for `type`:
    * "string"
    * "owned_string"
    * "int"
    * "uint"
    * "float"
    * "long"
    * "ulong"
    * "char"
    These correspond to a C type not MiLa types.
    Under the hood MiLa int variations are actually "long" and
    "ulong".
