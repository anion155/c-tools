#ifndef FS_H
#define FS_H

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fs_path.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(__linux__)
#elif defined(_WIN32)
#elif defined(__APPLE__)
#  include <mach-o/dyld.h>
#endif

int fs__file_exists(const char *path);
#define fs_file_exists(path) fs_path_cast_wrap_lvalue(int, fs_path_with_cast_cstr((path), cstr), fs__file_exists(cstr))

typedef enum Fs_File_Kind {
  FS_FILE_KIND_ERROR = -1,
  FS_FILE_KIND_REGULAR = 0,
  FS_FILE_KIND_DIRECTORY,
  FS_FILE_KIND_SYMLINK,
  FS_FILE_KIND_OTHER,
} Fs_File_Kind;
Fs_File_Kind fs__file_kind(const char *path);
#define fs_file_kind(path) fs_path_cast_wrap_lvalue(Fs_File_Kind, fs_path_with_cast_cstr((path), cstr), fs__file_kind(cstr))

#define fs__open(path, mode) fs_path_cast_wrap_lvalue(FILE *, fs_path_with_cast_cstr((path), cstr), fopen(cstr, (mode)))

ssize_t fs__read_unknown_size(String_Builder *sb, FILE *f, size_t buffer_size);
#define fs_read_unknown_size(sb, f, ...) fs__read_unknown_size((sb), (f), WITH_DEFAULT(64 * 1024, __VA_ARGS__))
ssize_t fs_read_seek_size(String_Builder *sb, FILE *f);
#define fs_read_from_file(sb, path) fs_read_seek_size((sb), fs__open(path, "rb"))
#define fs_read_from_stdin(sb) fs_read_unknown_size((sb), stdin)

ssize_t fs__write(Fs_Path_Render path, String_View sv, FILE *f);
#define fs__write_to_file(path, sv, mode) fs__write((path), (sv), fs__open(path, mode))
#define fs_write_to_file(path, sv) fs__write_to_file((path), (sv), "wb")
#define fs_append_to_file(path, sv) fs__write_to_file((path), (sv), "ab")

Fs_Path_Render fs__read_link(const char *path);
#define fs_read_link(path) fs_path_cast_wrap_path_render(fs_path_with_cast_cstr((path), cstr), fs__read_link(cstr))
Fs_Path_Render fs__real_path(const char *path);
#define fs_real_path(path) fs_path_cast_wrap_path_render(fs_path_with_cast_cstr((path), cstr), fs__real_path(cstr))

int fs__rename_file(const char *old_path, const char *new_path);
#define fs_rename_file(old_path, new_path)                                                                            \
  fs_path_cast_wrap_lvalue(int,                                                                                       \
                           fs_path_with_cast_cstr((old_path), old_cstr) fs_path_with_cast_cstr((new_path), new_cstr), \
                           fs__rename_file(old_cstr, new_cstr))
int fs__copy_file(const char *src_path, const char *dst_path);
#define fs_copy_file(src_path, dst_path)                                                                              \
  fs_path_cast_wrap_lvalue(int,                                                                                       \
                           fs_path_with_cast_cstr((src_path), src_cstr) fs_path_with_cast_cstr((dst_path), dst_cstr), \
                           fs__copy_file(src_cstr, dst_cstr))
int fs__delete_file(const char *path);
#define fs_delete_file(path) fs_path_cast_wrap_lvalue(int, fs_path_with_cast_cstr((path), cstr), fs__delete_file(cstr))

Fs_Path_Render fs_get_cwd();
int fs__set_cwd(const char *path);
#define fs_set_cwd(path) fs_path_cast_wrap_lvalue(int, fs_path_with_cast_cstr((path), cstr), fs__set_cwd(cstr))

Fs_Path_Render fs_executable_path();

int fs__mkdir(const char *path);
#define fs_mkdir(path) fs_path_cast_wrap_lvalue(int, fs_path_with_cast_cstr((path), cstr), fs__mkdir(cstr))
int fs__mkdir_recursive(Fs_Path_Render path);
#define fs_mkdir_recursive(path) fs_path_cast_wrap_lvalue(int, fs_path_with_cast_render((path), render), fs__mkdir_recursive(render))

