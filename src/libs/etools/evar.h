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
#undef  _XX

#define __EVAR_ITEM_LEN_MAP {0,  1, 2, 4, 8,  1, 2, 4, 8,  4, 8,  8, 8, 8}

//! __ETYPEV_ARR_MASK:
//!
//!   |type|size|cnt|v          |
//!                  (size *cnt)
//!
//! __ETYPEV_PTR_MASK:
//!
//!    1    2    4   8
//!   |type|size|cnt|ptr|
//!                   |-------> |   data    |
//!                              (size *cnt)
//!
typedef struct evar_s{
    u8      __;             // unused
    u8      type  ;         // type
    u16     esize ;         // element size
    uint    cnt   ;         // element count
    eval    v     ;         // val(data)
}evar_t, evar, * evarp;

#define EVAR_NAV            (evar){.type = E_NAV, .v = EVAL_ZORE }

#define EVAR_I8( v)         (evar){0, E_I8 , 1, 1, EVAL_I8 (v)}
#define EVAR_I16(v)         (evar){0, E_I16, 2, 1, EVAL_I16(v)}
#define EVAR_I32(v)         (evar){0, E_I32, 4, 1, EVAL_I32(v)}
#define EVAR_I64(v)         (evar){0, E_I64, 8, 1, EVAL_I64(v)}

#define EVAR_U8( v)         (evar){0, E_U8 , 1, 1, EVAL_U8 (v)}
#define EVAR_U16(v)         (evar){0, E_U16, 2, 1, EVAL_U16(v)}
#define EVAR_U32(v)         (evar){0, E_U32, 4, 1, EVAL_U32(v)}
#define EVAR_U64(v)         (evar){0, E_U64, 8, 1, EVAL_U64(v)}

#define EVAR_F32(v)         (evar){0, E_F32, 4, 1, EVAL_F32(v)}
#define EVAR_F64(v)         (evar){0, E_F64, 8, 1, EVAL_F64(v)}

#define EVAR_S(  v)         (evar){0, E_STR, 8, 1, EVAL_S(v)}
#define EVAR_P(  v)         (evar){0, E_PTR, 8, 1, EVAL_P(v)}

#define EVAR_CS( v)         (evar){0, E_STR, 8, 1, EVAL_CS(v)}
#define EVAR_CP( v)         (evar){0, E_PTR, 8, 1, EVAL_CP(v)}

#define EVAR_RAW(p, l)      (evar){0, E_RAW | __ETYPEV_PTR_MASK, (l), 1, EVAL_CP(p)}

//! for stack using
evar  evar_gen (etypev t, int cnt, int size);   // create automaticlly, call evar_free() after using it

//! for heap using
evarp evar_new (etypev t, int cnt, int size);   // create automaticlly

uint  evar_cnt  (evarp vp);     // element cnt
uint  evar_esize(evarp vp);     // element size
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

evar evar_at  (evarp vp, uint idx);

evar evar_val (evarp vp, uint idx);         // do not free the returned var
i64  evar_valI(evarp vp, uint idx);
f64  evar_valF(evarp vp, uint idx);
cstr evar_valS(evarp vp, uint idx);
cptr evar_valP(evarp vp, uint idx);
cptr evar_valR(evarp vp, uint idx);

uint evar_lenS(evarp vp, uint idx);
uint evar_lenR(evarp vp, uint idx);


#ifdef __cplusplus
}
#endif

#pragma pack()

#endif
