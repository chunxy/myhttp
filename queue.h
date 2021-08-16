#ifndef QUEUE_H_
#define QUEUE_H_

#define QUEUE(TYPE)  \
    {                \
        TYPE* first; \
        TYPE* last;  \
    }

#define queue_init(Q) (void)((Q)->first = (Q)->last = NULL)

#define queue_is_empty(Q) (!(Q)->first)

#define queue_push(Q, ITEM, NEXT)            \
    (void)({                                 \
        ((ITEM)->NEXT) = NULL;               \
        if (queue_is_empty(Q))               \
            (Q)->first = (Q)->last = (ITEM); \
        else {                               \
            (Q)->last->NEXT = (ITEM);        \
            (Q)->last = (ITEM);              \
        }                                    \
    })

#define queue_pop(Q, PITEM, NEXT)          \
    (void)({                               \
        if (queue_is_empty(Q))             \
            *(PITEM) = NULL;               \
        else {                             \
            *(PITEM) = (Q)->first;         \
            (Q)->first = (Q)->first->NEXT; \
        }                                  \
    })

#endif