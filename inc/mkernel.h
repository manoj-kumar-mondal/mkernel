
#ifndef _MK_KERNEL_H_
#define _MK_KERNEL_H_

#include "port.h"
#include "mk_config.h"

#ifndef MK_CONFIG_MINIMAL_STACK_SIZE
    #error Missing Definition: MK_CONFIG_MINIMAL_STACK_SIZE must be define in mk_config.h file.  configMINIMAL_STACK_SIZE defines the size (in words) of the stack allocated to the idle task
#endif

#ifndef MK_CONFIG_MAX_PRIORITIES
    #error Missing Definition:  MK_CONFIG_MAX_PRIORITIES must be define in mk_config.h file and atleast 1.
#endif

#ifndef MK_CONFIG_USE_PREEMPTION
    #error Missing Definition:  MK_CONFIG_USE_PREEMPTION must be define in mk_config.h file either 0 or 1.
#endif

#ifndef MK_CONFIG_TASK_NAME_MAX_LENGTH
    #define MK_CONFIG_TASK_NAME_MAX_LENGTH      (12U)
#endif

#ifndef MK_CONFIG_MEMORY_POOL_SIZE
    #define MK_CONFIG_MEMORY_POOL_SIZE          (1024)
#endif

#ifndef port_POINTER_SIZE_TYPE
    #define port_POINTER_SIZE_TYPE              mk_u32
#endif

#endif // _MK_KERNEL_H_