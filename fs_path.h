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

#define fs_path_slice(path) da_slice((path), Fs_Path_Slice)

#define fs__path_cast_assert(_path) STATIC_ASSERT( \
    _Generic(&_path,                               \
        Fs_Path *: true,                           \
        Fs_Path_Slice *: true,                     \
        Fs_Path_Render *: true,                    \
        String_View *: true,                       \
        default: _Generic(_path,                   \
            char *: true,                          \
            const char *: true,                    \
            default: false)),                      \
    "unsupported type")
#define fs_path_with_cast_path(_path, path_var)                                         \
  for (Fs_Path path_var = ({                                                            \
         fs__path_cast_assert(_path);                                                   \
         _Generic(&_path,                                                               \
             Fs_Path *: (*(Fs_Path *)(&_path)),                                         \
             Fs_Path_Slice *: fs__path_copy(*(Fs_Path_Slice *)(&_path)),                \
             Fs_Path_Render *: fs__path_copy(((Fs_Path_Render *)(&_path))->path),       \
             String_View *: fs_path_parse(*(String_View *)(&_path)),                    \
             default: _Generic(_path,                                                   \
                 char *: fs_path_parse(sv_from_cstr(*((const char **)(&_path)))),       \
                 const char *: fs_path_parse(sv_from_cstr(*((const char **)(&_path)))), \
                 default: (UNREACHABLE("unsuported type"), (Fs_Path){0})));             \
       }),                                                                              \
               *_cast_ = &path_var;                                                     \
       _cast_;                                                                          \
       ({                                                                               \
         _cast_ = NULL;                                                                 \
         if (_Generic(&_path, Fs_Path *: true, default: false)) {                       \
           *da__reassign(&path_var) = NULL;                                             \
         } else {                                                                       \
           fs_path_free(&path_var);                                                     \
         }                                                                              \
       }))
#define fs_path_with_cast_slice(_path, path_var)                                                                 \
  for (Fs_Path_Slice path_var = ({                                                                               \
         fs__path_cast_assert(_path);                                                                            \
         _Generic(&_path,                                                                                        \
             Fs_Path *: fs_path_slice(*(Fs_Path *)(&_path)),                                                     \
             Fs_Path_Slice *: (*(Fs_Path_Slice *)(&_path)),                                                      \
             Fs_Path_Render *: ((Fs_Path_Render *)(&_path))->path,                                               \
             String_View *: fs_path_slice(fs_path_parse(*(String_View *)(&_path))),                              \
             default: _Generic(_path,                                                                            \
                 char *: fs_path_slice(fs_path_parse(sv_from_cstr(*((const char **)(&_path))))),                 \
                 const char *: fs_path_slice(fs_path_parse(sv_from_cstr(*((const char **)(&_path))))),           \
                 default: (UNREACHABLE("unsuported type"), (Fs_Path_Slice){0})));                                \
       }),                                                                                                       \
                     *_cast_ = &path_var;                                                                        \
       _cast_;                                                                                                   \
       ({                                                                                                        \
         _cast_ = NULL;                                                                                          \
         if (_Generic(&_path, Fs_Path *: true, Fs_Path_Slice *: true, Fs_Path_Render *: true, default: false)) { \
           *da__reassign(&path_var) = NULL;                                                                      \
         } else {                                                                                                \
           free((void *)path_var.data);                                                                          \
         }                                                                                                       \
       }))
#define fs_path_with_cast_render(_path, path_var)                                              \
  for (Fs_Path_Render path_var = ({                                                            \
         fs__path_cast_assert(_path);                                                          \
         _Generic(&_path,                                                                      \
             Fs_Path *: fs__path_render(fs_path_slice(*(Fs_Path *)(&_path))),                  \
             Fs_Path_Slice *: fs__path_render(*(Fs_Path_Slice *)(&_path)),                     \
             Fs_Path_Render *: *((Fs_Path_Render *)(&_path)),                                  \
             String_View *: fs_path_render_parse(*((String_View *)(&_path))),                  \
             default: _Generic(_path,                                                          \
                 char *: fs_path_render_parse(sv_from_cstr(*((const char **)(&_path)))),       \
                 const char *: fs_path_render_parse(sv_from_cstr(*((const char **)(&_path)))), \
                 default: (UNREACHABLE("unsuported type"), (Fs_Path_Render){0})));             \
       }),                                                                                     \
                      *_cast_ = &path_var;                                                     \
       _cast_;                                                                                 \
       ({                                                                                      \
         _cast_ = NULL;                                                                        \
         if (_Generic(&_path, Fs_Path_Render *: true, default: false)) {                       \
           *da__reassign(&path_var) = NULL;                                                    \
         } else {                                                                              \
           fs_path_render_free(&path_var);                                                     \
         }                                                                                     \
       }))
