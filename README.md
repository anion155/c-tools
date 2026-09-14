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

[da.h](./da.h) provides typed dynamic arrays utilities (heavily inspired by [@tsoding's dynamic arrays](https://www.youtube.com/@TsodingDaily) and his [nob.h](https://github.com/tsoding/nob.h)).

```c
#include "da.h"

typedef Da(int, Int_Array) Int_Array;

int main(void) {
  Int_Array values = {0};

  da_append(&values, 10);
  da_append_many(&values, 20, 30, 40);

  da_foreach(values, value) {
    printf("%d\n", *value);
  }

  da_free(&values);
}
```

Structure creation helpers:
- [`Da(Value_Type, [struct_name])`](./da.h#L15 "Da") - represents array that can grow dynamically, owns data, and requires freeing after use.
- [`Da_Slice(Value_Type, [struct_name])`](./da.h#L22 "Da_Slice") - represents slice of an array, does not own data.
- [`Da_Const(Value_Type, [struct_name])`](./da.h#L28 "Da_Const") - represents constant size array compatible with da_any_ methods

Useful operations include:

- [`da_first(da)`](./da.h#L59): Returns the first element of the dynamic array.
- [`da_last(da)`](./da.h#L68): Returns the last element of the dynamic array.
- [`da_at(da, index)`](./da.h#L77): Returns the element at `index`.
- [`da_foreach(da, id)`](./da.h#L87): Iterates over `da`, creating a pointer loop variable named `id` and index `id##_index`.
- [`da_find(da, id, predicate)`](./da.h#L102): Returns index of first element satisfying `predicate` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`), or array's size if not found.
- [`da_find_macro(da, predicate)`](./da.h#L125): Returns index of first element satisfying `predicate` (function or macro that accepts size_t index and Value_Type item), or array's size if not found.
- [`da_find_right(da, id, predicate)`](./da.h#L145): Returns `index + 1` of first element from the end satisfying `predicate` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`), or `0` if not found.
- [`da_find_right_macro(da, predicate)`](./da.h#L168): Returns `index + 1` of first element from the end satisfying `predicate` (function or macro that accepts size_t index and Value_Type item), or `0` if not found.
- [`da_free(da)`](./da.h#L188): Frees `data` buffer, zeroes `capacity` and `count`, and resets pointer to `NULL`.
- [`da_reserve(da, target_capacity)`](./da.h#L201): Ensures capacity is at least `target_capacity` by doubling capacity geometrically.
- [`da_reserve_exact(da, target_capacity)`](./da.h#L215): Reallocates capacity to exactly `target_capacity` if needed without geometric growth.
- [`da_trim_realloc(da)`](./da.h#L228): Shrinks memory allocation so `capacity` matches `count`, or frees if `count == 0`.
- [`da_resize(da, new_size)`](./da.h#L245): Reserves memory for `new_size` and sets `count = new_size`.
- [`da_append(da, item)`](./da.h#L253): Appends a single `item` to the dynamic array, growing capacity if necessary.
- [`da_append_many_n(da, new_items, new_count)`](./da.h#L264): Appends `new_count` elements from buffer `new_items`.
- [`da_append_many(da, [item1, item2, ...])`](./da.h#L276): Appends variadic literal items to `da`.
- [`da_pop(da)`](./da.h#L289): Returns the last element and decrements `count`.
- [`da_remove_unordered(da, index)`](./da.h#L299): Removes element at `index` by swapping it with the last element.
- [`da_remove_ordered(da, index)`](./da.h#L309): Removes element at `index` by moving all further elements left.
- [`da_slice_init(da, [start], [count])`](./da.h#L318): Struct initializer expression for slices, with optional `start` (default `0`) and `count` (defaults to remaining elements).
- [`da_slice(da, Slice_Type, [start], [count])`](./da.h#L323): Returns a slice of `Slice_Type` starting at `start` (default `0`) for `count` elements (default remaining elements).
- [`da_slice_whole(da, Slice_Type)`](./da.h#L337): Creates a `Slice_Type` putting in it full range of `da`.
- [`da_slice_shift(das)`](./da.h#L345): Shifts slice head forward by 1, decrements `count`, and returns the dropped first element.
- [`da_slice_chop_left(das, [n])`](./da.h#L356): Advances slice head past `n` elements (default `1`) and returns a new slice containing the chopped prefix.
- [`da_slice_chop_right(das, [n])`](./da.h#L368): Shrinks slice tail by `n` elements (default `1`) and returns a new slice containing the chopped suffix.
- [`da_slice_chop_while(das, id, predicate)`](./da.h#L387): Chops and returns prefix of a slice while elements satisfy `predicate` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`).
- [`da_slice_chop_while_macro(das, predicate)`](./da.h#L396): Chops and returns prefix of a slice while elements satisfy `predicate` (function or macro that accepts size_t index and Value_Type item).
- [`da_slice_chop_right_while(das, id, predicate)`](./da.h#L410): Chops and returns suffix of a slice while elements satisfy `predicate` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`).
- [`da_slice_chop_right_while_macro(das, predicate)`](./da.h#L419): Chops and returns suffix of a slice while elements satisfy `predicate` (function or macro that accepts size_t index and Value_Type item).
- [`da_slice_chop_by(das, id, predicate)`](./da.h#L432): Chops slice up to (and including) the first element matching `predicate` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`).
- [`da_slice_chop_by_macro(das, predicate)`](./da.h#L450): Chops slice up to (and including) the first element matching `predicate` (function or macro that accepts size_t index and Value_Type item).
- [`da_slice_chop_right_by(das, id, predicate)`](./da.h#L471): Chops slice from the end up to (and including) the last element matching `predicate` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`).
- [`da_slice_chop_right_by_macro(das, predicate)`](./da.h#L490): Chops slice from the end up to (and including) the last element matching `predicate` (function or macro that accepts size_t index and Value_Type item).
- [`da_const_init_from_arraylit(Value_Type, [item1, item2, ...])`](./da.h#L508): Constructs a read-only dynamic array compatible structure initialized from a compound literal array.

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
