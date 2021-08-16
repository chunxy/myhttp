#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <pthread.h>

#include "queue.h"

typedef void* (*function)(void*);

typedef struct task task;
typedef struct taskq taskq;
typedef struct scheduler scheduler;

struct task {
    function func;
    void* arg;
    task* next;
};

struct taskq
    QUEUE(struct task);

struct scheduler {
    taskq* queue;
    pthread_mutex_t qlock;
    pthread_cond_t qcondv;
    pthread_t* pool;
    size_t size;
};

struct scheduler* tp_init(size_t size);

void tp_submit(struct scheduler* s, function func, void* arg);

void tp_destroy(struct scheduler* s);

#endif