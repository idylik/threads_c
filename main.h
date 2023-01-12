#ifndef __MAIN_H
#define __MAIN_H

#include "blocking_q.h"

typedef struct {
  int id;
  long sched_t;
  long work_t;
  long real_t;
  long wait_t;
  pthread_mutex_t lock;
  blocking_q *tasks;
} processor;

typedef struct {
  blocking_q *sched_q;
  processor *processors;
} sched_data;

#endif
