# Built-in Functions and Variables

The MiLa builtin functions.<br>
These collection of MiLa functions are always present
and doesnt require you to run an external file.

* [Text IO](#io-text)
* [File IO](#io-file)
* [File Operations](#file-ops)
* [Lists](#list)
* [Dictionaries](#dict)
* [Arrays](#arr)
* [Sorting](#sort)
* [Environments](#env)
* [Strings](#str)
* [Math](#math)
    * [Bitwise Logic](#math-bit)
* [Types](#cast)
* [Time](#time)
* [System](#system)
* [Running and Loading](#run)
* [Error Handling](#error)
* [JSON and MJSON](#json)
* [Threading](#th)
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

* `open(file: "string", mode: "string") -> "opaque:file"`

    Open a file, uses C `fopen`.

* `fdopen(fd: "int", mode: "string") -> "opaque:file"`

    Open a file descriptor.

* `fdredirect(to_fd: "[int,opaque:file]", from_fd: "[int,opaque:file]") -> "int"`

    Redirect `from_fd` into `to_fd` and return `from_fd`

* `fileno(file: "opaque:file") -> "int"`

    Return the file as a file descriptor.

* `fread(file: "opaque:file", num: "int") -> "string"`

    Read a certain amount of characters.

* `fread_all(file: "opaque:file") -> "string"`

    Read the entire file.

* `fread_bytes(file: "opaque:file", num: "int") -> "list[int]"`

    Read a certain amount of bytes.

* `fread_all(file: "opaque:file") -> "list[int]"`

    Read the entire file as bytes.

* `fprint(file: "opaque:file", value: "string")`

    Print the string into the file.

* `fprint_bytes(file: "opaque:file", value: "list[int]")`

    Print the bytes into the file.

* `ftell(file: "opaque:file") -> "int"`

    Return the current cursor position.

* `fseek(file: "opaque:file", offset: "int", whence: "int") -> "int"`

    Exactly like C fseek.
    For whence use the values `SEEK_CUR`, `SEEK_SET`, and `SEEK_END`

* `fclose(file: "opaque:file")`

    Close a file.

* `close(fd: "int")`

    Close a file descriptor.

### Variables related to File IO

* `SEEK_CUR`, `SEEK_END`, and `SEEK_SET`

    Integer values used for the `whence` paramater in `fseek`.

* `stderr`

    Standard error file.

* `stdout`

    Standard out file.

* `stdin`

    Standard in file. (safe for piping data in)

* `stderr_fd`

    Standard error file descriptor.

* `stdout_fd`

    Standard out file descriptor.

* `stdin_fd`

    Standard in file descriptor. (safe for piping data in)

## <a id="file-ops"></a>File Operations

* `file.exists(path: "string") -> "bool"`

    Check if a file exists.

* `file.is_file(path: "string") -> "bool"`

    Check if a path is a file.

* `file.is_dir(path: "string") -> "string"`

    Check if a path is a directory.

* `file.list_dir(path: "string") -> "list[dict]"`

    Return the contents of a directory.
    Return a dict with the following keys:
    
    * "name"
    
        File name.
    
    * "type"
    
        File type.
        * "d" for directory
        * "f" for file
        * "?" for others

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
    Example: `list.deconstruct("[...a, b]", [90, 70, 80])`
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

    Syntax to index an array item.
    Returns `null` when it isn't found.

* `set some_array[index] = value;`

    Set an arrays item.

## <a id="sort"></a>Sorting

* `qsort(obj: "opaque:list", function: "<callable>") -> "opaque:list"`

    Sort the given list, return the sorted list.
    The function accepts two arguments a and b, the function needs to return these values
    in correlation of which value is bigger. A positive value means a is greater than b, a negative value
    means b is greater than a, 0 means they are equal. So in most cases function may just as well be:
    ```MiLa
    fn compare(a, b) { return a - b; }
    ```
    The qsort function works as it does in the GNU C standard library.

## <a id="env"></a>Environments

* `env.set(name: "string", value: "any") -> int`

    Sets a variable in which scope has the pre-existing variable specified in `name`.
    If there are no variables in any subsequent scope, it sets the variable in the current scope.
    The function returns `1` when it fails to set the variable, this can happen if it finds a binding BUT is a constant,
    otherwise it returns `0` on success.

* `env.set_local(name: "string", value: "any") -> int`

    Sets a variable in the local scope with the specified name `name`.
    The function returns `1` if the variable `name` already exists and is constant otherwise 
    on success it returns `0`.

* `env.get(name: "string") -> "any"`

    Return the variable specified in `name`.
    Return `null` when the variable specified by `name` is not found or is not set.

* `env.get_names() -> "opaque:list[str]"`

    Returns a list of variable names found in the current scope.

* `env.get_type(name: "string") -> "string"`

    Returns a string representing the type of a variable.
    It returns the most shallowest binding.
    ```MiLa
var num: "int"= 0;
{
    var num: "float" = 0.0;
    println(env.get_type("num")); // float
}
    ```

* `export(d: "opaque:dict[str, any]")`

    Accepts a dictionary.
    Exports to the surrounding scope.

## <a id="str"></a>Strings

* `str.slice(str: "index", index: "int", len: "int") -> "string"`

    Slice a string. Note MiLa strings are immutable.

* `str.index(str: "index", index: "int") -> "string"`

    Index a character in a string.

* `str.patch(str: "string", needle: "string", replacement: "string") -> "string"`

    Replace every occurrence of the needle with the given
    replacement.

* `str.copy(str: "string") -> "string"`

    Copy a string.

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

* `str.match_findx(str: "string", pattern: "string") -> "list[int]"`

    Find the first match from the left of `pattern` in `str` and return the index to the first character of the match
    as the first item and return the length of the match as the second item.

* `str.toupper(str: "string") -> "string"`

    Self explanatory name.

* `str.tolower(str: "string") -> "string"`

    Self explanatory name.

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

* `as_opaque(any) -> "opaue"`

    Cast any type into an opaque.<br>
    May only work for certain primitives like:
    
    * int
    * float
    * uint
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

* `repr(any: "any") -> "string"`

    Cast a string to its representation.
    Useful for logging.

* `repr_raw(any: "any") -> "string"`

    Ignore the values set representation
    and use only the built in representations.

* `own(opq: "opaque") -> "owned_opaque"`

    Cast any opaque to an opaque.

* `unown(opq: "owned_opaque") -> "opaque"`

    Cast any opaque to an unowned opaque.

* `is_numeric(any: "any") -> "bool"`

    Returns true if a type is numeric.

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
    
    * "x86_64" (tested)
    
    * "x86" (not tested yet)
    
    * "arm64" (tested)
    
    * "arm" (not tested yet)
    
    * "riscv" (not tested yet)
    
    * "ppc" (not tested yet)
    
    * "mips" (not tested yet)
    
    * "unknown" (good luck)

* `sys.get_pid() -> "int"`

    Returns the interpreters PID.

* `sys.getenv(name: "string") -> "int"`

    Retrieves a environment variable and returns the string value.
    If the environment variable is unset, the function returns null.

* `sys.setenv(name: "string", value: "string") -> "int"`

    Sets an environment variable, returns the error code.
    Error code corresponds to `setenv` library function return codes.


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
    if used in the main interpreter this just exits.

## <a id="json"></a>JSON and MJSON

* `json.loads(json: "string") -> "opaque:list|opaque:dict"`

    Loads a json string as MiLa types.

* `json.dumps(mila: "opaque:list|opaque:dict") -> "string"`

    Dumps a MiLa type as json.

* `json.dumps_io(file: "opaque:file", mila: "opaque:list|opaque:dict") -> "string"`

    Dumps a MiLa type as json directly into a file.

* `mjson.loads(mjson: "string") -> "opaque:list|opaque:dict"`

    Loads an mjson string as MiLa types.

* `mjson.dumps(mila: "opaque:list|opaque:dict") -> "string"`

    Dumps a MiLa type as mjson.

* `mjson.dumps_io(file: "opaque:file", mila: "opaque:list|opaque:dict") -> "string"`

    Dumps a MiLa type as mjson directly into a file.

## MJSON vs JSON

* MiLa - JS Object Notation

    Is just json with MiLa functions.
    MJSON does not allow expression as values and thus
    does not execute code even when loading an mjson file.

* JSON (MiLa implementation)

    Supports:
    - Trailing Commas
    - Multi line and single line comments
    - Identifiers as keys
    
    Note every standard JSON is valid MJSON.

## <a id="th"></a>Threading

* `thread.make(func: "function", on_kill: "function") -> "int"`

    Create a thread and run it immediately, returns a thread ID.
    `on_kill` runs when the thread ends regardless of cause<br>
    (on interpreter halt this still runs except for signal interupts)
    <br><br>
    `func` sigature: `fn(thread_id)`
    `on_kill` signature: `fn(thread_id, cause)`

* `thread.join(thread_id: "int")`

* `thread.cancel(thread_id: "int")`

* `thread.check_cancel(thread_id: "int")`

    Must be called inside threads that might be cancelled.
    This exists because MiLa threads are cooperative.
    Even if we use pthreads underneath, we need to comply eith platforms
    such as Android (google hates non cooperstive threading on mobile)

* `thread.set_daemon(thread_id: "int")`

* `thread.get_pthread_id(thread_id: "int")`

    Useful for debugging.
    Returns pthread id rather than MiLa handled thread IDs.

* `thread.status(thread_id: "int")`

    Returns these values:
    
    * 0: pending
    * 1: running
    * 2: done

* `thread.mutex()`

    Create a mutex.

* `thread.mutex_lock(mutex: "opaque:mutex")`

* `thread.mutex_unlock(mutex: "opaque:mutex")`

* `thread.dump(thread_id: "int")`

    Dump thread info.

## <a id="other"></a>Miscellaneous

* `_breakpoint()`

    Trigger a breakpoint trap.
    NOTICE: not all platforms may support this

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

* `hash(any: "any") -> "int"`

    Hash any value.

* `hash._get_seed() -> "int"`

    Get the seed for hashing values.
    By default this is `5381`

* `hash.set_seed(seed: "int")`

    Set the seed for hashing.
