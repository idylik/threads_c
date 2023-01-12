#ifndef TP2SLN_BLOCKING_Q_H
#define TP2SLN_BLOCKING_Q_H

#include "task.h"

typedef struct blocking_q_node_struct blocking_q_node;

struct blocking_q_node_struct {
  task_ptr data;
  blocking_q_node *next;
};

typedef struct {
  pthread_mutex_t lock;
  pthread_cond_t cond;
  blocking_q_node *first;
  size_t sz;
} blocking_q;

// Internal, out for debugging
task_ptr __blocking_q_take(blocking_q *q);

/**
 * Create a blocking queue. Initializes the synchronisation primitives
 * and
 * @param q the queue
 * @return if init was successful.
 */
bool blocking_q_init(blocking_q *q);

/**
 * Destroy a blocking queue. Removes the allocations of the data
 * and destroys the sync. primitives.
 * @param q ptr to the blocking queue
 */
void blocking_q_destroy(blocking_q *q);

/**
 * Put a task in the blocking queue. This task can fail if no
 * memory is available to allocate a new entry in the queue
 * @param q the queue
 * @param data the data description to put inside the queue
 * @returns if the data was put correctly inside the queue.
 */
bool blocking_q_put(blocking_q *q, task_ptr data);

/**
 * Get an element in the blocking queue. If the queue is empty,
 * the current thread is put to sleep until an element is added
 * to the queue.
 * @param q the blocking queue
 * @return the element
 */
task_ptr blocking_q_get(blocking_q *q);

/**
 * Drain as many elements as possible into the area allowed
 * by the pointer. This function does not block.
 * @param data the pointer where to store the data
 * @param sz the maximum area available in the buffer
 * @return the number of entries written.
 */
size_t blocking_q_drain(blocking_q *q, task_ptr *data, size_t sz);

/**
 * Drain at least min elements in the buffer. This function
 * might block if there are not enough elements to drain.
 * @param data the pointer where to store the data
 * @param sz the maximum area available in the buffer
 * @param min the minimum amounts of elements to drain (must be less than sz)
 * @return the number of elements written
 */
size_t blocking_q_drain_at_least(blocking_q *q, task_ptr *data, size_t sz,
                                 size_t min);

/**
 * Check the first element in the queue. This will allocate storage for a copy
 * of the character. If the allocation fails, this function returns false.
 * @param q the queue
 * @param c pointer to a pointer where an allocated char will be stored
 * @return if there is an element allocated in the pointer
 */
bool blocking_q_peek(blocking_q *q, task **c);

#endif
