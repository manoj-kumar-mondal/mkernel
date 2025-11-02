
/*------------------------- Includes Files -------------------------*/
#include <stdint.h>
#include "port.h"

/*----------------------- Typedefs & Macros ------------------------*/
#define SYST_CSR                            (0x0E000E010)
#define SYST_RVR                            (0x0E000E014)
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

/**
 * @brief   Function is to configure the system clock and set the
 *          reload counter value and enable the systick interrupt
 * @param   systick_reload_value: reload value for tick count
 * @retval  none
 */
void PortConfigureSystemClock(uint32_t systick_reload_value) {
    uint32_t *psystick_rvr = (uint32_t*)SYST_RVR;
    uint32_t *psystick_csr = (uint32_t*)SYST_CSR;

    /* clear and reload the counter according to systick rate */
    *psystick_rvr &= (~(0xFFFFFF)); // 24 bit counter
    *psystick_rvr |= systick_reload_value; // 24 bit counter

    /* Enable the configure the systick */
    *psystick_csr |= (1 << 2); // indicates clock_source = processor clock
    *psystick_csr |= (1 << 1); // enable the systick inteerupt request
}

/**
 * @brief   Function is to start the schedduler from port pov
 * @param   none
 * @retval  none
 */
void PortStartScheduler(void) {

}

void SysTick_Handler(void) {

}