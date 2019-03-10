/// =====================================================================================
///
///       Filename:  edict.h
///
///    Description:  an easier dict
///
///        Version:  1.0
///        Created:  2017-05-03 10:00:18 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#ifndef __EDICT_H__
#define __EDICT_H__

#include "etype.h"
#include "eobj.h"

#ifdef __cplusplus
extern "C" {
#endif

/// ---------------------- edict ------------------------

typedef struct _editr_s* editr;

typedef enum{
    EDICT_KEYI   = 0x00,              // handle input key as a int
    EDICT_KEYS   = 0x01,              // handle input key as a str

    EDITR_UNSAFE = 0x00,
    EDITR_SAFE   = 0x01,
}edict_opts;

/// ---------------------- creator ------------------------
///
///     create a new edict.
///
///

edict edict_new(int opts);
edict edict_newKeyS();
edict edict_newKeyI();

int   edict_free(edict d);
int   edict_clear(edict d);

void  edict_privSet(edict d, eval priv);
eval  edict_privGet(edict d);

/// -------------------- operations -----------------------
///
///     to operate the edict.
///
/// @note:
///     1. the returned cptr or cstr of _addB(), _addN(),
/// _addR(), _addS() is the first addr of raw data, you can
/// also using it as a eobj by calling eobj_*() APIs or using
/// it directly;
///     2. if execute faild, return 0;
///
///     example:
///         s = "123";
///         a = edict_addS(root, 1, s);   // s: "123", a: "123"
///         strcpy(a, "456");
///         a = edict_find(root, 1);      // s: "123", a: "456"
///

eobj  edict_addI(edict d, ekey key, i64    val);              // Num
eobj  edict_addF(edict d, ekey key, f64    val);              // Num
eobj  edict_addP(edict d, ekey key, conptr ptr);              // Ptr
eobj  edict_addS(edict d, ekey key, constr str);              // Str
eobj  edict_addR(edict d, ekey key, uint   len);              // Raw

//! todo: add multi
eobj  edict_addMI(edict d, ekey key, i64    val);              // Num
eobj  edict_addMF(edict d, ekey key, f64    val);              // Num
eobj  edict_addMP(edict d, ekey key, conptr ptr);              // Ptr
eobj  edict_addMS(edict d, ekey key, constr str);              // Str
eobj  edict_addMR(edict d, ekey key, uint   len);              // Raw

/// -- edict get operation --
///
///
///
eobj  edict_find(edict d, ekey key);

eobj  edict_val (edict d, ekey key);    // Returns the eobj with the specific key
i64   edict_valI(edict d, ekey key);    // Returns the value i64  of eobj if exist and type matchs ENUM, else return 0
f64   edict_valF(edict d, ekey key);    // Returns the value f64  of eobj if exist and type matchs ENUM, else return 0
cptr  edict_valP(edict d, ekey key);    // Returns the ptr        of eobj if exist and type matchs EPTR, else return 0
cstr  edict_valS(edict d, ekey key);    // Returns the cstr       of eobj if exist and type matchs ESTR, else return 0
cptr  edict_valR(edict d, ekey key);    // Returns the ptr of raw in eobj if exist and type matchs ERAW, else return 0

etypeo edict_valType  (edict d, ekey key);     // Returns eobj's type if exist, else return EOBJ_UNKNOWN
uint   edict_valLen   (edict d, ekey key);     // Returns eobj's len  if found and type matchs ESTR, ERAW, EOBJ, else return 0
bool   edict_valIsTrue(edict d, ekey key);   // Returns true if the val in eobj is likely true:
                                                //  1. the type of obj is ETRUE
                                                //  2. the val of i64 or f64 is not 0
                                                //  3. the ptr val is not 0
                                                //  4. the str val is not empty
                                                //  5. the len of raw is not 0

bool  edict_isEmpty(edict d);               // Returns true if the dict contains no items, else return false
uint  edict_len    (edict d);               // Returns the number of items in the dict.
uint  edict_size   (edict d);               // Returns the number of items in the dict.


eobj  edict_takeO(edict d, eobj obj);       // Takes the specific obj from the dict, returns the took obj if exist, else return 0
eobj  edict_takeOne(edict d, ekey key);     // Takes the obj with the specific key from the dict, returns the took obj if exist, else return 0
//evec  edict_takeAll(edict d, ekey key);    // todo: Takes all objs with the specific key from the dict, returns a evec if exist, else return 0, the returned evec should be free'd after using


int   edict_freeO(edict d, eobj obj);
int   edict_freeOne(edict d, ekey key);
int   edict_freeAll(edict d, ekey key);

int   edict_clear(edict d);
int   edict_clearEx(edict d, eobj_rls_cb rls);

void  edict_show(edict d, uint cnt);

/// ---------------------- iterator -----------------------
///
///     to iterating the edict.
///
/// @note:
///     1. if it is a save iterator, you can using edict_rm*()
/// and edict_free*() when iterating, else, you can only
/// iterating it.
///

editr edict_getItr(edict d, int safe);                     // if safe == 1, the itr will create in safe mode, else not
int   edict_resetItr(editr itr, edict d, int safe);        // if d == 0, it will keep the inner edict; if safe != 0 && safe != 1,  the safe mode will not changed

#define edict_foreach(ditr, oitr) for(eobj itr; (itr = edict_next(ditr)); )

eobj  edict_next(editr itr);

void  edict_freeItr(editr itr);

constr edict_version();

#ifdef __cplusplus
}
#endif

#endif
