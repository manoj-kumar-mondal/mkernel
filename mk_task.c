
/*------------------------- Includes Files -------------------------*/
#include "mk_task.h"
#include "mk_memory.h"
#include "mk_scheduler.h"
#include <string.h>
#include <stdio.h>

/*----------------------- Typedefs & Macros ------------------------*/
#define STACK_FILL_BYTE                     (0xA5U)
#define RESERVED_MINIMUM_TASK_PRIORITY      (0U)

/*----------- Data Region (constants, variables, static) -----------*/

/*------------------ Static Functions Declaration ------------------*/
static TCB_t *_create_new_task(mk_TaskInit_t *ptask_init);
static void _initialize_new_task(mk_TaskInit_t *ptask_init, TCB_t *ptcb);
static void _Idle_task_func(void *param);

/*------------------ Extern Function Declarations ------------------*/
extern void e_mk_scheduler_add_task_to_list(TCB_t *ptcb);

/*------------------- All Functions Definitions --------------------*/

/**
 * @brief   Function is to create the user task
 * @param   ptask_init(mk_TaskInit_t): task init structure.
 * @retval  none
 */
mk_i32 mk_task_create(mk_TaskInit_t *ptask_init) {
    TCB_t *pnew_tcb = MK_NULL;
    pnew_tcb = _create_new_task(ptask_init);

    if (pnew_tcb != MK_NULL) {
        /* if tcb allocated then add new task to ready list */
        e_mk_scheduler_add_task_to_list(pnew_tcb);
        return MK_SUCCESS;
    }
    return MK_FAILURE;
}

/**
 * @brief   Private function that create new task
 * @param   ptask_init(mk_TaskInit_t): task init structure.
 * @retval  newly created tcb. 
 */
static TCB_t *_create_new_task(mk_TaskInit_t *ptask_init) {
    TCB_t *ptcb = MK_NULL;

    /* allocate memory for the task private stack */
    StackType_t *pstack = (StackType_t*)mk_mem_allocate( \
        (mk_size_t)ptask_init->stack_depth * (mk_size_t)sizeof(StackType_t));
        
        if (pstack != MK_NULL) {
        /* allocate memory for the tcb */
        ptcb = (TCB_t*)mk_mem_allocate(sizeof(TCB_t));

        if (ptcb != MK_NULL) {
            ptcb->pstack = pstack;
            /* initialize the task with default values */
            _initialize_new_task(ptask_init, ptcb);
        }
    }
    return ptcb;
}

/**
 * @brief   Function is to initialize the newly created task.
 * @param   ptask_init(mk_TaskInit_t): task init structure.
 * @param   ptcb(TCB_T*): pointer to the TCB.
 * @retval  none
 */
static void _initialize_new_task(mk_TaskInit_t *ptask_init, TCB_t *ptcb) {
    StackType_t *ptop_of_stack;
    mk_u8 index;

    /* Fill the stack with pre-detetmine value of 0xA5, before use */
    memset((void*)ptcb->pstack, STACK_FILL_BYTE, \
        (mk_size_t)ptask_init->stack_depth * (mk_size_t)sizeof(StackType_t));
    
    /**
     * As the stack grows from high memory to low memory, so end of the task would be
     * the last address of the allocated space for the stack.
     */
    ptop_of_stack = (StackType_t*)&(ptcb->pstack[ptask_init->stack_depth - (StackType_t)1]);
    /* make it align */
    ptop_of_stack = (StackType_t*)((port_POINTER_SIZE_TYPE)ptop_of_stack & (~((port_POINTER_SIZE_TYPE)port_WORD_ALIGNMENT_MASK)));

    /* Save the provided task name */
    if (ptask_init->ptask_name != MK_NULL) {
        for (index = 0 ; index < (mk_u8)MK_CONFIG_TASK_NAME_MAX_LENGTH ; index++)
        {
            char item = ptask_init->ptask_name[index];
            ptcb->task_name[index] = item;
            if (item == (char)0x00)
                break;
        }

        ptcb->task_name[MK_CONFIG_TASK_NAME_MAX_LENGTH - 1] = '\0';
    }

    /* Set the task priority */
    if (ptask_init->priority > MK_CONFIG_MAX_PRIORITIES) {
        ptask_init->priority = MK_CONFIG_MAX_PRIORITIES;
    }
    ptcb->priority = ptask_init->priority;

    /* Save the function pointer */
    ptcb->pfunc = (void*)ptask_init->task_func;

    /* Set the top od stack after initializtion*/
    ptcb->ptop_of_stack = PortInitializeStackSpace(ptop_of_stack, ptask_init->task_func);
}

/**
 * @brief   Idle function
 * @param   none
 * @retval  none
 */
static void _Idle_task_func(void *param) {
    while(1) {
        printf("Idle ...\n");
        #ifdef MK_CONFIG_IDLE_TASK_HOOK
            ApplicationIdleTaskHook();
        #endif
    }
}

/*------------------------ Extern Functions ------------------------*/

/**
 * @brief   Function that create idle task by the scheduler
 * @param   none
 * @retval  none
 */
void e_mk_task_create_idle_task(void) {
    mk_TaskInit_t idle_task = {
        .priority = RESERVED_MINIMUM_TASK_PRIORITY,
        .stack_depth = MK_CONFIG_MINIMAL_STACK_SIZE,
        .ptask_name = "idle_task",
        .task_func = _Idle_task_func,
    };

    mk_task_create(&idle_task);

}