typedef struct Fs_Dir_Iterator {
  Fs_Path_Render dir_path;
  DIR *dir;
  struct dirent *dirent;
  String_View dir_name;
  Fs_Path_Render absolute_path;
  Fs_File_Kind kind;
} Fs_Dir_Iterator;
bool fs__dir_iterator_open(Fs_Path_Render path, Fs_Dir_Iterator *it);
#define fs_dir_iterator_open(path, it) fs_path_cast_wrap_lvalue(int, fs_path_with_cast_render((path), render), fs__dir_iterator_open(render, (it)))
bool fs_dir_iterator_next(Fs_Dir_Iterator *it);
bool fs_dir_iterator_close(Fs_Dir_Iterator *it);

typedef struct Fs_Dir_Entry {
  String dir_name;
  Fs_Path_Render path;
  Fs_File_Kind kind;
} Fs_Dir_Entry;
typedef Da(Fs_Dir_Entry, Fs_Dir_Entries) Fs_Dir_Entries;
#define fs_dir_entries_free(entries) ({  \
  Fs_Dir_Entries *_entries_ = (entries); \
  da_foreach(*_entries_, entry) {        \
    string_free(&entry->dir_name);       \
    fs_path_render_free(&entry->path);   \
  }                                      \
  da_free(_entries_);                    \
})
ssize_t fs__read_dir(Fs_Path_Render path, Fs_Dir_Entries *children);
#define fs_read_dir(path, children) fs_path_cast_wrap_lvalue(ssize_t, fs_path_with_cast_render((path), render), fs__read_dir(render, (children)))

typedef struct Fs_Walk_Dir_Entry {
  Fs_Path_Render absolute_path;
  Fs_Path_Slice relative_path;
  Fs_File_Kind kind;
  void *data;
} Fs_Walk_Dir_Entry;
typedef enum Fs_Walk_Kind {
  FS_WALK_KIND_CONTINUE,
  FS_WALK_KIND_STOP,
  FS_WALK_KIND_SKIP,
  FS_WALK_KIND_ERROR,
} Fs_Walk_Kind;
typedef Fs_Walk_Kind (*Fs_Walk_Dir_Pred)(Fs_Walk_Dir_Entry entry);
typedef struct Fs_Walk_Dir_Opt {
  Fs_Walk_Dir_Pred predicate;
  void *data;
  bool post_order;
  Fs_Path_Slice relative_path;
} Fs_Walk_Dir_Opt;
Fs_Walk_Kind fs__walk_dir_opt(Fs_Path_Render path, Fs_File_Kind kind, Fs_Walk_Dir_Opt opt);
#define fs_walk_dir(path, _predicate, ...)                           \
  fs_path_cast_wrap_lvalue(bool,                                     \
                           fs_path_with_cast_render((path), render), \
                           fs__walk_dir_opt(                         \
                               render,                               \
                               fs__file_kind(render.cstr.data),      \
                               (Fs_Walk_Dir_Opt){.predicate = _predicate __VA_OPT__(, ) __VA_ARGS__}) != FS_WALK_KIND_ERROR)

bool fs__copy_directory(Fs_Path_Render src_path, Fs_Path_Render dst_path);
#define fs_copy_directory(src_path, dst_path)                                                                                 \
  fs_path_cast_wrap_lvalue(bool,                                                                                              \
                           fs_path_with_cast_render((src_path), src_render) fs_path_with_cast_render((dst_path), dst_render), \
                           fs__copy_directory(src_render, dst_render))
bool fs__delete_directory(Fs_Path_Render path);
#define fs_delete_directory(path) fs_path_cast_wrap_lvalue(bool, fs_path_with_cast_render((path), render), fs__delete_directory(render, dst_render))

#endif // FS_H

#if defined(FS_IMPL) && !defined(FS_IMPL_C)
#define FS_IMPL_C

int fs__file_exists(const char *path) {
  return access(path, F_OK) == 0;
}

Fs_File_Kind fs__file_kind(const char *path) {
  struct stat statbuf;
  if (lstat(path, &statbuf) < 0) return FS_FILE_KIND_ERROR;
  if (S_ISREG(statbuf.st_mode)) return FS_FILE_KIND_REGULAR;
  if (S_ISDIR(statbuf.st_mode)) return FS_FILE_KIND_DIRECTORY;
  if (S_ISLNK(statbuf.st_mode)) return FS_FILE_KIND_SYMLINK;
  return FS_FILE_KIND_OTHER;
}

ssize_t fs__read_unknown_size(String_Builder *sb, FILE *f, size_t buffer_size) {
  if (f == NULL) return -1;
  char buffer[buffer_size];
  size_t start = sb->count;
  while (true) {
    size_t readed_count = fread(buffer, sizeof(char), buffer_size, f);
    if (!readed_count) break;
    da_append_many(sb, buffer, readed_count);
  }
  if (ferror(f)) return -2;
  return sb->count - start;
}

