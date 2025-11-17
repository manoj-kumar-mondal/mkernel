
/*------------------------- Includes Files -------------------------*/
#include <stdint.h>
#include "port.h"
#include "mk_defs.h"

/*----------------------- Typedefs & Macros ------------------------*/
#define SYST_CSR                            (0xE000E010U)
#define SYST_RVR                            (0xE000E014U)
#define ICSR_ADDR                           (0xE000ED04U)

#define port_PENDSV_BIT_POSITION            (28U)

#define port_INITIAL_XPSR_VALUE             (0x01000000U)
#define port_TASK_RETURN_ADDRESS            (0xFFFFFFFDU)

#define enable_sys_tick_counter()           do { \
                                                uint32_t *psyst_csr = (uint32_t*)SYST_CSR; \
                                                *psyst_csr |= (1 << 0); \
                                            } while(0)

#define port_pendsv_set_bit()               do { \
                                                uint32_t *pitrp_ctrl_reg = (uint32_t*)ICSR_ADDR; \
                                                *pitrp_ctrl_reg |= (1 << port_PENDSV_BIT_POSITION); \
                                            } while(0)

/*----------- Data Region (constants, variables, static) -----------*/
extern TickType_t _tick_count;

/*------------------ Static Functions Declaration ------------------*/
static void _set_psp_as_sp(void) __attribute__((naked));
static void _switch_context(void) __attribute__((naked));

/*------------------ Extern Function Declarations ------------------*/
extern mk_bool e_mk_scheduler_increment_tick(void);
extern mk_u32 e_mk_scheduler_switch_psp(mk_u32 currentpsp);
extern mk_u32 e_mk_scheduler_run_first_task(void);
extern mk_u32 e_mk_scheduler_get_curr_psp(void);

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
    enable_sys_tick_counter();
    _set_psp_as_sp();
}

void PortDoContextSwitch(void) {
    port_pendsv_set_bit(); // set the bit for pendsv handler
}

/**
 * @brief Systick handler
 */
void port_systick_handler(void) {
    if (MK_TRUE == e_mk_scheduler_increment_tick()) {
        PortDoContextSwitch();
    }
}

void port_pendsv_handler(void) {
    _switch_context();
}

static void _set_psp_as_sp(void) {
    __asm volatile ("push {lr}"); // push the lr to msp
    __asm volatile ("bl e_mk_scheduler_get_curr_psp"); // get the current running task's sp
    __asm volatile ("msr psp, r0"); // set the psp as task's sp

    __asm volatile ("bl e_mk_scheduler_run_first_task"); // extract first task pc to run
    __asm volatile ("mov r1, r0"); // storing the pc at r1
    
    __asm volatile ("pop {lr}"); // before change the sp to psp, retrieve lr from msp
    __asm volatile ("mov r0, #0x02"); // set 2nd bit for to change sp to psp
    __asm volatile ("msr control, r0"); // now psp is used as sp

    __asm volatile ("mov r0, #0x00"); // set first argument as NULL
    __asm volatile ("bx r1"); // branch to first task
}

static void _switch_context(void) {
    __asm volatile ("push {lr}");
    __asm volatile ("mrs r0, psp");
    __asm volatile ("stmdb r0!, {r4-r11}"); // stacking in current psp
    __asm volatile ("bl e_mk_scheduler_switch_psp"); // function that save the cuurent psp and return new psp    
    __asm volatile ("ldmia r0!, {r4-r11}"); // unstacking in new psp
    __asm volatile ("msr psp, r0");
    __asm volatile ("pop {lr}");
    __asm volatile ("bx lr");
}
