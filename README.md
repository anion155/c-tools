# c-tools

A collection of small, dependency-free C utilities implemented primarily as std-styled single-header libraries.

The project provides typed dynamic arrays, string builders and views, UTF-8 helpers, reference-counted memory, random ID generation, floating-point conversion utilities, and common preprocessor helpers.

## Features

- [`da.h`](#dynamic-arrays) Typed dynamic arrays and slices
- [`str.h`](#strings) String, string builder and non-owning string views
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
- [`Da(Value_Type, [struct_name])`](./da.h#L15): represents array that can grow dynamically, owns data, and requires freeing after use.
- [`Da_Slice(Value_Type, [struct_name])`](./da.h#L22): represents slice of an array, does not own data.
- [`Da_Const(Value_Type, [struct_name])`](./da.h#L28): represents constant size array compatible with da_any_ methods

Useful operations include:

- [`da_first(da)`](./da.h#L69): Returns the first element of the dynamic array.
- [`da_last(da)`](./da.h#L78): Returns the last element of the dynamic array.
- [`da_at(da, index)`](./da.h#L87): Returns the element at `index`.
- [`da_foreach(da, id)`](./da.h#L97): Iterates over `da`, creating a pointer loop variable named `id` and index `id##_index`.
- [`da_find_expr(da, id, expr)`](./da.h#L112): Returns index of first element satisfying `expr` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`), or array's size if not found.
- [`da_find_pred(da, predicate)`](./da.h#L135): Returns index of first element satisfying `predicate` (function or macro that accepts (Value_Type item, size_t index)), or array's size if not found.
- [`da_find_right_expr(da, id, expr)`](./da.h#L155): Returns `index + 1` of first element from the end satisfying `expr` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`), or `0` if not found.
- [`da_find_right_pred(da, predicate)`](./da.h#L178): Returns `index + 1` of first element from the end satisfying `predicate` (function or macro that accepts (Value_Type item, size_t index)), or `0` if not found.
- [`da_free(da)`](./da.h#L196): Frees `data` buffer, zeroes `capacity` and `count`, and resets pointer to `NULL`.
- [`da_reserve(da, target_capacity)`](./da.h#L209): Ensures capacity is at least `target_capacity` by doubling capacity geometrically.
- [`da_reserve_exact(da, target_capacity)`](./da.h#L223): Reallocates capacity to exactly `target_capacity` if needed without geometric growth.
- [`da_trim_realloc(da)`](./da.h#L236): Shrinks memory allocation so `capacity` matches `count`, or frees if `count == 0`.
- [`da_resize(da, new_size)`](./da.h#L253): Reserves memory for `new_size` and sets `count = new_size`.
- [`da_append(da, item)`](./da.h#L261): Appends a single `item` to the dynamic array, growing capacity if necessary.
- [`da_append_many_n(da, new_items, new_count)`](./da.h#L272): Appends `new_count` elements from buffer `new_items`.
- [`da_append_many(da, [item1, item2, ...])`](./da.h#L284): Appends variadic literal items to `da`.
- [`da_pop(da)`](./da.h#L297): Returns the last element and decrements `count`.
- [`da_remove_unordered(da, index)`](./da.h#L307): Removes element at `index` by swapping it with the last element.
- [`da_remove_ordered(da, index)`](./da.h#L317): Removes element at `index` by moving all further elements left.
- [`da_slice_init(da, [start], [count])`](./da.h#L326): Struct initializer expression for slices, with optional `start` (default `0`) and `count` (defaults to remaining elements).
- [`da_slice(da, Slice_Type, [start], [count])`](./da.h#L331): Returns a slice of `Slice_Type` starting at `start` (default `0`) for `count` elements (default remaining elements).
- [`da_slice_whole(da, Slice_Type)`](./da.h#L345): Creates a `Slice_Type` putting in it full range of `da`.
- [`da_slice_shift(das)`](./da.h#L353): Shifts slice head forward by 1, decrements `count`, and returns the dropped first element.
- [`da_slice_chop_left(das, [n])`](./da.h#L364): Advances slice head past `n` elements (default `1`) and returns a new slice containing the chopped prefix.
- [`da_slice_chop_right(das, [n])`](./da.h#L376): Shrinks slice tail by `n` elements (default `1`) and returns a new slice containing the chopped suffix.
- [`da_slice_chop_while_expr(das, id, expr)`](./da.h#L395): Chops and returns prefix of a slice while elements satisfy `expr` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`).
- [`da_slice_chop_while_pred(das, predicate)`](./da.h#L404): Chops and returns prefix of a slice while elements satisfy `predicate` (function or macro that accepts (Value_Type item, size_t index)).
- [`da_slice_chop_right_while_expr(das, id, expr)`](./da.h#L418): Chops and returns suffix of a slice while elements satisfy `expr` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`).
- [`da_slice_chop_right_while_pred(das, predicate)`](./da.h#L427): Chops and returns suffix of a slice while elements satisfy `predicate` (function or macro that accepts (Value_Type item, size_t index)).
- [`da_slice_chop_by_delim_expr(das, id, expr)`](./da.h#L440): Chops slice up to the first element matching `expr` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`). If not found `das` gona be `.count = 0` and `data` pointed to memory after last character, and returned `.count = das.count, .data = NULL`.
- [`da_slice_chop_by_delim_pred(das, predicate)`](./da.h#L458): Chops slice up to the first element matching `predicate` (function or macro that accepts (Value_Type item, size_t index)). If not found `das` gona be `.count = 0` and `data` pointed to memory after last character, and returned `.count = das.count, .data = NULL`.
- [`da_slice_chop_right_by_delim_expr(das, id, expr)`](./da.h#L479): Chops slice from the end up to the last element matching `expr` (as tokens with available variables: `id` pointer to item, `id##_index`, `id##_data`, `id##_count`). If not found `das` gona be `.count = 0`, and returned `.count = das.count, .data = NULL`.
- [`da_slice_chop_right_by_delim_pred(das, predicate)`](./da.h#L498): Chops slice from the end up to the last element matching `predicate` (function or macro that accepts (Value_Type item, size_t index)). If not found `das` gona be `.count = 0`, and returned `.count = das.count, .data = NULL`.
- [`da_const_init_from_arraylit(Value_Type, [item1, item2, ...])`](./da.h#L506): Constructs a read-only dynamic array compatible structure initialized from a compound literal array.

The default initial capacity is `256` and can be overridden with `DA_INIT_CAP`.

## Strings

[str.h](./str.h) based on [da.h](#dynamic-arrays) provides types and methods to effectively work with strings (heavily inspired by [@tsoding's](https://www.youtube.com/@TsodingDaily) String_Builder, String_View and his [nob.h](https://github.com/tsoding/nob.h)).

Structures:
- [`String_Builder`](./str.h#L44): dynamic array of characters.
- [`String_View`](./str.h#L47): slice view to array of characters.
- [`String`](./str.h#L50): immutable string container.
- [`Stringify_State`](./str.h#L137): helper structure to create custom `sb_append_` compatible functions

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

- [`sb_free(sb)`](./str.h#L53): Frees the buffer of `sb` and resets fields to 0.
- [`sb_null_terminate(sb)`](./str.h#L56): Reallocate underlying string as cstr with null terminal and `capacity == count + 1`.
- [`sb_first(sb)`](./str.h#L59): Returns the first character of `sb`.
- [`sb_last(sb)`](./str.h#L61): Returns the last character of `sb`.
- [`sb_at(sb, index)`](./str.h#L63): Returns the character at `index`.
- [`sb_append(sb, character)`](./str.h#L69): Appends a single character. Returns number of characters added.
- [`sb_append_buf(sb, buffer, count)`](./str.h#L77): Appends `count` characters from `buffer`. Returns number of characters added.
- [`sb_vappendf(sb, fmt, ap)`](./str.h#L85): Appends formatted string. Returns number of characters added.
- [`sb_appendf(sb, fmt, ...)`](./str.h#L90): Appends formatted string. Returns number of characters added.
- [`sb_append_cstr(sb, cstr)`](./str.h#L95): Appends a null-terminated C string. Returns number of characters added.
- [`sb_append_strlit(sb, str)`](./str.h#L102): Appends a string literal. Returns number of characters added.
- [`sb_append_sv(sb, sv)`](./str.h#L110): Appends a `String_View` compatible value. Returns number of characters added.
- [`sb_append_repeat(sb, character, count)`](./str.h#L115): Appends `character` repeated `count` times. Returns number of characters added.
- [`sb_append_null(sb)`](./str.h#L126): Appends `'\0'` directly to `sb`. Returns number of characters added.
- [`sb_append_pad_align(sb, size, [filler])`](./str.h#L131): Pads `sb` up to a multiple boundary of `size` using `filler` (defaults to `'\0'`). Returns number of characters added.
- [`sb_substr(sb, [[start], count])`](./str.h#L134): Extracts a `String_View` slice from `sb`.
- [`sb_copy(some_sb_append, ...some_sb_append_arguments)`](./str.h#L143): Executes `some_sb_append` callback on a temporary `String_Builder` and returns it.

### Stringify_State

Helper utilities, usefull to create `sb_append_` like functions. All `sb_append_` functions can accept `NULL` pointer as `sb` and will still return number of characters that would have been written.

- [`make_stringify_state(sb, capacity)`](./str.h#L178): Initializes a `Stringify_State` structure, ensuring `sb` has target capacity.
- [`stringify_append(state, some_sb_append, ...)`](./str.h#L183): Runs `some_sb_append` callback into the state's `sb` and updates total appended count.
- [`stringify_ptr(state, [offset])`](./str.h#L188): Computes pointer relative to the start position of this stringify session.

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

- [`fprintf_stringify(f, some_sb_append, ...)`](./str.h#L201): Formats output directly to `FILE *f` using `some_sb_append` without allocating a long-lived string.
- [`printf_stringify(some_sb_append, ...)`](./str.h#L215): Formats directly to `stdout` using `some_sb_append`.

Example of usage:

```c
String str = stringify(sb_append, 't');
fprintf_stringify(stderr, sb_append_sv, sv_from_strlit("test"));
```

### String

- [`string_from_sb(sb)`](./str.h#L218): Consumes `sb`, null-terminates it, transfers dynamic buffer ownership to a `String`.
- [`string_from(some_sb_append, ...some_sb_append_arguments)`](./str.h#L228): Runs `some_sb_append` to populate a builder and returns an owning `String`.
- [`string_from_strlit(str)`](./str.h#L233): Initializes an immutable `String` wrapper pointing directly to `str`.
- [`string_assign(dst, src)`](./str.h#L235): Asserts `dst` is empty, then copies fields from `src`.
- [`string_free(str)`](./str.h#L240): Frees the heap allocation backing `str` and zeroes out the struct.

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

- [`sv_from_parts(data, count)`](./str.h#L250): Constructs a `String_View` from pointer and length.
- [`sv_from_like(value)`](./str.h#L252): Polymorphic conversion macro mapping any String_View-like into `String_View`.
- [`sv_from_strlit(str)`](./str.h#L258): Creates a `String_View` from a compile-time string literal.
- [`sv_from_cstr(str)`](./str.h#L260): Creates a `String_View` from a standard C-string (`strlen`).
- [`sv_eq(a, b)`](./str.h#L268): Compares two string views.
- [`sv_starts_with(sv, prefix)`](./str.h#L282): Returns `true` if `sv` starts with `prefix`.
- [`sv_ends_with(sv, suffix)`](./str.h#L275): Returns `true` if `sv` ends with `suffix`.

Slicing, Chopping & Trimming:

- [`sv_chop_left(sv, [n])`](./str.h#L292): Mutates `sv` by removing `n` elements (default `1`) from the left and returns the chopped prefix.
- [`sv_chop_right(sv, [n])`](./str.h#L302): Mutates `sv` by removing `n` elements (default `1`) from the right and returns the chopped suffix.
- [`sv_find_expr(sv, id, expr)`](./str.h#L305): Searches forward for element matching `expr`. Same as `da_find_expr`.
- [`sv_find_pred(sv, predicate)`](./str.h#L307): Searches forward for element satisfying `predicate`. Same as `da_find_pred`.
- [`sv_find_right_expr(sv, id, expr)`](./str.h#L309): Searches backward for element matching `expr`. Same as `da_find_right_expr`.
- [`sv_find_right_pred(sv, predicate)`](./str.h#L311): Searches backward for element satisfying `predicate`. Same as `da_find_right_pred`.
- [`sv_chop_while_expr(sv, id, expr)`](./str.h#L314): Chops prefix while `expr` evaluates to true. Same as `da_slice_chop_while_expr`.
- [`sv_chop_while_pred(sv, predicate)`](./str.h#L316): Chops prefix while `predicate` evaluates to true. Same as `da_slice_chop_while_pred`.
- [`sv_chop_while(sv, predicate)`](./str.h#L321): Polymorphic chop-left macro accepting `int(*)(int)`, `bool(*)(char)`, or `bool(*)(char, size_t)`.
- [`sv_chop_right_while_expr(sv, id, expr)`](./str.h#L327): Chops suffix from right while `expr` is true. Same as `da_slice_chop_right_while_expr`.
- [`sv_chop_right_while_pred(sv, predicate)`](./str.h#L329): Chops suffix from right while `predicate` is true. Same as `da_slice_chop_right_while_pred`.
- [`sv_chop_right_while(sv, predicate)`](./str.h#L334): Polymorphic chop-right macro accepting `int(*)(int)`, `bool(*)(char)`, or `bool(*)(char, size_t)`.
- [`sv_chop_by_delim_expr(sv, id, expr)`](./str.h#L340): Chops prefix up to first match of `expr`. Same as `da_slice_chop_by_delim_expr`.
- [`sv_chop_by_delim_pred(sv, predicate)`](./str.h#L342): Chops prefix up to first match of `predicate`. Same as `da_slice_chop_by_delim_pred`.
- [`sv_chop_by_delim(sv, predicate)`](./str.h#L347): Polymorphic delimiter chop accepting single-char predicate variants.
- [`sv_chop_right_by_delim_expr(sv, id, expr)`](./str.h#L353): Chops suffix back to last match of `expr`. Same as `da_slice_chop_right_by_delim_expr`.
- [`sv_chop_right_by_delim_pred(sv, predicate)`](./str.h#L355): Chops suffix back to last match of `predicate`. Same as `da_slice_chop_right_by_delim_pred`.
- [`sv_chop_right_by_delim(sv, predicate)`](./str.h#L360): Polymorphic delimiter chop-right accepting single-char predicate variants.
- [`sv_trim_left(sv)`](./str.h#L366): Strips leading whitespace (`isspace`) from `sv`.
- [`sv_trim_right(sv)`](./str.h#L370): Strips trailing whitespace (`isspace`) from `sv`.
- [`sv_trim(sv)`](./str.h#L374): Trims both leading and trailing whitespace from `sv`.
- [`sv_chop_by_char_delim(sv, delimeter)`](./str.h#L381): Splits `sv` at the first occurrence of character `delimeter`.
- [`sv_chop_right_by_char_delim(sv, delimeter)`](./str.h#L383): Splits `sv` from the right at the last occurrence of character `delimeter`.
- [`sv_chop_by_sv(sv, delimeter)`](./str.h#L386): Splits `sv` at first occurrence of `String_View` `delimeter`.
- [`sv_chop_right_by_sv(sv, delimeter)`](./str.h#L388): Splits `sv` from right at last occurrence of `String_View` `delimeter`.

Utilities:

- [`SV_FMT`](./str.h#L391): Format string constant `"%.*s"` for `printf` family functions.
- [`sv_fmt_arg(sv)`](./str.h#L393): Expands to `(int)(sv).count, (sv).data` for use with `SV_FMT`.

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

[str_utf.h](./str_utf.h) provides utf related utilities.

- [`utf8_character_lengths[0x100]`](./str_utf.h#L): Array that maps every `char` value to utf8 bytes length.
- [`sv_first_utf_length(sv)`](./str_utf.h#L): Returns bytes length of first character in a `String_View`.
- [`sv_chop_left_utf(sv)`](./str_utf.h#L): Chop 1 utf character from left.
- [`sv_utf_length(sv)`](./str_utf.h#L): Calculate `String_View`'s length in characters.

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