ssize_t fs_read_seek_size(String_Builder *sb, FILE *f) {
  ssize_t result = 0;
#define defer_return(value) ({ result = value; goto defer; })
  if (f == NULL) defer_return(-1);
  if (fseek(f, 0, SEEK_END) < 0) defer_return(-2);
  long count = ftell(f);
  if (count < 0) defer_return(-3);
  if (fseek(f, 0, SEEK_SET) < 0) defer_return(-2);
  da_reserve(sb, sb->count + count);
  result = fread(sb->data + sb->count, sizeof(char), count, f);
  if (result != count) defer_return(-4);
  sb->count += result;
  if (ferror(f)) defer_return(-5);
defer:
#undef defer_return
  if (f && fclose(f) && !result) return -10;
  return result;
}

ssize_t fs__write(Fs_Path_Render path, String_View sv, FILE *f) {
  ssize_t result = 0;
#define defer_return(value) ({ result = value; goto defer; })
  if (f == NULL) defer_return(-1);
  String_View it = sv;
  while (it.count > 0) {
    size_t count = fwrite(it.data, sizeof(char), it.count, f);
    if (ferror(f)) defer_return(-2);
    it.data += count;
    it.count -= count;
  }
defer:
#undef defer_return
  if (f && fclose(f) && !result) return -10;
  return result;
}

Fs_Path_Render fs__read_link(const char *path) {
  char buf[PATH_MAX + 1];
  ssize_t count = readlink(path, buf, PATH_MAX);
  if (count < 0) return fs_path_render_parse(sv_from_strlit(""));
  buf[count] = '\0';
  return fs_path_render_parse(sv_from_parts(buf, count));
}

Fs_Path_Render fs__real_path(const char *path) {
  char buf[PATH_MAX + 1];
  char *result = realpath(path, buf);
  if (!result) return fs_path_render_parse(sv_from_strlit(""));
  return fs_path_render_parse(sv_from_cstr(buf));
}

int fs__rename_file(const char *old_path, const char *new_path) {
  return rename(old_path, new_path);
}

int fs__copy_file(const char *src_path, const char *dst_path) {
  int result = 0;
#define defer_return(value) ({ result = value; goto defer; })
  int src_fd = -1;
  int dst_fd = -1;
  char *read_buf = NULL;
  src_fd = open(src_path, O_RDONLY);
  if (src_fd < 0) defer_return(-1);
  struct stat src_stat;
  if (fstat(src_fd, &src_stat) < 0) defer_return(-2);
  dst_fd = open(dst_path, O_CREAT | O_TRUNC | O_WRONLY, src_stat.st_mode);
  if (dst_fd < 0) defer_return(-3);
  size_t buf_size = src_stat.st_blksize * sizeof(char);
  read_buf = malloc(buf_size);
  if (!read_buf) defer_return(-4);
  while (true) {
    ssize_t readed_count = read(src_fd, read_buf, buf_size);
    if (readed_count == 0) break;
    if (readed_count < 0) defer_return(-5);
    char *write_buf = read_buf;
    while (readed_count > 0) {
      ssize_t written_count = write(dst_fd, write_buf, readed_count);
      if (written_count < 0) defer_return(-6);
      readed_count -= written_count;
      write_buf += written_count;
    }
  }
defer:
#undef defer_return
  if (read_buf) free(read_buf);
  if (src_fd >= 0 && close(src_fd) && !result) result = -10;
  if (dst_fd >= 0 && close(dst_fd) && !result) result = -11;
  return result;
}

int fs__delete_file(const char *path) {
  return remove(path);
}

Fs_Path_Render fs_get_cwd() {
  char buffer[PATH_MAX];
  getcwd(buffer, PATH_MAX);
  return fs_path_render_parse(sv_from_cstr(buffer));
}

int fs__set_cwd(const char *path) {
  return chdir(path);
}

Fs_Path_Render fs_executable_path() {
  char buf[PATH_MAX + 1];
#if defined(__linux__)
  ssize_t count = readlink("/proc/self/exe", buf, PATH_MAX);
  ASSERT(count > 0, "failed to get executable path");
  buf[count] = '\0';
#elif defined(_WIN32)
  DWORD count = GetModuleFileNameA(NULL, buf, PATH_MAX + 1);
  ASSERT(count > 0, "failed to get executable path");
#elif defined(__APPLE__)
  uint32_t size = PATH_MAX + 1;
  int result = _NSGetExecutablePath(buf, &size);
  ASSERT(result, "failed to get executable path");
#else
  TODO("fs_executable_path is not implemented for this platform\n");
#endif
  return fs_real_path(buf);
}

