/// =====================================================================================
///
///       Filename:  erb.h
///
///    Description:  easy rb_tree, rebuild from linux-4.6.3
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

#ifndef __ERB_H__
#define __ERB_H__

#include "etype.h"
#include "eobj.h"

#define ERB_VERSION     "erb 1.1.2"     // fix build in old gcc

#ifdef __cplusplus
extern "C" {
#endif

typedef struct erb_type_s{
    eval            prvt;       // private data
    eobj_cmp_ex_cb  cmp;        // compare func
    eobj_rls_ex_cb  rls;        // release func, only ERAW and EPTR type will call
}erb_type_t, * erb_type;

typedef enum{
    ERB_KEYI = 0x00,            // using key as a integer
    ERB_KEYS = 0x01,            // using key as a str
}erb_opts;

/// ---------------------- creator -------------------------
///
///     create a new erb_tree handle
///
/// @note:
///     1.
///

erb  erb_new (int opts, erb_type type);

erb  erb_setPrvt(erb t, eval prvt);
erb  erb_setCmp (erb t, eobj_cmp_ex_cb cmp);    // if rbtree is not empty, will set failed
erb  erb_setRls (erb t, eobj_rls_ex_cb cmp);

/// ---------------------- adder -------------------------
///
///     creat add add eobj to our erb tree
///
///

eobj   erb_newO(etypeo type, uint len);    // create new eobj by type

eobj   erb_addI(erb r, ekey key, i64    val);               // EOBJ_NUM i64
eobj   erb_addF(erb r, ekey key, f64    val);               // EOBJ_NUM f64
eobj   erb_addS(erb r, ekey key, constr str);               // EOBJ_STR
eobj   erb_addP(erb r, ekey key, conptr ptr);               // EOBJ_PTR
eobj   erb_addR(erb r, ekey key, size   len);               // EOBJ_RAW
eobj   erb_addO(erb r, ekey key, eobj   obj);               // EOBJ_OBJ

//! multi add
eobj   erb_addMI(erb r, ekey key, i64    val);              // EOBJ_NUM i64
eobj   erb_addMF(erb r, ekey key, f64    val);              // EOBJ_NUM f64
eobj   erb_addMS(erb r, ekey key, constr str);              // EOBJ_STR
eobj   erb_addMP(erb r, ekey key, conptr ptr);              // EOBJ_PTR
eobj   erb_addMR(erb r, ekey key, size   len);              // EOBJ_RAW
eobj   erb_addMO(erb r, ekey key, eobj   obj);              // EOBJ_OBJ

/// -- erb set operation --
///
///     set or reset the value of obj in rb tree
///
///  1. if the key not exsit, create automaticly
///  2. we only set the first find one
///  3. set operation is likely be successful always,
///     and the val will be seted to what wanted
///  4. it may do realloc opearation, be careful by
///     using it
///
///
eobj   erb_setI(erb t, ekey key, i64    val);
eobj   erb_setF(erb t, ekey key, f64    val);
eobj   erb_setP(erb t, ekey key, conptr ptr);
eobj   erb_setS(erb t, ekey key, constr str);
eobj   erb_setR(erb t, ekey key, u32    len);
eobj   erb_setO();          // todo

eobj   erb_setMB();         // todo
eobj   erb_setMI();         // todo
eobj   erb_setMF();         // todo
eobj   erb_setMS();         // todo
eobj   erb_setMP();         // todo
eobj   erb_setMR();         // todo
eobj   erb_setMO();         // todo

/// -- erb get operation --
///
///     get the obj or value in rbtree
///
///
///
///
eobj   erb_find(erb r, ekey key);   // Returns the first found eobj by key if exist, else return 0

eobj   erb_val (erb r, ekey key);   // Returns the first found eobj by key, see erb_find()
i64    erb_valI(erb r, ekey key);   // Returns the value i64  of eobj if exist and type matchs ENUM, else return 0
f64    erb_valF(erb r, ekey key);   // Returns the value f64  of eobj if exist and type matchs ENUM, else return 0
cstr   erb_valS(erb r, ekey key);   // Returns the cstr       of eobj if exist and type matchs ESTR, else return 0
cptr   erb_valR(erb r, ekey key);   // Returns the ptr        of eobj if exist and type matchs EPTR, else return 0
cptr   erb_valP(erb r, ekey key);   // Returns the ptr of raw in eobj if exist and type matchs ERAW, else return 0


etypeo erb_valType(erb r, ekey key);    // Returns eobj's type if exist, else return EOBJ_UNKNOWN
uint   erb_valLen (erb r, ekey key);    // Returns eobj's len  if exist and type matchs ESTR, ERAW, EOBJ, else return 0

bool   erb_valIsTrue(erb r, ekey key);   // Returns true if the val in eobj is likely true:
                                                //  1. the type of obj is ETRUE
                                                //  2. the val of i64 or f64 is not 0
                                                //  3. the ptr val is not 0
                                                //  4. the str val is not empty
                                                //  5. the len of raw is not 0

/// find fuzzy
///

typedef eobj (*get_needed_cb)(eobj o);

eobj  erb_valEx(erb r, ekey key, get_needed_cb cb);

eobj   erb_atKey(erb r, ekey key, int drct);
eobj   erb_atFirst();
eobj   erb_atLast();

eobj   erb_valFuz (erb r, ekey key, int drct);   // todo


uint   erb_len (erb r);         // Returns the number of items in the rbtree.

//! travese
#define erb_foreach(root, itr) for(cptr _INNER_ = erb_first(root); (itr = _INNER_, _INNER_ = erb_next(_INNER_), itr); )

eobj   erb_first(erb   rb);            // Returns the first item of rbtree if have; otherwise returns 0.
eobj   erb_last (erb   rb);            // Returns the last  item of rbtree if have; otherwise returns 0.
eobj   erb_next (eobj obj);            // Returns the next  item of eobj if have; otherwise returns 0. For performance, we do not check the obj type, be careful.
eobj   erb_prev (eobj obj);            // Returns the prev  item of eobj if have; otherwise returns 0. For performance, we do not check the obj type, be careful.

//! take and free
eobj   erb_takeH(erb t);                    // Takes the first element
eobj   erb_takeT(erb t);                    // Takes the last  element
eobj   erb_takeO(erb t, eobj obj);

eobj   erb_takeOne(erb t, ekey key);
int    erb_takeAll(erb t, ekey key);

int    erb_freeH(erb t);
int    erb_freeT(erb t);
int    erb_freeO(erb t, eobj obj);

int    erb_freeOne(erb t, ekey key);
int    erb_freeAll(erb t, ekey key);

int    erb_clear(erb t);
int    erb_clearEx(erb t);

int    erb_free (erb t);
int    erb_freeEx(erb t, eobj_rls_cb rls);

//! utils
void   erb_show(erb root, uint len);


constr erb_version();

#ifdef __cplusplus
}
#endif
#endif
