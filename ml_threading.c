// This project is licensed under the GNU Affero General Public License
#pragma once

#include <pthread.h>
#include <string.h>
#include "mila.h"
#include "ml_threading.h"

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
    if (!arg) return NULL;
    VAR_UNUSED Value *result = NULL; // not all code paths uses this (compiler gets grumpy)
    if (!((ThreadContext*)arg)->is_cgen) {
        // ctx here is ThreadContext
        ThreadContext *ctx = (ThreadContext *)arg;
        ctx->status = 1;
        result = call_function_with(NULL, ctx->func, vint(ctx->public_thread_id), NULL);
        // NOTE: Race conditions when cleaning up, use gdb?
        // if (IS_ERROR_TAGGED(result) && GET_TAGGED_ERROR_TYPE(result) == E_THREAD_HALT)
        // {
        //     val_release(result);
        //     ctx->result = vnull();
        // }
        // else
        // {
        //     if (ctx->is_generator) {
        //         val_release(result);
        //     }
        //     else ctx->result = result;
        // }
        ctx->finished = 1;
        ctx->status = 2;
        pthread_cond_signal(&ctx->yield_cond);
    } else {
        // ctx here is actually a CGenData
        ThreadContext *ctx = (((CGenData*)arg)->ctx);
        ctx->c_gen((CGenData*)arg);
        ctx->finished = 1;
        val_release(ctx->result);
        ctx->result = NULL;
        pthread_cond_signal(&ctx->yield_cond);
        free(arg);
    }
    
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

int make_cthread(Generator c_gen) {
    ThreadContext *ctx = mila_malloc(sizeof(ThreadContext));
    ctx->func = NULL;
    ctx->c_gen = c_gen;
    ctx->result = NULL;
    ctx->status = 0;
    ctx->is_daemon = 1;
    ctx->on_kill = NULL;
    ctx->is_cancelled = 0;
    ctx->has_value = 0;
    ctx->finished = 0;
    ctx->is_generator = 0;
    ctx->is_cgen = 1;
    ctx->public_thread_id = -1;

    CGenData* cgen_data = (CGenData*)malloc(sizeof(CGenData));
    cgen_data->ctx = ctx;
    cgen_data->data = NULL;

    pthread_mutex_init(&ctx->yield_lock, NULL);
    pthread_cond_init(&ctx->yield_cond, NULL);

    int thread_id = thread_registry_add(ctx);

    pthread_create(&ctx->thread_id, NULL,
                   mila_thread_worker, cgen_data);
    if (thread_id < 0)
    {
        mila_free(ctx);
        return -1;
    }
    return thread_id;
}

