/*------------------------- Includes Files -------------------------*/
#include "mk_scheduler.h"
#include "mk_memory.h"

/*----------------------- Typedefs & Macros ------------------------*/
#define INITIAL_TICK_COUNT              (0U)

typedef struct TaskHandlerForScheduler{
    mk_u8 task_id;
    mk_TaskState_t state;
    TCB_t *ptcb;
    struct TaskHandlerForScheduler *pnext;
} TaskHandlerForScheduler_t;

/*----------- Data Region (constants, variables, static) -----------*/
static TaskHandlerForScheduler_t *pstart_block = MK_NULL;
static TaskHandlerForScheduler_t *pcurrent_task = MK_NULL;
static mk_u8 _total_task_created = 0;
static mk_bool _scheduler_running = MK_FALSE;
static TickType_t _tick_count;
 
/*------------------ Static Functions Declaration ------------------*/
static void _configure_sys_tick(void);

/*------------------- All Functions Definitions --------------------*/

/**
 * @brief   Function is to add a nrewly create mkernel task into scheduler.
 * @param   ptcb(TCB_t*) task's TCB
 * @retval  none
 */
void mk_scheduler_add_task_to_list(TCB_t *ptcb) {
    /* Indivisual Taskblock for scheduler */
    TaskHandlerForScheduler_t *phandler = (TaskHandlerForScheduler_t*)mk_mem_allocate(sizeof(TaskHandlerForScheduler_t));

    if (MK_NULL != phandler) {
        /* provide default values */
        phandler->ptcb = ptcb;
        phandler->state = e_ready;
        phandler->pnext = MK_NULL;
        pcurrent_task = phandler;

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

    if (pstart_block != NULL) {
        _configure_sys_tick();
        mk_task_create_idle_task();

        _tick_count = INITIAL_TICK_COUNT;
        _scheduler_running = MK_TRUE;
        PortStartScheduler();
    }
}

static void _configure_sys_tick(void) {
    const mk_u32 systick_reload_value = ((MK_CONFIG_CPU_CLOCK_HZ)/(MK_CONFIG_TICK_RATE_HZ));
    PortConfigureSystemClock(systick_reload_value);
}