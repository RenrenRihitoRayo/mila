# Runtime

## Prerequisites

Semantics et Miscellaneous

* `none` vs `null`

    `none` is meant as a placeholder,
    an empty value, a value that exists but
    has not yet been assigned its value.
    This is true ALWAYS.
    <br><br>
    `null` is meant as a missing value,
    or an invalid operation (for recoverable operations),
    a value that does not exist.
    This is true ALWAYS.
    <br><br>
    Notably:
    * `none != null` is true
    * `none == none` is true
    * `null == null` is true
    * Calling either results in an error.

* Fail as little as possible

    MiLa tries to stay running on invalid operations,
    but there are cases where MiLa will try to error out.
    <br><br>
    These actions will raise an error:
    * Missing file
    * Calling something thats not a function
    * Putting null bytes in string literals

* MiLa's true purpose

    Semantics may make MiLa seem its for low level
    embedding but no, MiLa is more suited for application
    embedding, all the while being a good standalone
    scripting language.

## Parts

* [Memory Management](#mem)
* [Values](#value)
* [Errors and Control Flow Values](#err-control)

---

## <a id="mem"></a>Memory Management

MiLa handles memory using reference counting.
"But how about cycles?", thats the neat part,
we shove those under a rag untill we hit an OOM
exception.
<br><br>
In a formal tone, MiLa does not handle cyclic references
automatically. You must use indirection to avoid this.
<br><br>
The MiLa C API has more information on this.

## <a id="value"></a>Values

Simply a container that stores the representations for
each value with metadata attached. This is not a hard
concept to understand. Importantly, values represent
anything a MiLa script can ever return. (see next section)
<br><br>
For a much better understanding consult the code base
or the MiLa C API documentation.

## <a id="err-control"></a>Errors and Control Flow Values

For simplicity and API Consistency
Errors and Control Flow signalling is done by
reusing the exisiting value propagation logic.
There is no separate flags and stack for errors and
control flow values.
<br><br>
Control Values currently are "break", "continue", and "return" types.
Error values are well "tagged_error", and "error" types.
<br><br>
This allows MiLa to be somewhat thread safe.

---

TODO: COMPLETE
