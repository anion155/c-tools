# c-tools

A collection of small, dependency-free C utilities implemented primarily as std-styled single-header libraries.

The project provides typed dynamic arrays, string builders and views, UTF-8 helpers, reference-counted memory, random ID generation, floating-point conversion utilities, and common preprocessor helpers.

## Features

- [`da.h`](#dynamic-arrays) Typed dynamic arrays and slices
- [`str.h`](#strings) String, string builder and non-owning string views
- [`str_utf.h`](#strings-utf) UTF utilities for `str.h` library
- [`str_integers.h`](#stringify-integers) Integer utilities for `str.h` library
- [`str_floats.h`](#stringify-floats) Floating utilities for `str.h` library
- [`str_numbers.h`](#stringify-numbers) Number utilities for `str.h` library
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

[da.h](./da.h) provides typed dynamic arrays utilities (heavily inspired by [@tsoding's](https://www.youtube.com/@TsodingDaily) dynamic arrays and his [nob.h](https://github.com/tsoding/nob.h)).

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

- [`Da(Value_Type, [struct_name])`](./da.h#L51): represents array that can grow dynamically, owns data, and requires freeing after use.
- [`Da_Slice(Value_Type, [struct_name])`](./da.h#L62): represents slice of an array, does not own data.
- [`Da_Const(Value_Type, [struct_name])`](./da.h#L72): represents constant size array compatible with da_any_ methods

Useful operations include:

- [`da_first(da)`](./da.h#L82): Returns the first element of the dynamic array.
- [`da_last(da)`](./da.h#L92): Returns the last element of the dynamic array.
- [`da_at(da, index)`](./da.h#L102): Returns the element at `index`.
- [`da_foreach(da, id)`](./da.h#L113): Iterates over `da`, creating a pointer loop variable named `id` and index `id##_index`.
- [`da_find_expr(da, id, expr)`](./da.h#L129): Returns index of first element satisfying `expr` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`), or array's size if not found.
- [`da_find_pred(da, predicate)`](./da.h#L151): Returns index of first element satisfying `predicate` (function or macro that accepts (Value_Type item, size_t index)), or array's size if not found.
- [`da_find_right_expr(da, id, expr)`](./da.h#L172): Returns `index + 1` of first element from the end satisfying `expr` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`), or `0` if not found.
- [`da_find_right_pred(da, predicate)`](./da.h#L194): Returns `index + 1` of first element from the end satisfying `predicate` (function or macro that accepts (Value_Type *item, size_t index)), or `0` if not found.
- [`da_free(da)`](./da.h#L215): Frees `data` buffer, zeroes `capacity` and `count`, and resets pointer to `NULL`.
- [`da_reserve(da, target_capacity)`](./da.h#L228): Ensures capacity is at least `target_capacity` by doubling capacity geometrically.
- [`da_reserve_exact(da, target_capacity)`](./da.h#L242): Reallocates capacity to exactly `target_capacity` if needed without geometric growth.
- [`da_trim_realloc(da)`](./da.h#L255): Shrinks memory allocation so `capacity` matches `count`, or frees if `count == 0`.
- [`da_resize(da, new_size)`](./da.h#L272): Reserves memory for `new_size` and sets `count = new_size`.
- [`da_append(da, item)`](./da.h#L281): Appends a single `item` to the dynamic array, growing capacity if necessary.
- [`da_append_many_n(da, new_items, new_count)`](./da.h#L292): Appends `new_count` elements from buffer `new_items`.
- [`da_append_many(da, [item1, item2, ...])`](./da.h#L304): Appends variadic literal items to `da`.
- [`da_pop(da)`](./da.h#L317): Returns the last element and decrements `count`.
- [`da_remove_unordered(da, index)`](./da.h#L327): Removes element at `index` by swapping it with the last element.
- [`da_remove_ordered(da, index)`](./da.h#L338): Removes element at `index` by moving all further elements left.

Slice methods:

- [`da_slice_init(da, [start], [count])`](./da.h#L348): Struct initializer expression for slices, with optional `start` (default `0`) and `count` (defaults to remaining elements).
- [`da_slice(da, Slice_Type, [start], [count])`](./da.h#L353): Returns a slice of `Slice_Type` starting at `start` (default `0`) for `count` elements (default remaining elements).
- [`da_slice_whole(da, Slice_Type)`](./da.h#L367): Creates a `Slice_Type` putting in it full range of `da`.
- [`da_slice_shift(das)`](./da.h#L375): Shifts slice head forward by 1, decrements `count`, and returns the dropped first element.
- [`da_slice_chop_left(das, [n])`](./da.h#L386): Advances slice head past `n` elements (default `1`) and returns a new slice containing the chopped prefix.
- [`da_slice_chop_right(das, [n])`](./da.h#L399): Shrinks slice tail by `n` elements (default `1`) and returns a new slice containing the chopped suffix.
- [`da_slice_chop_while_expr(das, id, expr)`](./da.h#L417): Chops and returns prefix of a slice while elements satisfy `expr` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`).
- [`da_slice_chop_while_pred(das, predicate)`](./da.h#L427): Chops and returns prefix of a slice while elements satisfy `predicate` (function or macro that accepts (Value_Type item, size_t index)).
- [`da_slice_chop_right_while_expr(das, id, expr)`](./da.h#L440): Chops and returns suffix of a slice while elements satisfy `expr` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`).
- [`da_slice_chop_right_while_pred(das, predicate)`](./da.h#L450): Chops and returns suffix of a slice while elements satisfy `predicate` (function or macro that accepts (Value_Type item, size_t index)).
- [`da_slice_chop_by_delim_expr(das, id, expr)`](./da.h#L463): Chops slice up to the first element matching `expr` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`). If not found `das` gona be `.count = 0` and `data` pointed to memory after last character, and returned `.count = das.count, .data = NULL`.
- [`da_slice_chop_by_delim_pred(das, predicate)`](./da.h#L483): Chops slice up to the first element matching `predicate` (function or macro that accepts (Value_Type item, size_t index)). If not found `das` gona be `.count = 0` and `data` pointed to memory after last character, and returned `.count = das.count, .data = NULL`.
- [`da_slice_chop_right_by_delim_expr(das, id, expr)`](./da.h#L496): Chops slice from the end up to the last element matching `expr` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`). If not found `das` gona be `.count = 0`, and returned `.count = das.count, .data = NULL`.
- [`da_slice_chop_right_by_delim_pred(das, predicate)`](./da.h#L516): Chops slice from the end up to the last element matching `predicate` (function or macro that accepts (Value_Type item, size_t index)). If not found `das` gona be `.count = 0`, and returned `.count = das.count, .data = NULL`.

Const array methods:

- [`da_const_init_from_arraylit(Value_Type, [item1, item2, ...])`](./da.h#L523): Constructs a read-only dynamic array compatible structure initialized from a compound literal array.

The default initial capacity is `256` and can be overridden with `DA_INIT_CAP`.

## Floats

[`floats.h`](./floats.h) provides fixed width float types, also tryies to solve `long double` state in c language.

First of all it declares canonical float types for f16, f80 (IEEE 754 Extended Precision), f128 (IEEE 754 Quadruple Precision), f64pair (IBM double-double).
Then it decides which types actually repressents `f16_t`, `f80_t`, `f128_t`, `f64pair_t` types, canonical repressentation or actual native types.

It defines constants:

- [`FLOATS_LD_KIND_F64 = 0`](./floats.h#L40): is when `long double` is same as `double`
- [`FLOATS_LD_KIND_F80 = 1`](./floats.h#L41): is when `long double` is IEEE 754 Extended Precision
- [`FLOATS_LD_KIND_F128 = 2`](./floats.h#L42): is when `long double` is IEEE 754 Quadruple Precision
- [`FLOATS_LD_KIND_F64PAIR = 3`](./floats.h#L43): is when `long double` is IBM double-double
- [`FLOATS_LD_KIND`](./floats.h#L46): is actual detected type

Example of usage:

```c
#if FLOATS_LD_KIND == FLOATS_LD_KIND_F64
#  message "long double is IEEE 754 Double Precision"
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F80
#  message "long double is IEEE 754 Extended Precision"
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F128
#  message "long double is IEEE 754 Quadruple Precision"
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
#  message "long double is IBM double-double"
#else
#  error "Unsupported or unknown long double architecture."
#endif
```

- [`f16_canonical_t`](./floats.h#L74): structure which has guaranteed size of 16 bits.
- [`f80_canonical_t`](./floats.h#L77): structure which has guaranteed size of 80 bits.
- [`f128_canonical_t`](./floats.h#L81): structure which has guaranteed size of 128 bits.
- [`f64pair_canonical_t`](./floats.h#L88): structure which has guaranteed size of 128 bits but also can be parsed as two consequtive doubles.

- [`f16_t`](./floats.h#L106): can be either native `_Float16` or `f16_canonical_t`
- [`f32_t`](./floats.h#L110): is always native `float`
- [`f64_t`](./floats.h#L111): is always native `double`
- [`f80_t`](./floats.h#L113): can be either native `long double` or `f80_canonical_t`
- [`f128_t`](./floats.h#L114): can be either native `long double` or `f128_canonical_t`
- [`f64pair_t`](./floats.h#L115): can be either native `long double` or `f64pair_canonical_t`

Conversion helpers (`_Generic` dispatch macros, each backed by explicit `_from_*`/`_to_*` functions) convert between `float`, `double`, `long double`, and the canonical types:

- [`f16_canonical_from(value)`](./floats.h#L151): converts `value` (`float`, `double`, `long double`, `_Float16` when available, or any canonical type) into `f16_canonical_t`.
- [`f16_canonical_to(value, type)`](./floats.h#L171): converts an `f16_canonical_t` `value` into the requested `type` (`float`, `double`, `long double`, `_Float16` when available, or any canonical type).
- [`f80_canonical_from(value)`](./floats.h#L190): converts `value` into `f80_canonical_t`.
- [`f80_canonical_to(value, type)`](./floats.h#L209): converts an `f80_canonical_t` `value` into the requested `type`.
- [`f128_canonical_from(value)`](./floats.h#L228): converts `value` into `f128_canonical_t`.
- [`f128_canonical_to(value, type)`](./floats.h#L247): converts an `f128_canonical_t` `value` into the requested `type`.
- [`f64pair_canonical_from(value)`](./floats.h#L266): converts `value` into `f64pair_canonical_t`.
- [`f64pair_canonical_to(value, type)`](./floats.h#L285): converts an `f64pair_canonical_t` `value` into the requested `type`.
- [`f_canonical_to_native(value)`](./floats.h#L299): converts any native or canonical float `value` into its corresponding native type (`float`, `double`, or `long double`).
- [`f_canonical_from_native(value, type)`](./floats.h#L308): converts a native float `value` into the requested `type` (native or canonical).

Bit-pattern accessors:

- [`f16_to_bits(value)`](./floats.h#L317): returns the raw 16-bit pattern of `value` as `uint16_t`.
- [`f32_to_bits(value)`](./floats.h#L318): returns the raw 32-bit pattern of `value` as `uint32_t`.
- [`f64_to_bits(value)`](./floats.h#L319): returns the raw 64-bit pattern of `value` as `uint64_t`.
- [`f80_to_bits(value)`](./floats.h#L320): returns the raw 80-bit pattern of `value`, zero-extended to `__uint128_t`.
- [`f128_to_bits(value)`](./floats.h#L321): returns the raw 128-bit pattern of `value` as `__uint128_t`.
- [`f64pair_to_bits(value)`](./floats.h#L322): returns the raw 128-bit pattern of `value` as `__uint128_t`.

## Strings

[`str.h`](./str.h) based on [`da.h`](#dynamic-arrays) provides types and methods to effectively work with strings (heavily inspired by [@tsoding's](https://www.youtube.com/@TsodingDaily) String_Builder, String_View and his [nob.h](https://github.com/tsoding/nob.h)).

Structures:
- [`String_Builder`](./str.h#L45): dynamic array of characters.
- [`String_View`](./str.h#L48): slice view to array of characters.
- [`String`](./str.h#L51): immutable string container.
- [`Stringify_State`](./str.h#L153): helper structure to create custom `sb_append_` compatible functions

Example of usage:

```c
#define STR_IMPL
#include "str.h"

int main(void) {
  String_Builder sb = {0};
  sb_append_strlit(&values, "test");
  sb_append(&values, ':');
  sb_appendf(&values, " %d", 10);
  printf("sb: capacity: %zu, count: %zu, data: '"SV_FMT"'", values.capacity, values.count, sv_fmt_arg(values));

  String str = string_from_sb(&sb);
  printf("sb: capacity: %zu, count: %zu, data: '"SV_FMT"'", values.capacity, values.count, sv_fmt_arg(values));
  printf("str: capacity: %zu, count: %zu, data: '"SV_FMT"'", str.capacity, str.count, sv_fmt_arg(str));

  string_free(&str);
  printf("str: capacity: %zu, count: %zu, data: '"SV_FMT"'", str.capacity, str.count, sv_fmt_arg(str));
}
```

### String_Builder

Common string operations include:

- [`sb_free(sb)`](./str.h#L54): Frees the buffer of `sb` and resets fields to 0.
- [`sb_null_terminate(sb)`](./str.h#L57): Reallocate underlying string as cstr with null terminal and `capacity == count + 1`.
- [`sb_first(sb)`](./str.h#L60): Returns the first character of `sb`.
- [`sb_last(sb)`](./str.h#L62): Returns the last character of `sb`.
- [`sb_at(sb, index)`](./str.h#L64): Returns the character at `index`.
- [`sb_append(sb, character)`](./str.h#L70): Appends a single character. Returns number of characters added.
- [`sb_append_buf(sb, buffer, count)`](./str.h#L78): Appends `count` characters from `buffer`. Returns number of characters added.
- [`sb_vappendf(sb, fmt, ap)`](./str.h#L86): Appends formatted string. Returns number of characters added.
- [`sb_appendf(sb, fmt, ...)`](./str.h#L91): Appends formatted string. Returns number of characters added.
- [`sb_append_cstr(sb, cstr)`](./str.h#L96): Appends a null-terminated C string. Returns number of characters added.
- [`sb_append_strlit(sb, str)`](./str.h#L103): Appends a string literal. Returns number of characters added.
- [`sb_append_sv(sb, sv)`](./str.h#L111): Appends a `String_View` compatible value. Returns number of characters added.
- [`sb_append_repeat(sb, character, count)`](./str.h#L116): Appends `character` repeated `count` times. Returns number of characters added.
- [`sb_append_null(sb)`](./str.h#L127): Appends `'\0'` directly to `sb`. Returns number of characters added.
- [`sb_append_pad_align(sb, size, [filler])`](./str.h#L133): Pads `sb` up to a multiple boundary of `size` using `filler` (defaults to `'\0'`). Returns number of characters added.
- [`sb_substr(sb, [[start], count])`](./str.h#L136): Extracts a `String_View` slice from `sb`.
- [`sb_copy(some_sb_append, ...some_sb_append_arguments)`](./str.h#L146): Executes `some_sb_append` callback on a temporary `String_Builder` and returns it.

### Stringify_State

Helper utilities, usefull to create `sb_append_` like functions. All `sb_append_` functions can accept `NULL` pointer as `sb` and will still return number of characters that would have been written.

- [`make_stringify_state(sb, capacity)`](./str.h#L186): Initializes a `Stringify_State` structure, ensuring `sb` has target capacity.
- [`stringify_append(state, some_sb_append, ...)`](./str.h#L191): Runs `some_sb_append` callback into the state's `sb` and updates total appended count.
- [`stringify_ptr(state, [offset])`](./str.h#L196): Computes pointer relative to the start position of this stringify session.

Example of usage:

```c
size_t sb_append_rand(String_Builder *sb, size_t size) {
  Stringify_State state = make_stringify_state(sb, size - 1); // won't be enough eventually, but it's ok, cause we are using String_Builder
  stringify_append(&state, sb_append, ':');
  for (size_t index = 0; index < size; index += 1) {
    stringify_append(&state, sb_append, 'a' + rand() % 10);
  }
  if (state.sb) {
    for (size_t index = 0; index < size; index += 1) {
      state.sb.data[index] += 1;
    }
    da_reserve(&state.sb, state.sb.count + 2);
    *stringify_ptr(&state) = '(';
    state.count += 1;
    *stringify_ptr(&state) = ')';
    state.count += 1;
  } else {
    state.count += 2;
  }
  return state.count;
}
```

### Stringify sinks

Library provides couple of

- [`fprintf_stringify(f, some_sb_append, ...)`](./str.h#L209): Formats output directly to `FILE *f` using `some_sb_append` without allocating a long-lived string.
- [`printf_stringify(some_sb_append, ...)`](./str.h#L223): Formats directly to `stdout` using `some_sb_append`.

Example of usage:

```c
String str = stringify(sb_append, 't');
fprintf_stringify(stderr, sb_append_sv, sv_from_strlit("test"));
```

### String

- [`string_from_sb(sb)`](./str.h#L226): Consumes `sb`, null-terminates it, transfers dynamic buffer ownership to a `String`.
- [`string_from(some_sb_append, ...some_sb_append_arguments)`](./str.h#L236): Runs `some_sb_append` to populate a builder and returns an owning `String`.
- [`string_from_strlit(str)`](./str.h#L241): Initializes an immutable `String` wrapper pointing directly to `str`.
- [`string_assign(dst, src)`](./str.h#L243): Asserts `dst` is empty, then copies fields from `src`.
- [`string_free(str)`](./str.h#L248): Frees the heap allocation backing `str` and zeroes out the struct.

Example of usage:

```c
String str1 = string_from_sb(sb_copy(sb_append, 1));
String str2 = string_from(sb_append, 't');
String str3 = string_from_strlit("test");
String str4 = {0};
string_assign(&str4, string_from_strlit("test"));
string_free(&str1);
string_free(&str2);
string_free(&str3);
string_free(&str4);
```

### String_View

- [`sv_from_parts(data, count)`](./str.h#L258): Constructs a `String_View` from pointer and length.
- [`sv_from_like(value)`](./str.h#L260): Polymorphic conversion macro mapping any String_View-like into `String_View`.
- [`sv_from_strlit(str)`](./str.h#L266): Creates a `String_View` from a compile-time string literal.
- [`sv_from_cstr(str)`](./str.h#L268): Creates a `String_View` from a standard C-string (`strlen`).
- [`sv_eq(a, b)`](./str.h#L275): Compares two string views.
- [`sv_ends_with(sv, suffix)`](./str.h#L283): Returns `true` if `sv` ends with `suffix`.
- [`sv_starts_with(sv, prefix)`](./str.h#L290): Returns `true` if `sv` starts with `prefix`.

Slicing, Chopping & Trimming:

- [`sv_chop_left(sv, [n])`](./str.h#L303): Mutates `sv` by removing `n` elements (default `1`) from the left and returns the chopped prefix.
- [`sv_chop_right(sv, [n])`](./str.h#L315): Mutates `sv` by removing `n` elements (default `1`) from the right and returns the chopped suffix.
- [`sv_find_expr(sv, id, expr)`](./str.h#L318): Searches forward for element matching `expr`. Same as `da_find_expr`.
- [`sv_find_pred(sv, predicate)`](./str.h#L320): Searches forward for element satisfying `predicate`. Same as `da_find_pred`.
- [`sv_find_right_expr(sv, id, expr)`](./str.h#L322): Searches backward for element matching `expr`. Same as `da_find_right_expr`.
- [`sv_find_right_pred(sv, predicate)`](./str.h#L324): Searches backward for element satisfying `predicate`. Same as `da_find_right_pred`.
- [`sv_chop_while_expr(sv, id, expr)`](./str.h#L327): Chops prefix while `expr` evaluates to true. Same as `da_slice_chop_while_expr`.
- [`sv_chop_while_pred(sv, predicate)`](./str.h#L329): Chops prefix while `predicate` evaluates to true. Same as `da_slice_chop_while_pred`.
- [`sv_chop_while(sv, predicate)`](./str.h#L334): Polymorphic chop-left macro accepting `int(*)(int)`, `bool(*)(char)`, or `bool(*)(char, size_t)`.
- [`sv_chop_right_while_expr(sv, id, expr)`](./str.h#L340): Chops suffix from right while `expr` is true. Same as `da_slice_chop_right_while_expr`.
- [`sv_chop_right_while_pred(sv, predicate)`](./str.h#L342): Chops suffix from right while `predicate` is true. Same as `da_slice_chop_right_while_pred`.
- [`sv_chop_right_while(sv, predicate)`](./str.h#L347): Polymorphic chop-right macro accepting `int(*)(int)`, `bool(*)(char)`, or `bool(*)(char, size_t)`.
- [`sv_chop_by_delim_expr(sv, id, expr)`](./str.h#L353): Chops prefix up to first match of `expr`. Same as `da_slice_chop_by_delim_expr`.
- [`sv_chop_by_delim_pred(sv, predicate)`](./str.h#L355): Chops prefix up to first match of `predicate`. Same as `da_slice_chop_by_delim_pred`.
- [`sv_chop_by_delim(sv, predicate)`](./str.h#L360): Polymorphic delimiter chop accepting single-char predicate variants.
- [`sv_chop_right_by_delim_expr(sv, id, expr)`](./str.h#L366): Chops suffix back to last match of `expr`. Same as `da_slice_chop_right_by_delim_expr`.
- [`sv_chop_right_by_delim_pred(sv, predicate)`](./str.h#L368): Chops suffix back to last match of `predicate`. Same as `da_slice_chop_right_by_delim_pred`.
- [`sv_chop_right_by_delim(sv, predicate)`](./str.h#L373): Polymorphic delimiter chop-right accepting single-char predicate variants.
- [`sv_trim_left(sv)`](./str.h#L379): Strips leading whitespace (`isspace`) from `sv`.
- [`sv_trim_right(sv)`](./str.h#L383): Strips trailing whitespace (`isspace`) from `sv`.
- [`sv_trim(sv)`](./str.h#L387): Trims both leading and trailing whitespace from `sv`.
- [`sv_chop_by_char_delim(sv, delimeter)`](./str.h#L394): Splits `sv` at the first occurrence of character `delimeter`.
- [`sv_chop_right_by_char_delim(sv, delimeter)`](./str.h#L396): Splits `sv` from the right at the last occurrence of character `delimeter`.
- [`sv_chop_by_sv(sv, delimeter)`](./str.h#L399): Splits `sv` at first occurrence of `String_View` `delimeter`.
- [`sv_chop_right_by_sv(sv, delimeter)`](./str.h#L401): Splits `sv` from right at last occurrence of `String_View` `delimeter`.

Utilities:

- [`SV_FMT`](./str.h#L404): Format string constant `"%.*s"` for `printf` family functions.
- [`sv_fmt_arg(sv)`](./str.h#L406): Expands to `(int)(sv).count, (sv).data` for use with `SV_FMT`.

Example of usage:

```c
String_View sv_lit  = sv_from_strlit("  (define foo 42)  ");
String_View sv_cstr = sv_from_cstr("hello world");
String_View trimmed = sv_trim(sv_lit);
printf("Trimmed: '" SV_FMT "'\n", sv_fmt_arg(trimmed)); // "(define foo 42)"

String_View input = sv_from_strlit("car,cdr,cons,quote");
printf("Chopping CSV tokens:\n");
while (input.count) {
  String_View token = sv_chop_by_char_delim(&input, ',');
  printf(" - Token: " SV_FMT "\n", sv_fmt_arg(token));
}

String_View sexpr = sv_from_strlit("(+ 10 20)");
if (sv_starts_with(sexpr, sv_from_strlit("("))) sv_chop_left(&sexpr, 1);
if (sv_ends_with(sexpr, sv_from_strlit(")"))) sv_chop_right(&sexpr, 1);
printf("\nInside parens: '" SV_FMT "'\n", sv_fmt_arg(sexpr)); // "+ 10 20"

String_View op = sv_chop_by_char_delim(&sexpr, ' ');
if (sv_eq(op, sv_from_strlit("+"))) printf("Op is addition\n");
```

## Strings UTF

[`str_utf.h`](./str_utf.h) provides utf related utilities for [`str.h`](#strings).

- [`utf8_character_lengths[0x100]`](./str_utf.h#L38): Array that maps every `char` value to utf8 bytes length.
- [`sv_first_utf_length(sv)`](./str_utf.h#L41): Returns bytes length of first character in a `String_View`.
- [`sv_chop_left_utf(sv)`](./str_utf.h#L44): Chop 1 utf character from left.
- [`sv_utf_length(sv)`](./str_utf.h#L51): Calculate `String_View`'s length in characters.

Example of usage:

```c
  #define STR_UTF_IMPL
  #include "str_utf.h"

  int main(void) {
    ...
    String_View it = source;
    while (it.count) {
      size_t bytes = sv_first_utf_length(it);
      printf("character[%zu]: "SV_FMT"\n", bytes, (int)bytes, it.data);
      sv_chop_left(&it, bytes);
    }
  }
```

## Stringify Integers

[`str_integers.h`](./str_integers.h) provides necessary functions to natively support integer numbers for [`str.h`](#strings).

- [`struct Sb_Integer_Format`](./str_integers.h#L57): Represents how integer must be formated in the string.

Provides couple of integer number format kinds:

- [`SB_INTEGER_FORMAT_KIND_DECIMAL`](./str_integers.h#L49): regular decimal format.
- [`SB_INTEGER_FORMAT_KIND_BINARY`](./str_integers.h#L50): binary number format, provides default prefix `0b`.
- [`SB_INTEGER_FORMAT_KIND_OCTAL`](./str_integers.h#L51): octal number format, provides default prefix `0o`.
- [`SB_INTEGER_FORMAT_KIND_HEX`](./str_integers.h#L52): hex number format, using a-f alphadigits, provides default prefix `0x`.
- [`SB_INTEGER_FORMAT_KIND_HEX_BIG`](./str_integers.h#L53): hex number format, using A-F alphadigits, provides default prefix `0x`.

And options:

- [`hide_prefix`](./str_integers.h#L59): hides prefix, even if `prefix` provided.
- [`prefix`](./str_integers.h#L60): custom prefix.
- [`min_width`](./str_integers.h#L61): minimal width of whole resulting string, if string is too small it will pad with spaces at string begining.

Formatter functions:

- [`sb_append_i8_number_fmt(sb, value, fmt)`](./str_integers.h#L65): int8_t formatter.
- [`sb_append_i16_number_fmt(sb, value, fmt)`](./str_integers.h#L66): int16_t formatter.
- [`sb_append_i32_number_fmt(sb, value, fmt)`](./str_integers.h#L67): int32_t formatter.
- [`sb_append_i64_number_fmt(sb, value, fmt)`](./str_integers.h#L68): int64_t formatter.
- [`sb_append_i128_number_fmt(sb, value, fmt)`](./str_integers.h#L69): __int128_t formatter.
- [`sb_append_u8_number_fmt(sb, value, fmt)`](./str_integers.h#L78): uint8_t formatter.
- [`sb_append_u16_number_fmt(sb, value, fmt)`](./str_integers.h#L79): uint16_t formatter.
- [`sb_append_u32_number_fmt(sb, value, fmt)`](./str_integers.h#L80): uint32_t formatter.
- [`sb_append_u64_number_fmt(sb, value, fmt)`](./str_integers.h#L81): uint64_t formatter.
- [`sb_append_u128_number_fmt(sb, value, fmt)`](./str_integers.h#L82): __uint128_t formatter.

All formatters function also have `struct as macro __VA_ARGS__` variant

- [`sb_append_i8_number(sb, value, ...fmt_fields)`](./str_integers.h#L71): int8_t formatter.
- [`sb_append_i16_number(sb, value, ...fmt_fields)`](./str_integers.h#L72): int16_t formatter.
- [`sb_append_i32_number(sb, value, ...fmt_fields)`](./str_integers.h#L73): int32_t formatter.
- [`sb_append_i64_number(sb, value, ...fmt_fields)`](./str_integers.h#L74): int64_t formatter.
- [`sb_append_i128_number(sb, value, ...fmt_fields)`](./str_integers.h#L75): __int128_t formatter.
- [`sb_append_u8_number(sb, value, ...fmt_fields)`](./str_integers.h#L84): uint8_t formatter.
- [`sb_append_u16_number(sb, value, ...fmt_fields)`](./str_integers.h#L85): uint16_t formatter.
- [`sb_append_u32_number(sb, value, ...fmt_fields)`](./str_integers.h#L86): uint32_t formatter.
- [`sb_append_u64_number(sb, value, ...fmt_fields)`](./str_integers.h#L87): uint64_t formatter.
- [`sb_append_u128_number(sb, value, ...fmt_fields)`](./str_integers.h#L88): __uint128_t formatter.

And there is also polymorphic variants:

- [`sb_append_signed_integer_number_fmt(sb, value, fmt)`](./str_integers.h#L157): polymorphic signed integer types formatter.
- [`sb_append_signed_integer_number(sb, value, ...fmt_args)`](./str_integers.h#L166): polymorphic signed integer types formatter.
- [`sb_append_unsigned_integer_number_fmt(sb, value, fmt)`](./str_integers.h#L169): polymorphic unsigned integer types formatter.
- [`sb_append_unsigned_integer_number(sb, value, ...fmt_args)`](./str_integers.h#L178): polymorphic unsigned integer types formatter.
- [`sb_append_integer_number_fmt(sb, value, fmt)`](./str_integers.h#L181): polymorphic integer types formatter.
- [`sb_append_integer_number(sb, value, ...fmt_args)`](./str_integers.h#L191): polymorphic integer types formatter.

Example of usage:

```c
#define STR_INTEGERS_IMPL
#include "str_integers.h"

int main(void) {
  String_Builder sb = {0};
  sb_append_integer_number(&sb, (long)120349);
  sb_append_integer_number(&sb, (long long)120349, .kind = SB_INTEGER_FORMAT_KIND_HEX);
  sb_append_integer_number(&sb, (char)126, .kind = SB_INTEGER_FORMAT_KIND_BINARY);
}
```

## Stringify Floats

[`str_floats.h`](./str_floats.h) provides necessary functions to natively support floating point numbers in `String_Builder` for [`str.h`](#strings).
It is using ryu algorithm to parse floating number, actual implementation of Ulf Adams from https://github.com/ulfjack/ryu.

- [`struct Sb_Floating_Format`](./str_floats.h#L58): Represents how floating point number must be formated in the string.

Provides couple of integer number format kinds:

- [`SB_FLOATING_FORMAT_KIND_DECIMAL`](./str_floats.h#L52): regular decimal format.
- [`SB_FLOATING_FORMAT_KIND_FIXED`](./str_floats.h#L53): fixed decimal format.
- [`SB_FLOATING_FORMAT_KIND_HEX`](./str_floats.h#L54): hex mantissa format, using a-f alphadigits.
- [`SB_FLOATING_FORMAT_KIND_HEX_BIG`](./str_floats.h#L55): hex mantissa format, using A-F alphadigits.

And options:

- [`min_width`](./str_floats.h#L60): minimal width of whole resulting string, if string is too small it will pad with spaces at string begining.
- [`precision`](./str_floats.h#L61): precision for fixed format.

Formatter functions:

- [`sb_append_f16_number_fmt(sb, f16_t value, fmt)`](./str_floats.h#L65): f16_t formatter
- [`sb_append_f32_number_fmt(sb, float value, fmt)`](./str_floats.h#L66): float formatter
- [`sb_append_f64_number_fmt(sb, double value, fmt)`](./str_floats.h#L67): double formatter
- [`sb_append_f80_number_fmt(sb, f80_t value, fmt)`](./str_floats.h#L68): f80_t formatter
- [`sb_append_f128_number_fmt(sb, f128_t value, fmt)`](./str_floats.h#L69): f128_t formatter
- [`sb_append_f64pair_number_fmt(sb, f64pair_t value, fmt)`](./str_floats.h#L70): f64pair_t formatter
- [`sb_append_f16_canonical_number_fmt(sb, f16_canonical_t value, fmt)`](./str_floats.h#L71): f16_canonical_t formatter
- [`sb_append_f80_canonical_number_fmt(sb, f80_canonical_t value, fmt)`](./str_floats.h#L72): f80_canonical_t formatter
- [`sb_append_f128_canonical_number_fmt(sb, f128_canonical_t value, fmt)`](./str_floats.h#L73): f128_canonical_t formatter
- [`sb_append_f64pair_canonical_number_fmt(sb, f64pair_canonical_t value, fmt)`](./str_floats.h#L74): f64pair_canonical_t formatter

All formatters function also have `struct as macro __VA_ARGS__` variant

- [`sb_append_f16_number(sb, f16_t value, ...fmt_args)`](./str_floats.h#L76): f16_t formatter
- [`sb_append_f32_number(sb, float value, ...fmt_args)`](./str_floats.h#L77): float formatter
- [`sb_append_f64_number(sb, double value, ...fmt_args)`](./str_floats.h#L78): double formatter
- [`sb_append_f80_number(sb, f80_t value, ...fmt_args)`](./str_floats.h#L79): f80_t formatter
- [`sb_append_f128_number(sb, f128_t value, ...fmt_args)`](./str_floats.h#L80): f128_t formatter
- [`sb_append_f64pair_number(sb, f64pair_t value, ...fmt_args)`](./str_floats.h#L81): f64pair_t formatter
- [`sb_append_f16_canonical_number(sb, f16_canonical_t value, ...fmt_args)`](./str_floats.h#L82): f16_canonical_t formatter
- [`sb_append_f80_canonical_number(sb, f80_canonical_t value, ...fmt_args)`](./str_floats.h#L83): f80_canonical_t formatter
- [`sb_append_f128_canonical_number(sb, f128_canonical_t value, ...fmt_args)`](./str_floats.h#L84): f128_canonical_t formatter
- [`sb_append_f64pair_canonical_number(sb, f64pair_canonical_t value, ...fmt_args)`](./str_floats.h#L85): f64pair_canonical_t formatter

And there is also polymorphic variants:

- [`sb_append_floating_number_fmt(sb, value, fmt)`](./str_floats.h#L148): polymorphic floating types formatter.
- [`sb_append_floating_number(sb, value, ...fmt_args)`](./str_floats.h#L157): polymorphic floating types formatter.

Example of usage:

```c
#define STR_FLOATS_IMPL
#include "str_floats.h"

int main(void) {
  String_Builder sb = {0};
  sb_append_floating_number(&sb, (__Float16)120.3);
  sb_append_floating_number(&sb, (float)120.349, .kind = SB_INTEGER_FORMAT_KIND_HEX);
  sb_append_floating_number(&sb, (double)120.349, .kind = SB_INTEGER_FORMAT_KIND_FIXED, .precision = 5);
}
```

## Stringify Numbers

[`str_numbers.h`](./str_numbers.h) provides convinient polymorphic function to stringify
any kind of number, using [`str_integers.h`](#strings-integers) and [`str_floats.h`](#strings-floats).

- [`sb_append_number_fmt(sb, value, fmt)`](./str_numbers.h#L39): polymorphic floating types formatter.
- [`sb_append_number(sb, value, ...fmt_args)`](./str_numbers.h#L55): polymorphic floating types formatter.

## Defines

[`defines.h`](./defines.h) provides small preprocessor utilities used throughout the rest of the libraries (variadic dispatch, argument defaults, stringification, etc).

- [`FOR_EACH(MACRO, ...)`](./defines.h#L21): Runs `MACRO` for each argument, separated by comma.
- [`UNUSED(...)`](./defines.h#L25): Variadic macro that marks all passed variables as used.
- [`NOOP()`](./defines.h#L28): 'No operation' macro.
- [`STRINGIFY(x)`](./defines.h#L32): Creates a string literal from tokens, expanding `x` first.

Argument manipulation:

- [`EXPAND(...)`](./defines.h#L35): Expands to `__VA_ARGS__`.
- [`EXPAND_WITH_COMMA(...)`](./defines.h#L37): Expands to `__VA_ARGS__` with a comma before it (empty if no arguments).
- [`EXPAND_PARENTHESES(args)`](./defines.h#L39): Removes parentheses from `args`.
- [`EXPAND_MACRO(MACRO, ...)`](./defines.h#L41): Expands to a call of `MACRO` with `__VA_ARGS__`.
- [`REST_ARGS(a, ...)`](./defines.h#L44): Removes the first argument, expanding to the rest.
- [`FIRST_ARG(a, ...)`](./defines.h#L47): Expands to the first argument.
- [`SECOND_ARG(a, b, ...)`](./defines.h#L49): Expands to the second argument.
- [`THIRD_ARG(a, b, c, ...)`](./defines.h#L51): Expands to the third argument.
- [`FORTH_ARG(a, b, c, d, ...)`](./defines.h#L53): Expands to the fourth argument.

Defaults:

- [`WITH_DEFAULT(d, ...)`](./defines.h#L56): Expands to the first argument in `__VA_ARGS__`, or to `d` if none was provided.
- [`WITH_TWO_DEFAULTS(d1, d2, ...)`](./defines.h#L58): Expands to the first two arguments in `__VA_ARGS__`, or to `d1, d2` for any that are missing.
- [`IF_VA_OPT(yes, no, ...)`](./defines.h#L61): Expands to `yes` if it was provided with `__VA_ARGS__`, or to `no` if not.

Misc:

- [`MIN(a, b)`](./defines.h#L64): Compares two values and returns the minimal.
- [`PRINTF_ATTRIBUTE(STRING_INDEX, FIRST_TO_CHECK)`](./defines.h#L67): Marks a function with a printf-style static check.
- [`PRINTF_FMT_PARAM`](./defines.h#L69): Marks a function parameter as the printf format argument.

Example of usage:

```c
#include "defines.h"

#define LOG(message, args, ...) fprintf(WITH_DEFAULT(stdout, __VA_ARGS__), message, EXPAND_PARENS(args))

void example(int x, void *unused_ptr, ...) {
  UNUSED(x, unused_ptr);
  int a = FIRST_ARG(1, 2, 3);
  int b = SECOND_ARG(1, 2, 3);
}
```

## Abort

[`abort.h`](./abort.h) provides small panic and assertion utilities for c23, built on top of [`defines.h`](#preprocessor-utilities).

- [`panicf(file, line, label, format, ...)`](./abort.h#L17): Prints `file:line: label: <formatted message>` to `stderr` and calls `abort()`. Does not return.
- [`TODO(...)`](./abort.h#L21): Panics with a `"TODO"` label. Message defaults to [`TODO_DEFAULT_MESSAGE`](./abort.h#L18) (`"not implemented yet"`), or accepts a printf-style format and arguments.
- [`UNREACHABLE(format, ...)`](./abort.h#L23): Panics with an `"UNREACHABLE"` label. Message defaults to `"should never happen"`, or accepts a printf-style format and arguments.

- [`ASSERT(condition, format, ...)`](./abort.h#L26): If `condition` is false, panics via `UNREACHABLE` with the given message.
- [`STATIC_ASSERT(condition, msg)`](./abort.h#L28): Compile-time assertion usable as an expression (evaluates to a `sizeof`).
- [`TYPE_ASSERT(value, expected, ...)`](./abort.h#L30): Compile-time assertion that `value`'s type matches `expected`, using `STATIC_ASSERT`. Message defaults to `"Type mismatch"`.

Example of usage:

```c
#define ABORT_IMPL
#include "abort.h"

int divide(int a, int b) {
  ASSERT(b != 0, "division by zero: %d / %d", a, b);
  return a / b;
}

void unimplemented_feature(void) {
  TODO("wire up the new parser");
}

void handle_state(int state) {
  switch (state) {
    case 0: /* ... */ break;
    case 1: /* ... */ break;
    default: UNREACHABLE("unexpected state: %d", state);
  }
}
```

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

## nanoid

[nanoid.h](./nanoid.h) generates random identifiers using rand().

```c
#define NANOID_IMPL
#include "nanoid.h"

char *id = nanoid("item_", 12);
/* Use id... */
free(id);
```
