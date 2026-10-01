#define FLOATS_IGNORE_F16_WARNINGS
#define DA_IMPL
#include <da.h>
#define STR_IMPL
#include "str.h"
#define STR_UTF_IMPL
#include "str_utf.h"
#define STR_NUMBERS_IMPL
#include "str_numbers.h"
#define ABORT_IMPL
#include "abort.h"
#define RC_IMPL
#include "rc.h"
#define FS_IMPL
#include "fs.h"
#define FS_PATH_IMPL
#include "fs_path.h"

static void show(const char *label, Fs_Path path) {
  Fs_Path_Render r = fs_path_render(path);
  printf("%-10s %-22s %zu segments: [", label, r.cstr.data, r.path.count);
  for (size_t index = 0; index < path.count; index += 1) {
    if (index) printf(",");
    printf("\"" SV_FMT "\"", sv_fmt_arg(path.data[index]));
  }
  printf("]\n");
  fs_path_render_free(&r);
}

int main(void) {
  Fs_Path cwd = fs_path_parse(sv_from_strlit("/home/user/project"));
  Fs_Path rel = fs_path_parse(sv_from_strlit("../lib//./foo.c"));
  Fs_Path abs = fs_path_parse(sv_from_strlit("/etc/../usr/bin"));
  Fs_Path up = fs_path_parse(sv_from_strlit("../../.."));
  Fs_Path none = {0};

  show("cwd", cwd);
  show("rel", rel);
  show("abs", abs);
  show("up", up);
  show("none", none);
  printf("\n");

  Fs_Path a = fs_path_join(cwd, rel);   // /home/user/lib/foo.c
  Fs_Path b = fs_path_join(cwd, abs);   // /usr/bin (absolute replaces base)
  Fs_Path c = fs_path_join(cwd, up);    // /
  Fs_Path d = fs_path_join(none, none); // .
  fs_path_normalize(&rel);              // ../lib/foo.c (stays relative)

  show("cwd+rel", a);
  show("cwd+abs", b);
  show("cwd+up", c);
  show("empty", d);
  show("normalize", rel);
  printf("\n");

  printf("a absolute? %d, d relative? %d\n", fs_path_is_absolute(a), fs_path_is_relative(d));

  fs_path_free(&cwd);
  fs_path_free(&rel);
  fs_path_free(&abs);
  fs_path_free(&up);
  fs_path_free(&a);
  fs_path_free(&b);
  fs_path_free(&c);
  fs_path_free(&d);
}
