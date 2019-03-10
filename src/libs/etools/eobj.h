/// =====================================================================================
///
///       Filename:  eobj.h
///
///    Description:  some shared obj operation for elist, erb..., for internal use.
///
///        Version:  1.0
///        Created:  06/09/2017 05:15:18 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#ifndef __EOBJ_H__
#define __EOBJ_H__

#define EOBJ_VERSION     "eobj 1.0.4"   // update eobj, working...

#include "etype.h"

#ifdef __cplusplus
extern "C" {
#endif

//! eobj container type
typedef enum{
    EJSON   = 0x01u,    // (1) json
    ELL     ,           // (2) linklist
    EDICT   ,           // (3) dict
    ERB     ,           // (4) rbtree
    ESL     ,           // (5) skiplist
    EQL     ,           // (6) quicklist
    EZL     ,           // (7) ziplist
    EVEC    ,           // (8) vector

    ECONTAINER_UNKNOWN = 0
}etypec;

//! eobj val type
typedef enum{
    EFALSE = 0x00u ,    // 0
    ETRUE  ,            // 1
    ENULL  ,            // 2
    ENUM   ,            // 3
    EPTR   ,            // 4
    ESTR   ,            // 5
    ERAW   ,            // 6
    EOBJ   ,            // 7

    EOBJ_UNKNOWN  = 0x0f
}etypeo;

typedef enum{
    EKEY_NO = 0,
    EKEY_I  = 1,
    EKEY_U  = 2,
    EKEY_S  = 3,
}etypek;

/// ---------------------- ecan -------------------------
///
///     cantainer type for obj using;
///

typedef struct _ell_s  * ell;
typedef struct _edict_s* edict;
typedef struct _erb_s  * erb;

typedef union ecan_s{
    ell     ll;
    edict   dict;
    erb     rb;
}ecan_t, * ecan;

etypec ecan_typec(ecan c);

/// ---------------------- ekey -------------------------
///
///     key type for obj using;
///

typedef union __ekey_s{
    u64    u;
    i64    i;
    cptr   p;
    cstr   s;
}ekey_t, ekey;

//#define __ __always_inline
//static __ ekey ekey_i(u64    i) { return *(ekey_t*)&i; }
//static __ ekey ekey_s(constr s) { return *(ekey_t*)&s; }
//#undef __

#define ekey_i(I) (ekey){.i = I}
#define ekey_s(S) (ekey){.s = S}


/// ---------------------- eobj -------------------------
///
///     a obj type for elist, erb, edict...;
///

typedef union eobj_s{
    i64     i;          // i64
    u64     u;          // u64
    f64     f;          // f64
    cptr    p;          // ptr
    cstr    s;          // str
    char    r[1];       // raw
    eval_t  v;          // val
}eobj_t, *eobj;

typedef void (*EOBJ_RLS_CB)(eobj);

typedef eobj (*eobj_init_cb)(eobj o);                       /// init eobj
typedef void (*eobj_rls_cb )(eobj o);                       /// release eobj's data, note: do not free @param o
typedef int  (*eobj_cmp_cb )(eobj a, eobj b);               /// compare two eobj, we assume returned -1 when @param a less than @param b

typedef eobj (*eobj_init_ex_cb)(eobj o, eval prvt);         /// init eobj
typedef void (*eobj_rls_ex_cb )(eobj o, eval prvt);         /// release eobj's data, note: do not free @param o
typedef int  (*eobj_cmp_ex_cb )(eobj a, eobj b, eval prvt); /// compare two eobj, we assume returned -1 when @param a less than @param b

eobj  eobj_setKeyI(eobj o, i64    key);
eobj  eobj_setKeyS(eobj o, constr key);

/// - if you kown the detail of eobj, you can using those macros to get the value directly
#define EOBJ_VALI(o) (o)->i
#define EOBJ_VALF(o) (o)->f
#define EOBJ_VALS(o) ((cstr)(o))
#define EOBJ_VALP(o) (o)->p
#define EOBJ_VALR(o) ((cptr)(o))

#define __ __always_inline

/// - else, you can using the following APIs
static __ ekey   eobj_key  (eobj obj) { return obj ? ((ekey* )obj)[-2]    : (ekey){0}; }
static __ i64    eobj_keyI (eobj obj) { return obj ? ((i64*  )obj)[-2]    :  0; }
static __ cstr   eobj_keyS (eobj obj) { return obj ? (((ekey*)obj)[-2].s) :  0; }
static __ etypeo eobj_typeo(eobj obj) { typedef struct __type{ uint _0:16; uint _1:4; uint t_o : 4; uint _2:8;} __type_t; return obj ? ((__type_t*)obj)[-1].t_o : -1; }
static __ etypec eobj_typec(eobj obj) { typedef struct __type{ uint _0:16; uint _1:4; uint t_o : 4; uint _2:8;} __type_t; return obj ? ((__type_t*)obj)[-1]._1  : -1; }
static __ uint   eobj_len  (eobj obj) { return obj ? ((uint*)obj)[-2] :  0; }
static __ i64    eobj_valI (eobj obj) { typedef struct __type{ uint _0:16; uint _1:4; uint t_on: 5; uint _2:7;} __type_t; if(obj) switch (((__type_t*)obj)[-1].t_on) { case ENUM : return EOBJ_VALI(obj); case ENUM | 1<<4: return EOBJ_VALF(obj); } return   0; }
static __ f64    eobj_valF (eobj obj) { typedef struct __type{ uint _0:16; uint _1:4; uint t_on: 5; uint _2:7;} __type_t; if(obj) switch (((__type_t*)obj)[-1].t_on) { case ENUM : return EOBJ_VALI(obj); case ENUM | 1<<4: return EOBJ_VALF(obj); } return 0.0; }
static __ cstr   eobj_valS (eobj obj) { typedef struct __type{ uint _0:16; uint _1:4; uint t_o : 4; uint _2:8;} __type_t; if(obj) return ESTR == ((__type_t*)obj)[-1].t_o ? EOBJ_VALS(obj) :   0; return   0; }
static __ cptr   eobj_valP (eobj obj) { typedef struct __type{ uint _0:16; uint _1:4; uint t_o : 4; uint _2:8;} __type_t; if(obj) return EPTR == ((__type_t*)obj)[-1].t_o ? EOBJ_VALP(obj) :   0; return   0; }
static __ cptr   eobj_valR (eobj obj) { typedef struct __type{ uint _0:16; uint _1:4; uint t_o : 4; uint _2:8;} __type_t; if(obj) return ERAW == ((__type_t*)obj)[-1].t_o ? EOBJ_VALR(obj) :   0; return   0; }

#undef __



int  eobj_free  (eobj o);
int  eobj_freeEX(eobj o, eobj_rls_cb rls);

#ifdef __cplusplus
}
#endif

#endif
