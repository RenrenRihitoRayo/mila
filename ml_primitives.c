// This project is licensed under the GNU Affero General Public License
#pragma once

#include "mila.h"
#include "ml_ll.c"
#include "ml_dict.c"
#include <stdlib.h>
#include "ml_threading.c"

// Define meta tables

MethodTable *file_meta = NULL;
MethodTable *dict_meta = NULL;
MethodTable *list_meta = NULL;
MethodTable *array_meta = NULL;
MethodTable *range_meta = NULL;
MethodTable *istring_meta = NULL;

Value *list_to_iter(Value *self)
{
    Value **iter = ll_to_iter(self->v.opaque);
    return vopaque(iter);
}

Value *list_repr(Value *self)
{
    LinkedList *lst = (LinkedList *)self->v.opaque;
    if (lst->size > MAX_ITEMS_DISPLAYED)
        return vstring_fmt("list(%zu items)", lst->size);
    Value **iter = ll_to_iter(lst);

    char *buffer = mila_strdup("[");
    for (size_t i = 0; iter[i]; i++)
    {
        char *repr = as_c_string_repr(iter[i]);
        if (i < lst->size - 1)
            our_asprintf(&buffer, "%s, ", repr);
        else
            our_asprintf(&buffer, "%s", repr);
        val_release(iter[i]);
        mila_free(repr);
    }
    our_asprintf(&buffer, "]");
    mila_free(iter);
    return vstring_take(buffer);
}

Value *list_str(Value *self)
{
    LinkedList *lst = (LinkedList *)self->v.opaque;
    Value **iter = ll_to_iter(lst);

    char *buffer = mila_strdup("[");
    for (size_t i = 0; iter[i]; i++)
    {
        char *repr = as_c_string_repr(iter[i]);
        if (i < lst->size - 1)
            our_asprintf(&buffer, "%s, ", repr);
        else
            our_asprintf(&buffer, "%s", repr);
        val_release(iter[i]);
        mila_free(repr);
    }
    our_asprintf(&buffer, "]");
    mila_free(iter);
    return vstring_take(buffer);
}

Value *native_list_new(Env *e, int argc, Value **argv)
{
    (void)e;
    LinkedList *list = ll_create();
    for (int i = 0; i < argc; i++)
    {
        ll_append(list, val_retain(argv[i]));
    }
    Value *res = vopaque_extra(list, NULL, MILA_LPREFIX "list");
    val_set_table(res, list_meta);
    return res;
}

Value *native_list_index(Env *e, int argc, Value **argv)
{
    long cur = 0;
    LinkedList* l = (LinkedList*)GET_OPAQUE(argv[0]);
    for (LLNode *v = l->head; v; v = v->next)
    {
        Value* cond = binary_op(v->value, BMethodEq, argv[1]);
        if (is_truthy(cond))
        {
            val_release(cond);
            return vint(cur);
        }
        val_release(cond);
        cur++;
    }
    return vint(-1);
}

Value *native_list_set(Env *e, int argc, Value **argv)
{
    (void)e;
    (void)argc;
    ll_set(GET_OPAQUE(argv[0]), GET_INTEGER(argv[1]), val_retain(argv[2]));
    return NULL;
}

Value *native_list_get(Env *e, int argc, Value **argv)
{
    (void)e;
    (void)argc;
    return ll_get(GET_OPAQUE(argv[0]), GET_INTEGER(argv[1]));
}

Value *set_list(Value *self, Value *index, Value *value)
{
    ll_set(self->v.opaque, index->v.i, val_retain(value));
    return NULL;
}

Value *get_list(Value *self, Value *index)
{
    return ll_get(self->v.opaque, index->v.i);
}

Value *native_list_len(Env *e, int argc, Value **argv)
{
    (void)e;
    if (argc != 1)
        return verror("list.len(l): requires one argument!");
    LinkedList *ll = (LinkedList *)argv[0]->v.opaque;
    return vint(ll->size);
}

Value *native_list_pop(Env *e, int argc, Value **argv)
{
    if (argc == 1)
        return ll_pop(argv[0]->v.opaque, -1);
    else if (argc == 2)
        return ll_pop(argv[0]->v.opaque, argv[1]->v.i);
    return vtagged_error(E_RUNTIME,
                         "list.pop(l, index?): Missing list argument!");
}