int fs__mkdir(const char *path) {
  if (mkdir(path, 0755) < 0) return -1;
  if (errno != EEXIST) return -2;
  struct stat statbuf;
  if (lstat(path, &statbuf) < 0) return -3;
  if (S_ISDIR(statbuf.st_mode)) return -4;
  return 0;
}

int fs__mkdir_recursive(Fs_Path_Render path) {
  for (size_t index = 0; index < path.path.count; index += 1) {
    String_View part = path.path.data[index];
    if (index == 0 && !part.count) continue;
    char value = part.data[part.count];
    part.data[part.count] = '\0';
    int result = fs__mkdir(path.cstr.data);
    part.data[part.count] = value;
    if (result < 0) return result;
  }
  return 0;
}

bool fs__dir_iterator_open(Fs_Path_Render path, Fs_Dir_Iterator *it) {
  memset(it, 0, sizeof(Fs_Dir_Iterator));
  Fs_Path_Render path_copy = fs_path_render_copy(path);
  memcpy(&it->dir_path, &path_copy, sizeof(Fs_Path_Render));
  it->dir = opendir(path.cstr.data);
  it->dirent = NULL;
  return it->dir != NULL;
}

bool fs_dir_iterator_next(Fs_Dir_Iterator *it) {
  if (!it->dir) return false;
  errno = 0;
  it->dirent = readdir(it->dir);
  if (it->dirent == NULL) return false;
#if defined(__APPLE__) || defined(_DIRENT_HAVE_D_NAMLEN)
  String_View dir_name = sv_from_parts(it->dirent->d_name, it->dirent->d_namlen);
#else
  String_View dir_name = sv_from_cstr(it->dirent->d_name);
#endif
  if (sv_eq(dir_name, sv_from_strlit("."))) return fs_dir_iterator_next(it);
  if (sv_eq(dir_name, sv_from_strlit(".."))) return fs_dir_iterator_next(it);
  it->dir_name = dir_name;
  Fs_Path absolute_path = {0};
  fs_path_append_path(&absolute_path, it->dir_path.path);
  fs_path_append_file(&absolute_path, dir_name);
  Fs_Path_Render absolute_path_render = fs_path_render(absolute_path);
  fs_path_free(&absolute_path);
  fs_path_render_free(&it->absolute_path);
  memcpy(&it->absolute_path, &absolute_path_render, sizeof(Fs_Path_Render));
  switch (it->dirent->d_type) {
    case DT_REG: it->kind = FS_FILE_KIND_REGULAR; break;
    case DT_DIR: it->kind = FS_FILE_KIND_DIRECTORY; break;
    case DT_LNK: it->kind = FS_FILE_KIND_SYMLINK; break;
    case DT_UNKNOWN: it->kind = fs_file_kind(it->absolute_path); break;
    default: it->kind = FS_FILE_KIND_OTHER;
  }
  return true;
}

bool fs_dir_iterator_close(Fs_Dir_Iterator *it) {
  if (it->dir_path.data) fs_path_render_free(&it->dir_path);
  if (!it->dir) return false;
  bool result = true;
  if (closedir(it->dir) < 0) result = false;
  it->dir = NULL;
  it->dirent = NULL;
  fs_path_render_free(&it->absolute_path);
  memset(it, 0, sizeof(Fs_Dir_Iterator));
  return result;
}

ssize_t fs__read_dir(Fs_Path_Render path, Fs_Dir_Entries *children) {
  ssize_t result = 0;
#define defer_return(value) ({ result = value; goto defer; })
  Fs_Dir_Iterator it = {0};
  if (fs_dir_iterator_open(path, &it) < 0) defer_return(-1);
  while (fs_dir_iterator_next(&it)) {
    Fs_Dir_Entry entry = {
        .dir_name = string_from(sb_append_sv, it.dir_name),
        .path = fs_path_render_copy(it.absolute_path),
        .kind = it.kind};
    da_append(children, entry);
  }
  if (errno != 0) defer_return(-2);
defer:
#undef defer_return
  if (!fs_dir_iterator_close(&it) && !result) return -3;
  return result;
}

