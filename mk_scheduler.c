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
static mk_ScheduleType_t _schedule_algo = e_roundrobin;

/*------------------ Static Functions Declaration ------------------*/
static void _configure_sys_tick(void);

/*------------------ Extern Function Declarations ------------------*/
extern void e_mk_task_create_idle_task(void);

/*------------------- All Functions Definitions --------------------*/

/**
 * @brief   Function is to start the mkernel scheduler.
 * @param   schedule_type(mk_ScheduleType_t): scheduling algo
 * @retval  none
 */
void mk_scheduler_start(mk_ScheduleType_t schedule_type) {

    _scheduler_running = MK_TRUE;
    _tick_count = INITIAL_TICK_COUNT;
    _schedule_algo = schedule_type;

    e_mk_task_create_idle_task();
    _configure_sys_tick();
    PortStartScheduler();
    
}

/**
 * @brief   Function that returns current sys tick count
 * @param   none
 * @retval  current systick count
 */
mk_u32 get_systick_count(void) {
    return _tick_count;
}

/**
 * @brief   Function is to configure the system's systick
 * @param   none
 * @retval  none
 */
static void _configure_sys_tick(void) {
    const mk_u32 systick_reload_value = ((MK_CONFIG_CPU_CLOCK_HZ)/(MK_CONFIG_TICK_RATE_HZ));
    PortConfigureSystemClock(systick_reload_value);
}


/*------------------ Extern Function Definitions -------------------*/

/**
 * @brief   Function is to add a nrewly create mkernel task into scheduler.
 * @param   ptcb(TCB_t*) task's TCB
 * @retval  none
 */
void e_mk_scheduler_add_task_to_list(TCB_t *ptcb) {
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
 * @brief   Function that increment the current rtos tick by '1' also
 *          checks the context switch is required or not
 * @param   none
 * @retval  context switch is required or not 
 */
mk_bool e_mk_scheduler_increment_tick(void) {
    mk_bool switch_context = MK_FALSE;
    _tick_count += (TickType_t)1;

    if (e_roundrobin == _schedule_algo) {
        // switch_context = MK_TRUE;
    } else if (e_priorityPremption == _schedule_algo) {

    } else if (e_cooperative == _schedule_algo) {

    }
    return switch_context;
}

/**
 * @brief   Function that return's the current running task's stack pointer
 * @param   none
 * @retval  stackpointer value of current running task
 */
mk_u32 e_mk_scheduler_current_task_sp(void) {
    return (mk_u32)pcurrent_task->ptcb->ptop_of_stack;
}

/**
 * @brief   Function that run the first task while schedule starts
 * @param   none
 * @retval  none
 */
void e_mk_scheduler_run_first_task(void) {
    TaskFunction pTaskFunction = (TaskFunction)pcurrent_task->ptcb->pfunc;
    pTaskFunction(NULL);
}