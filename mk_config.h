
#ifndef _MK_CONFIG_H_
#define _MK_CONFIG_H_

#define MK_CONFIG_MINIMAL_STACK_SIZE            (100)
#define MK_CONFIG_MAX_PRIORITIES                (4U)
#define MK_CONFIG_USE_PREEMPTION                (1U)
#define MK_CONFIG_MEMORY_POOL_SIZE              (8U * 1024U)

#define MK_CONFIG_CPU_CLOCK_HZ                  (16000000UL)
#define MK_CONFIG_TICK_RATE_HZ                  (1000U)
#endif //_MK_CONFIG_H_