Fs_Walk_Kind fs__walk_dir_opt(Fs_Path_Render path, Fs_File_Kind kind, Fs_Walk_Dir_Opt opt) {
  if (kind == FS_FILE_KIND_ERROR) return FS_WALK_KIND_ERROR;
  if (!opt.post_order) {
    switch (opt.predicate((Fs_Walk_Dir_Entry){.absolute_path = path, .relative_path = opt.relative_path, .data = opt.data, .kind = kind})) {
      case FS_WALK_KIND_CONTINUE: break;
      case FS_WALK_KIND_STOP: return FS_WALK_KIND_STOP;
      case FS_WALK_KIND_SKIP: return FS_WALK_KIND_CONTINUE;
      case FS_WALK_KIND_ERROR: return FS_WALK_KIND_ERROR;
      default: break;
    }
  }
  if (kind == FS_FILE_KIND_DIRECTORY) {
    Fs_Dir_Entries children = {0};
    if (fs_read_dir(path, &children) < 0) {
      fs_dir_entries_free(&children);
      return FS_WALK_KIND_ERROR;
    }
    Fs_Path_Slice dir_relative_path = opt.relative_path;
    Fs_Path relative_path = fs_path_copy(dir_relative_path);
    da_foreach(children, child) {
      fs_path_append_file(&relative_path, sv_from_like(child->dir_name));
      opt.relative_path = fs_path_slice(relative_path);
      Fs_Walk_Kind walk = fs__walk_dir_opt(child->path, child->kind, opt);
      relative_path.count -= 1;
      switch (walk) {
        case FS_WALK_KIND_CONTINUE: break;
        case FS_WALK_KIND_STOP: {
          fs_path_free(&relative_path);
          fs_dir_entries_free(&children);
          return FS_WALK_KIND_STOP;
        } break;
        case FS_WALK_KIND_SKIP: break;
        case FS_WALK_KIND_ERROR: {
          fs_path_free(&relative_path);
          fs_dir_entries_free(&children);
          return FS_WALK_KIND_ERROR;
        } break;
        default: break;
      }
    }
    opt.relative_path = dir_relative_path;
    fs_path_free(&relative_path);
    fs_dir_entries_free(&children);
  }
  if (opt.post_order) {
    switch (opt.predicate((Fs_Walk_Dir_Entry){.absolute_path = path, .relative_path = opt.relative_path, .data = opt.data, .kind = kind})) {
      case FS_WALK_KIND_CONTINUE: break;
      case FS_WALK_KIND_STOP: return FS_WALK_KIND_STOP;
      case FS_WALK_KIND_SKIP: break;
      case FS_WALK_KIND_ERROR: return FS_WALK_KIND_ERROR;
      default: break;
    }
  }
  return FS_WALK_KIND_CONTINUE;
}

Fs_Walk_Kind fs__copy_directory_predicate(Fs_Walk_Dir_Entry entry) {
  switch (entry.kind) {
    case FS_FILE_KIND_ERROR: UNREACHABLE();
    case FS_FILE_KIND_REGULAR: break;
    case FS_FILE_KIND_DIRECTORY: break;
    case FS_FILE_KIND_SYMLINK: TODO("fs_copy_directory symlinks are not implemented yet");
    case FS_FILE_KIND_OTHER: TODO("fs_copy_directory non regular files are not implemented yet");
  }
  Fs_Path_Render *dst_base_path = entry.data;
  Fs_Path_Render dst_path = fs__path_render_join(dst_base_path->path, entry.relative_path);
  Fs_Walk_Kind result = FS_WALK_KIND_CONTINUE;
  if (entry.kind == FS_FILE_KIND_DIRECTORY) {
    if (fs_mkdir(dst_path) < 0) result = FS_WALK_KIND_ERROR;
  } else {
    if (fs_copy_file(entry.absolute_path, dst_path) < 0) result = FS_WALK_KIND_ERROR;
  }
  fs_path_render_free(&dst_path);
  return result;
}

bool fs__copy_directory(Fs_Path_Render src_path, Fs_Path_Render dst_path) {
  return fs_walk_dir(src_path, fs__copy_directory_predicate, .data = &dst_path);
}

Fs_Walk_Kind fs__delete_directory_predicate(Fs_Walk_Dir_Entry entry) {
  if (fs_delete_file(entry.absolute_path) < 0) return FS_WALK_KIND_ERROR;
  return FS_WALK_KIND_CONTINUE;
}

bool fs__delete_directory(Fs_Path_Render path) {
  return fs_walk_dir(path, fs__delete_directory_predicate, .post_order = true);
}

#endif // FS_IMPL_C

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
