#include <pthread.h>
#include <string.h>
#include "mila.h"
#include "ml_string.c"

/* Thread context for language-level threads */
typedef struct
{
    char *src;           /* Source code to execute */
    char *on_kill;       /* ran when thread dies */
    Env *env;            /* Environment (child of creator's env) */
    Value *result;       /* Result value */
    pthread_t thread_id; /* POSIX thread ID */
    int status;          /* 0 = pending, 1 = running, 2 = done */
    int is_deamon;       /* Doesnt keep MiLa awake */
} ThreadContext;

/* Global thread registry (simple array) */
#define MAX_THREADS 256
typedef struct
{
    ThreadContext *threads[MAX_THREADS];
    int count;
    pthread_mutex_t lock;
} ThreadRegistry;

static ThreadRegistry thread_registry = {
    .count = 0,
};

static void thread_registry_init(void)
{
    static int initialized = 0;
    if (!initialized)
    {
        pthread_mutex_init(&thread_registry.lock, NULL);
        initialized = 1;
    }
}

static int thread_registry_add(ThreadContext *ctx)
{
    thread_registry_init();
    pthread_mutex_lock(&thread_registry.lock);

    if (thread_registry.count >= MAX_THREADS)
    {
        pthread_mutex_unlock(&thread_registry.lock);
        return -1;
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

static void *mila_thread_worker(void *arg)
{
    ThreadContext *ctx = (ThreadContext *)arg;

    if (!ctx || !ctx->src || !ctx->env)
    {
        if (ctx)
            ctx->status = 2;
        return NULL;
    }

    ctx->status = 1; /* Running */

    Src *S = src_new(ctx->src);
    if (!S)
    {
        ctx->result = verror("Failed to create source");
        ctx->status = 2;
        return NULL;
    }

    ctx->result = eval_source(S, ctx->env);
    ctx->status = 2; /* Done */

    src_free(S);
    return NULL;
}

Value *native_thread_create(Env *env, int argc, Value **argv)
{
    if (argc < 1)
    {
        return verror("thread.make(code, on_kill?): Requires source code");
    }

    if (argv[0]->type != T_STRING || (argv == 2 && argv[1]->type != T_STRING))
    {
        return verror("thread.make(code, on_kill?): Requires string argument");
    }

    /* Create child environment */
    Env *child_env = env_new(env);

    /* Create thread context */
    ThreadContext *ctx = mila_malloc(sizeof(ThreadContext));
    ctx->src = mila_strdup(argv[0]->v.s);
    ctx->env = child_env;
    ctx->result = NULL;
    ctx->status = 0;
    ctx->is_deamon = 0;
    ctx->on_kill = argc == 2 ? argv[1]->v.s : NULL;

    /* Register thread */
    int thread_id = thread_registry_add(ctx);
    if (thread_id < 0)
    {
        mila_free(ctx->src);
        env_free(child_env);
        mila_free(ctx);
        return verror("Maximum threads reached");
    }

    /* Create actual thread */
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

    /* Wait for thread */
    pthread_join(ctx->thread_id, NULL);

    if (ctx->on_kill)
    {
        val_release(eval_str(ctx->on_kill, ctx->env));
            free(ctx->on_kill);
        ctx->on_kill = NULL;
    }
    /* Get result */
    Value *result = ctx->result ? val_retain(ctx->result) : vnull();

    return result;
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

    /* 0=pending, 1=running, 2=done */
    return vint(ctx->status);
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
        if (ctx && !ctx->is_deamon)
        {
            pthread_join(ctx->thread_id, NULL);
            if (ctx->result)
                val_release(ctx->result);
            mila_free(ctx->src);
            env_free(ctx->env);
            mila_free(ctx);
        } else if (ctx) {
            if (ctx->on_kill)
            {
                val_release(eval_str(ctx->on_kill, ctx->env));
                free(ctx->on_kill);
                ctx->on_kill = NULL;
            }
        }
    }

    pthread_mutex_unlock(&thread_registry.lock);
    pthread_mutex_destroy(&thread_registry.lock);
}

void register_thread_builtins(Env *env)
{
    env_register_native(env, "thread.make", native_thread_create);
    env_register_native(env, "thread.join", native_thread_join);
    env_register_native(env, "thread.set_deamon", native_thread_set_deamon);
    env_register_native(env, "thread.get_id", native_thread_id);
    env_register_native(env, "thread.status", native_thread_status);
}