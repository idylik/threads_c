#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "blocking_q.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#define TODO printf("TODO!\n");

/**
 * Internal function to blocking_q. Takes an element
 * in the queue. This functions assumes the following
 * preconditions:
 *  - The thread has safe access to the queue
 *  - The queue is NOT empty
 * Also update the size.
 * @param q the queue
 * @return an element
 */
task_ptr __blocking_q_take(blocking_q *q) { // NOLINT(bugprone-reserved-identifier)
    blocking_q_node *q_first = q->first;
    q->first = q->first->next;
    q->sz--;
    task_ptr pointeur = (task_ptr) q_first->data;
    return pointeur;
}

/**
 * Create a blocking queue. Initializes the synchronisation primitives
 * @param q the queue
 * @return if init was successful.
 */
bool blocking_q_init(blocking_q *q) {
    if (q == NULL) return false;

    q->first = NULL;
    q->sz = 0;

    if (pthread_mutex_init(&(q->lock), NULL) == 0 &&
            pthread_cond_init(&(q->cond), NULL) == 0) {
        return true;
    }

    return false;
}

/**
 * Destroy a blocking queue. Removes the allocations of the data
 * and destroys the sync. primitives.
 * @param q ptr to the blocking queue
 */
void blocking_q_destroy(blocking_q *q) {
    //printf("blocking queue destroy\n");

    //Libérer chaque noeud:
    blocking_q_node *current_node = q->first;
    blocking_q_node *next_node;

    while (current_node != NULL) {
        next_node = current_node->next;
        free(current_node);
        current_node = next_node;
    }

    pthread_mutex_destroy(&(q->lock));
    pthread_cond_destroy(&(q->cond));

}

/**
 * Put a task in the blocking queue. This task can fail if no
 * memory is available to allocate a new entry in the queue
 * @param q the queue
 * @param data the data description to put inside the queue
 * @returns if the data was put correctly inside the queue.
 */
bool blocking_q_put(blocking_q *q, task_ptr data) {


    blocking_q_node *new_node = malloc(sizeof(blocking_q_node));
    if (new_node == NULL) return false;

    new_node->data = data;
    new_node->next = NULL;

    //printf("mutex blocking queue locked\n");
    pthread_mutex_lock(&(q->lock));
    //Accès privilégié à q et son contenu
    //Rajouter à la fin de liste chaînée
    if (q->first == NULL) {
        q->first = new_node;
    } else {
        blocking_q_node *last_node = q->first;
        while (last_node->next != NULL) {
            last_node = last_node->next;
        }
        last_node->next = new_node;
    }

    //Mettre à jour la taille
    q->sz++;

    //Signaler qu'une nouvelle tâche a été ajoutée:
    pthread_cond_broadcast(&(q->cond));

    //Libérer le lock de cette blocking queue
    pthread_mutex_unlock(&(q->lock));
    //printf("mutex blocking queue UNlocked\n");

    return true;

}

/**
 * Get an element in the blocking queue. If the queue is empty,
 * the current thread is put to sleep until an element is added
 * to the queue.
 * @param q the blocking queue
 * @return the element
 */
task_ptr blocking_q_get(blocking_q *q) {
    pthread_mutex_lock(&(q->lock));

    //Accès privilégié aux données de q

    while (q->sz == 0) {
        pthread_cond_wait(&(q->cond), &(q->lock));

    }

    task_ptr first_elem = __blocking_q_take(q);
    //Libérer le lock de cette blocking queue
    pthread_mutex_unlock(&(q->lock));

    return first_elem;
}

/**
 * Drain as many elements as possible into the area allowed
 * by the pointer. This function does not block.
 * @param q the queue
 * @param data the pointer where to store the data
 * @param sz the maximum area available in the buffer
 * @return the number of entries written.
 */
size_t blocking_q_drain(blocking_q *q, task_ptr *data, size_t sz) {
    pthread_mutex_lock(&(q->lock));
    //Accès privilégié à la queue

    size_t i = 0;
    while(q->sz > 0 && sz > 0) {
        data[i] = __blocking_q_take(q);
        sz--;
        i++;
    }
    //Libérer le lock de cette blocking queue
    pthread_mutex_unlock(&(q->lock));

    return i;
}

/**
 * Drain at least min elements in the buffer. This function
 * might block if there are not enough elements to drain.
 * @param q the queue
 * @param data the pointer where to store the data
 * @param sz the maximum area available in the buffer
 * @param min the minimum amounts of elements to drain (must be less than sz)
 * @return the number of elements written
 */
size_t blocking_q_drain_at_least(blocking_q *q, task_ptr *data, size_t sz, size_t min) {
    size_t i = 0;
    //Tant qu'on n'a pas donné le minimum et qu'il reste de la place dans buffer
    while (min > 0 && sz > 0) {
        pthread_mutex_lock(&(q->lock));
        //Accès privilégié à la queue
        while (q->sz <= 0) { //Tant que la queue est vide
            pthread_cond_wait(&(q->cond), &(q->lock));
        }

            data[i] = __blocking_q_take(q);
            min--;
            sz--;
            i++;

        //Libérer le lock de cette blocking queue
        pthread_mutex_unlock(&(q->lock));
    }

    return i;
}

/**
 * Check the first element in the queue. This will allocate storage for a copy
 * of the character. If the allocation fails, this function returns false.
 * @param q the queue
 * @param c pointer to a pointer where an allocated char will be stored
 * @return if there is an element allocated in the pointer
 */
bool blocking_q_peek(blocking_q *q, task **c) {
    pthread_mutex_lock(&(q->lock));
    //Accès privilégié à la queue

    bool first_elem = false;
    if (q->first != NULL) {
        first_elem = true;

        c = malloc(sizeof(task_ptr*));
        c = &(q->first->data);

    }

    //Libérer le lock de cette blocking queue
    pthread_mutex_unlock(&(q->lock));

    return first_elem;
}
