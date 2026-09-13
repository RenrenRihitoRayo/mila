# Standard for MiLa Type Annotations

Standard for MiLa type hint syntax

* `type` just the type `type`

* `type[i_type]` a collection of type `type` with items of type `i_type`

* `{type1, type2, type3}` is a union.

    In a union only comprised of side effects, the union is treated implicitly
    as `{null}`, such that `{io!}` is equivalent to `{null, io!}`

* `<type>` a generic type `type`.

    Example use case: `fn add(a: "<T>", b: "<T>") -> "<T>"`

* `type[<g_type>]` a collection of type `type` with the generic type `g_type`

* `type[{type1, type2, type3}]` a collection of type `type` with the union type `type1`, `type2`, and `type3`

* `type[key_type=val_type]` a relational collection of type `type` with the key type being `key_type` and value type being `val_type`

* `type[{type1=type2, type3=type4}]` a relational collection of type `type` with the union types for two pairs.

* `type(i_1, i_2, i_3)` a structural type for a collection with the exact types `i_1`, `i_2`, and `i_3` corresponding to the colelctions items.

	Example: `[1, 1.0, true]` is `list(int, float, bool)`

* `type(i_1=i_2, i_3=i_4)` a structural type for a relational collection with the exact types for the individual pairs in the collection.

	Example: `[@ "test" = 90, 90 = "test"]` is `dict(string=int, int=string)`

## Standard types

* `any`

    Anything

* `int`, `float`, and `uint`

    Standard numeric types.

* `string`

    Standard string type.

* `dict`, `list`, and `array`

    Standard collections

* `bool`

    Standard boolean

* `file`

    Standard file

* `numeric`

    For numeric types, shorthand for `{int, uint, float}` or any numeric acting type.

* `callable`

    For callable types, shorthand for `{function, native}`

Note identifiers ending in `!` are special types.
They are side effects rather than actual expected values.

* `noreturn!`

    A function that may not return

* `exit!`

    A function that may exit

* `io!`

    A function that may use IO

* `impure!`

    A function that is impure in general

## Function syntax

* `fn (type1, type2, type2) -> ret_type`

    Example:
    * `println` would be `fn (any...) -> null`
    * `open` would be `fn (string, string) -> file`

    Optional arguments are denoted with question marks.

    How are errors denoted?
    `report` would be `fn (string?) -> E_GENERIC!`

## Example

For an entire script (using parameterized scripts)

```MiLa
!fn (argc, ...argv) -> "{io!, exit!}"

println("Hello, world!");
```

For a "main" function

```MiLa
fn main() -> "{int, io!, exit!}" {
    println("Hello, world!");
    return 0;
}

exit(main());
```