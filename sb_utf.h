#ifndef SB_UTF_H
#define SB_UTF_H

#include <sb.h>
#include <stddef.h>
#include <stdint.h>

extern const uint8_t utf8_character_lengths[0x100];

size_t sv__utf_length(String_View sv, size_t *bytes_overrun);
#define sv_utf_length(sv, ...) sv__utf_length(sv_from_like(sv), WITH_DEFAULT(NULL, __VA_ARGS__))

#endif // SB_UTF_H

#if defined(SB_UTF_IMPL) && !defined(SB_UTF_IMPL_C)
#define SB_UTF_IMPL_C

// clang-format off
const uint8_t utf8_character_lengths[] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,5,5,5,5,6,6,6,6,
};
// clang-format on

size_t sv__utf_length(String_View sv, size_t *bytes_overrun) {
  size_t count = 0;
  size_t bytes;
  while (sv.count) {
    bytes = sv_first_utf_length(sv);
    if (bytes_overrun && sv.count <= bytes) *bytes_overrun = bytes - sv.count;
    sv_chop_left(&sv, bytes);
    count += 1;
  }
  return count;
}

#endif // SB_UTF_IMPL_C
