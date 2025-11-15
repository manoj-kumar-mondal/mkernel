
#ifndef _MK_PORT_H_
#define _MK_PORT_H_

/*----------------------- Typedefs & Macros ------------------------*/
#define port_WORD_ALIGNMENT                         (4U) // 4 bytes
#define port_WORD_ALIGNMENT_MASK                    ((port_WORD_ALIGNMENT) - 1)
#define port_POINTER_SIZE_TYPE                      uint32_t
#define port_Stack_Type                             uint32_t
#define port_Tick_Type                              uint32_t

#define port_svc_handler                            SVC_Handler
#define port_systick_handler                        SysTick_Handler
#define port_pendsv_handler                         PendSV_Handler

typedef port_Stack_Type StackType_t;
typedef port_Tick_Type  TickType_t;

/*---------------------- Function Declaration ----------------------*/
StackType_t *PortInitializeStackSpace(StackType_t *ptop_of_stack, void* pcode);
void PortConfigureSystemClock(uint32_t systick_reload_value);
void PortStartScheduler(void);
void PortDoContextSwitch(void);

#endif // _MK_PORT_H_