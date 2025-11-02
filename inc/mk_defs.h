
#ifndef _MK_DEFS_H_
#define _MK_DEFS_H_

/*------------------------- Includes Files -------------------------*/
#include <stdint.h>
#include <stddef.h>

/*----------------------- Typedefs & Macros ------------------------*/
typedef uint8_t         mk_u8;
typedef int8_t          mk_i8;
typedef uint16_t        mk_u16;
typedef int16_t         mk_i16;
typedef uint32_t        mk_u32;
typedef int32_t         mk_i32;
typedef uint64_t        mk_u64;
typedef int64_t         mk_i64;
typedef uint8_t         mk_bool;
typedef size_t          mk_size_t;

#define MK_TRUE        ((mk_bool)1)
#define MK_FALSE       ((mk_bool)0)
#define MK_NULL        ((void*)0)
#define MK_SUCCESS     ((mk_i32)0)
#define MK_FAILURE     ((mk_i32)-1)

typedef void (*TaskFunction)(void*);

#endif // _MK_DEFS_H_