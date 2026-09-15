#define DA_IMPL
#include <da.h>
#define STR_IMPL
#include "str.h"

int main(void) {
  String_Builder sb = {0};
  sb_append_strlit(&sb, "test");
  sb_append(&sb, ':');
  sb_appendf(&sb, " %d", 10);
  printf("sb: capacity: %zu, count: %zu, data: '" SV_FMT "'\n", sb.capacity, sb.count, sv_fmt_arg(sb));

  String str = string_from_sb(&sb);
  printf("sb: capacity: %zu, count: %zu, data: '" SV_FMT "'\n", sb.capacity, sb.count, sv_fmt_arg(sb));
  printf("str: count: %zu, data: '" SV_FMT "'\n", str.count, sv_fmt_arg(str));

  string_free(&str);
  printf("str: count: %zu, data: '" SV_FMT "'\n", str.count, sv_fmt_arg(str));
}
