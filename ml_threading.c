#pragma once

#include <pthread.h>
#include <string.h>
#include "mila.h"
#include "ml_builtins.c"
#include "ml_string.c"


typedef struct CGenData CGenData;
typedef Value*(*Generator)(CGenData*);

/* Thread context for language-level threads */
typedef struct
{
    int is_cgen;
    Generator c_gen;     /* either src or this is set at a time */
    char *src;           /* Source code to execute */
    char *on_kill;       /* ran when thread dies */
    Env *env;            /* Environment (child of creator's env) */
    Value *result;       /* Result value */
    pthread_t thread_id; /* POSIX thread ID */
    int status;          /* 0 = pending, 1 = running, 2 = done */
    int is_deamon;       /* Doesnt keep MiLa awake */
    int is_cancelled;    /* True if thread is cancelled. */
    
    /* Generator/Yield synchronization */
    pthread_mutex_t yield_lock;
    pthread_cond_t  yield_cond;
    int has_value;       /* Flag: value is ready to consume */
    int finished;        /* Flag: generator finished execution */
    int is_generator;
} ThreadContext;

struct CGenData
{
    int is_cgen;
    ThreadContext* ctx;
    Value* data;
};

// TODO: Change into a linked list to be able to clean
// Dead threads to avoid build up across program
// lifetime
typedef struct
{
    ThreadContext **threads;
    int count;
    int capacity;
    pthread_mutex_t lock;
} ThreadRegistry;

static ThreadRegistry thread_registry = {
    .threads = NULL,
    .count = 0,
    .capacity = 0,
};

static void thread_registry_init(void)
{
    static int initialized = 0;
    if (!initialized)
    {
        pthread_mutex_init(&thread_registry.lock, NULL);
        thread_registry.capacity = 16;
        thread_registry.threads = mila_malloc(sizeof(ThreadContext *) * thread_registry.capacity);
        initialized = 1;
    }
}

static void thread_registry_expand(void)
{
    int new_capacity = thread_registry.capacity * 2;
    ThreadContext **new_threads = mila_malloc(sizeof(ThreadContext *) * new_capacity);
    memcpy(new_threads, thread_registry.threads, sizeof(ThreadContext *) * thread_registry.count);
    mila_free(thread_registry.threads);
    thread_registry.threads = new_threads;
    thread_registry.capacity = new_capacity;
}

static int thread_registry_add(ThreadContext *ctx)
{
    thread_registry_init();
    pthread_mutex_lock(&thread_registry.lock);

    if (thread_registry.count >= thread_registry.capacity)
    {
        thread_registry_expand();
    }

    int id = thread_registry.count;
    thread_registry.threads[id] = ctx;
    thread_registry.count++;

    pthread_mutex_unlock(&thread_registry.lock);
    return id;
}

static ThreadContext *thread_registry_get(int id)
{
    thread_registry_init();
    pthread_mutex_lock(&thread_registry.lock);

    ThreadContext *ctx = NULL;
    if (id >= 0 && id < thread_registry.count)
    {
        ctx = thread_registry.threads[id];
    }

    pthread_mutex_unlock(&thread_registry.lock);
    return ctx;
}

void thread_yield(ThreadContext *ctx, Value *val)
{
    pthread_mutex_lock(&ctx->yield_lock);

    /* Wait until any previously yielded value is consumed */
    while (ctx->has_value && !ctx->is_cancelled)
    {
        pthread_cond_wait(&ctx->yield_cond, &ctx->yield_lock);
    }

    /* Check cancellation after wake-up */
    if (ctx->is_cancelled)
    {
        pthread_mutex_unlock(&ctx->yield_lock);
        return;
    }

    /* Store copy of value and signal readiness */
    ctx->result = val_copy(val);
    ctx->has_value = 1;
    pthread_cond_signal(&ctx->yield_cond);

    pthread_mutex_unlock(&ctx->yield_lock);
}

Value *thread_get_yield(ThreadContext *ctx)
{
    pthread_mutex_lock(&ctx->yield_lock);

    /* Wait for a value to be yielded or generator to finish */
    while (!ctx->has_value && !ctx->finished && !ctx->is_cancelled)
    {
        pthread_cond_wait(&ctx->yield_cond, &ctx->yield_lock);
    }

    /* No value available and generator is done or cancelled */
    if ((ctx->finished || ctx->is_cancelled) && !ctx->has_value)
    {
        pthread_mutex_unlock(&ctx->yield_lock);
        return NULL;
    }

    /* Retrieve the value and clear the flag */
    Value *val = ctx->result;
    ctx->has_value = 0;
    pthread_cond_signal(&ctx->yield_cond);

    pthread_mutex_unlock(&ctx->yield_lock);

    return val;
}

