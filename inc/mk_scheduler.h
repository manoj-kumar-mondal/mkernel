
#ifndef _MK_SCHEDULER_H_
#define _MK_SCHEDULER_H_

/*------------------------- Includes Files -------------------------*/
#include "mk_task.h"

/*----------------------- Typedefs & Macros ------------------------*/
typedef enum {
    e_ready,
    e_running,
    e_blocked,
    e_suspended,
    e_deleted
} mk_TaskState_t;

typedef enum {
    e_cooperative,
    e_priorityPremption,
    e_roundrobin,
} mk_ScheduleType_t;

/*---------------------- Function Declaration ----------------------*/
void mk_schd_add_task_to_ready_list(TCB_t *ptcb);
void mk_scheduler_start(mk_ScheduleType_t schedule_type);

#endif // _MK_SCHEDULER_H_