Value *list_free(Value *self)
{
    Value **iter = ll_to_iter(self->v.opaque);

    for (int i = 0; iter[i]; i++)
    {
        val_release(iter[i]);
        val_release(iter[i]);
    }

    mila_free(iter);
    ll_free(self->v.opaque);
    self->type = T_NULL;
    self->v.opaque = NULL;
    return NULL;
}

Value *native_new_dict(Env *env, int argc, Value **argv)
{
    (void)env;
    if (((argc % 2) != 0) && argc != 0)
    {
        return verror(
            "dict(...): Provide even number of arguments or none at all! Got %i.",
            argc);
    }

    Dict *d = dict_create();

    for (int i = 0; i < argc;)
    {
        Value *key = argv[i++];
        Value *value = argv[i++];
        dict_set(d, key, value);
    }

    if (d)
    {
        Value *v = vopaque(d);
        val_set_table(v, dict_meta);
        v->type_name = mila_strdup(MILA_LPREFIX "dict");
        return v;
    }
    return verror("couldnt make a dict.");
}

Value *native_set_dict(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 3)
    {
        return verror("invalid number of arguments given or incorrect types.");
    }

    dict_set(argv[0]->v.opaque, argv[1], val_retain(argv[2]));
    return vnull();
}

Value *native_get_dict(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2)
    {
        return verror("invalid number of arguments given or incorrect types.");
    }

    Value *v = dict_get(argv[0]->v.opaque, argv[1]);
    return v ? v : vnull();
}

Value *set_dict(Value *self, Value *name, Value *val)
{
    dict_set(self->v.opaque, name, val);
    return NULL;
}

Value *get_dict(Value *self, Value *name)
{
    Value *v = dict_get(self->v.opaque, name);
    return v;
}

Value *native_rem_dict(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2)
    {
        return verror("invalid number of arguments given or incorrect types.");
    }

    dict_remove(argv[0]->v.opaque, argv[1]);
    return vnull();
}

Value *free_dict(Value *self)
{
    dict_free(self->v.opaque);
    return NULL;
}

/* Array Functions: store Value* per slot so we keep proper refcounts */
typedef struct
{
    Value **array; /* array of Value* */
    int size;
} Array;

Value *array_to_str(Value *self)
{
    char *buffer = NULL;
    if (!self || self->type != T_OPAQUE)
    {
        our_asprintf(&buffer, "<not-an-array>");
        return vstring_take(buffer);
    }

    Array *arr = (Array *)self->v.opaque;
    if (!arr)
    {
        our_asprintf(&buffer, "<null-array-data>");
        return vstring_take(buffer);
    }

    our_asprintf(&buffer, "array.from(");
    for (int i = 0; i < arr->size; i++)
    {
        Value *slot = arr->array[i];
        if (!slot)
        {
            our_asprintf(&buffer, "?null?");
        }
        else
        {
            char *s = as_c_string_repr(slot);
            our_asprintf(&buffer, "%s", s);
            mila_free(s);
        }
        if (i < arr->size - 1)
            our_asprintf(&buffer, ", ");
    }
    our_asprintf(&buffer, ")");
    return vstring_take(buffer);
}

Value *array_to_repr(Value *self)
{
    char *buffer = NULL;
    if (!self || self->type != T_OPAQUE)
    {
        our_asprintf(&buffer, "<not-an-array>");
        return vstring_take(buffer);
    }

    Array *arr = (Array *)self->v.opaque;
    if (!arr)
    {
        our_asprintf(&buffer, "<null-array-data>");
        return vstring_take(buffer);
    }

    if (arr->size > MAX_ITEMS_DISPLAYED)
        return vstring_fmt("array(%i items)", arr->size);

    our_asprintf(&buffer, "array.from(");
    for (int i = 0; i < arr->size; i++)
    {
        Value *slot = arr->array[i];
        if (!slot)
        {
            our_asprintf(&buffer, "?null?");
        }
        else
        {
            char *s = as_c_string_repr(slot);
            our_asprintf(&buffer, "%s", s);
            mila_free(s);
        }
        if (i < arr->size - 1)
            our_asprintf(&buffer, ", ");
    }
    our_asprintf(&buffer, ")");
    return vstring_take(buffer);
}

