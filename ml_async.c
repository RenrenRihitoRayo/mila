#include "mila.h"
#include <stdlib.h>
#include <string.h>

AsyncPool *async_pool_new(void) {
  AsyncPool *pool = (AsyncPool *)mila_malloc(sizeof(AsyncPool));
  if (!pool)
    return NULL;

  pool->task_count = 0;
  memset(pool->tasks, 0, sizeof(pool->tasks));
  return pool;
}

int async_pool_add(AsyncPool *pool, Src *src, Env *env) {
  if (!pool || !src || !env)
    return -1;

  if (pool->task_count >= MAX_ASYNC_TASKS)
    return -1;

  int task_id = pool->task_count;
  pool->tasks[task_id].src = src;
  pool->tasks[task_id].env = env;
  pool->tasks[task_id].active = 1;
  pool->tasks[task_id].unyield_pending = 0;

  pool->task_count++;
  return task_id;
}

Value **async_next(AsyncPool *pool) {
  if (!pool || pool->task_count == 0)
    return NULL;

  // Allocate result array (task_count + 1 for NULL terminator)
  Value **results =
      (Value **)mila_malloc((pool->task_count + 1) * sizeof(Value *));
  if (!results)
    return NULL;

  int result_idx = 0;

  for (int i = 0; i < pool->task_count; i++) {
    AsyncTask *task = &pool->tasks[i];

    // Skip inactive tasks
    if (!task->active) {
      results[result_idx++] = NULL;
      continue;
    }

    // Check if this task is pending unyield
    if (task->unyield_pending) {
      task->active = 0;
      results[result_idx++] = NULL;
      continue;
    }

    // Execute one statement
    Value *stmt_result = eval_statement(task->src, task->env);

    // Handle yield
    if (stmt_result && stmt_result->type == T_YIELD) {
      if (stmt_result->v.yield.is_unyield) {
        // Mark task as dead for next call
        task->unyield_pending = 1;
        results[result_idx++] = NULL;
        val_release(stmt_result);
      } else {
        // Regular yield: return the value and keep task alive
        Value *yielded = stmt_result->v.yield.value;
        val_retain(yielded);
        val_release(stmt_result);
        results[result_idx++] = yielded;
      }
    } else if (stmt_result && stmt_result->type == T_RETURN) {
      // Return terminates this task
      Value *return_val = (Value *)stmt_result->v.opaque;
      val_retain(return_val);
      val_release(stmt_result);
      task->active = 0;
      results[result_idx++] = return_val;
    } else if (IS_ERROR(stmt_result)) {
      // Error terminates the task and returns the error
      results[result_idx++] = stmt_result;
      task->active = 0;
    } else {
      // Normal value: return it
      results[result_idx++] = stmt_result;
    }
  }

  // NULL terminate
  results[result_idx] = NULL;

  return results;
}

int async_kill_task(AsyncPool *pool, int task_id) {
  if (!pool || task_id < 0 || task_id >= pool->task_count)
    return -1;

  AsyncTask *task = &pool->tasks[task_id];
  if (!task->active)
    return -1;

  task->active = 0;
  // Note: We don't free src or env here as they may be shared
  // That's the caller's responsibility

  return 0;
}

void async_pool_free(AsyncPool *pool) {
  if (!pool)
    return;

  for (int i = 0; i < pool->task_count; i++) {
    pool->tasks[i].active = 0;
    // src and env cleanup is caller's responsibility
  }

  mila_free(pool);
}

int async_pool_active_count(AsyncPool *pool) {
  if (!pool)
    return 0;

  int count = 0;
  for (int i = 0; i < pool->task_count; i++) {
    if (pool->tasks[i].active && !pool->tasks[i].unyield_pending)
      count++;
  }
  return count;
}

int async_task_is_active(AsyncPool *pool, int task_id) {
  if (!pool || task_id < 0 || task_id >= pool->task_count)
    return 0;

  AsyncTask *task = &pool->tasks[task_id];
  return task->active && !task->unyield_pending;
}