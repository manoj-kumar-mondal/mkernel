
/*------------------------- Includes Files -------------------------*/
#include "mk_memory.h"
#include "mkernel.h"

/*----------------------- Typedefs & Macros ------------------------*/
#ifndef MK_CONFIG_MALLOC_FAILED_HOOK
    #define MK_CONFIG_MALLOC_FAILED_HOOK        (0U)
#endif

typedef struct MemBlock {
    mk_size_t block_size;
    struct MemBlock *pnext_fb; // next free-block
} MemBlock_t;

/**
 * The size of the structure placed at the beginning of each allocated memory
 * block must by correctly byte aligned.
 */
#define MEM_BLOCK_SIZE                      ((sizeof(MemBlock_t) + (mk_size_t)(port_WORD_ALIGNMENT - 1)) & \
                                            (~(size_t)port_WORD_ALIGNMENT_MASK))
#define VALIDATE_BLOCK_POINTER(pblock)      ((((mk_u8*)pblock >= &_memory_pool[0]) && \
                                            ((mk_u8*)pblock <= &_memory_pool[MK_CONFIG_MEMORY_POOL_SIZE -1])) ? 1 : 0)

/*----------- Data Region (constants, variables, static) -----------*/
static mk_u8 _memory_pool[MK_CONFIG_MEMORY_POOL_SIZE];
static MemBlock_t _start_block;
static MemBlock_t *_pend_block = MK_NULL;
static mk_size_t _total_remaining_size = 0;

/*------------------ Static Functions Declaration ------------------*/
static void *_allocate_memory(mk_size_t size);
static void _add_block_to_free_list(MemBlock_t *pblock);

/*------------------- All Functions Definitions --------------------*/

/**
 * @brief   Function that initialize the memory for the kernel space
 * @param   none
 * @retval  none
 */
void mk_mem_init(void) {
    port_POINTER_SIZE_TYPE start_address, end_address;
    mk_size_t total_mem_pool_size = MK_CONFIG_MEMORY_POOL_SIZE;
    MemBlock_t *pfirst_free_block;

    start_address = (port_POINTER_SIZE_TYPE)_memory_pool;

    /* check for word alignment and make it aligned if required */
    if (0 != (start_address & port_WORD_ALIGNMENT_MASK)) {
        start_address += (port_WORD_ALIGNMENT - 1);
        start_address &= (~(port_WORD_ALIGNMENT_MASK));
        total_mem_pool_size -= ((mk_size_t)(start_address - (port_POINTER_SIZE_TYPE)_memory_pool));
    }

    /* capture the first block of the memory pool */
    _start_block.pnext_fb = (void*)start_address;
    _start_block.block_size = 0U;
    
    end_address = start_address + (port_POINTER_SIZE_TYPE)total_mem_pool_size;
    end_address -= (port_POINTER_SIZE_TYPE)MEM_BLOCK_SIZE;
    _pend_block = (MemBlock_t*)end_address;

    /* capture the last block of the memory pool */
    _pend_block->block_size = 0U;
    _pend_block->pnext_fb = (MemBlock_t*)MK_NULL;

    /**
     * To start with there is a single free block that is sized to take up the
     * entire heap space, minus the space taken by pxEnd.
     */
    pfirst_free_block = (MemBlock_t*)start_address;
    pfirst_free_block->block_size = (mk_size_t)(end_address - (port_POINTER_SIZE_TYPE)pfirst_free_block);
    pfirst_free_block->pnext_fb = (MemBlock_t*)_pend_block;

    _total_remaining_size = pfirst_free_block->block_size;
}

/**
 * @brief   Function that validate the size wanted and allocate the memory.
 * @param   size(mk_size_t): size wanted to allocate
 * @retval  (void*) pointer to the allocated memory
 */
void *mk_mem_allocate(mk_size_t size) {
    if (size > 0) {
        /* size need to increase to fit MemBlock */
        size += MEM_BLOCK_SIZE;
        
        /* check for the word alignment */
        mk_u8 is_byte_aligned = (size & port_WORD_ALIGNMENT_MASK);
        if (is_byte_aligned) {
            /* add remaining bytes to make it word align */
            size += (port_WORD_ALIGNMENT - is_byte_aligned); 
        }

        void *pmem = _allocate_memory(size);
        return pmem;
    }
    return MK_NULL;
}

