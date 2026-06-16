# Runtime

## Prerequisites

Semantics et Miscellaneous

* `none` vs `null`

    `none` is meant as a placeholder,
    an empty value, this is true ALWAYS.
    <br><br>
    `null` is meant as a missing value,
    or an invalid operation (for recoverable operations),
    thisnis true ALWAYS.

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
    embedding, all the while also being a good standalone
    scripting language.

## Parts

* [Memory Management](#mem)
* [Values](#value)

---

## <a id="mem"></a>Memory Management

MiLa handles memory using reference counting.
"But how about cycles?", thats the neat part,
we shove those under a rag untill we hit an OOM
exception.
<br><br>
In a formal tone, MiLa does not handle cyclic references
automatically. You must use inde
<br><br>
The MiLa C API has more information on this.

## <a id=value"></a>Values

Simply a container that stores the representations for
each value with metadata attached. This is not a hard
concept to understand.
<br><br>
For a much better understanding consult the code base
or the MiLa C API documentation.

---

TODO: COMPLETE
