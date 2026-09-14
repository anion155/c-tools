# c-tools

A collection of small, dependency-free C utilities implemented primarily as std-styled single-header libraries.

The project provides typed dynamic arrays, string builders and views, UTF-8 helpers, reference-counted memory, random ID generation, floating-point conversion utilities, and common preprocessor helpers.

## Features

- `da.h` Typed dynamic arrays and slices
- `sb.h` String builders and non-owning string views
- `utf.h` UTF-8 character-length lookup
- `rc.h` Reference-counted and weak references
- `nanoid`-style random identifiers
- `floats.h` Floating-point representation and conversion helpers
- `ryu.h` Ryu floating-point formatting support
- `abort.h` Assertion, panic, and preprocessor utilities

## Requirements

The headers use GNU C extensions and modern C features, including:

- C23
- GNU statement expressions
- 128 bit supported platform

Clang or GCC with GNU C extensions enabled is recommended.

## Usage

Most headers separate declarations from implementation. Include the header normally wherever it is needed, and define its implementation macro in a translation unit.

For example:

```c
// main.c
#define SB_IMPL
#include "sb.h"

#include <stdio.h>

int main(void) {
  String_Builder builder = {0};

  sb_append_cstr(&builder, "hello ");
  sb_appendf(&builder, "%s", "world");

  sb_null_terminate(&builder);
  printf("%s\n", builder.data);

  sb_free(&builder);
  return 0;
}
```

Compile with:

```c
clang -std=gnu23 main.c -o example
./example
```

Additionally to std-style headers have implementation guard as well, and can be safely included multiple times in one translation unit.

```c
// other.c
#define SB_IMPL
#include "sb.h"

// main.c
#define SB_IMPL
#include "sb.h"

int main(void) {
  ...
}

#include "other.c"
```

As with stb it is adviced to never include implementation more than once in different translation units (different *.c files compiled to separate *.o), as there is no way to prevent duplications this way.

Can be easily precompiled to separate object file if you need it.

```
clang -x c -std=gnu23 -c sb.h -DSB_IMPL -o sb.o
```

## Dynamic arrays

[da.h](./da.h) provides typed dynamic arrays without requiring a separate element type declaration.

```c
#include "da.h"

typedef Da(int, Int_Array) Int_Array;

int main(void) {
  Int_Array values = {0};

  da_append(&values, 10);
  da_append_many(&values, 20, 30, 40);

  da_foreach(&values, value) {
    printf("%d\n", *value);
  }

  da_free(&values);
}
```

Useful operations include:

- `da_append`
- `da_append_many`
- `da_append_many_n`
- `da_reserve`
- `da_resize`
- `da_pop`
- `da_at`
- `da_first`
- `da_last`
- `da_slice`
- `da_foreach`
- `da_remove_unordered`
- `da_free`

The default initial capacity is `256` and can be overridden with `DA_INIT_CAP`.

## Strings

[sb.h](./sb.h) provides:

- `String_Builder`: an owning, growable character buffer
- `String_View`: a non-owning character slice
- `String`: an immutable character slice

```c
#define SB_IMPL
#include "sb.h"

#include <stdio.h>

int main(void) {
  String_Builder builder = {0};

  sb_append_cstr(&builder, "value: ");
  sb_appendf(&builder, "%d", 42);

  String_View view = sb_substr(&builder, 0, builder.count);
  printf("%.*s\n", SV_FMT_ARG(view));

  sb_free(&builder);
}
```

Common string operations include:

- `sb_append`
- `sb_append_cstr`
- `sb_append_strlit`
- `sb_append_sv`
- `sb_appendf`
- `sb_append_repeat`
- `sb_null_terminate`
- `sb_free`
- `sv_eq`
- `sv_starts_with`
- `sv_ends_with`
- `sv_trim`
- `sv_chop_left`
- `sv_chop_right`
- `sv_chop_by_delim`
- `sv_utf_length`

## Reference counting

[rc.h](./rc.h) provides strong and weak references with optional destructors and
cycle detection hooks.

Define `RC_IMPL` in one source file:

```c
#define RC_IMPL
#include "rc.h"
```

Basic usage:

```c
int *value = rc_malloc(sizeof(*value));
*value = 42;

rc_acquire(value);
rc_release(value);
rc_release(value);
```

Important operations include:

- `rc_malloc`
- `rc_manage`
- `rc_realloc`
- `rc_acquire`
- `rc_release`
- `rc_move`
- `rc_downgrade`
- `rc_upgrade`
- `rc_weak_free`
- `rc_guarded`

## Random identifiers

[nanoid.h](./nanoid.h) generates random identifiers using rand().

```c
#define NANOID_IMPL
#include "nanoid.h"

char *id = nanoid("item_", 12);
/* Use id... */
free(id);
```

## Floating-point utilities

[floats.h](./floats.h) contains portable canonical representations and conversion
helpers for floating-point formats including:

- 16-bit floating point
- 80-bit extended precision
- 128-bit floating point
- 128-bit double-pair representations

[ryu.h](./ryu.h) contains Ryu-based floating-point conversion support for
round-trippable decimal formatting.

## Other headers

- [abort.h](./abort.h): panicf, TODO, UNREACHABLE, and ASSERT
- [defines.h](./defines.h): preprocessor helpers, type assertions, stringification, defaults, and compiler attributes
- [utf.h](./utf.h): UTF-8 character-length lookup table
