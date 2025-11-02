
#ifndef _MK_PORT_H_
#define _MK_PORT_H_

/*----------------------- Typedefs & Macros ------------------------*/
#define port_WORD_ALIGNMENT                         (4U) // 4 bytes
#define port_WORD_ALIGNMENT_MASK                    ((port_WORD_ALIGNMENT) - 1)
#define port_POINTER_SIZE_TYPE                      uint32_t
#define port_Stack_Type                             uint32_t

typedef port_Stack_Type StackType_t;

/*---------------------- Function Declaration ----------------------*/
StackType_t *PortInitializeStackSpace(StackType_t *ptop_of_stack, void* pcode);

#endif // _MK_PORT_H_