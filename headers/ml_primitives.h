// This project is licensed under the GNU Affero General Public License
#pragma once
#include "../mila.h"
#include "ml_threading.h"

extern MethodTable *file_meta;
extern MethodTable *dict_meta;
extern MethodTable *list_meta;
extern MethodTable *array_meta;
extern MethodTable *range_meta;
extern MethodTable *istring_meta;

typedef struct
{
    long start;
    long end;
    long step;
} Range;

typedef struct {
  Value** array;
  size_t index;
  size_t end;
} ArrayIterState;

typedef struct
{
    Value **array; /* array of Value* */
    int size;
} Array;

Value *list_to_iter(Value *self);
Value *list_repr(Value *self);
Value *list_str(Value *self);
Value *native_list_new(Env *e, int argc, Value **argv);
Value *native_list_index(Env *e, int argc, Value **argv);
Value *native_list_set(Env *e, int argc, Value **argv);
Value *native_list_get(Env *e, int argc, Value **argv);
Value *set_list(Value *self, Value *index, Value *value);
Value *get_list(Value *self, Value *index);
Value *native_list_len(Env *e, int argc, Value **argv);
Value *native_list_pop(Env *e, int argc, Value **argv);
Value *list_free(Value *self);
Value *native_new_dict(Env *env, int argc, Value **argv);
Value* native_list_append(Env*, int, Value**);
Value *native_keys_dict(Env* env, int argc, Value** argv);
Value *native_set_dict(Env *env, int argc, Value **argv);
Value *native_get_dict(Env *env, int argc, Value **argv);
Value *set_dict(Value *self, Value *name, Value *val);
Value *get_dict(Value *self, Value *name);
Value *native_rem_dict(Env *env, int argc, Value **argv);
Value *free_dict(Value *self);
Value *array_to_str(Value *self);
Value *array_to_repr(Value *self);
Value *array_to_iter(Value *self);
ArrayIterState* array_iter_init(Value* self);
Value* array_iter_next(ArrayIterState* state);
void array_iter_cleanup(ArrayIterState* state);
size_t range_len(long start, long stop, long step);
Value *range_to_iter(Value *self);
Value* range_genth_worker(CGenData* cgen_data);
Value *range_to_gen(Value *self);
Value *range_to_str(Value *self);
Value *range_free(Value *self);
Value *native_range(Env *env, int argc, Value **argv);
Value *native_new_array(Env *env, int argc, Value **argv);
Value *native_from_array(Env *env, int argc, Value **argv);
Value *native_set_array(Env *env, int argc, Value **argv);
Value *native_get_array(Env *env, int argc, Value **argv);
Value *native_len_array(Env *env, int argc, Value **argv);
Value *get_array(Value *self, Value *index);
Value *set_array(Value *self, Value *index, Value *val);
Value *free_array(Value *self);
Value *native_str_pop_start(Env *env, int argc, Value **argv);
Value *native_str_pop_end(Env *env, int argc, Value **argv);
Value *native_ascii_from_int(Env *env, int argc, Value **argv);
Value *native_ascii_from_string(Env *env, int argc, Value **argv);
Value *native_str_slice(Env *env, int argc, Value **argv);
Value *native_str_copy(Env *env, int argc, Value **argv);
Value *native_str_index(Env *env, int argc, Value **argv);
Value *native_str_patch(Env *env, int argc, Value **argv);
Value *native_str_len(Env *env, int argc, Value **argv);
Value* native_str_split(Env* env, int argc, Value** argv);
Value* native_str_join(Env* env, int argc, Value** argv);
Value* native_str_startsw(Env* env, int argc, Value** argv);
Value* native_str_endsw(Env* env, int argc, Value** argv);
Value* native_str_contains(Env* env, int argc, Value** argv);
Value* native_str_contains_caseless(Env* env, int argc, Value** argv);
Value* native_str_find(Env* env, int argc, Value** argv);
Value* native_str_caseless_find(Env* env, int argc, Value** argv);