#define fs_path_with_cast_cstr(_path, path_var)                               \
  for (Fs_Path_Render path_var##_render = ({                                  \
         fs__path_cast_assert(_path);                                         \
         _Generic(&_path,                                                     \
             Fs_Path *: fs__path_render(fs_path_slice(*(Fs_Path *)(&_path))), \
             Fs_Path_Slice *: fs__path_render(*(Fs_Path_Slice *)(&_path)),    \
             default: (Fs_Path_Render){0});                                   \
       }),                                                                    \
                      *_castr_ = &path_var##_render;                          \
       _castr_;                                                               \
       _castr_ = NULL)                                                        \
    for (const char *path_var = _Generic(&_path,                              \
             Fs_Path *: path_var##_render.cstr.data,                          \
             Fs_Path_Slice *: path_var##_render.cstr.data,                    \
             Fs_Path_Render *: ((Fs_Path_Render *)(&_path))->cstr.data,       \
             String_View *: ((String_View *)(&_path))->data,                  \
             default: _Generic(_path,                                         \
                 char *: (*((const char **)(&_path))),                        \
                 const char *: (*((const char **)(&_path))),                  \
                 default: (UNREACHABLE("unsuported type"), NULL))),           \
                    **_castc_ = &path_var;                                    \
         _castc_; _castc_ = NULL)
#define fs_path_cast_wrap(Return_Type, result_var, casts, wrapped) ({ \
  Return_Type result_var = {0};                                       \
  casts { wrapped; }                                                  \
  result_var;                                                         \
})
#define fs_path_cast_wrap_lvalue(Return_Type, casts, wrapped) fs_path_cast_wrap(Return_Type, _result_, casts, _result_ = wrapped)
#define fs_path_cast_wrap_path(casts, wrapped) fs_path_cast_wrap(Fs_Path, _result_, casts, fs_path_assign(&_result_, wrapped))
#define fs_path_cast_wrap_path_render(casts, wrapped) fs_path_cast_wrap(Fs_Path_Render, _result_, casts, fs_path_render_assign(&_result_, wrapped))
#define fs_path_cast_wrap_void(casts, wrapped) ({ \
  casts { wrapped; }                              \
  (void)0;                                        \
})

Fs_Path fs_path_parse(String_View str);
Fs_Path fs__path_copy(Fs_Path_Slice path);
#define fs_path_copy(path) fs_path_cast_wrap_path(fs_path_with_cast_slice((path), slice), fs__path_copy(slice))
static inline void fs_path_assign(Fs_Path *dst, Fs_Path src) {
  da_const_assign(dst, src);
}

void fs_path_append_file(Fs_Path *path, String_View file);
void fs_path_append_many_n(Fs_Path *path, String_View *files, size_t n);
#define fs_path_append_many(path, ...) IF_VA_OPT(                                                                            \
    fs_path_append_many_n((path), (String_View[]){__VA_ARGS__}, sizeof((String_View[]){__VA_ARGS__}) / sizeof(String_View)), \
    NOOP(),                                                                                                                  \
    __VA_ARGS__)
void fs_path_append_path_slice(Fs_Path *path, Fs_Path_Slice other);
#define fs_path_append_path(path, other) fs_path_cast_wrap_void(fs_path_with_cast_slice((other), other_slice), fs_path_append_path_slice((path), other_slice))
void fs_path_append_root(Fs_Path *path);
Fs_Path fs__path_join(Fs_Path_Slice base, Fs_Path_Slice other);
#define fs_path_join(base, other) fs_path_cast_wrap_path(                                      \
    fs_path_with_cast_slice((base), base_slice) fs_path_with_cast_slice((other), other_slice), \
    fs__path_join(base_slice, other_slice))
Fs_Path_Render fs__path_render_join(Fs_Path_Slice base, Fs_Path_Slice other);
#define fs_path_render_join(base, other) fs_path_cast_wrap_path(                               \
    fs_path_with_cast_slice((base), base_slice) fs_path_with_cast_slice((other), other_slice), \
    fs__path_render_join(base_slice, other_slice))

void fs_path_normalize(Fs_Path *path);

#define fs_path_is_absolute(path) ({     \
  typeof(path) _fsia_ = (path);          \
  _fsia_.count && !_fsia_.data[0].count; \
})
#define fs_path_is_relative(path) ({     \
  typeof(path) _fsir_ = (path);          \
  !_fsir_.count || _fsir_.data[0].count; \
})
#define fs_path_is_root(path) ({              \
  typeof(path) _fsir_ = (path);               \
  _fsir_.count == 1 && !_fsir_.data[0].count; \
})
String_View fs_path_file_name(Fs_Path path);
String_View fs_path_file_ext(Fs_Path path);
Fs_Path fs_path_dir_name(Fs_Path path);

size_t sb_append_fs_path(String_Builder *sb, Fs_Path path);
Fs_Path_Render fs__path_render(Fs_Path_Slice path);
#define fs_path_render(path) fs_path_cast_wrap_path_render(fs_path_with_cast_slice((path), slice), fs__path_render(slice))
static inline void fs_path_render_assign(Fs_Path_Render *dst, Fs_Path_Render src) {
  da_const_assign(dst, src);
}
Fs_Path_Render fs_path_render_copy(Fs_Path_Render path);
Fs_Path_Render fs_path_render_parse(String_View str);

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