int make_cgen(Generator c_gen, Value* val) {
    ThreadContext *ctx = mila_malloc(sizeof(ThreadContext));
    ctx->func = NULL;
    ctx->c_gen = c_gen;
    ctx->result = NULL;
    ctx->status = 0;
    ctx->is_daemon = 1;
    ctx->on_kill = NULL;
    ctx->is_cancelled = 0;
    ctx->has_value = 0;
    ctx->finished = 0;
    ctx->is_generator = 0;
    ctx->is_cgen = 1;
    ctx->public_thread_id = -1;

    CGenData* cgen_data = (CGenData*)malloc(sizeof(CGenData));
    cgen_data->ctx = ctx;
    cgen_data->data = val;

    pthread_mutex_init(&ctx->yield_lock, NULL);
    pthread_cond_init(&ctx->yield_cond, NULL);

    int thread_id = thread_registry_add(ctx);

    pthread_create(&ctx->thread_id, NULL,
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

    if (argv[0]->type != T_FUNCTION || (argc == 2 && argv[1]->type != T_FUNCTION))
    {
        return verror("thread.make(code, on_kill?): Requires function argument");
    }

    ThreadContext *ctx = mila_malloc(sizeof(ThreadContext));
    ctx->func = val_retain(argv[0]);
    ctx->c_gen = NULL;
    ctx->result = NULL;
    ctx->status = 0;
    ctx->is_daemon = 0;
    ctx->on_kill = NULL;
    ctx->is_cancelled = 0;
    ctx->has_value = 0;
    ctx->finished = 0;
    ctx->is_generator = 0;
    ctx->is_cgen = 0;
    ctx->public_thread_id = -1;
    
    pthread_mutex_init(&ctx->yield_lock, NULL);
    pthread_cond_init(&ctx->yield_cond, NULL);
    
    if (argc == 2)
    {
        ctx->on_kill = val_retain(argv[1]);
    }

    int thread_id = thread_registry_add(ctx);
    ctx->public_thread_id = thread_id;
    if (thread_id < 0)
    {
        val_release(ctx->func);
        mila_free(ctx);
        return verror("Failed to register thread");
    }

    int pth_result = pthread_create(&ctx->thread_id, NULL,
                                    mila_thread_worker, ctx);
    if (pth_result != 0)
    {
        val_release(ctx->func);
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

    int thread_id = (int)argv[0]->v->i;
    ThreadContext *ctx = thread_registry_get(thread_id);

    if (!ctx)
    {
        return verror("Invalid thread ID: %d", thread_id);
    }

    if (ctx->is_cancelled) return verror("Thread %i was already cancelled!", thread_id);

    pthread_join(ctx->thread_id, NULL);

    /* Execute cleanup handler if registered */
    if (ctx->on_kill)
    {
        Env* frame = env_new(NULL);
        val_release(call_function_with(frame, ctx->on_kill, vint(ctx->public_thread_id), vstring_dup("normal"), NULL));
        val_release(ctx->on_kill);
        env_free(frame);
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

    int thread_id = (int)argv[0]->v->i;
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

    int thread_id = (int)argv[0]->v->i;
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

    int thread_id = (int)argv[0]->v->i;
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

Value *native_thread_pthread_id(Env *env, int argc, Value **argv)
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
        Env* frame = env_new(NULL);
        val_release(call_function_with(frame, ctx->on_kill, vint(ctx->public_thread_id), vstring_dup("cancelled"), NULL));
        val_release(ctx->on_kill);
        env_free(frame);
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

Value *native_thread_set_daemon(Env *env, int argc, Value **argv)
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

    ctx->is_daemon = 1;
    return vnull();
}

void mila_threads_cleanup(void)
{
    thread_registry_init();
    pthread_mutex_lock(&thread_registry.lock);

    for (MThreadID i = 0; i < thread_registry.count; i++)
    {
        ThreadContext *ctx = thread_registry.threads[i];
        if (!ctx) continue;

        /* Non-daemon threads: join and cleanup normally */
        if (!ctx->is_daemon)
        {
            if (ctx->on_kill)
            {
                Env* frame = env_new(NULL);
                val_release(call_function_with(frame, ctx->on_kill, vint(ctx->public_thread_id), vstring_dup("normal"), NULL));
                val_release(ctx->on_kill);
                env_free(frame);
                ctx->on_kill = NULL;
            }
        }
        /* Daemon threads: execute cleanup handler on halt */
        else if (ctx->on_kill)
        {
            ctx->is_cancelled = 1;
            Env* frame = env_new(NULL);
            val_release(call_function_with(frame, ctx->on_kill, vint(ctx->public_thread_id), vstring_dup(ctx->status == 2 ? "normal" : "interpreter halt"), NULL));
            val_release(ctx->on_kill);
            env_free(frame);
            ctx->on_kill = NULL;
        }

        pthread_mutex_destroy(&ctx->yield_lock);
        pthread_cond_destroy(&ctx->yield_cond);
        if (ctx->result) val_release(ctx->result);
        if (ctx->func) val_release(ctx->func);
        mila_free(ctx);
    }

    pthread_mutex_unlock(&thread_registry.lock);
    pthread_mutex_destroy(&thread_registry.lock);
}