static void *mila_thread_worker(void *arg)
{
    Value *result;
    if (!((ThreadContext*)arg)->is_cgen) {
        // ctx here is ThreadContext
        ThreadContext *ctx = (ThreadContext *)arg;
        if (!ctx || !ctx->src || !ctx->env)
        {
            if (ctx)
                ctx->status = 2;
            return NULL;
        }
        ctx->status = 1;
        Src *S = src_new(ctx->src);
        if (!S)
        {
            ctx->result = verror("Failed to create source");
            ctx->status = 2;
            return NULL;
        }
        result = eval_source(S, ctx->env);
        src_free(S);
        if (IS_ERROR_TAGGED(result) && GET_ERROR(result) == E_THREAD_HALT)
        {
            val_release(result);
            ctx->result = vnull();
        }
        else
        {
            if (ctx->is_generator) {
                val_release(result);
            }
            else ctx->result = result;
        }
        ctx->finished = 1;
        ctx->status = 2;
        pthread_cond_signal(&ctx->yield_cond);
    } else {
        // ctx here is actually a CGenData
        result = (((CGenData*)arg)->ctx)->c_gen((CGenData*)arg);
        free(arg);
    }
    /* Handle thread halt error */
    
    return NULL;
}

Value *native_make_mutex(Env *env, int argc, Value **argv)
{
    if (argc != 0)
        return verror("thread.mutex(): Expected no arguments");

    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);
    return vowned_opaque_extra(mutex, NULL, MILA_LPREFIX "mutex");
}

Value *native_mutex_lock(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return verror("thread.mutex_lock(mut): Expected 1 argument");
    if (strcmp(GET_TYPENAME(argv[0]), MILA_LPREFIX "mutex") != 0)
        return verror("thread.mutex_lock(mut): Expected a " MILA_LPREFIX "mutex but got %s", GET_TYPENAME(argv[0]));

    pthread_mutex_lock(GET_OPAQUE(argv[0]));
    return vnull();
}

Value *native_mutex_unlock(Env *env, int argc, Value **argv)
{
    if (argc != 1)
        return verror("thread.mutex_unlock(mut): Expected 1 argument");
    if (strcmp(GET_TYPENAME(argv[0]), MILA_LPREFIX "mutex") != 0)
        return verror("thread.mutex_unlock(mut): Expected a " MILA_LPREFIX "mutex but got %s", GET_TYPENAME(argv[0]));

    pthread_mutex_unlock(GET_OPAQUE(argv[0]));
    return vnull();
}

int make_cgen(Generator c_gen, Value* val) {
    ThreadContext *ctx = mila_malloc(sizeof(ThreadContext));
    ctx->src = NULL;
    ctx->env = NULL;
    ctx->c_gen = c_gen;
    ctx->result = NULL;
    ctx->status = 0;
    ctx->is_deamon = 1;
    ctx->on_kill = NULL;
    ctx->is_cancelled = 0;
    ctx->has_value = 0;
    ctx->finished = 0;
    ctx->is_generator = 0;
    ctx->is_cgen = 1;

    CGenData* cgen_data = (CGenData*)malloc(sizeof(CGenData));
    cgen_data->ctx = ctx;
    cgen_data->data = val;

    pthread_mutex_init(&ctx->yield_lock, NULL);
    pthread_cond_init(&ctx->yield_cond, NULL);

    int thread_id = thread_registry_add(ctx);

    int pth_result = pthread_create(&ctx->thread_id, NULL,
                                mila_thread_worker, cgen_data);
    if (thread_id < 0)
    {
        mila_free(ctx);
        return -1;
    }
    return thread_id;
}