Fs_Path fs_path_parse(String_View str) {
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

Fs_Path fs__path_copy(Fs_Path_Slice path) {
  Fs_Path result = {0};
  da_append_many_n(&result, path.data, path.count);
  return result;
}

void fs_path_append_file(Fs_Path *path, String_View file) {
  static String_View same_dir = sv_from_strlit(".");
  static String_View top_dir = sv_from_strlit("..");
  if (!file.count) return;
  if (sv_eq(file, same_dir)) return;
  if (sv_eq(file, top_dir) && path->count) {
    if (fs_path_is_root(*path)) return;
    if (!sv_eq(path->data[path->count - 1], top_dir)) {
      path->count -= 1;
      return;
    }
  }
  da_append(path, file);
}

void fs_path_append_many_n(Fs_Path *path, String_View *files, size_t n) {
  for (size_t index = 0; index < n; index += 1) {
    fs_path_append(path, files[index]);
  }
}

void fs_path_append_path_slice(Fs_Path *path, Fs_Path_Slice other) {
  if (fs_path_is_absolute(other)) {
    path->count = 0;
    da_append(path, other.data[0]);
  }
  fs_path_append_many_n(path, other.data, other.count);
}

void fs_path_append_root(Fs_Path *path) {
  ASSERT(!path->count, "Can't append root to non empty path");
  da_append(path, sv_from_parts(NULL, 0));
}

Fs_Path fs__path_join(Fs_Path_Slice base, Fs_Path_Slice other) {
  Fs_Path result = {0};
  if (!fs_path_is_absolute(other)) fs_path_append_path_slice(&result, base);
  fs_path_append_path_slice(&result, other);
  da_trim_realloc(&result);
  return result;
}

Fs_Path_Render fs__path_render_join(Fs_Path_Slice base, Fs_Path_Slice other) {
  Fs_Path path = fs__path_join(base, other);
  Fs_Path_Render result = fs_path_render(path);
  fs_path_free(&path);
  return result;
}

void fs_path_normalize(Fs_Path *path) {
  Fs_Path result = fs__path_copy(fs_path_slice(*path));
  path->count = 0;
  fs_path_append_path_slice(path, result);
  fs_path_free(&result);
}

String_View fs_path_file_name(Fs_Path path) {
  if (!path.count || fs_path_is_root(path)) return sv_from_parts(NULL, 0);
  return path.data[path.count - 1];
}

String_View fs_path_file_ext(Fs_Path path) {
  String_View file_name = fs_path_file_name(path);
  if (!file_name.count) return sv_from_parts(NULL, 0);
  String_View ext = sv_chop_right_by_char_delim(&file_name, '.');
  if (ext.data == NULL) return sv_from_parts(NULL, 0);
  return ext;
}

Fs_Path fs_path_dir_name(Fs_Path path) {
  if (!path.count) return (Fs_Path){0};
  if (fs_path_is_root(path)) {
    Fs_Path result = {0};
    fs_path_append_root(&result);
    return result;
  }
  Fs_Path result = fs_path_copy(path);
  result.count -= 1;
  return result;
}

size_t sb_append_fs_path(String_Builder *sb, Fs_Path path) {
  Stringify_State state = {.sb = sb};
  if (!path.count) {
    stringify_append(&state, sb_append, '.');
    return state.count;
  }
  if (fs_path_is_root(path)) {
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

Fs_Path_Render fs__path_render(Fs_Path_Slice path) {
  String_Builder sb = {0};
  sb_append_fs_path(&sb, path);
  size_t cstr_count = sb.count;
  void *data = malloc(path.count * sizeof(String_View) + (cstr_count + 1) * sizeof(char));
  ASSERT(data, "Out of memory");
  String_View *sv_data = (String_View *)data;
  char *cstr = (char *)(sv_data + path.count);
  memcpy(cstr, sb.data, cstr_count);
  cstr[cstr_count] = '\0';
  for (size_t index = 0, offset = 0; index < path.count; index += 1) {
    size_t part_count = path.data[index].count;
    sv_data[index] = sv_from_parts(cstr + offset, part_count);
    offset += part_count + 1;
  }
  sb_free(&sb);
  return (Fs_Path_Render){.data = data, .cstr = {.data = cstr, .count = cstr_count}, .path = {.data = sv_data, .count = path.count}};
}

Fs_Path_Render fs_path_render_copy(Fs_Path_Render path) {
  void *data = malloc(path.path.count * sizeof(String_View) + (path.cstr.count + 1) * sizeof(char));
  ASSERT(data, "Out of memory");
  String_View *sv_data = (String_View *)data;
  memcpy(sv_data, path.path.data, sizeof(String_View) * path.path.count);
  char *cstr = (char *)(sv_data + path.path.count);
  memcpy(cstr, path.cstr.data, sizeof(char) * (path.cstr.count + 1));
  return (Fs_Path_Render){.data = data, .cstr = {.data = cstr, .count = path.cstr.count}, .path = {.data = sv_data, .count = path.path.count}};
}

Fs_Path_Render fs_path_render_parse(String_View str) {
  Fs_Path path = fs_path_parse(str);
  Fs_Path_Render render = fs_path_render(path);
  fs_path_free(&path);
  return render;
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
