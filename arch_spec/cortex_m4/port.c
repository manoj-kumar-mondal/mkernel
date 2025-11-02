
/*------------------------- Includes Files -------------------------*/
#include <stdint.h>
#include "port.h"

/*----------------------- Typedefs & Macros ------------------------*/
#define port_INITIAL_XPSR_VALUE             (0x01000000U)
#define port_TASK_RETURN_ADDRESS            (0xFFFFFFFDU)

/*----------- Data Region (constants, variables, static) -----------*/

/*------------------ Static Functions Declaration ------------------*/

/*------------------- All Functions Definitions --------------------*/

/**
 * @brief   port function that inialize the private stack with default value
 * @param   ptop_of_stack: top of the stack
 * @param   pcode: Task Function address
 * @retval  none
 */
StackType_t *PortInitializeStackSpace(StackType_t *ptop_of_stack, void* pcode) {
    ptop_of_stack--;
    *ptop_of_stack = port_INITIAL_XPSR_VALUE; /* XPSR Value*/

    ptop_of_stack--;
    *ptop_of_stack = (StackType_t)pcode; /* PC */

    ptop_of_stack--;
    *ptop_of_stack = port_TASK_RETURN_ADDRESS;

    for (uint8_t index = 0 ; index < (5+8) ; index++) {
        ptop_of_stack--;
        *ptop_of_stack = 0x0; /* R12, R3-R0, R4-R11*/
    }
    return ptop_of_stack;
}