Value *native_thread_create(Env *env, int argc, Value **argv)
{
    if (argc < 1)
    {
        return verror("thread.make(code, on_kill?): Requires source code");
    }

    if (argv[0]->type != T_STRING || (argc == 2 && argv[1]->type != T_STRING))
    {
        return verror("thread.make(code, on_kill?): Requires string argument");
    }

    Env *child_env = env_new(env);

    ThreadContext *ctx = mila_malloc(sizeof(ThreadContext));
    ctx->src = mila_strdup(argv[0]->v.s);
    ctx->env = child_env;
    ctx->c_gen = NULL;
    ctx->result = NULL;
    ctx->status = 0;
    ctx->is_deamon = 0;
    ctx->on_kill = NULL;
    ctx->is_cancelled = 0;
    ctx->has_value = 0;
    ctx->finished = 0;
    ctx->is_generator = 0;
    ctx->is_cgen = 0;
    
    pthread_mutex_init(&ctx->yield_lock, NULL);
    pthread_cond_init(&ctx->yield_cond, NULL);
    
    if (argc == 2)
    {
        ctx->on_kill = mila_strdup(argv[1]->v.s);
    }

    int thread_id = thread_registry_add(ctx);
    if (thread_id < 0)
    {
        mila_free(ctx->src);
        env_free(child_env);
        mila_free(ctx);
        return verror("Failed to register thread");
    }

    int pth_result = pthread_create(&ctx->thread_id, NULL,
                                    mila_thread_worker, ctx);
    if (pth_result != 0)
    {
        mila_free(ctx->src);
        env_free(child_env);
        mila_free(ctx);
        return verror("pthread_create failed: %d", pth_result);
    }

    return vint(thread_id);
}

Value *native_thread_join(Env *env, int argc, Value **argv)
{
    if (argc < 1)
    {
        return verror("thread.join(id): requires thread ID");
    }

    if (argv[0]->type != T_INT)
    {
        return verror("thread.join(id): requires integer thread ID");
    }

    int thread_id = (int)argv[0]->v.i;
    ThreadContext *ctx = thread_registry_get(thread_id);

    if (!ctx)
    {
        return verror("Invalid thread ID: %d", thread_id);
    }

    pthread_join(ctx->thread_id, NULL);

    /* Execute cleanup handler if registered */
    if (ctx->on_kill)
    {
        env_set_local_raw(ctx->env, "_reason", vstring_dup("normal"));
        val_release(eval_str(ctx->on_kill, ctx->env));
        free(ctx->on_kill);
        ctx->on_kill = NULL;
    }

    Value *result = ctx->result ? val_retain(ctx->result) : vnull();

    return result;
}

Value *native_thread_yield(Env *env, int argc, Value **argv)
{
    if (argc < 2)
    {
        return verror("thread.yield(id, value): requires thread ID and a value");
    }

    if (argv[0]->type != T_INT)
    {
        return verror("thread.yield(id, value): requires integer thread ID");
    }

    int thread_id = (int)argv[0]->v.i;
    ThreadContext *ctx = thread_registry_get(thread_id);

    if (!ctx)
    {
        return verror("Invalid thread ID: %d", thread_id);
    }
    ctx->is_generator = 1;
    thread_yield(ctx, argv[1]);
    return vnull();
}

Value *native_thread_next(Env *env, int argc, Value **argv)
{
    if (argc < 1)
    {
        return verror("thread.next(id): requires thread ID");
    }

    if (argv[0]->type != T_INT)
    {
        return verror("thread.next(id): requires integer thread ID");
    }

    int thread_id = (int)argv[0]->v.i;
    ThreadContext *ctx = thread_registry_get(thread_id);

    if (!ctx)
    {
        return verror("Invalid thread ID: %d", thread_id);
    }

    Value *res = thread_get_yield(ctx);
    return res ? res : vnull();
}

Value *native_thread_dump(Env *env, int argc, Value **argv)
{
    if (argc < 1)
    {
        return verror("thread.next(id): requires thread ID");
    }

    if (argv[0]->type != T_INT)
    {
        return verror("thread.next(id): requires integer thread ID");
    }

    int thread_id = (int)argv[0]->v.i;
    ThreadContext *ctx = thread_registry_get(thread_id);

    if (!ctx)
    {
        return verror("Invalid thread ID: %d", thread_id);
    }

    printf("Thread #%ld\n", GET_INTEGER(argv[0]));
    printf("    is_cancelled = %s\n", ctx->is_cancelled ? "true" : "false");
    printf("    has_value = %s\n", ctx->has_value ? "true" : "false");
    printf("    finished = %s\n", ctx->finished ? "true" : "false");
    printf("    status = %i\n", ctx->status);
    printf("    result = "); print_value_debug(ctx->result);

    return vnull();
}

Value *native_thread_id(Env *env, int argc, Value **argv)
{
    pthread_t tid = pthread_self();
    return vint((long)tid);
}

Value *native_thread_status(Env *env, int argc, Value **argv)
{
    if (argc < 1)
    {
        return verror("thread.status(id) requires thread ID");
    }

    int thread_id = (int)GET_INTEGER(argv[0]);
    ThreadContext *ctx = thread_registry_get(thread_id);

    if (!ctx)
    {
        return verror("Invalid thread ID: %d", thread_id);
    }

    return vint(ctx->status);
}

