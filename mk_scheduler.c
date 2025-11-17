/*------------------------- Includes Files -------------------------*/
#include "mk_scheduler.h"
#include "mk_memory.h"

/*----------------------- Typedefs & Macros ------------------------*/
#define INITIAL_TICK_COUNT              (0U)
#define INITIAL_TASK_ID                 (0U)

typedef struct TaskHandlerForScheduler{
    mk_u8 task_id;
    mk_TaskState_t state;
    TCB_t *ptcb;
    struct TaskHandlerForScheduler *pnext;
} TaskHandlerForScheduler_t;

/*----------- Data Region (constants, variables, static) -----------*/
static TaskHandlerForScheduler_t start_task_block = {
    .pnext = MK_NULL,
    .ptcb = MK_NULL,
    .state = e_deleted,
    .task_id = INITIAL_TASK_ID
}; // this block is dedicated to idle task

static TaskHandlerForScheduler_t *pcurrent_task = &start_task_block;
static TaskHandlerForScheduler_t *pnext_task = MK_NULL;
static mk_u8 _total_task_created = 1U;
static mk_bool _scheduler_running = MK_FALSE;
static TickType_t _tick_count;
static mk_ScheduleType_t _schedule_algo = e_roundrobin;

/*------------------ Static Functions Declaration ------------------*/
static void _configure_sys_tick(void);
static mk_bool _is_context_switch_required(void);

/*------------------ Extern Function Declarations ------------------*/
extern TCB_t *e_mk_task_create_idle_task(void);

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

    /* initialize idle task */
    start_task_block.ptcb = e_mk_task_create_idle_task();
    start_task_block.state = e_ready;

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

/**
 * @brief   Function that keep monitor all the tasks in each tick
 * @param   none
 * @retval  none
 */
static void _tasks_monitor(void) {
    TaskHandlerForScheduler_t *pcurr = start_task_block.pnext;

    while(pcurr) {
        if (e_blocked == pcurr->state) {
            if (pcurr->ptcb->block_time == 0) {
                pcurr->state = e_ready;
            } else {
                pcurr->ptcb->block_time--;
            }
        }
        pcurr = pcurr->pnext;
    }
}

/**
 * @brief   Function that decide with task to run and return the 
 */
static mk_bool _is_context_switch_required(void) {
    TaskHandlerForScheduler_t *pnext = pcurrent_task->pnext;
    mk_u8 loop_index = 0U, loop_max_count = (_total_task_created - 1U);

    /* if loop complete then the pointer reached the place from where it is started */
    for ( ; loop_index <  loop_max_count ; loop_index++) {
        if (pnext == MK_NULL) {
            pnext = start_task_block.pnext;
        }

        if (e_ready == pnext->state) {
            break;
        }
        pnext = pnext->pnext; // what a line !!!
    }

    if (loop_index == loop_max_count) { // pointer completes one cycle
        if (e_running == pcurrent_task->state) {
            return MK_FALSE;
        } else {
            pnext_task = &start_task_block;
            return MK_TRUE;
        }
    } else {
    	pnext_task = pnext;
        return MK_TRUE;
    }
}

/*------------------ Extern Function Definitions -------------------*/

/**
 * @brief   Function is to add a nrewly create mkernel task into scheduler.
 * @param   ptcb(TCB_t*) task's TCB
 * @retval  none
 */
void e_mk_scheduler_add_task_to_list(TCB_t *ptcb) {
    /* Individual Taskblock for scheduler */
    TaskHandlerForScheduler_t *phandler = (TaskHandlerForScheduler_t*)mk_mem_allocate(sizeof(TaskHandlerForScheduler_t));

    if (MK_NULL != phandler) {
        /* provide default values */
        phandler->ptcb = ptcb;
        phandler->state = e_ready;
        phandler->pnext = MK_NULL;
        phandler->task_id = _total_task_created++;

        TaskHandlerForScheduler_t *ptemp = start_task_block.pnext;

        /* add new block in a list */
        if (MK_NULL == ptemp) {
            /**
             * newly created task will be added after the start block and
             * the start block is always dedicated to idle task.
             */
            start_task_block.pnext = phandler;
        } else {
            while(ptemp->pnext != MK_NULL) {
                ptemp = ptemp->pnext;
            }
            ptemp->pnext = phandler;
        }
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
    _tasks_monitor();
    switch_context = _is_context_switch_required();
    return switch_context;
}

mk_u32 e_mk_scheduler_get_curr_psp(void) {
    if ((MK_NULL == pnext_task) && (pcurrent_task == &start_task_block)) {
        if (_is_context_switch_required()) {
            pcurrent_task = pnext_task;
            if (e_ready == pcurrent_task->state) {
                pcurrent_task->state = e_running;
            }
        }
    }
    return (mk_u32)pcurrent_task->ptcb->ptop_of_stack;
}

mk_u32 e_mk_scheduler_switch_psp(mk_u32 currentpsp) {

    /* save the updated top of stack of the current running task */
    pcurrent_task->ptcb->ptop_of_stack = (StackType_t*)currentpsp;
    if (e_running == pcurrent_task->state) {
        pcurrent_task->state = e_ready;
    }

    pcurrent_task = pnext_task;
    pcurrent_task->state = e_running;
    return (mk_u32)pcurrent_task->ptcb->ptop_of_stack;
}

/**
 * @brief   Function that run the first task while schedule starts
 * @param   none
 * @retval  none
 */
mk_u32 e_mk_scheduler_run_first_task(void) {
    TaskFunction pTaskFunction = (TaskFunction)pcurrent_task->ptcb->pfunc;
    return (mk_u32)pTaskFunction;
}

/**
 * @brief   Function that block the task for time specified
 * @param   timeout(TickType_t)
 * @retval  none
 */
void mk_task_delay(TickType_t time_out) {
    pcurrent_task->ptcb->block_time = time_out;
    pcurrent_task->state = e_blocked;
    
    if (_is_context_switch_required()) {
        PortDoContextSwitch();
    }
}
