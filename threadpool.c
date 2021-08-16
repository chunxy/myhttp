#include <pthread.h>
#include <stdlib.h>

#include "threadpool.h"

static void* worker(void* arg)
{
    while (1) {
        scheduler* s = (scheduler*)arg;
        pthread_mutex_lock(&s->qlock);
        while (queue_is_empty(s->queue))
            pthread_cond_wait(&s->qcondv, &s->qlock);

        task* t;
        queue_pop(s->queue, &t, next);
        pthread_mutex_unlock(&s->qlock);

        t->func(t->arg);
        free(t);
    }
}

struct scheduler* tp_init(size_t size)
{
    struct scheduler* s = malloc(sizeof(struct scheduler));

    s->queue = malloc(sizeof(taskq));
    queue_init(s->queue);
    pthread_mutex_init(&s->qlock, NULL);
    pthread_cond_init(&s->qcondv, NULL);

    s->pool = malloc(sizeof(pthread_t) * size);
    s->size = size;

    for (int i = 0; i < size; i++) {
        pthread_create(s->pool + i, NULL, worker, s);
        pthread_detach(s->pool[i]);
    }

    return s;
}

void tp_submit(struct scheduler* s, function func, void* arg)
{
    task* t = malloc(sizeof(task));
    t->func = func;
    t->arg = arg;
    pthread_mutex_lock(&s->qlock);
    queue_push(s->queue, t, next);
    pthread_cond_signal(&s->qcondv);
    pthread_mutex_unlock(&s->qlock);
}

void tp_destroy(struct scheduler* s)
{
    ;
}