Value *native_thread_cancel(Env *env, int argc, Value **argv)
{
    if (argc < 1)
    {
        return verror("thread.cancel(id) requires thread ID");
    }

    int thread_id = (int)GET_INTEGER(argv[0]);
    ThreadContext *ctx = thread_registry_get(thread_id);

    if (!ctx)
    {
        return verror("Invalid thread ID: %d", thread_id);
    }

    ctx->is_cancelled = 1;
    pthread_cond_signal(&ctx->yield_cond);  /* Wake up if blocked on yield */
    pthread_join(ctx->thread_id, NULL);

    /* Execute cleanup handler if registered */
    if (ctx->on_kill)
    {
        env_set_local_raw(ctx->env, "_reason", vstring_dup("cancelled"));
        val_release(eval_str(ctx->on_kill, ctx->env));
        free(ctx->on_kill);
        ctx->on_kill = NULL;
    }

    Value *result = ctx->result ? val_retain(ctx->result) : vnull();

    return result;
}

Value *native_thread_check_cancel(Env *env, int argc, Value **argv)
{
    if (argc < 1)
    {
        return verror("thread.check_cancel(id) requires thread ID");
    }

    int thread_id = (int)GET_INTEGER(argv[0]);
    ThreadContext *ctx = thread_registry_get(thread_id);

    if (!ctx)
    {
        return verror("Invalid thread ID: %d", thread_id);
    }

    if (ctx->is_cancelled)
        return vtagged_error(E_THREAD_HALT, "thread halted");
    return vnull();
}

Value *native_thread_set_deamon(Env *env, int argc, Value **argv)
{
    if (argc < 1)
    {
        return verror("thread.set_deamon(id) requires thread ID");
    }

    int thread_id = (int)GET_INTEGER(argv[0]);
    ThreadContext *ctx = thread_registry_get(thread_id);

    if (!ctx)
    {
        return verror("Invalid thread ID: %d", thread_id);
    }

    ctx->is_deamon = 1;
    return vnull();
}

void mila_threads_cleanup(void)
{
    thread_registry_init();
    pthread_mutex_lock(&thread_registry.lock);

    for (int i = 0; i < thread_registry.count; i++)
    {
        ThreadContext *ctx = thread_registry.threads[i];
        if (!ctx) continue;

        /* Non-daemon threads: join and cleanup normally */
        if (ctx->status < 2 && !ctx->is_deamon)
        {
            pthread_join(ctx->thread_id, NULL);
            if (ctx->on_kill)
            {
                env_set_local_raw(ctx->env, "_reason", vstring_dup("normal"));
                val_release(eval_str(ctx->on_kill, ctx->env));
                free(ctx->on_kill);
                ctx->on_kill = NULL;
            }
        }
        /* Daemon threads: execute cleanup handler on halt */
        else if (ctx->on_kill)
        {
            env_set_local_raw(ctx->env, "_reason", vstring_dup("interpreter halt"));
            val_release(eval_str(ctx->on_kill, ctx->env));
            free(ctx->on_kill);
            ctx->on_kill = NULL;
        }

        pthread_mutex_destroy(&ctx->yield_lock);
        pthread_cond_destroy(&ctx->yield_cond);
        if (ctx->src) mila_free(ctx->src);
        if (ctx->env) env_free(ctx->env);
        mila_free(ctx);
    }

    pthread_mutex_unlock(&thread_registry.lock);
    pthread_mutex_destroy(&thread_registry.lock);
}

void register_thread_builtins(Env *env)
{
    env_register_native(env, "thread.make", native_thread_create);
    env_register_native(env, "thread.join", native_thread_join);
    env_register_native(env, "thread.cancel", native_thread_cancel);
    env_register_native(env, "thread.check_cancel", native_thread_check_cancel);
    env_register_native(env, "thread.set_deamon", native_thread_set_deamon);
    env_register_native(env, "thread.get_id", native_thread_id);
    env_register_native(env, "thread.status", native_thread_status);
    env_register_native(env, "thread.mutex", native_make_mutex);
    env_register_native(env, "thread.mutex_unlock", native_mutex_unlock);
    env_register_native(env, "thread.mutex_lock", native_mutex_lock);
    env_register_native(env, "thread.yield", native_thread_yield);
    env_register_native(env, "thread.next", native_thread_next);
    env_register_native(env, "thread.dump", native_thread_dump);
}