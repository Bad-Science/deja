#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef struct scheduler {
  scheduleable_t items[];
} Scheduler_t;

typedef struct scheduleable {
  int64_t (*behavior)();
} scheduleable_t;

Scheduler_t scheduler_init();
bool scheduler_add_item(Scheduler_t sched, scheduleable_t item);

#endif