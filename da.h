#ifndef DA_H
#define DA_H

#include <assert.h>
#include <defines.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef DA_INIT_CAP
#  define DA_INIT_CAP 256
#endif

#define Da(Value_Type, ...)         \
  struct __VA_ARGS__ {              \
    typeof(Value_Type) *const data; \
    size_t count;                   \
    size_t capacity;                \
  }

#define Da_Slice(Value_Type, ...) \
  struct __VA_ARGS__ {            \
    typeof(Value_Type) *data;     \
    size_t count;                 \
  }

#define Da_Const(Value_Type, ...)         \
  struct __VA_ARGS__ {                    \
    const typeof(Value_Type) *const data; \
    const size_t count;                   \
  }

#define da_any_first(da) ({     \
  typeof((da)) *_da_f_ = &(da); \
  assert(_da_f_->count);        \
  _da_f_->data[0];              \
})

#define da_any_last(da) ({         \
  typeof((da)) *_da_l_ = &(da);    \
  assert(_da_l_->count);           \
  _da_l_->data[_da_l_->count - 1]; \
})

#define da_any_at(da, index) ({                     \
  typeof((da)) *_da_a_ = &(da);                     \
  size_t _index_ = (index);                         \
  assert(_da_a_->count && _da_a_->count > _index_); \
  _da_a_->data[_index_];                            \
})

#define da_any_foreach(da, id)                                                         \
  for (typeof((da)) *_da_##id = &(da); _da_##id; _da_##id = NULL)                      \
    for (size_t id##_index = 0, count = _da_##id->count; !id##_index; id##_index += 1) \
      for (typeof(*_da_##id->data) *id = _da_##id->data; id##_index < count; id += 1, id##_index += 1)

#define da_any_find_macro(da, id, predicate) ({            \
  typeof((da)) *_da_fm_ = &(da);                           \
  const typeof(*_da_fm_->data) *id##_data = _da_fm_->data; \
  UNUSED(id##_data);                                       \
  size_t id##_count = _da_fm_->count;                      \
  UNUSED(id##_count);                                      \
  const typeof(*_da_fm_->data) *id;                        \
  UNUSED(id);                                              \
  size_t id##_index = 0;                                   \
  while (id##_index < id##_count) {                        \
    id = &id##_data[id##_index];                           \
    if (!(predicate)) break;                               \
    id##_index += 1;                                       \
  }                                                        \
  id##_index;                                              \
})

#define da_any_find_right_macro(da, id, predicate) ({        \
  typeof((da)) *_da_fmr_ = &(da);                            \
  const typeof(*_da_fmr_->data) *id##_data = _da_fmr_->data; \
  UNUSED(id##_data);                                         \
  size_t id##_count = _da_fmr_->count;                       \
  UNUSED(id##_count);                                        \
  const typeof(*_da_fmr_->data) *id;                         \
  UNUSED(id);                                                \
  ssize_t id##_index = id##_count;                           \
  while (id##_index > 0) {                                   \
    id = &id##_data[id##_index - 1];                         \
    if (!(predicate)) break;                                 \
    id##_index -= 1;                                         \
  }                                                          \
  id##_index;                                                \
})

#define da__reassign(da) ({                    \
  typeof(*(da)) *_da_dnc_ = (da);              \
  (typeof(*_da_dnc_->data) **)&_da_dnc_->data; \
})

#define da_free(da) ({            \
  typeof(*(da)) *_da_f_ = (da);   \
  if (_da_f_->data) {             \
    free(_da_f_->data);           \
    *da__reassign(_da_f_) = NULL; \
    _da_f_->capacity = 0;         \
    _da_f_->count = 0;            \
  }                               \
})

#define da_reserve(da, target_capacity) ({                                                                         \
  typeof(*(da)) *_da_r_ = (da);                                                                                    \
  if ((target_capacity) > _da_r_->capacity) {                                                                      \
    if (_da_r_->capacity == 0) _da_r_->capacity = DA_INIT_CAP;                                                     \
    while ((target_capacity) > _da_r_->capacity) _da_r_->capacity *= 2;                                            \
    *da__reassign(_da_r_) = (typeof(_da_r_->data))realloc(_da_r_->data, _da_r_->capacity * sizeof(*_da_r_->data)); \
    assert(_da_r_->data != NULL && "Failed to reallocate dynamic array");                                          \
  }                                                                                                                \
  _da_r_->capacity;                                                                                                \
})

