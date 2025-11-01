
#ifndef _MK_MEMORY_H_
#define _MK_MEMORY_H_

#include "mk_defs.h"

/*---------------------- Function Declaration ----------------------*/
void mk_mem_init(void);
void *mk_mem_allocate(mk_size_t size);
void mk_mem_free(void* pmem);

#endif // _MK_MEMORY_H_