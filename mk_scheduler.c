/*------------------------- Includes Files -------------------------*/
#include "mk_scheduler.h"
#include "mk_memory.h"

/*----------------------- Typedefs & Macros ------------------------*/
typedef struct TaskHandlerForScheduler{
    mk_u8 task_id;
    mk_TaskState_t state;
    TCB_t *ptcb;
    struct TaskHandlerForScheduler *pnext;
} TaskHandlerForScheduler_t;

/*----------- Data Region (constants, variables, static) -----------*/
static TaskHandlerForScheduler_t *pstart_block = MK_NULL;
static mk_u8 _total_task_created = 0;

/*------------------ Static Functions Declaration ------------------*/

/*------------------- All Functions Definitions --------------------*/

/**
 * @brief   Function is to add a nrewly create mkernel task into scheduler.
 * @param   ptcb(TCB_t*) task's TCB
 * @retval  none
 */
void mk_schd_add_task_to_ready_list(TCB_t *ptcb) {
    /* Indivisual Taskblock for scheduler */
    TaskHandlerForScheduler_t *phandler = (TaskHandlerForScheduler_t*)mk_mem_allocate(sizeof(TaskHandlerForScheduler_t));

    if (MK_NULL != phandler) {
        /* provide default values */
        phandler->ptcb = ptcb;
        phandler->state = e_ready;
        phandler->pnext = MK_NULL;
        
        /* add new block in a list */
        if (MK_NULL == pstart_block) {
            pstart_block = phandler;
        } else {
            TaskHandlerForScheduler_t *ptemp = pstart_block;

            while(ptemp->pnext != MK_NULL) {
                ptemp = ptemp->pnext;
            }
            ptemp->pnext = phandler;
        }
        _total_task_created += 1;
    }
}

/**
 * @brief   Function is to start the mkernel scheduler.
 * @param   schedule_type(mk_ScheduleType_t): scheduling algo
 * @retval  none
 */
void mk_scheduler_start(mk_ScheduleType_t schedule_type) {
    
}