#define da_reserve_exact(da, target_capacity) ({                                                                        \
  typeof(*(da)) *_da_re_ = (da);                                                                                        \
  if ((target_capacity) > _da_re_->capacity) {                                                                          \
    _da_re_->capacity = (target_capacity);                                                                              \
    *da__reassign(_da_re_) = (typeof(_da_re_->data))realloc(_da_re_->data, (target_capacity) * sizeof(*_da_re_->data)); \
    assert(_da_re_->data != NULL && "Failed to reallocate dynamic array");                                              \
  }                                                                                                                     \
  _da_re_->capacity;                                                                                                    \
})

#define da_trim_realloc(da) ({                                                                                         \
  typeof(*(da)) *_da_tr_ = (da);                                                                                       \
  if (_da_tr_->capacity != _da_tr_->count) {                                                                           \
    if (_da_tr_->count == 0) {                                                                                         \
      da_free(_da_tr_);                                                                                                \
    } else {                                                                                                           \
      _da_tr_->capacity = _da_tr_->count;                                                                              \
      *da__reassign(_da_tr_) = (typeof(_da_tr_->data))realloc(_da_tr_->data, _da_tr_->count * sizeof(*_da_tr_->data)); \
      assert(_da_tr_->data != NULL && "Failed to reallocate dynamic array");                                           \
    }                                                                                                                  \
  }                                                                                                                    \
  _da_tr_->capacity;                                                                                                   \
})

#define da_resize(da, new_size) ({ \
  typeof(*(da)) *_da_r_ = (da);    \
  da_reserve(_da_r_, (new_size));  \
  _da_r_->count = (new_size);      \
})

#define da_append(da, item) ({           \
  typeof(*(da)) *_da_a_ = (da);          \
  da_reserve(_da_a_, _da_a_->count + 1); \
  _da_a_->data[_da_a_->count] = (item);  \
  _da_a_->count += 1;                    \
  1;                                     \
})

#define da_append_many_n(da, new_items, new_count) ({                                           \
  typeof(*(da)) *_da_amn_ = (da);                                                               \
  size_t _new_count_ = (new_count);                                                             \
  da_reserve(_da_amn_, _da_amn_->count + _new_count_);                                          \
  memcpy(_da_amn_->data + _da_amn_->count, (new_items), _new_count_ * sizeof(*_da_amn_->data)); \
  _da_amn_->count += _new_count_;                                                               \
  _new_count_;                                                                                  \
})
#define da_append_many(da, ...) ({                              \
  typeof(*(da)) *_da_am_ = (da);                                \
  size_t _added_ = 0;                                           \
  __VA_OPT__(                                                   \
      typeof(*_da_am_->data) items[] = {__VA_ARGS__};           \
      _added_ = sizeof(items) / sizeof(typeof(*_da_am_->data)); \
      da_append_many_n(_da_am_, items, _added_);)               \
  _added_;                                                      \
})

#define da_pop(da) ({                                \
  typeof(*(da)) *_da_p_ = (da);                      \
  typeof(*_da_p_->data) last = da_any_last(*_da_p_); \
  _da_p_->count -= 1;                                \
  last;                                              \
})

#define da_remove_unordered(da, index) ({                 \
  typeof(*(da)) *_da_ru_ = (da);                          \
  assert(index < _da_ru_->count);                         \
  _da_ru_->data[index] = _da_ru_->data[--_da_ru_->count]; \
  _da_ru_->count;                                         \
})

#define da_slice_init(da, ...) {.data = (da).data + WITH_DEFAULT(0, __VA_ARGS__), .count = WITH_DEFAULT(((da).count - WITH_DEFAULT(0, __VA_ARGS__)), SECOND_ARG(__VA_ARGS__, ))}

#define da_slice(da, Slice_Type, ...) ({                                                                       \
  typeof(da) _da_s_ = (da);                                                                                    \
  size_t start = WITH_DEFAULT(0, __VA_ARGS__);                                                                 \
  assert(_da_s_.count >= start);                                                                               \
  size_t slice_count = EXPAND_MACRO(WITH_DEFAULT, _da_s_.count - start __VA_OPT__(, ) REST_ARGS(__VA_ARGS__)); \
  assert(_da_s_.count >= start + slice_count);                                                                 \
  typeof_unqual(*((Slice_Type){0}).data) *_data_ = _da_s_.data;                                                \
  UNUSED(_data_);                                                                                              \
  (Slice_Type){.data = (typeof(((Slice_Type){0}).data))_da_s_.data + start, .count = slice_count};             \
})

