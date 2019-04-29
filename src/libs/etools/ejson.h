/// =====================================================================================
///
///       Filename:  ejson.h
///
///    Description:  a easier way to handle json, you can also using it as a simple dic
///
///             1. we using list and dict to handle items in list and obj
///             2. we recorded the item which you accessed last time in list, we'll search
///                from the recorded item in next time
///
///        Version:  0.8
///        Created:  12/18/2016 08:51:34 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#ifndef __EJSON_H__
#define __EJSON_H__

#include "etype.h"
#include "eobj.h"
#include "estr.h"

#ifdef __cplusplus
extern "C" {
#endif

/** -----------------------------------------------------
 *
 *  ejson basic
 *
 *  -----------------------------------------------------
 */

eobj ejson_new(etypeo type, uint len);                        // create a ejson obj

eobj ejson_parseS  (constr json);                             // parse str to ejson obj
eobj ejson_parseSEx(constr json, constr* err, eopts opts);    // parse str to ejson obj
eobj ejson_parseF  (constr path);                             // parse file to ejson obj
eobj ejson_parseFEx(constr path, constr* err, eopts opts);    // parse file to ejson obj

uint ejson_checkS  (constr json);                             // check if json format is correct in str, returns the checked obj's cnt
uint ejson_checkSEx(constr json, constr* err, eopts opts);    // check if json format is correct in str, returns the checked obj's cnt
uint ejson_checkF  (constr path);                             // check if json format is correct in file, returns the checked obj's cnt
uint ejson_checkFEx(constr path, constr* err, eopts opts);    // check if json format is correct in filr, returns the checked obj's cnt

int  ejson_clear  (eobj r);                                   // clear a ejson obj, only effect on EOBJ and EARR
int  ejson_clearEx(eobj r, eobj_rls_ex_cb rls, eval prvt);    // clear a ejson obj, only effect on EOBJ and EARR

int  ejson_free  (eobj o);                                    // release a ejson obj, including all children
int  ejson_freeEx(eobj o, eobj_rls_ex_cb rls, eval prvt);     // release a ejson obj, including all children

etypeo ejson_type   (eobj o);
uint   ejson_len    (eobj o);         // Returns the len of ESTR or ERAW or number of items of EOBJ or EARR, else return 0
bool   ejson_isEmpty(eobj o);         // Returns false if ejson contains items, otherwise return true

void ejson_show(ejson r);

constr ejson_errp();
constr ejson_err ();

/** -----------------------------------------------------
 *
 *  ejson add
 *
 *  @note
 *      1. if root obj to add to is a ARR obj, the key from
 *  param of obj/src will never check, use the exist key in
 *  obj or src.
 *
 *      2. if root obj to add to is a OBJ obj, must have a
 *  key from param or obj/src, if key is set in param, the
 *  key in obj/src will always be replaced by it
 *
 *      3. see 'ejson val's note to get the meaning of 'r',
 *  'k' APIs.
 *
 *  -----------------------------------------------------
 */
eobj ejson_addJ(eobj r, constr key, constr json);                 // parse a json to obj and add it to root
eobj ejson_addT(eobj r, constr key, etypeo type);                 // add an obj to root, the obj can be defined as EFLASE, ETURE, ENULL, EARR, EOBJ
eobj ejson_addI(eobj r, constr key, i64    val );                 // add an NUM obj to root
eobj ejson_addF(eobj r, constr key, f64    val );                 // add an NUM obj to root
eobj ejson_addP(eobj r, constr key, conptr ptr );                 // add a  PTR obj to root
eobj ejson_addS(eobj r, constr key, constr str );                 // add a  STR obj to root
eobj ejson_addR(eobj r, constr key, uint   len );                 // add a  RAW obj to root, alloc a new space(len) for data
eobj ejson_addO(eobj r, constr key, eobj   o   );                 // add an exist obj to obj

eobj ejson_raddJ(eobj r, constr rawk, constr key, constr json);   // add an json  obj to specific obj in root via rawk
eobj ejson_raddT(eobj r, constr rawk, constr key, etypeo type);   // add an type  obj to specific obj in root via rawk, only support EFLASE, ETURE, ENULL, EARR, EOBJ
eobj ejson_raddI(eobj r, constr rawk, constr key, i64    val );   // add an NUM   obj to specific obj in root via rawk
eobj ejson_raddF(eobj r, constr rawk, constr key, f64    val );   // add an NUM   obj to specific obj in root via rawk
eobj ejson_raddP(eobj r, constr rawk, constr key, conptr ptr );   // add a  PTR   obj to specific obj in root via rawk
eobj ejson_raddS(eobj r, constr rawk, constr key, constr str );   // add a  STR   obj to specific obj in root via rawk
eobj ejson_raddR(eobj r, constr rawk, constr key, uint   len );   // add a  RAW   obj to specific obj in root via rawk, alloc a new space(len) for data
eobj ejson_raddO(eobj r, constr rawk, constr key, eobj   o   );   // add an exist obj to specific obj in root via rawk

eobj ejson_kaddJ(eobj r, constr keys, constr key, constr json);   // add an json  obj to specific obj in root via path
eobj ejson_kaddT(eobj r, constr keys, constr key, etypeo type);   // add an type  obj to specific obj in root via path, only support EFLASE, ETURE, ENULL, EARR, EOBJ
eobj ejson_kaddI(eobj r, constr keys, constr key, i64    val );   // add an NUM   obj to specific obj in root via path
eobj ejson_kaddF(eobj r, constr keys, constr key, f64    val );   // add an NUM   obj to specific obj in root via path
eobj ejson_kaddP(eobj r, constr keys, constr key, conptr ptr );   // add a  PTR   obj to specific obj in root via path
eobj ejson_kaddS(eobj r, constr keys, constr key, constr str );   // add a  STR   obj to specific obj in root via path
eobj ejson_kaddR(eobj r, constr keys, constr key, uint   len );   // add a  RAW   obj to specific obj in root via path, alloc a new space(len) for data
eobj ejson_kaddO(eobj r, constr keys, constr key, eobj   o   );   // add an exist obj to specific obj in root via path

/** -----------------------------------------------------
 *
 * @brief ejson val
 *
 *      get obj or val from a ejson obj
 *
 * @note
 *      1. for r APIs, we consider rawk as a whole key and
 *  can not be splited
 *
 *      2. for k APIs, we consider keys as a continues key
 *  chan like "fruits[0].name", then we will found: 'fruits'
 *   -> '0' -> 'name'.
 *
 *     {
 *          "fruits[0].name" : "tomato",
 *          ---------------------------
 *                      ^-------------------------------------- r found this
 *
 *          "fruits": [ {"name":"apple"}, {"name":"pear"} ]
 *                       --------------
 *                          ^---------------------------------- k found this
 *     }
 *
 *     3. for k APIs, you can split a key with '.' or '[]',
 *  they are simply the same:
 *
 *      fruits[0].name      : fruits -> 0 -> name
 *      fruits.0[name]      : fruits -> 0 -> name
 *      fruits.0.[name]     : fruits -> 0 -> "" -> name
 *
 *     4. for k APIs, the key in '[]' can not be split again
 *
 *      fruits[0.name]      : fruits -> 0.name
 *
 *
 */
eobj   ejson_valr      (eobj r, constr rawk);   // Returns the eobj with the specific rawk
i64    ejson_valrI     (eobj r, constr rawk);   // Returns the value i64  of eobj if exist and type matchs ENUM, else return 0
f64    ejson_valrF     (eobj r, constr rawk);   // Returns the value f64  of eobj if exist and type matchs ENUM, else return 0
constr ejson_valrS     (eobj r, constr rawk);   // Returns the cstr       of eobj if exist and type matchs EPTR, else return 0
cptr   ejson_valrP     (eobj r, constr rawk);   // Returns the ptr        of eobj if exist and type matchs ESTR, else return 0
cptr   ejson_valrR     (eobj r, constr rawk);   // Returns the ptr of raw in eobj if exist and type matchs ERAW, else return 0
etypeo ejson_valrType  (eobj r, constr rawk);   // Returns eobj's type if exist, else return EOBJ_UNKNOWN
constr ejson_valrTypeS (eobj r, constr rawk);   // Returns eobj's type in string type
uint   ejson_valrLen   (eobj r, constr rawk);   // Returns eobj's len  if found and type matchs ESTR, ERAW, EOBJ, EARR else return 0
bool   ejson_valrIsTrue(eobj r, constr rawk);   // Returns true if the val in eobj is likely true:
                                                    //  1. the type of obj is ETRUE
                                                    //  2. the val of i64 or f64 is not 0
                                                    //  3. the ptr val is not 0
                                                    //  4. the str val is not empty
                                                    //  5. the len of raw is not 0

eobj   ejson_valk      (eobj r, constr keys);   // Returns the eobj with the specific keys
i64    ejson_valkI     (eobj r, constr keys);   // Returns the value i64  of eobj if exist and type matchs ENUM, else return 0
f64    ejson_valkF     (eobj r, constr keys);   // Returns the value f64  of eobj if exist and type matchs ENUM, else return 0
constr ejson_valkS     (eobj r, constr keys);   // Returns the cstr       of eobj if exist and type matchs EPTR, else return 0
cptr   ejson_valkP     (eobj r, constr keys);   // Returns the ptr        of eobj if exist and type matchs ESTR, else return 0
cptr   ejson_valkR     (eobj r, constr keys);   // Returns the ptr of raw in eobj if exist and type matchs ERAW, else return 0
etypeo ejson_valkType  (eobj r, constr keys);   // Returns eobj's type if exist, else return EOBJ_UNKNOWN
constr ejson_valkTypeS (eobj r, constr keys);   // Returns eobj's type in string type
uint   ejson_valkLen   (eobj r, constr keys);   // Returns eobj's len  if exist and type matchs ESTR, ERAW, EOBJ, EARR else return 0
bool   ejson_valIksTrue(eobj r, constr keys);   // Returns true if the val in eobj is likely true:
                                                    //  1. the type of obj is ETRUE
                                                    //  2. the val of i64 or f64 is not 0
                                                    //  3. the ptr val is not 0
                                                    //  4. the str val is not empty
                                                    //  5. the len of raw is not 0

eobj   ejson_vali      (eobj r, int idx);       // Returns the eobj in the specific idx
i64    ejson_valiI     (eobj r, int idx);       // Returns the value i64  of eobj if exist and type matchs ENUM, else return 0
f64    ejson_valiF     (eobj r, int idx);       // Returns the value f64  of eobj if exist and type matchs ENUM, else return 0
constr ejson_valiS     (eobj r, int idx);       // Returns the cstr       of eobj if exist and type matchs EPTR, else return 0
cptr   ejson_valiP     (eobj r, int idx);       // Returns the ptr        of eobj if exist and type matchs ESTR, else return 0
cptr   ejson_valiR     (eobj r, int idx);       // Returns the ptr of raw in eobj if exist and type matchs ERAW, else return 0
etypeo ejson_valiType  (eobj r, int idx);       // Returns eobj's type if exist, else return EOBJ_UNKNOWN
constr ejson_valiTypeS (eobj r, int idx);       // Returns eobj's type in string type
uint   ejson_valiLen   (eobj r, int idx);       // Returns eobj's len  if found and type matchs ESTR, ERAW, EOBJ, EARR else return 0
bool   ejson_valiIsTrue(eobj r, int idx);       // Returns true if the val in eobj is likely true:
                                                    //  1. the type of obj is ETRUE
                                                    //  2. the val of i64 or f64 is not 0
                                                    //  3. the ptr val is not 0
                                                    //  4. the str val is not empty
                                                    //  5. the len of raw is not 0

/** -----------------------------------------------------
 *
 *  ejson format
 *
 *  @note
 *      if passed in 'out' is 0, create and returned a new
 *  buf, else write to it;
 *
 *  -----------------------------------------------------
 */
estr ejson_toS (eobj o,              estr* out, eopts opts);
estr ejson_rtoS(eobj o, constr rawk, estr* out, eopts opts);
estr ejson_ktoS(eobj o, constr keys, estr* out, eopts opts);

/** -----------------------------------------------------
 *
 *  ejson take and free
 *
 *  -----------------------------------------------------
 */
eobj ejson_takeH(eobj r);               // for EOBJ, EARR
eobj ejson_takeT(eobj r);               // for EOBJ, EARR
eobj ejson_takeO(eobj r, eobj      o);  // for EOBJ, EARR
eobj ejson_takeK(eobj r, constr keys);  // for EOBJ, EARR
eobj ejson_takeR(eobj r, constr rawk);  // for EOBJ
eobj ejson_takeI(eobj r, int     idx);  // for EARR


int  ejson_freeH(eobj r);               // for EOBJ, EARR
int  ejson_freeT(eobj r);               // for EOBJ, EARR
int  ejson_freeO(eobj r, eobj    obj);  // for EOBJ, EARR
int  ejson_freeK(eobj r, constr keys);  // for EOBJ, EARR
int  ejson_freeR(eobj r, constr rawk);  // for EOBJ
int  ejson_freeI(eobj r, int     idx);  // for EARR

int  ejson_freeHEx(eobj r,              eobj_rls_ex_cb rls, eval prvt);
int  ejson_freeTEx(eobj r,              eobj_rls_ex_cb rls, eval prvt);
int  ejson_freeOEx(eobj r, eobj    obj, eobj_rls_ex_cb rls, eval prvt);
int  ejson_freeKEx(eobj r, constr keys, eobj_rls_ex_cb rls, eval prvt);
int  ejson_freeREx(eobj r, constr rawk, eobj_rls_ex_cb rls, eval prvt);
int  ejson_freeIEx(eobj r, int     idx, eobj_rls_ex_cb rls, eval prvt);


/** -----------------------------------------------------
 *
 *  ejson comparing
 *
 * @return
 *       1: obj >  val
 *       0: obj == val
 *      -1: obj <  val
 *      -2: obj is NULL
 *      -3: type not match
 *      -4: val of cstr is null
 *  -----------------------------------------------------
 */

#define ejson_cmpI eobj_cmpI
#define ejson_cmpF eobj_cmpF
#define ejson_cmpS eobj_cmpS

int  ejson_rcmpI(eobj r, constr rawk, i64    val);
int  ejson_rcmpF(eobj r, constr rawk, f64    val);
int  ejson_rcmpS(eobj r, constr rawk, constr str);

int  ejson_kcmpI(eobj r, constr keys, i64    val);
int  ejson_kcmpF(eobj r, constr keys, f64    val);
int  ejson_kcmpS(eobj r, constr keys, constr str);

/** -----------------------------------------------------
 * @brief
 *
 *  ejson iterationg
 *
 *  @note
 *      for perfomance, we do not check type of o in
 *  ejson_next() and ejson_prev()
 *
 */
eobj  ejson_first(eobj r);
eobj  ejson_last (eobj r);
eobj  ejson_next (eobj o);
eobj  ejson_prev (eobj o);

eobj  ejson_rfirst(eobj r, constr rawk);
eobj  ejson_rlast (eobj r, constr rawk);

eobj  ejson_kfirst(eobj r, constr keys);
eobj  ejson_klast (eobj r, constr keys);

#define ejson_foreach( r,     itr)   for(itr = ejson_first (r    ); (itr); itr = ejson_next(itr))
#define ejson_rforeach(r, rk, itr)   for(itr = ejson_rfirst(r, rk); (itr); itr = ejson_next(itr))
#define ejson_kforeach(r, ks, itr)   for(itr = ejson_kfirst(r, ks); (itr); itr = ejson_next(itr))

#define ejson_foreach_s( r,     itr) for(eobj _INNER_ = ejson_first (r    ), itr; (itr = _INNER_, _INNER_ = ejson_next(_INNER_), itr); )
#define ejson_rforeach_s(r, rk, itr) for(eobj _INNER_ = ejson_rfirst(r, rk), itr; (itr = _INNER_, _INNER_ = ejson_next(_INNER_), itr); )
#define ejson_kforeach_s(r, ks, itr) for(eobj _INNER_ = ejson_kfirst(r, ks), itr; (itr = _INNER_, _INNER_ = ejson_next(_INNER_), itr); )


/// -----------------------------------------------------
//! @brief ejson set
///
/// @note:
/// 1. if target not exsit, create automatically if the last
///    found obj is a EOBJ obj
/// 2. the found obj will be reset always, any children of it
///    will be delete automatically, be careful by using it,
///    it may cause memleak when have EPTR or ERAW obj associated
///    with the delete obj
/// 3. we do not create any not exsit obj for EARR obj
///
eobj ejson_setTr(eobj r, constr rawk, etypeo   t);
eobj ejson_setIr(eobj r, constr rawk, i64    val);
eobj ejson_setFr(eobj r, constr rawk, f64    val);
eobj ejson_setSr(eobj r, constr rawk, constr str);
eobj ejson_setPr(eobj r, constr rawk, constr ptr);
eobj ejson_setRr(eobj r, constr rawk, uint   len);

eobj ejson_setTk(eobj r, constr rawk, etypeo   t);
eobj ejson_setIk(eobj r, constr rawk, i64    val);
eobj ejson_setFk(eobj r, constr rawk, f64    val);
eobj ejson_setSk(eobj r, constr rawk, constr str);
eobj ejson_setPk(eobj r, constr rawk, constr ptr);
eobj ejson_setRk(eobj r, constr rawk, uint   len);


/// -----------------------------------------------------
//! @brief ejson substitute string
///
/// @return the modified obj if setted
///
eobj ejson_rsubS(eobj root, constr keys, constr from, constr to);
eobj ejson_ksubS(eobj root, constr rawk, constr from, constr to);

#define ejson_subSr ejsonr_subS
#define ejson_subSk ejsonk_subS

/// -----------------------------------------------------
/// @brief ejson counter
///
/// @note:
///     1. if NUM obj not exsit in root, will auto create one
///     2. only support NUM obj if target obj exist
///     3. return LLONG_MIN if failed
///
i64  ejson_pp  (eobj o);                // increase 1
i64  ejson_mm  (eobj o);                // decrease 1
i64  ejson_incr(eobj o, i64 v);         // increase v
i64  ejson_decr(eobj o, i64 v);         // decrease v

i64  ejsonr_pp  (eobj r, constr rawk);
i64  ejsonr_mm  (eobj r, constr rawk);
i64  ejsonr_incr(eobj r, constr rawk, i64 v);
i64  ejsonr_decr(eobj r, constr rawk, i64 v);

i64  ejsonk_pp  (eobj r, constr rawk);
i64  ejsonk_mm  (eobj r, constr rawk);
i64  ejsonk_incr(eobj r, constr keys, i64 v);
i64  ejsonk_decr(eobj r, constr keys, i64 v);

/** -----------------------------------------------
 *  @brief
 *      ejson sort operation
 *
 *  @note:
 *      it only effect on EOBJ and EARR obj of ejson
 *
 *
 */

//! supplied default sort cb
int    __KEYS_ACS(eobj a, eobj b);      // Ascending  via key string in all obj, dictionary sequence
int    __KEYS_DES(eobj a, eobj b);      // Descending via key string in all obj, dictionary sequence
int    __VALI_ACS(eobj a, eobj b);      // Ascending  via int value in NUM obj
int    __VALI_DES(eobj a, eobj b);      // Descending via int value in NUM obj

eobj  ejson_sort (eobj r,              eobj_cmp_cb cmp);
eobj  ejson_sortr(eobj r, constr rawk, eobj_cmp_cb cmp);
eobj  ejson_sortk(eobj r, constr keys, eobj_cmp_cb cmp);

#define ejsonr_sort  ejson_sortr
#define ejsonk_sort  ejson_sortk

/// -- ejson version
constr ejson_version();

#ifdef __cplusplus
}
#endif

#endif