Value *array_to_iter(Value *self)
{
    Value *arrv = self;
    if (arrv->type != T_OPAQUE)
    {
        return verror("array<UMethodToIter>: first arg must be an array (opaque)");
    }

    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array<UMethodToIter>: null array data");
    }

    Value **values = (Value **)mila_malloc(sizeof(Value *) * (arr->size + 1));
    values[arr->size] = NULL;

    int i = 0;
    for (int t = 0; t < arr->size; ++t)
    {
        if (arr->array[t] == NULL)
            continue;
        values[i++] = val_retain(arr->array[t]);
    }

    return vopaque(values);
}

typedef struct
{
    long start;
    long end;
    long step;
} Range;

size_t range_len(long start, long stop, long step)
{
    if (step == 0)
        return 0;
    if (step > 0)
    {
        if (start >= stop)
            return 0;
        return (stop - start + step - 1) / step;
    }
    else
    {
        if (start <= stop)
            return 0;
        return (start - stop - step - 1) / (-step);
    }
}

Value *range_to_iter(Value *self)
{
    Range *data = (Range *)(self->v.opaque);
    Value **v = (Value **)mila_malloc(
        sizeof(Value *) * (range_len(data->start, data->end, data->step) + 1));
    long index = 0;
    for (long i = data->start; i < data->end; i += data->step)
    {
        v[index++] = vint(i);
    }
    v[index] = NULL;
    return vopaque(v);
}

Value* range_genth_worker(CGenData* cgen_data)
{
    ThreadContext* ctx = cgen_data->ctx;
    Range* data = GET_OPAQUE(cgen_data->data);
    for (long i = data->start; i < data->end; i += data->step)
    {
        thread_yield(ctx, vint(i));
    }
    return NULL;
}

Value *range_to_gen(Value *self)
{
    int th_id = make_cgen(range_genth_worker, self);
    return vint(th_id);
}

Value *range_to_str(Value *self)
{
    Range *data = (Range *)(self->v.opaque);
    return vstring_fmt("range(%zd, %zd, %zd)", data->start, data->end,
                       data->step);
}

Value *range_free(Value *self)
{
    mila_free(self->v.opaque);
    return NULL;
}

Value *native_range(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc == 1 && argv[0]->type == T_INT)
    {
        Range *r = (Range *)mila_malloc(sizeof(Range));
        r->start = 0;
        r->end = argv[0]->v.i;
        r->step = 1;
        Value *res = vopaque_extra(r, NULL, MILA_LPREFIX "range");
        val_set_table(res, range_meta);
        return res;
    }
    if (argc == 2 && argv[0]->type == T_INT && argv[1]->type == T_INT)
    {
        Range *r = (Range *)mila_malloc(sizeof(Range));
        r->start = argv[0]->v.i;
        r->end = argv[1]->v.i;
        r->step = 1;
        Value *res = vopaque_extra(r, NULL, MILA_LPREFIX "range");
        val_set_table(res, range_meta);
        return res;
    }
    if (argc == 3 && argv[0]->type == T_INT && argv[1]->type == T_INT &&
        argv[2]->type == T_INT)
    {
        Range *r = (Range *)mila_malloc(sizeof(Range));
        r->start = argv[0]->v.i;
        r->end = argv[1]->v.i;
        r->step = argv[2]->v.i;
        Value *res = vopaque_extra(r, NULL, MILA_LPREFIX "range");
        val_set_table(res, range_meta);
        return res;
    }
    return vnull();
}

Value *native_new_array(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
    {
        return verror("array(size): Requires one argument, array size (int)\n");
    }
    if (!match_types(argv, T_INT, T_ARG_END))
    {
        return verror("array(size): Expected the argument type int\n");
    }

    int size = (int)argv[0]->v.i;
    if (size < 0)
    {
        return verror("array(size): negative size\n");
    }

    Value *res = val_new(T_OPAQUE);
    Array *array = mila_malloc(sizeof(Array));
    array->size = size;
    array->array = mila_malloc(sizeof(Value *) * size);

    for (int i = 0; i < size; i++)
    {
        array->array[i] = NULL;
    }

    res->v.opaque = array;
    res->type_name = mila_strdup(MILA_LPREFIX "array");
    val_set_table(res, array_meta);
    return res;
}