#define da_slice_whole(da, Slice_Type) ({                     \
  typeof((da)) _da_sw_ = (da);                                \
  (Slice_Type){.data = _da_sw_.data, .count = _da_sw_.count}; \
})

#define da_slice_shift(das) ({                           \
  typeof(*(das)) *_das_s_ = (das);                       \
  typeof(*_das_s_->data) first = da_any_first(*_das_s_); \
  _das_s_->data += 1;                                    \
  _das_s_->count -= 1;                                   \
  first;                                                 \
})

#define da_slice_chop_left(das, ...) ({                                  \
  typeof(*(das)) *_das_cl_ = (das);                                      \
  size_t _count_ = WITH_DEFAULT(1, __VA_ARGS__);                         \
  if (_count_ > _das_cl_->count) _count_ = _das_cl_->count;              \
  typeof(*_das_cl_) result = {.data = _das_cl_->data, .count = _count_}; \
  _das_cl_->data += _count_;                                             \
  _das_cl_->count -= _count_;                                            \
  result;                                                                \
})

#define da_slice_chop_right(das, ...) ({                                                               \
  typeof(*(das)) *_das_cl_ = (das);                                                                    \
  size_t _count_ = WITH_DEFAULT(1, __VA_ARGS__);                                                       \
  if (_count_ > _das_cl_->count) _count_ = _das_cl_->count;                                            \
  typeof(*_das_cl_) result = {.data = _das_cl_->data + (_das_cl_->count - _count_), .count = _count_}; \
  _das_cl_->count -= _count_;                                                                          \
  result;                                                                                              \
})

#define da_slice_chop_while_macro(das, id, predicate) ({           \
  typeof(*(das)) *_das_cwm_ = (das);                               \
  size_t _index_ = da_any_find_macro(*_das_cwm_, id, (predicate)); \
  da_slice_chop_left(_das_cwm_, _index_);                          \
})

#define da_slice_chop_right_while_macro(das, id, predicate) ({            \
  typeof(*(das)) *_das_crwm_ = (das);                                     \
  size_t _index_ = da_any_find_right_macro(*_das_crwm_, id, (predicate)); \
  da_slice_chop_right(_das_crwm_, _das_crwm_->count - _index_);           \
})

#define da_slice_chop_by_macro(das, id, predicate) ({               \
  typeof(*(das)) *_das_cbm_ = (das);                                \
  size_t _index_ = da_any_find_macro(*_das_cbm_, id, !(predicate)); \
  typeof(*(das)) _head_;                                            \
  if (_index_ >= _das_cbm_->count) {                                \
    _head_.data = _das_cbm_->data;                                  \
    _head_.count = 0;                                               \
    _das_cbm_->data += _das_cbm_->count;                            \
    _das_cbm_->count = 0;                                           \
  } else {                                                          \
    _head_ = da_slice_chop_left(_das_cbm_, _index_ + 1);            \
  }                                                                 \
  _head_;                                                           \
})

#define da_slice_chop_right_by_macro(das, id, predicate) ({                    \
  typeof(*(das)) *_das_crbm_ = (das);                                          \
  size_t _index_ = da_any_find_right_macro(*_das_crbm_, id, !(predicate));     \
  typeof(*(das)) _tail_;                                                       \
  if (_index_ == 0) {                                                          \
    _tail_.data = _das_crbm_->data;                                            \
    _tail_.count = 0;                                                          \
    _das_crbm_->data += _das_crbm_->count;                                     \
    _das_crbm_->count = 0;                                                     \
  } else {                                                                     \
    _tail_ = da_slice_chop_right(_das_crbm_, _das_crbm_->count - _index_ + 1); \
  }                                                                            \
  _tail_;                                                                      \
})

#define da_const_init_from_arraylit(Value_Type, ...) EXPAND2(IF_VA_OPT(                                         \
    ({.data = (Value_Type[]){__VA_ARGS__}, .count = sizeof((Value_Type[]){__VA_ARGS__}) / sizeof(Value_Type)}), \
    ({.data = NULL, .count = 0}), __VA_ARGS__))

#endif // DA_H
