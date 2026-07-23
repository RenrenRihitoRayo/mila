// This project is licensed under the GNU Affero General Public License
#pragma once

#include <pthread.h>
#include "../mila.h"

typedef struct CGenData CGenData;
typedef Value*(*Generator)(CGenData*);

typedef long MThreadID;

/* Thread context for language-level threads */
typedef struct
{
    Value *func;           /* Source code to execute */
    Value *on_kill;       /* ran when thread dies */
    Value *result;       /* Result value */
    pthread_t thread_id; /* POSIX thread ID */
    MThreadID public_thread_id; /* MiLa visible thread ID */
    int status;          /* 0 = pending, 1 = running, 2 = done */
    int is_daemon;       /* Doesnt keep MiLa awake */
    int is_cancelled;    /* True if thread is cancelled. */
} ThreadContext;

// Note: this stays as is
typedef struct
{
    ThreadContext **threads;
    // we use MThreadID to ensure IDs are in bounds ie Max IDs = Max Cap/Count
    MThreadID count;
    MThreadID capacity;
    pthread_mutex_t lock;
} ThreadRegistry;

static ThreadRegistry thread_registry = {
    .threads = NULL,
    .count = 0,
    .capacity = 0,
};

static void thread_registry_init(void);
static void thread_registry_expand(void);
static int thread_registry_add(ThreadContext *ctx);
static ThreadContext *thread_registry_get(int id);
static void *mila_thread_worker(void *arg);
Value *native_make_mutex(Env *env, int argc, Value **argv);
Value *native_mutex_lock(Env *env, int argc, Value **argv);
Value *native_mutex_unlock(Env *env, int argc, Value **argv);
Value *native_thread_create(Env *env, int argc, Value **argv);
Value *native_thread_join(Env *env, int argc, Value **argv);
Value *native_thread_yield(Env *env, int argc, Value **argv);
Value *native_thread_next(Env *env, int argc, Value **argv);
Value *native_thread_dump(Env *env, int argc, Value **argv);
Value *native_thread_pthread_id(Env *env, int argc, Value **argv);
Value *native_thread_status(Env *env, int argc, Value **argv);
Value *native_thread_cancel(Env *env, int argc, Value **argv);
Value *native_thread_check_cancel(Env *env, int argc, Value **argv);
Value *native_thread_set_daemon(Env *env, int argc, Value **argv);
void mila_threads_cleanup(void);
void register_thread_builtins(Env *env);