# Basic MiLa Code

## Basic Constructs

MiLa shares syntax with JavaScript and C.
Some constructs may feel natural while some not so much.

* [Primitives](#prim)
* [Identifiers](#id)
* [Expressions](#expr)
* [Declarations and Assignments](#decl)
	* [Usual Stuff](#decl-usual)
	* [References](#decl-ref)
* [Functions](#func)
	* [Defining a Function](#func-def)
		* [Function Closures](#func-closure)
		* [Function Contextuals](#func-context)
* [Loops](#loops)
* [Namespaces](#namespaces)
* [Blocks](#blocks)
* [Parameterized Scripts](#param)

Comments in MiLa are just C-style comments.

---

## <a id="prim"></a>Primitives

Primitives are:
* Strings `"Hello, Reiley"`

    By default every string in MiLa can be
    a multi line string. Beware MiLa does not
    support unicode in source! You must use their
    hex code via `\u` for 16 bit or `\U` for 32 bit.
    All standard C escapes are supported.

* String blocks `!{Hello, Reiley}`

    These strings are special, they also
    dedent the string inside.
    They do not process escapes and
    curly braces must be balanced.
    <br><br>
    These are used for storing MiLa code
    as strings.

* Integers `90`
* Unsigned Integers `90u`
* Big Integers `90~`
* Float `90.0`
* Big Float `90.0~`
* Booleans `true` and `false`
* None sentinel as `none`
* Null sentinel as `null`
* Functions (lambdas)
* Natives

## <a id="id"></a>Identifiers

In MiLa there are two types of identifiers.
Your normal identifiers with some twists `id.one.two`
and string identifiers which are quoted `'this is a valid id!'`.
String identifiers aim to allow users to name variables anything,
aiding non english programmers, "but why not just support unicode",
I dont want to bloat my language with handling unicode characters.
<br><br>
Regular identifiers can be composed of any alphanumeric characters and
the characters `.` and `_`, in MiLa the dots in identifiers are part of the identifier
and is not an operator unlike in other languages. Dots in identifiers act as "namespaces".

```MiLa
// String IDs can contain anything except NULL
// and the closing single quote.
var '你好' = "hello";
var 'this is a valid identifier' = 90;
```

## <a id="expr"></a>Expressions

Expressions in MiLa are just like any other language's.
MiLa only has binary operations.

| Operation      | Symbol | MiLa VIOO Methods | MiLa OIOO Methods |
|----------------|--------|-------------------|-------------------|
| Addition       | `+`    | BMethodAdd        | `:+`  or `+:`     |
| Subtraction    | `-`    | BMethodSub        | `:-`  or `-:`     |
| Multiplication | `*`    | BMethodMul        | `:*`  or `*:`     |
| Division       | `/`    | BMethodDiv        | `:/`  or `/:`     |
| Modulo         | `%`    | BMethodMod        | `:%`  or `%:`     |
| Equal          | `==`   | BMethodEq         | `:==` or `>>:`    |
| Not Equal      | `!=`   | BMethodNe         | `:!=` or `<<:`    |
| Greater Than   | `>`    | BMethodGreat      | `:>=` or `==:`    |
| Less Than      | `<`    | BMethodLess       | `:<=` or `!=:`    |
| Greater/Eq     | `>=`   | BMethodGE         | `:>`  or `>:`     |
| Less/Eq        | `<=`   | BMethodLE         | `:<`  or `<:`     |
| R Shift        | `>>`   | BMethodRShift     | `:>>` or `>=:`    |
| L Shift        | `<<`   | BMethodLShift     | `:<<` or `<=:`    |
| Globbing       | `=>`   | BMethodGlob       | Not Exposed       |
| Default Op     | `??`   | BMethodDefault    | Not Exposed       |

OIOO Methods are read this way.
The colon represents the operand that owns the overload.
In the case of `a + b` the interpreter first asks if `a` has `:+` then asks `b` if it has `+:`,
if both dont have an overload, the expression resolves to null.
<br><br>
For more details about VIOO and OIOO check out the "Runtime.md" file
<br><br>
You cannot nest calls like in other languages, you must wrap the expression first
before you call it. If in python you do `call()()()` in MiLa you do `((call())())()`.
<br><br>
There is also subscription,<br>
it's exposed in VIOO as `BMethodGetItem` and `BMethodSetItem`.
<br><br>
Function calls are the same as in C (yes we don't allow keyword arguments.)

## <a id="decl"></a>Declarations and Assignments

Declarations in MiLa are straightforward.

### <a id="decl-usual"></a>Usual Stuff

* `var` keyword.

```MiLa
var name; // declares `name` and sets it to `none`.
var name: "type"; // declares `name` with a type hint and sets it to `none`.
var name = value; // declare then assign a value.
var name: "type" = value; // declare then assign a value with a type hint.

// You can declare a variable multiple times in a scope.
{
	var name;

	var name; // overrides previous `name`
}
```

* `set` keyword.

```MiLa
set name = value; // assignment. Must always provide the value.
// You cannot add a type hint to assignments.

// set supports inplace operations.
set age = 16;
set age += 17;

// set also supports setting object fields unlike `sync` and `var`.
object obj {}
set obj["attr"] = 90;
```

### <a id="decl-ref"></a>References

MiLa doesnt hide that values are inherently pointers and are
references.

* `sync` keyword.

`sync` is similar to set, but it sets underlying value instance' value to the provided value.
In C `set` would be `var = value;`, `sync` would be `*var = value;`

```MiLa
fn fn_ref(int) {
	sync int = 90; // every binding to int's value instance now holds 90
}

fn fn_set(int) {
	set int = 90; // local bindings only
}

var my_int = 0;
fn_set(my_int);
println(my_int); // 0
fn_ref(my_int);
println(my_int); // 90
```

## <a id="func">Functions

Functions in MiLa is straightforward.

### <a id="func-def"></a>Defining Functions

```MiLa
// General Function Structure

fn name(params)[contextuals]:[closure_captures] {
	// body
}

// Function parameters can have type hints in the same way
// as `var` statements do, and support for return types.

fn greet(name: "string") -> "null" {
	println("Hello, " + name + "!");
}

// Functions can also have default parameters.

fn greet(name: "string" = "Rando") -> "null" {
	println("Hello, " + name + "!");
}

```

#### <a id="func-closure"></a>Function Closures

I'll suppose you already know what closures are.
In MiLa its more explicit and you have to manually list
the variables you want to capture. This is due to MiLa
being a one pass interpreter.

```MiLa
// Classic counter example

fn make_counter() {
	var count = -1; // we dont have post fix operations
	return fn():[count] {
		set count += 1;
		return count;
	};
}

var counter = make_counter();
println(counter()); // 0
println(counter()); // 1
println(counter()); // 2
```

You can list multiple closure captures at once.
Like so: `fn complicated_add():[logger, some_obj, etc]{}`

#### <a id="func-context"></a>Function Contextuals

Contextuals have been mentioned in the MiLa C API.
These are implicit (kind of) arguments, and has been
inspired by Scala.

```MiLa
var current_used = 0;

fn my_allocator(size):[current_used] {
	println("Allocated " + size + " bytes!");
	set current_used += size;
	return size;
}

// The question mark makes the contextual optional
// By default all contextuals are required and
// produces errors when not found.
fn some_function()[_allocator?]
{
	println(_allocator);
	if (_allocator) {
		var what = _allocator(90);
	} else {
		// fallback logic
	}
}

{
	contextual my_allocator as _allocator;
	some_function();
}
```

Notable differences is that in MiLa you cannot pass these contextuals explicitly.

### Contextuals and Closure Lists

Both are optional. Be careful on closures.
Closures require a colon before it's list,
this to differentiate whether that list is
a closures list or a contextuals list.

## <a id="loops"></a>Loops

```MiLa
foreach item : items {
	// body
}

foreach yield item : generator {
	// body
}

while (cond) {
	// body
}
```

## <a id="namespaces"></a>Namespaces

Namespaces are simple in MiLa.
All they do is mangle every name that starts
with a dot.

```MiLa
namespace my_lib {
    var .my_var = 90; // my_lib.my_var
}
```

## <a id="blocks"></a>Blocks

MiLa does not have line tracking.
This is due to MiLa being a one pass interpreter.

```MiLa
block block_name {
    // any error here gets the name of `block_name`
}
```

If an error ever propagates, the error will be mangled to
have tbe blocks name.

## <a id="param"></a>Parameterized Scripts

```MiLa

// gracefully capture argv
// (argc is always the first argument,
// argv gets expand to the rest of the parameters)
!fn (argc, name, ...argv)
```

Note: MiLa will ignore shebangs when present.
