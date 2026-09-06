# Standard for MiLa Type Annotations

* `type` just the type `type`

* `type[i_type]` a collection of type `type` with items of type `i_type`

* `{type1, type2, type3}` is a union.

* `type<g_type>` a collection of type `type` with the generic type `g_type`

* `type[{type1, type2, type3}]` a collection of type `type` with the union type `type1`, `type2`, and `type3`

* `type[key_type=val_type]` a relational collection of type `type` with the key type being `key_type` and value type being `val_type`

* `type[{type1=type2, type3=type4}]` a relational collection of type `type` with the union types for two pairs.

* `type(i_1, i_2, i_3)` a structural type for a collection with the exact types `i_1`, `i_2`, and `i_3` corresponding to the colelctions items.

	Example: `[1, 1.0, true]` is `list(int, float, bool)`

* `type(i_1=i_2, i_3=i_4)` a structural type for a relational collection with the exact types for the individual pairs in the collection.

	Example: `[@ "test" = 90, 90 = "test"]` is `dict(string=int, int=string)`

## Standard types

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

