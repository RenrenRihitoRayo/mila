# Using the MiLa C API

## <a id="topics"></a>Topics

### Minimal Requirements

* [Library Example](#lib-ex)
* [Embedded Example](#embedded-ex)
* [Manipulating Environments](#env)
* [Creating Values and Handling them](#values)
	* [Constructors](#values-cons)
	* [Control Values](#values-ctrl)
	* [Getting Values](#values-get)
	* [Value Types](#values-type)
* [VIOO](#vioo)

---

## <a id="lib-ex"></a>Library Example

```C
#define ML_NO_MATH
#define ML_LIB
#include "mila.c"

Value* native_fn(Env* env, int argc, Value** argv) {
	return vstring_dup("Hello, world!");
}

void _mila_lib_init(Env* g) {
	printf("Library init!\n");
}

void _mila_lib_deinit(Env* g) {
	printf("Library cleanup!\n");
}

const NativeEntry lib_function_entries[] = {
	{"my_fn", native_fn},
	{NULL, NULL} // always must be the last entry
};
```

## <a id="embedded-ex"></a>Embedded Example

```C
#define ML_NO_MATH
#define ML_LIB
#include "mila.c"

Value* native_fn(Env* env, int argc, Value** argv) {
	return vstring_dup("Hello, world!");
}

int main() {
	Env* g = mila_global_init();
	env_register_native(g, "native_fn", native_fn);
	Value* result = eval_str("println(native_fn());");
	val_release(result);
	mila_global_deinit(g);
	return 0;
}
```

## <a id="env"></a>Manipulating Environments

Environments are just scopes.
You can set values in them and can be inherited by another environment
for scope like behavior.

### Creating and Destroying an `Env*`

* `Env* env_new(Env* parent);`

Allocates and returns an Environment.

* `void env_free(Env* env);`

Deallocates env and releases all values in it.
Uses `val_release`.

* `void env_kill(Env* env);`

Deallocates env and kills all values in it.
Uses `val_kill`.

### "But the examples use 'mila_global_init' and 'mila_global_deinit'"

`mila_global_init` must always be called before executing any code or MiLa function.
`mila_global_deinit` must always be called after any calls to any mila function or MiLa execution.
<br><br>
These two functions handle the global state.
If you wish to create an instance separate from the primary instance, use
`mila_init` and `mila_deinit` respectively.

### Setting Variables

Return codes of the following functions must be treated more as an indicator
rather than an error code.

* `int env_set(Env* env, const char* name, Value* value);`

	Sets value `value` into `env` with the name `name`.
	Note that `env` will acquire `value` and increments its refcount, additionally
	`env_set` looks for an existing variable called `name` in outer scopes before setting
	the value. This emulates `set name = value;` statements.
	<br><br>
	Return code:
	* 0 if it finds a binding
	* 1 if it needed to create a new local binding

* `int env_set_raw(Env* env, const char* name, Value* value);`

	Sets value `value` into `env` with the name `name`.
	Note that `env` will not acquire `value`, additionally
	`env_set_raw` looks for an existing variable called `name` in outer scopes before setting
	the value. This emulates `set name = value;` statements.
	<br><br>
	Return code:
	* 0 if it finds a binding
	* 1 if it needed to create a new local binding

* `int env_set_local(Env* env, const char* name, Value* value);`

	Sets value `value` into `env` with the name `name`.
	Note that `env` will acquire `value` and increments its refcount.
	This emulates `var name = value;` statements.
	<br><br>
	Return code:
	* 0 if it found an existing local binding
	* 1 if it needed to create a new binding

* `int env_set_local_raw(Env* env, const char* name, Value* value);`

	Sets value `value` into `env` with the name `name`.
	Note that `env` will not acquire `value`.
	This emulates `var name = value;` statements.
	<br><br>
	Return code:
	* 0 if it found an existing local binding
	* 1 if it needed to create a new binding

* `int env_remove(Env* env, const char* name);`

	Looks at the closest binding of `name` in `env` and removes it.
	<br><br>
	Return code:
	* 0 if it found the binding
	* 1 if it did not find the binding

* `void env_register_native(Env* env, const char* name, NativeFn fn);`

	Binds the native function `fn` into the environment `env` with the
	name of `name`.
	<br><br>
	`NativeFn` is just `Value*(*NativeFn)(Env* env, int argc, Value** argv)`

The following values operate on the contextual side of the environment rather than
the regular scope variables, this is isn't necessary for common scripts unless you
want to mess around with functions.

* `int env_set_contextual(Env* env, const char* name, Value* value);`

	Looks at the closest binding of `name` in `env` and sets it to `value`.
	<br><br>
	Return code:
	* 0 if it found an existing binding
	* 1 if it needed to create a new binding

* `int env_set_raw_contextual(Env* env, const char* name, Value* value);`

	Looks at the closest binding of `name` in `env` and sets it to `value`.
	<br><br>
	Return code:
	* 0 if it found an existing binding
	* 1 if it needed to create a new binding

* `int env_set_local_contextual(Env* env, const char* name, Value* value);`

	Looks at the closest binding of `name` in `env` and sets it to `value`, always in the local scope.
	<br><br>
	Return code:
	* 0 if it found an existing binding
	* 1 if it needed to create a new binding

* `int env_set_local_raw_contextual(Env* env, const char* name, Value* value);`

	Looks at the closest binding of `name` in `env` and sets it to `value`, always in the local scope.
	<br><br>
	Return code:
	* 0 if it found an existing binding
	* 1 if it needed to create a new binding

* `int env_remove_contextual(Env* env, const char* name);`

	Looks at the closest binding of `name` in `env` and removes it.
	<br><br>
	Return code:
	* 0 if it found the binding
	* 1 if it did not find the binding

### Woah, Wait! What are Contextuals?

Contextuals are just like variables but in a different container within a scope.
Technically an environment in MiLa has two parts. Your regular
scope, and the contextual scope.
<br><br>
Think of keeping two lists beside each other.
<br><br>
In MiLa, functions can have implicit arguments called "contextual parameters".
Unlike closures, contextuals are captured at the caller's site rather than the definition.
However a normal variable must be first declared as such `contextual var_name;`; to actually be
considered contextual. This emulates the behavior of implicit arguments in Scala. Furthermore
contextuals do not get remembered by the callee, every new call recaptures the contextual.

## <a id="values"></a>Creating Values and Handling them

Values are simple in MiLa, they are refcounted or not.

### Memory Handling

By default MiLa value instances are refcounted.

* `Value* val_retain(Value* val);`

	Increases the refcount and return the given value.

* `void val_release(Value* val);`

	Decreases the refcount and frees it if refcount hits `0`.

* `void val_kill(Value* val);`

	Deallocates the value regardless of the refcount.
	NOTE: ONLY USE WHEN YOURE SURE THERE ARE NO REFERENCES LEFT!

### Finally the Constructors and Getters

#### <a id="values-cons"></a>Constructors

* `Value* vstring_dup(const char* string);`
	
	Duplicate the given string.

* `Value* vstring_take(char* string);`
	
	Take a string and return its boxed representation.
	This assumes MiLa can free the given string.

* `Value* vstring_fmt(const char* fmt, ...);`

	Create an allocated string based on the given `fmt` string and
	values.

* `Value *vstring_slice(const char *src, size_t start, size_t len);`

	Slice a string and duplicate that slice.

* `Value *vstring_index(const char *src, size_t index);`

	Return a single character that was indexed into a string.

* `Value *vstring_replace(const char *src, const char *needle, const char *replacement);`

	Return a new string of which all of the occurrence of `needle` has been
	replaced with `replacement`.

* `Value* vint(long num);`

	Create an int.

* `Value* vuint(unsigned long num);`

	Create a uint.

* `Value* vbint(__int128 num);`

	Create a big int.

* `Value* vfloat(double num);`

	Create a float.

* `Value* vbfloat(mila_float128_internal num);`

	Create a bfloat.

* `Value* vbool(bool b);`

	Create a bool.

* `Value* vopaque(void* ptr);`

	Create an opaque pointer.

* `Value* vowned_opaque(void* ptr);`

	Create an owned opaque pointer.
	Once this value has no references MiLa will
	try to free the pointer.

* `Value* vweak_opaque(void* ptr);`

	Create a weak opaque.
	Refcount operations do nothing.
	<br>
	NOTE: `val_kill` will STILL kill an instance of this type.

* `Value* vnone();`

	Create a `none`

* `Value* vnull();`

	Create a `null`

* `Value *vnative(NativeFn fn, const char *name);`

	`NativeFn` is just `Value*(*NativeFn)(Env* env, int argc, Value** argv)`

* `Value *vfunction(char **params, char** defaults, char** contextuals, Env* closure, char *body_src);`

	Where `params`, `defaults`, and `contextuals` must be terminated with `NULL`.
	`params` is the parameter list for the function.
	`defaults` is the default values for params as expressions in strings, if a parameter doesnt
	have a default value, its default value is `NULL`.
	Any of the four: `params`, `defaults`, `contextuals`, and `closure` may be set to `NULL`.

* `Value *vopaque_extra(void *p, VPrinter display, const char *type_name);`

	`display` can be `NULL`.
	`display` is a function that accepts the opaque (as a Value* so you have to unwrap it)
	in order to print the representation for the opaque value.
	Note: DO NOT FREE THE VALUE REF PASSED INTO THE PRINTER: the `Value*` referenced passed to the printer function is the wrapper that is returned
	from this function which may be managed by MiLa's refcounting.

* `Value *vowned_opaque_extra(void *p, VPrinter display, const char *type_name);`

	`display` can be `NULL`.
	`display` is a function that accepts the opaque (as a Value* so you have to unwrap it)
	in order to print the representation for the opaque value.
	Note: DO NOT FREE THE VALUE REF PASSED INTO THE PRINTER: the `Value*` referenced passed to the printer function is the wrapper that is returned
	from this function which may be managed by MiLa's refcounting.

#### <a id="values-ctrl"></a>Control Values

Values that are propagated and can control flow.
<br><br>
Used internally:
* `Value* vcontinue();`
* `Value* vbreak();`
These two can be used to emulate control flow via functions if needed.

Below is the usual API for control flow from native functions:
* `Value* verror(const char* fmt, ...);`
* `Value* vtagged_error(ErrorType error, const char* fmt, ...);`
* `Value* vtagged_coded_error(ErrorType error, int ret_code, const char* fmt, ...);`

#### <a id="values-get"></a>Getting Values

All of these are macros that accept the Value* instances.
Most of them are self explanatory.

* `GET_STRING(s)`

* `GET_INTEGER(i)`

* `GET_UINTEGER(ui)`

* `GET_BINTEGER(bi)`

* `GET_FLOAT(f)`

* `GET_BFLOAT(bf)`

* `GET_BOOL(b)`

* `GET_OPAQUE(b)`

	This is used for all three opaque types
	`T_WEAK_OPAQUE`, `T_OWNED_OPAQUE`, and `T_OPAQUE`.

* `GET_TYPE(any)`

	Used to get the type of a value instance.

* `GET_TYPENAME(any)`

	Used to get the type of a value instance as a string.

Used internally:

* `GET_FUNCTION(fn)`

* `GET_NATIVE(nfn)`

* `GET_TAGGED_ERROR_TYPE(te)`

* `GET_TAGGED_ERROR_MESSAGE(te)`

* `GET_ERROR_TYPE(e)`

* `GET_ERROR_MESSAGE(e)`

#### <a id="values-type"></a>Value Types

MiLa has different enums for different kinds of types.

##### Value Types (what you should know)

| C Enum Type      | MiLa Type      | C Type                   |
|------------------|----------------|--------------------------|
| `T_INT`          | `int`          | `long`                   |
| `T_UINT`         | `uint`         | `unsigned long`          |
| `T_BINT`         | `bint`         | `__int128`               |
| `T_FLOAT`        | `float`        | `double`                 |
| `T_BFLOAT`       | `bfloat`       | `mila_float128_internal` |
| `T_STRING`       | `string`       | `char*`                  |
| `T_OPAQUE`       | `opaque`       | `const void*`            |
| `T_WEAK_OPAQUE`  | `weak_opaque`  | `const void*`            |
| `T_OWNED_OPAQUE` | `owned_opaque` | `void*`                  |
| `T_BOOL`         | `bool`         | `bool / _Bool`           |
| `T_FUNCTION`     | `function`     | `struct FunctionV*`      |
| `T_NATIVE`       | `native`       | `struct NativeV*`        |
| `T_NONE`         | `none`         | `struct Value*`          |
| `T_NULL`         | `null`         | `struct Value*`          |

## <a id="vioo"></a>VIOO

This is only a highlight, check VIOO.md for more.
<br><br>
VIOO is MiLas way of adding pseudo dynamic but
fast operator overloading for types.
It internally uses an artay of function pointers each
corresponding to a method.

| Operation    | VIOO Method    |
|--------------|----------------|
| `+`          | TMethodBinop   |
| `-`          | TMethodBinop   |
| `*`          | TMethodBinop   |
| `/`          | TMethodBinop   |
| `%`          | TMethodBinop   |
| `v[...]`     | BMethodGetItem |
| `set v[...]` | TMethodSetItem |