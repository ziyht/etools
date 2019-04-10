/// =====================================================================================
///
///       Filename:  evar.h
///
///    Description:  a struct to hold variant data
///
///        Version:  1.1
///        Created:  04/01/2019 08:51:34 AM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#ifndef __EVAR_H__
#define __EVAR_H__

#include "etype.h"

#pragma pack(1)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum etypev_s{

    __ETYPEV_ARR_MASK = 1 << 5,
    __ETYPEV_PTR_MASK = 1 << 6,
    __ETYPEV_NEW_MASK = 1 << 7,
    __ETYPEV_VAR_MASK = 0x1f  ,     // mostly supported 31 type val
    __E_EVAR          = 0x1f  ,     // internal using

    //! val
    E_NAV =  0,         // not a var

#undef  _XX
#define _XX(type, v)                            \
    E_ ## type       = v,                       \
    E_ ## type ## _a = v |  __ETYPEV_ARR_MASK,  \
    E_ ## type ## _p = v |  __ETYPEV_PTR_MASK

    _XX(CHAR,  1),      // E_CHAR       treat CHAR as I8, they are typically the same

    _XX(I8  ,  1),      // E_I8
    _XX(I16 ,  2),      // E_I16
    _XX(I32 ,  3),      // E_I32
    _XX(I64 ,  4),      // E_I64

    _XX(U8  ,  5),      // E_U8
    _XX(U16 ,  6),      // E_U16
    _XX(U32 ,  7),      // E_U32
    _XX(U64 ,  8),      // E_U64

    _XX(F32 ,  9),      // E_F32
    _XX(F64 , 10),      // E_F64

    _XX(PTR , 11),      // E_PTR
    _XX(STR , 12),      // E_STR        handle as ptr internal
    _XX(RAW , 13),      // E_RAW        handle as ptr internal

    _XX(USER, 14),      // E_USER

}etypev;

#define __EVAR_ITEM_LEN_MAP {0,  1, 2, 4, 8,  1, 2, 4, 8,  4, 8,  8, 8, 8}

//! __ETYPEV_ARR_MASK:
//!
//!   |type|size|cnt|v          |
//!                  (size *cnt)
//!
//! __ETYPEV_PTR_MASK:
//!
//!    8   24    32  8
//!   |type|size|cnt| v  |
//!                   |-------> |   data    |
//!                              (size *cnt)
//!
typedef struct evar_s{
    uint    type :  8;        // type
    uint    size : 24;        // element size
    uint    cnt      ;        // element count
    eval    v        ;        // val(data)
}evar_t, evar, * evarp;

#define EVAR_MK(_t, _p, _v) (evar){.type = _t, .v._p = _v}

#define EVAR_ZORE           EVAR_MK(E_NAV, p, 0)

#define EVAR_I8( v)         EVAR_MK(E_I8 , i8 , (v))
#define EVAR_I16(v)         EVAR_MK(E_I16, i16, (v))
#define EVAR_I32(v)         EVAR_MK(E_I32, i32, (v))
#define EVAR_I64(v)         EVAR_MK(E_I64, i64, (v))

#define EVAR_U8( v)         EVAR_MK(E_U8 , u8 , (v))
#define EVAR_U16(v)         EVAR_MK(E_U16, u16, (v))
#define EVAR_U32(v)         EVAR_MK(E_U32, u32, (v))
#define EVAR_U64(v)         EVAR_MK(E_U64, u64, (v))

#define EVAR_F32(v)         EVAR_MK(E_F32, f32, (v))
#define EVAR_F64(v)         EVAR_MK(E_F64, f64, (v))

#define EVAR_S(  v)         EVAR_MK(E_STR, s, (v))
#define EVAR_P(  v)         EVAR_MK(E_PTR, p, (v))

#define EVAR_CS( v)         EVAR_MK(E_STR, C_s, (v))
#define EVAR_CP( v)         EVAR_MK(E_PTR, C_p, (v))

#define EVAR_RAW(p, l)      (evar){E_RAW | __ETYPEV_PTR_MASK, (l), 1, EVAL_CP(p)}

//! for stack using
evar  evar_gen (etypev t, int cnt, int size);   // create automaticlly, call evar_free() after using it

//! for heap using
evarp evar_new (etypev t, int cnt, int size);   // create automaticlly

uint  evar_cnt  (evarp vp);     // element cnt
uint  evar_size (evarp vp);     // element size
uint  evar_space(evarp vp);     // cnt * size

etypev evar_type( evarp vp);
bool   evar_isArr(evarp vp);
bool   evar_isPtr(evarp vp);

uint  evar_clear(evarp vp);
uint  evar_free (evarp vp);

bool evar_set (evarp vp, uint idx, conptr  in, int ilen);
bool evar_setI(evarp vp, uint idx, i64    val);
bool evar_setF(evarp vp, uint idx, f64    val);
bool evar_setS(evarp vp, uint idx, constr str);
bool evar_setP(evarp vp, uint idx, conptr ptr);
bool evar_setR(evarp vp, uint idx, conptr  in, int ilen);

evar evar_at  (evarp v, uint idx);

evar evar_val (evarp v, uint idx);
i64  evar_valI(evarp v, uint idx);
f64  evar_valF(evarp v, uint idx);
cstr evar_valS(evarp v, uint idx);
cptr evar_valP(evarp v, uint idx);
cptr evar_valR(evarp v, uint idx);

uint evar_lenS(evarp v, uint idx);
uint evar_lenR(evarp v, uint idx);


#ifdef __cplusplus
}
#endif

#pragma pack()

#endif
