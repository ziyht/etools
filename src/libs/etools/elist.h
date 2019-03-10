/// =====================================================================================
///
///       Filename:  elist.h
///
///    Description:  an easier double link list
///
///        Version:  1.2
///        Created:  05/04/2017 04:50:18 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#ifndef __ELIST_H__
#define __ELIST_H__

#include "etype.h"
#include "eobj.h"
#include "ealloc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct elist_s { char __[44]; } elist_t;

typedef struct _elist_s* elist;

typedef union{
    eobj_t o;
}ello_t, * ello;

elist elist_new();

eobj  elist_newO(etypev type, uint len);             // Returns a new eobj used by elist, len will have effect when @param type is ESTR, ERAW

/// -- elist push operation --
///
///     create a obj in front of the list
///
/// returns the created obj if succed, otherwise return 0
///
eobj  elist_pushI(elist l, i64    val);         // EOBJ_NUM i64
eobj  elist_pushF(elist l, f64    val);         // EOBJ_NUM f64
eobj  elist_pushS(elist l, constr str);         // EOBJ_STR
eobj  elist_pushP(elist l, conptr ptr);         // EOBJ_PTR
eobj  elist_pushR(elist l, size_t len);         // EOBJ_RAW
eobj  elist_pushO(elist l, eobj   obj);         // EOBJ_OBJ

/// -- elist appd operation --
///
///     create a obj in last of the list
///
/// returns the created obj if succeed, otherwise return 0
///
eobj  elist_appdI(elist l, i64    val);         // EOBJ_NUM i64
eobj  elist_appdF(elist l, f64    val);         // EOBJ_NUM f64
eobj  elist_appdS(elist l, constr str);         // EOBJ_STR
eobj  elist_appdP(elist l, conptr ptr);         // EOBJ_PTR
eobj  elist_appdR(elist l, size_t len);         // EOBJ_RAW
eobj  elist_appdO(elist l, eobj   obj);         // EOBJ_OBJ

/// -- elist set operation --
///
///     reset(replace) the value of obj in @param idx of
/// the inner list
///
/// returns the created obj if succeed, otherwise return nullptr
///
eobj  elist_setI(elist l, uint idx, i64    val);        // todo
eobj  elist_setF(elist l, uint idx, f64    val);        // todo
eobj  elist_setS(elist l, uint idx, constr str);        // todo
eobj  elist_setP(elist l, uint idx, conptr ptr);        // todo
eobj  elist_setR(elist l, uint idx, size_t len);        // todo
eobj  elist_setO(elist l, uint idx, eobj   obj);        // todo

/// -- elist insert operation --
///
///     insert an obj at the idx of list
///
/// returns the created obj if succeed, otherwise return 0
///
eobj  elist_insertI(elist l, int idx, i64    val);        // todo
eobj  elist_insertF(elist l, int idx, f64    val);        // todo
eobj  elist_insertS(elist l, int idx, constr str);        // todo
eobj  elist_insertP(elist l, int idx, conptr ptr);        // todo
eobj  elist_insertR(elist l, int idx, size_t len);        // todo
eobj  elist_insertO(elist l, int idx, eobj   obj);        // todo

/// -- elist get operation --
///
///
///
eobj  elist_at  (elist  l, uint idx);       // Returns the eobj at pos idx

eobj  elist_val (elist  l, uint idx);       // Returns the eobj at pos idx, see elist_at()
i64   elist_valI(elist  l, uint idx);       // Returns the value i64  of eobj if exist and type matchs EOBJ_NUM, else return 0
f64   elist_valF(elist  l, uint idx);       // Returns the value f64  of eobj if exist and type matchs EOBJ_NUM, else return 0
cstr  elist_valS(elist  l, uint idx);       // Returns the cstr       of eobj if exist and type matchs EOBJ_STR, else return 0
cptr  elist_valP(elist  l, uint idx);       // Returns the ptr        of eobj if exist and type matchs EOBJ_PTR, else return 0
cptr  elist_valR(elist  l, uint idx);       // Returns the ptr of raw in eobj if exist and type matchs EOBJ_RAW, else return 0

etypev elist_valType(elist l, uint idx);    // Returns val type
uint   elist_valLen (elist l, uint idx);    // Returns the len of eobj if exist, else return 0

bool   elist_valIsTrue(elist l, uint idx);  // Returns true if the val in eobj is likely true:
                                                //  1. the val of i64 or f64 is not 0
                                                //  2. the ptr val is not 0
                                                //  3. the str val is not empty
                                                //  4. the len of raw is not 0

int   elist_isEmpty(elist l);               // Returns 1 if the list contains no items; otherwise returns 0 or -1 for l is nullptr.
uint  elist_len    (elist l);               // Returns the number of items in the list.
uint  elist_size   (elist l);               // Returns the number of items in the list.

eobj  elist_first(elist  l);                // Returns the first item of list if have; otherwise returns 0.
eobj  elist_last (elist  l);                // Returns the last  item of list if have; otherwise returns 0.
eobj  elist_next (eobj obj);                // Returns the next  item of eobj if have; otherwise returns 0. For performance, we do not check the obj type, be careful.
eobj  elist_prev (eobj obj);                // Returns the prev  item of eobj if have; otherwise returns 0. For performance, we do not check the obj type, be careful.

/// elist_foreach(elist l, eobj itr)
#define elist_foreach(l, itr)     for(itr = (cptr)elist_first(l); (itr); itr = (cptr)elist_next((eobj)itr))
#define elist_foreach_s(l, itr)   for(eobj __INNER__ = elist_first(l); ((itr) = (cptr)__INNER__, __INNER__ = elist_next(__INNER__), (itr));)

int   elist_swap(elist l, uint idx1, uint idx2);    // todo

eobj  elist_takeH(elist l);                 // Takes the head obj
eobj  elist_takeT(elist l);                 // Takes the tail obj
eobj  elist_takeO(elist l, eobj obj);       // Takes the obj which passed in, it will success only when the obj is in this list
eobj  elist_takeAt(elist l, uint idx);      // Takes the obj at the idx if exist

elist elist_takeAll(elist l, eobj val);     // todo
eobj  elist_takeOne(elist l, eobj val);     // todo

elist elist_takeAllEx(elist l, eobj val, eobj_cmp_cb cmp);  // todo
elist elist_takeOneEx(elist l, eobj val, eobj_cmp_cb cmp);  // todo

int   elist_freeH(elist l);                 // Release the head obj
int   elist_freeT(elist l);                 // Release the tail obj
int   elist_freeO(elist l, eobj obj);       // Release the obj which passed in, it will success only when the obj is in this list or the pass in list is null and the obj is independent and a ELIST obj
int   elist_freeI(elist l, uint idx);       // Release the obj in idx

int   elist_freeAll(elist l, eobj val);     // todo
int   elist_freeOne(elist l, eobj val);     // todo

int   elist_freeAllEx(elist l, eobj val, eobj_cmp_cb cmp, eobj_rls_cb rls); // todo
int   elist_freeOneEx(elist l, eobj val, eobj_cmp_cb cmp, eobj_rls_cb rls); // todo

elist elist_clear  (elist l);
elist elist_clearEx(elist l, eobj_rls_cb rls);

int   elist_free  (elist l);
int   elist_freeEx(elist l, eobj_rls_cb rls);

void  elist_show(elist l, uint max);                    // todo
void  elist_showSpan(elist l, uint begin, uint end);    // todo

constr elist_version();

#ifdef __cplusplus
}
#endif

#endif
