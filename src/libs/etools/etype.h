/// =====================================================================================
///
///       Filename:  etype.h
///
///    Description:  types for etools
///
///        Version:  1.1
///        Created:  03/09/2017 08:51:34 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#ifndef __ETYPE_H__
#define __ETYPE_H__

#define ETYPE_VERSION "etype 1.0.1"   // s64 -> i64

#include <stdbool.h>

#ifndef __DEF_PTR__
#define __DEF_PTR__
typedef const char* constr;
typedef const void* conptr;
typedef char* cstr;
typedef void* cptr;
#endif

#ifndef __DEF_INT__
#define __DEF_INT__
#define __signed

#if defined(_MSC_VER) && _MSC_VER < 1600
typedef __signed       char int8_t;
typedef unsigned       char uint8_t;
typedef __signed short int  int16_t;
typedef unsigned short int  uint16_t;
typedef __signed       int  int32_t;
typedef unsigned       int  uint32_t;
typedef __signed long  long int64_t;
typedef unsigned long  long uint64_t;
#else
#include <stdint.h>
#endif

#include <limits.h>
#include <inttypes.h>

typedef unsigned long  int ulong;
typedef unsigned short int ushort;

typedef   int8_t i8;
typedef  uint8_t u8;
typedef  int16_t i16;
typedef uint16_t u16;
typedef  int32_t i32;
typedef uint32_t u32, uint;
typedef  int64_t i64;
typedef uint64_t u64;

#ifndef _ASM_GENERIC_INT_LL64_H
#define _ASM_GENERIC_INT_LL64_H
typedef   int8_t __i8;
typedef  uint8_t __u8;
typedef  int16_t __i16;
typedef uint16_t __u16;
typedef  int32_t __i32;
typedef uint32_t __u32;
typedef  int64_t __i64;
typedef uint64_t __u64;
#endif

typedef const unsigned char conu8;
typedef const __signed char coni8;

typedef float  f32, __f32;
typedef double f64, __f64;

#undef  PRId64
#undef  PRIu64

#define PRId64 "ld"
#define PRIu64 "lu"

#endif

#ifndef __DEF_TYPE__
#define __DEF_TYPE__

#include <sys/types.h>

typedef size_t size;

#endif

typedef union eval_s{
    __i8      i8,  i8_[8];
    __u8      u8,  u8_[8];
    __i16    i16, i16_[4];
    __u16    u16, u16_[4];
    __i32    i32, i32_[2];
    __u32    u32, u32_[2];
    __i64    i64, i64_[1], i;
    __u64    u64, u64_[1], u;
    __f32    f32, f32_[2];
    __f64    f64, f64_[1], f;

    cptr     p, p_[1];
    cstr     s, s_[1];
    char     r[1];
}eval_t, eval, * evalp;

#endif
