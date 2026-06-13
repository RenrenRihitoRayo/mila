# Using the MiLa C API

<a id="topics"></a>Topics:
* [Manipulating Environments](#env)
* [Creating Values and Handling them](#values)
	* [Constructors](#values-cons)
	* [Control Values](#values-ctrl)

---

## <a id="env"></a>Manipulating Environments

Environments are just scopes. Thats it,
you can set values in it and it can be inherited by another environment
for scopes.

### Creating and Destroying an Env*

* `Env* env_new(Env* parent);`

Allocates and returns an Environment.

* `void env_free(Env* env);`

Deallocates env and releases all values in it.
Uses `val_release`.

* `void env_kill(Env* env);`

Deallocates env and kills all values in it.
Uses `val_kill`.

### Setting Variables

* `void env_set(Env* env, const char* name, Value* value);`

Sets the value into `env` with the name `name` and value from `value`.
Note that `env` will acquire `value` and increments its refcount, additionally
`env_set` looks for an existing variable called `name` in outer scopes before setting
the value. This emulates `set name = value;` statements.

* `void env_set_raw(Env* env, const char* name, Value* value);`

Sets the value into `env` with the name `name` and value from `value`.
Note that `env` will not acquire `value`, additionally
`env_set_raw` looks for an existing variable called `name` in outer scopes before setting
the value. This emulates `set name = value;` statements.

* `void env_set_local(Env* env, const char* name, Value* value);`

Sets the value into `env` with the name `name` and value from `value`.
Note that `env` will acquire `value` and increments its refcount.
This emulates `var name = value;` statements.

* `void env_set_local_raw(Env* env, const char* name, Value* value);`

Sets the value into `env` with the name `name` and value from `value`.
Note that `env` will not acquire `value`.
This emulates `var name = value;` statements.

* `void env_remove(Env* env, const char* name);`

Looks at the closest binding of `name` in `env` and removes it.

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
* `Value* vstring_take(char* string);`
* `Value* vstring_fmt(const char* fmt, ...);`
* `Value *vstring_slice(const char *src, size_t start, size_t len);`
* `Value *vstring_index(const char *src, size_t index);`
* `extern Value *vstring_replace(const char *src, const char *needle, const char *replacement);`
* `Value* vint(long num);`
* `Value* vuint(unsigned long num);`
* `Value* vbint(__int128 num);`
* `Value* vfloat(double num);`
* `Value* vbfloat(mila_float128_internal num);`
* `Value* vbool(bool b);`
* `Value* vopaque(void* ptr);`
* `Value* vowned_opaque(void* ptr);`
* `Value* vweak_opaque(void* ptr);`
* `Value* vnone();`
* `Value* vnull();`
* `Value *vnative(NativeFn fn, const char *name);`
`NativeFn` is just `Value*(*NativeFn)(Env* env, int argc, Value** argv)`
* `Value *vfunction(char **params, char** defaults, char** contextuals, Env* closure, char *body_src);`
Where `params`, `defaults`, and `contextuals` must be terminated with `NULL`.
`params` is the parameter list for the function.
`defaults` is the default values for params as expressions in strings, if a parameter doesnt
have a default value, its default value is `NULL`.
* `Value *vopaque_extra(void *p, VPrinter display, const char *type);`
`display` can be `NULL`.
* `Value *vowned_opaque_extra(void *p, VPrinter display, const char *type);`
`display` can be `NULL`.

#### <a id="values-ctrl"></a>Control Values

Values that are propagated and can control flow.

* `Value* vcontinue();`
* `Value* vbreak();`
* `Value* verror(const char* fmt, ...);`
* `Value* vtagged_error(ErrorType error, const char* fmt, ...);`
* `Value* vtagged_coded_error(ErrorType error, int ret_code, const char* fmt, ...);`
