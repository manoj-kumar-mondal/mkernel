
#ifndef _MK_TASK_H_
#define _MK_TASK_H_

/*------------------------- Includes Files -------------------------*/
#include "mk_defs.h"
#include "mkernel.h"

/*----------------------- Typedefs & Macros ------------------------*/
typedef struct {
    StackType_t *ptop_of_stack;
    StackType_t *pstack;
    mk_u8 priority;
    char task_name[MK_CONFIG_TASK_NAME_MAX_LENGTH];
} TCB_t;

typedef struct {
    TaskFunction task_func;
    const char *ptask_name;
    /**
     * The value will considered as words, not in bytes,
     * if value 100 is provided and the word size is 4 bytes, then 400 total bytes will be allocated.
     */
    const StackType_t stack_depth;
    mk_u8   priority;
} mk_TaskInit_t;

/*---------------------- Function Declaration ----------------------*/
mk_i32 mk_task_create(mk_TaskInit_t *ptask_init);
void mk_task_create_idle_task(void);

#endif // _MK_TASK_H_