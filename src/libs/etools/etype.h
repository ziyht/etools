#ifndef __ETYPE_H__
#define __ETYPE_H__

#ifndef __DEF_PTR__
#define __DEF_PTR__
typedef const char* constr;
typedef const void* conptr;
typedef char* cstr;
typedef void* cptr;
#endif

#ifndef __DEF_INT__
#define __DEF_INT__
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef unsigned       char u8;
typedef unsigned short int  u16;
typedef unsigned       int  u32, uint;
typedef unsigned long  long u64;
typedef                char s8;
typedef          short int  s16;
typedef                int  s32;
typedef          long  long s64;
#else
#include <stdint.h>
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32, uint;
typedef uint64_t u64;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef const unsigned char cuchar;
typedef const char cschar;

#endif
#endif

#ifndef __DEF_TYPE__
#define __DEF_TYPE__

#include <sys/types.h>

typedef size_t size;

#endif

#endif