Value *native_from_array(Env *env, int argc, Value **argv)
{
    (void)env;

    int size = argc;

    Value *res = val_new(T_OPAQUE);
    Array *array = mila_malloc(sizeof(Array));
    array->size = size;
    array->array = mila_malloc(sizeof(Value *) * size);

    for (int i = 0; i < size; i++)
    {
        array->array[i] = val_retain(argv[i]);
    }

    res->v.opaque = array;
    res->type_name = mila_strdup(MILA_LPREFIX "array");
    val_set_table(res, array_meta);
    return res;
}

Value *native_set_array(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 3)
    {
        return verror("array.set(array, index, value): requires 3 args");
    }

    Value *arrv = argv[0];
    if (arrv->type != T_OPAQUE)
    {
        return verror(
            "array.set(array, index, value): first arg must be an array (opaque)");
    }

    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array.set(array, index, value): null array data");
    }

    if (argv[1]->type != T_INT)
    {
        return verror("array.set(array, index, value): index must be int");
    }

    int idx = (int)argv[1]->v.i;
    if (idx < 0 || idx >= arr->size)
    {
        return verror(
            "array.set(array, index, value): index %d out of bounds (size %d)", idx,
            arr->size);
    }

    Value *old = arr->array[idx];
    if (old)
        val_release(old);

    arr->array[idx] = argv[2];
    val_retain(argv[2]);

    return vnull();
}

Value *native_get_array(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 2)
    {
        return verror("array.get(array, index): requires 2 args");
    }

    Value *arrv = argv[0];
    if (arrv->type != T_OPAQUE)
    {
        return verror(
            "array.get(array, index): first arg must be an array (opaque)");
    }

    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array.get(array, index): null array data");
    }

    if (argv[1]->type != T_INT)
    {
        return verror("array.get(array, index): index must be int");
    }

    int idx = (int)argv[1]->v.i;
    if (idx < 0 || idx >= arr->size)
    {
        return verror("array.get(array, index): index %d out of bounds (size %d)",
                      idx, arr->size);
    }

    Value *val = arr->array[idx];
    if (val)
        val_retain(val);
    else
        return vnull();

    return val;
}

Value *native_len_array(Env *env, int argc, Value **argv)
{
    (void)env;
    if (argc != 1)
    {
        return verror("array.len(array): requires 1 arg");
    }

    Value *arrv = argv[0];
    if (arrv->type != T_OPAQUE)
    {
        return verror("array.len(array): first arg must be an array (opaque)");
    }

    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array.len(array): null array data");
    }

    return vint(arr->size);
}

Value *get_array(Value *self, Value *index)
{
    Value *arrv = self;
    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array.get(array, index): null array data");
    }

    if (index->type != T_INT)
    {
        return verror("array.get(array, index): index must be int");
    }

    int idx = (int)index->v.i;
    if (idx < 0 || idx >= arr->size)
    {
        return verror("array.get(array, index): index %d out of bounds (size %d)",
                      idx, arr->size);
    }

    Value *val = arr->array[idx];

    return val ? val : vnull();
}

Value *set_array(Value *self, Value *index, Value *val)
{
    Value *arrv = self;
    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return verror("array.set(array, index, value): null array data");
    }

    if (index->type != T_INT)
    {
        return verror("array.set(array, index, value): index must be int");
    }

    int idx = (int)index->v.i;
    if (idx < 0 || idx >= arr->size)
    {
        return verror(
            "array.set(array, index, value): index %d out of bounds (size %d)", idx,
            arr->size);
    }

    Value *old = arr->array[idx];
    if (old)
        val_release(old);
    arr->array[idx] = val_retain(val);
    return NULL;
}

Value *free_array(Value *self)
{
    Value *arrv = self;
    if (arrv->type != T_OPAQUE)
    {
        return NULL;
    }

    Array *arr = (Array *)arrv->v.opaque;
    if (!arr)
    {
        return NULL;
    }

    for (int i = 0; i < arr->size; i++)
        val_release(arr->array[i]);
    mila_free(arr->array);
    mila_free(arr);

    return NULL;
}

