#ifndef FS_PATH_H
#define FS_PATH_H

#include <abort.h>
#include <da.h>
#include <stdlib.h>
#include <str.h>
#include <string.h>

typedef Da(String_View, Fs_Path) Fs_Path;
typedef Da_Slice(String_View, Fs_Path_Slice) Fs_Path_Slice;
typedef struct Fs_Path_Render {
  void *const data;
  String_View cstr;
  Fs_Path_Slice path;
} Fs_Path_Render;

Fs_Path fs_path_parse_sv(String_View str);
#define fs_path_parse_n(str, n) fs_path_parse_sv(sv_from_parts(str, n))
#define fs_path_parse_cstr(str) fs_path_parse_sv(sv_from_cstr(str))
#define fs_path_parse_strlit(str) fs_path_parse_sv(sv_from_strlit(str))
Fs_Path fs_path_copy(Fs_Path path);
void fs_path_append(Fs_Path *path, String_View file);
void fs_path_append_root(Fs_Path *path);
void fs_path_append_path(Fs_Path *path, Fs_Path other);
Fs_Path fs_path_join(Fs_Path base, Fs_Path other);
void fs_path_normalize(Fs_Path *path);
bool fs_path_is_absolute(Fs_Path path);
bool fs_path_is_relative(Fs_Path path);
size_t sb_append_fs_path(String_Builder *sb, Fs_Path path);
Fs_Path_Render fs_path_render(Fs_Path path);
#define fs_path_free(path) da_free(path)
#define fs_path_render_free(path_) ({  \
  Fs_Path_Render *_path_ = (path_);    \
  if (_path_->data) {                  \
    free(_path_->data);                \
    *da__reassign(_path_) = NULL;      \
    _path_->cstr = (String_View){0};   \
    _path_->path = (Fs_Path_Slice){0}; \
  }                                    \
})

#endif // FS_PATH_H

#if defined(FS_PATH_IMPL) && !defined(FS_PATH_IMPL_C)
#define FS_PATH_IMPL_C

Fs_Path fs_path_parse_sv(String_View str) {
  Fs_Path path = {0};
  if (str.count && *str.data == '/') da_append(&path, sv_from_parts(str.data, 0));
  String_View start = str;
  for (String_View it = str; it.count; sv_chop_left(&it)) {
    if (*it.data != '/') continue;
    size_t count = it.data - start.data;
    if (count) da_append(&path, sv_from_parts(start.data, count));
    start = sv_from_parts(it.data + 1, it.count - 1);
  }
  if (start.count) da_append(&path, start);
  return path;
}

Fs_Path fs_path_copy(Fs_Path path) {
  Fs_Path result = {0};
  da_append_many_n(&result, path.data, path.count);
  return result;
}

void fs_path_append(Fs_Path *path, String_View file) {
  static String_View same_dir = sv_from_strlit(".");
  static String_View top_dir = sv_from_strlit("..");
  if (!file.count) return;
  if (sv_eq(file, same_dir)) return;
  if (sv_eq(file, top_dir) && path->count) {
    if (path->count == 1 && !path->data[0].count) return;
    if (!sv_eq(path->data[path->count - 1], top_dir)) {
      path->count -= 1;
      return;
    }
  }
  da_append(path, file);
}

void fs_path_append_root(Fs_Path *path) {
  ASSERT(!path->count, "Can't append root to non empty path");
  da_append(path, sv_from_parts(NULL, 0));
}

void fs_path_append_path(Fs_Path *path, Fs_Path other) {
  if (fs_path_is_absolute(other)) {
    path->count = 0;
    da_append(path, other.data[0]);
  }
  for (size_t index = 0; index < other.count; index += 1) {
    fs_path_append(path, other.data[index]);
  }
}

Fs_Path fs_path_join(Fs_Path base, Fs_Path other) {
  Fs_Path result = {0};
  if (!fs_path_is_absolute(other)) fs_path_append_path(&result, base);
  fs_path_append_path(&result, other);
  da_trim_realloc(&result);
  return result;
}

void fs_path_normalize(Fs_Path *path) {
  Fs_Path result = fs_path_copy(*path);
  path->count = 0;
  fs_path_append_path(path, result);
}

bool fs_path_is_absolute(Fs_Path path) {
  return path.count && !path.data[0].count;
}

bool fs_path_is_relative(Fs_Path path) {
  return !path.count || path.data[0].count;
}

size_t sb_append_fs_path(String_Builder *sb, Fs_Path path) {
  Stringify_State state = {.sb = sb};
  if (!path.count) {
    stringify_append(&state, sb_append, '.');
    return state.count;
  }
  if (path.count == 1 && !path.data[0].count) {
    stringify_append(&state, sb_append, '/');
    return state.count;
  }
  for (size_t index = 0, to = path.count - 1; index < to; index += 1) {
    stringify_append(&state, sb_append_sv, path.data[index]);
    stringify_append(&state, sb_append, '/');
  }
  stringify_append(&state, sb_append_sv, path.data[path.count - 1]);
  return state.count;
}

Fs_Path_Render fs_path_render(Fs_Path path) {
  String_Builder sb = {0};
  sb_append_fs_path(&sb, path);
  size_t count = sb.count;
  void *data = malloc(path.count * sizeof(String_View) + (count + 1) * sizeof(char));
  ASSERT(data, "Out of memory");
  String_View *sv_data = (String_View *)data;
  char *cstr = (char *)(sv_data + path.count);
  memcpy(cstr, sb.data, count);
  cstr[count] = '\0';
  for (size_t index = 0, offset = 0; index < path.count; index += 1) {
    size_t part_count = path.data[index].count;
    sv_data[index] = sv_from_parts(cstr + offset, part_count);
    offset += part_count + 1;
  }
  sb_free(&sb);
  return (Fs_Path_Render){.data = data, .cstr = {.data = cstr, .count = count}, .path = {.data = sv_data, .count = path.count}};
}

#endif // FS_PATH_IMPL_C

/**
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <https://unlicense.org/>
 */