/**
 * @brief   Function that free the allocated memory created using mk_mem_allocate.
 * @param   pmem(void*): pointer of the allocated memory
 * @retval  none
 */
void mk_mem_free(void* pmem) {
    MemBlock_t *pblock;

    if (pmem != MK_NULL) {
        pblock = (MemBlock_t*)((mk_u8*)pmem - MEM_BLOCK_SIZE);

        if (VALIDATE_BLOCK_POINTER(pblock) && (pblock->pnext_fb == MK_NULL)) {
            _total_remaining_size += pblock->block_size;
            _add_block_to_free_list(pblock);
        }
    }
}

/**
 * @brief   Function that allocate & manage memory from the memory pool
 * @param   wanted_size(mk_size_t): required size
 * @retval  (void*) returns the pointer of allocated memory
 */
static void *_allocate_memory(mk_size_t wanted_size) {
    MemBlock_t *pprevious_block, *pblock, *pnew_block;
    void *preturn = MK_NULL;

    if (wanted_size < _total_remaining_size) {
        pprevious_block = &_start_block;
        pblock = pprevious_block->pnext_fb;

        /**
         * Trace all the free block and choose the correct block according 
         * to the wanted size and also validate the block.
         */
        while ((pblock->block_size < wanted_size) && (pblock->pnext_fb != MK_NULL)) {
            pprevious_block = pblock;
            pblock = pblock->pnext_fb;
            if (!VALIDATE_BLOCK_POINTER(pblock)) {
                return MK_NULL;
            }
        }

        /**
         * If the endblock reached, then the desired block is not found within mem pool
         */
        if (pblock != _pend_block) {
            preturn = (void*)((mk_u8*)pblock + MEM_BLOCK_SIZE);
            if (!VALIDATE_BLOCK_POINTER(preturn)) {
                return MK_NULL;
            }

            pnew_block = (MemBlock_t*)((mk_u8*)pblock + wanted_size);
            pnew_block->block_size = (pblock->block_size - wanted_size);
            pnew_block->pnext_fb = pblock->pnext_fb;

            pblock->block_size = wanted_size;
            pblock->pnext_fb = MK_NULL;

            pprevious_block->pnext_fb = pnew_block;
            _total_remaining_size -= pblock->block_size;
        }
    }

    // #if (MK_CONFIG_MALLOC_FAILED_HOOK == 1)
    // {
    //     mk_hook_malloc_failed();
    // }
    // #endif

    return preturn;
}

/**
 * @brief   Function that add the block that freeed using my_free to the free list
 * @param   pblock(MemBlock_t): pointer to the memory block
 * @retval  none
 */
static void _add_block_to_free_list(MemBlock_t *pblock) {
    MemBlock_t *pnext_block = &_start_block;
    mk_u8 *pu8 = MK_NULL;

    while(pnext_block->pnext_fb < pblock) {
        /* Traverse through the right and find the correct place of this block */
        pnext_block = pnext_block->pnext_fb;
    }

    pu8 = (mk_u8*)pnext_block;
    /* If pblock and pnext_block are consecutive to each other than merge the blocks to one block. */
    if ((pu8 + pnext_block->block_size) == (mk_u8*)pblock) {
        pnext_block->block_size += pblock->block_size;
        pblock = pnext_block;
    }

    pu8 = (mk_u8*)pblock;
    /**
     * Now again check above block to the next free block
     */
    if ((pu8 + pblock->block_size) == (mk_u8*)pnext_block->pnext_fb) {
        if (pnext_block != _pend_block) {
            pblock->block_size += ((pnext_block->pnext_fb)->block_size);
            pblock->pnext_fb = (pnext_block->pnext_fb)->pnext_fb;
        } else {
            pblock->pnext_fb = _pend_block;
        }
    } else {
        /* Make a link to the next free block */
        pblock->pnext_fb = pnext_block->pnext_fb;
    }

    if (pblock != pnext_block) {
        pnext_block->pnext_fb = pblock;
    }
}