Value *native_str_pop_start(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_STRING, T_ARG_END))
        return vnull();
    char *raw_string = argv[0]->v.s;
    char ch = *raw_string; // get first char

    char *copy = mila_strdup(raw_string + 1);

    mila_free(argv[0]->v.s);
    argv[0]->v.s = copy;

    return vstring_dup((char[]){ch, 0});
}

Value *native_str_pop_end(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_STRING, T_ARG_END))
        return vnull();
    char *raw_string = argv[0]->v.s;
    char ch = *(raw_string + strlen(raw_string) - 1); // get last char

    uint64_t size = strlen(raw_string) - 1;
    char *copy = (char *)mila_malloc(sizeof(char) * size);
    memcpy(copy, raw_string, size);

    mila_free(argv[0]->v.s);
    argv[0]->v.s = copy;

    return vstring_dup((char[]){ch, 0});
}

Value *native_ascii_from_int(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_INT, T_ARG_END))
        return vnull();
    return vstring_dup((char[]){(char)argv[0]->v.i, '\0'});
}

Value *native_ascii_from_string(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if ((!match_types(argv, T_STRING, T_ARG_END)) || strlen(argv[0]->v.s) != 1)
        return vnull();
    return vint(argv[0]->v.s[0]);
}

Value *native_str_slice(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_STRING, T_INT, T_INT, T_ARG_END))
        return vnull();
    return vstring_slice(argv[0]->v.s, argv[1]->v.i, argv[2]->v.i);
}

Value *native_str_copy(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_STRING))
        return vnull();
    return vstring_dup(argv[0]->v.s);
}

Value *native_str_index(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_STRING, T_INT, T_ARG_END))
        return vnull();
    return vstring_index(argv[0]->v.s, argv[1]->v.i);
}

Value *native_str_patch(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_STRING, T_STRING, T_STRING, T_ARG_END))
        return vnull();
    return vstring_replace(argv[0]->v.s, argv[1]->v.s, argv[2]->v.s);
}

Value *native_str_len(Env *env, int argc, Value **argv)
{
    (void)env;
    (void)argc;
    if (!match_types(argv, T_STRING, T_ARG_END))
        return vnull();
    return vint(strlen(argv[0]->v.s));
}

Value* native_str_split(Env* env, int argc, Value** argv) {
    if (argc != 2) return verror("str.split(string, deliminator): Expected a string and a deliminator!");
    if (GET_TYPE(argv[0]) != T_STRING) return verror("str.split(string, deliminator): Must deliminator be a string!");
    if (GET_TYPE(argv[1]) != T_STRING) return verror("str.split(string, deliminator): Must string be a string!");
    char* save_ptr = NULL;
    char* copy = mila_strdup(GET_STRING(argv[0]));

    Value* append_fn = env_get(env, "list.append");

    Value* list = call_function_str(env, "list", NULL);

    char* part = strtok_r(copy, GET_STRING(argv[1]), &save_ptr);
    while (part) {
        val_release(call_function_with(env, append_fn, val_retain(list), vstring_dup(part), NULL));
        part = strtok_r(NULL, GET_STRING(argv[1]), &save_ptr);
    }
    free(copy);
    return list;
}

Value* native_str_join(Env* env, int argc, Value** argv) {
    if (argc != 2) return verror("str.join(delim, list): Expected a deliminator and a list");
    if (GET_TYPE(argv[0]) != T_STRING) return verror("str.join(delim, list): Must deliminator be a string!");
    if (strcmp(GET_TYPENAME(argv[1]), MILA_LPREFIX"list")) return verror("str.join(delim, list): Must list be a list!");
    char *delim = GET_STRING(argv[0]), *string = NULL;
    LinkedList* l = (LinkedList*)GET_OPAQUE(argv[1]);
    for (LLNode *v = l->head; v; v = v->next)
    {
        char* vstr = as_c_string(v->value);
        our_asprintf(&string, "%s", vstr);
        if (v->next)
            our_asprintf(&string, "%s", delim);
        free(vstr);
    }
    return vstring_take(string);
}