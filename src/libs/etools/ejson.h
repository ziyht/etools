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

eobj   ejson_new(etypeo type, uint len);                        // create a ejson obj

eobj   ejson_parseS  (constr json);                             // parse str to ejson obj
eobj   ejson_parseSEx(constr json, constr* err, eopts opts);    // parse str to ejson obj
eobj   ejson_parseF  (constr path);                             // parse file to ejson obj
eobj   ejson_parseFEx(constr path, constr* err, eopts opts);    // parse file to ejson obj

uint   ejson_checkS  (constr json);                             // check if json format is correct in str, returns the checked obj's cnt
uint   ejson_checkSEx(constr json, constr* err, eopts opts);    // check if json format is correct in str, returns the checked obj's cnt
uint   ejson_checkF  (constr path);                             // check if json format is correct in file, returns the checked obj's cnt
uint   ejson_checkFEx(constr path, constr* err, eopts opts);    // check if json format is correct in filr, returns the checked obj's cnt

int    ejson_clear  (eobj r);                                   // clear a ejson obj, only effect on EOBJ and EARR
int    ejson_clearEx(eobj r, eobj_rls_ex_cb rls, eval prvt);    // clear a ejson obj, only effect on EOBJ and EARR

int    ejson_free  (eobj o);                                    // release a ejson obj, including all children
int    ejson_freeEx(eobj o, eobj_rls_ex_cb rls, eval prvt);     // release a ejson obj, including all children

etypeo ejson_type   (eobj o);
uint   ejson_len    (eobj o);         // Returns the len of ESTR or ERAW or number of items of EOBJ or EARR, else return 0
bool   ejson_isEmpty(eobj o);         // Returns false if ejson contains items, otherwise return true

void   ejson_show(ejson r);

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
eobj ejson_addS(eobj r, constr key, constr str );                 // add a  STR obj to root
eobj ejson_addP(eobj r, constr key, conptr ptr );                 // add a  PTR obj to root
eobj ejson_addR(eobj r, constr key, uint   len );                 // add a  RAW obj to root, alloc a new space(len) for data
eobj ejson_addO(eobj r, constr key, eobj   o   );                 // add an exist obj to obj

eobj ejsonk_addJ(eobj r, constr rawk, constr key, constr json);   // add an json  obj to specific obj in root via rawk
eobj ejsonk_addT(eobj r, constr rawk, constr key, etypeo type);   // add an type  obj to specific obj in root via rawk, only support EFLASE, ETURE, ENULL, EARR, EOBJ
eobj ejsonk_addI(eobj r, constr rawk, constr key, i64    val );   // add an NUM   obj to specific obj in root via rawk
eobj ejsonk_addF(eobj r, constr rawk, constr key, f64    val );   // add an NUM   obj to specific obj in root via rawk
eobj ejsonk_addS(eobj r, constr rawk, constr key, constr str );   // add a  STR   obj to specific obj in root via rawk
eobj ejsonk_addP(eobj r, constr rawk, constr key, conptr ptr );   // add a  PTR   obj to specific obj in root via rawk
eobj ejsonk_addR(eobj r, constr rawk, constr key, uint   len );   // add a  RAW   obj to specific obj in root via rawk, alloc a new space(len) for data
eobj ejsonk_addO(eobj r, constr rawk, constr key, eobj   o   );   // add an exist obj to specific obj in root via rawk

eobj ejsoni_addJ(eobj r, u32    idx , constr key, constr json);   // add an json  obj to specific obj in root via idx
eobj ejsoni_addT(eobj r, u32    idx , constr key, etypeo type);   // add an type  obj to specific obj in root via idx, only support EFLASE, ETURE, ENULL, EARR, EOBJ
eobj ejsoni_addI(eobj r, u32    idx , constr key, i64    val );   // add an NUM   obj to specific obj in root via idx
eobj ejsoni_addF(eobj r, u32    idx , constr key, f64    val );   // add an NUM   obj to specific obj in root via idx
eobj ejsoni_addS(eobj r, u32    idx , constr key, constr str );   // add a  STR   obj to specific obj in root via idx
eobj ejsoni_addP(eobj r, u32    idx , constr key, conptr ptr );   // add a  PTR   obj to specific obj in root via idx
eobj ejsoni_addR(eobj r, u32    idx , constr key, uint   len );   // add a  RAW   obj to specific obj in root via idx, alloc a new space(len) for data
eobj ejsoni_addO(eobj r, u32    idx , constr key, eobj   o   );   // add an exist obj to specific obj in root via idx

eobj ejsonp_addJ(eobj r, constr keys, constr key, constr json);   // add an json  obj to specific obj in root via path
eobj ejsonp_addT(eobj r, constr keys, constr key, etypeo type);   // add an type  obj to specific obj in root via path, only support EFLASE, ETURE, ENULL, EARR, EOBJ
eobj ejsonp_addI(eobj r, constr keys, constr key, i64    val );   // add an NUM   obj to specific obj in root via path
eobj ejsonp_addF(eobj r, constr keys, constr key, f64    val );   // add an NUM   obj to specific obj in root via path
eobj ejsonp_addS(eobj r, constr keys, constr key, constr str );   // add a  STR   obj to specific obj in root via path
eobj ejsonp_addP(eobj r, constr keys, constr key, conptr ptr );   // add a  PTR   obj to specific obj in root via path
eobj ejsonp_addR(eobj r, constr keys, constr key, uint   len );   // add a  RAW   obj to specific obj in root via path, alloc a new space(len) for data
eobj ejsonp_addO(eobj r, constr keys, constr key, eobj   o   );   // add an exist obj to specific obj in root via path


/** -----------------------------------------------------
 *
 * @brief ejson val
 *
 *      get obj or val from a ejson obj
 *
 * @note
 *
 *      1. we have three race of APIs:
 *
 *          RACE  MEAN    KEY_TYPE   SUPPORT
 *          ---------------------------------
 *          k     key     cstr       EOBJ/EARR
 *          i     idx     int        EARR
 *          p     path    cstr       EOBJ/EARR
 *
 *
 *      for k race APIs, we consider rawk as a whole key and
 *  can not be splited
 *
 *      for p race APIs, we consider keys as a continues key
 *  chan like "fruits[0].name", then we will found the target
 *  like this: 'fruits' -> '0' -> 'name'.
 *
 *     {
 *          "fruits[0].name" : "tomato",
 *                             --------
 *                                ^---------------------------- k found this
 *
 *          "fruits": [ {"name":"apple"}, {"name":"pear"} ]
 *                              -------
 *                                 ^--------------------------- p found this
 *     }
 *
 *     3. for p race APIs, you can split a key with '.' or '[]',
 *  they are simply the same:
 *
 *      fruits[0].name      : fruits -> 0 -> name
 *      fruits.0[name]      : fruits -> 0 -> name
 *      fruits.0.[name]     : fruits -> 0 -> "" -> name
 *
 *     4. for p race APIs, the key in '[]' can not be split again
 *
 *      fruits[0.name]      : fruits -> 0.name
 *
 *
 */
eobj   ejson_valk      (eobj r, constr rawk);   // Returns the eobj with the specific rawk
i64    ejson_valIk     (eobj r, constr rawk);   // Returns the value i64  of eobj if exist and type matchs ENUM, else return 0
f64    ejson_valFk     (eobj r, constr rawk);   // Returns the value f64  of eobj if exist and type matchs ENUM, else return 0
constr ejson_valSk     (eobj r, constr rawk);   // Returns the cstr       of eobj if exist and type matchs EPTR, else return 0
cptr   ejson_valPk     (eobj r, constr rawk);   // Returns the ptr        of eobj if exist and type matchs ESTR, else return 0
cptr   ejson_valRk     (eobj r, constr rawk);   // Returns the ptr of raw in eobj if exist and type matchs ERAW, else return 0
etypeo ejson_valTypek  (eobj r, constr rawk);   // Returns eobj's type if exist, else return EOBJ_UNKNOWN
constr ejson_valTypeSk (eobj r, constr rawk);   // Returns eobj's type in string type
uint   ejson_valLenk   (eobj r, constr rawk);   // Returns eobj's len  if found and type matchs ESTR, ERAW, EOBJ, EARR else return 0
bool   ejson_valIsTruek(eobj r, constr rawk);   // Returns true if the val in eobj is likely true:
                                                    //  1. the type of obj is ETRUE
                                                    //  2. the val of i64 or f64 is not 0
                                                    //  3. the ptr val is not 0
                                                    //  4. the str val is not empty
                                                    //  5. the len of raw is not 0

eobj   ejson_vali      (eobj r, int idx);       // Returns the eobj in the specific idx
i64    ejson_valIi     (eobj r, int idx);       // Returns the value i64  of eobj if exist and type matchs ENUM, else return 0
f64    ejson_valFi     (eobj r, int idx);       // Returns the value f64  of eobj if exist and type matchs ENUM, else return 0
constr ejson_valSi     (eobj r, int idx);       // Returns the cstr       of eobj if exist and type matchs EPTR, else return 0
cptr   ejson_valPi     (eobj r, int idx);       // Returns the ptr        of eobj if exist and type matchs ESTR, else return 0
cptr   ejson_valRi     (eobj r, int idx);       // Returns the ptr of raw in eobj if exist and type matchs ERAW, else return 0
etypeo ejson_valTypei  (eobj r, int idx);       // Returns eobj's type if exist, else return EOBJ_UNKNOWN
constr ejson_valTypeSi (eobj r, int idx);       // Returns eobj's type in string type
uint   ejson_valLeni   (eobj r, int idx);       // Returns eobj's len  if found and type matchs ESTR, ERAW, EOBJ, EARR else return 0
bool   ejson_valIsTruei(eobj r, int idx);       // Returns true if the val in eobj is likely true:
                                                    //  1. the type of obj is ETRUE
                                                    //  2. the val of i64 or f64 is not 0
                                                    //  3. the ptr val is not 0
                                                    //  4. the str val is not empty
                                                    //  5. the len of raw is not 0

eobj   ejson_valp      (eobj r, constr keys);   // Returns the eobj with the specific keys
i64    ejson_valIp     (eobj r, constr keys);   // Returns the value i64  of eobj if exist and type matchs ENUM, else return 0
f64    ejson_valFp     (eobj r, constr keys);   // Returns the value f64  of eobj if exist and type matchs ENUM, else return 0
constr ejson_valSp     (eobj r, constr keys);   // Returns the cstr       of eobj if exist and type matchs EPTR, else return 0
cptr   ejson_valPp     (eobj r, constr keys);   // Returns the ptr        of eobj if exist and type matchs ESTR, else return 0
cptr   ejson_valRp     (eobj r, constr keys);   // Returns the ptr of raw in eobj if exist and type matchs ERAW, else return 0
etypeo ejson_valTypep  (eobj r, constr keys);   // Returns eobj's type if exist, else return EOBJ_UNKNOWN
constr ejson_valTypeSp (eobj r, constr keys);   // Returns eobj's type in string type
uint   ejson_valLenp   (eobj r, constr keys);   // Returns eobj's len  if exist and type matchs ESTR, ERAW, EOBJ, EARR else return 0
bool   ejson_valIsTruep(eobj r, constr keys);   // Returns true if the val in eobj is likely true:
                                                    //  1. the type of obj is ETRUE
                                                    //  2. the val of i64 or f64 is not 0
                                                    //  3. the ptr val is not 0
                                                    //  4. the str val is not empty
                                                    //  5. the len of raw is not 0

#define ejsonk          ejson_valk
#define ejsonk_valI     ejson_valIk
#define ejsonk_valF     ejson_valFk
#define ejsonk_valS     ejson_valSk
#define ejsonk_valP     ejson_valPk
#define ejsonk_valR     ejson_valRk
#define ejsonk_type     ejson_valTypek
#define ejsonk_typeS    ejson_valTypeSk
#define ejsonk_len      ejson_valLenk
#define ejsonk_isTrue   ejson_valIsTruek

#define ejsoni          ejson_vali
#define ejsoni_valI     ejson_valIi
#define ejsoni_valF     ejson_valFi
#define ejsoni_valS     ejson_valSi
#define ejsoni_valP     ejson_valPi
#define ejsoni_valR     ejson_valRi
#define ejsoni_type     ejson_valTypei
#define ejsoni_typeS    ejson_valTypeSi
#define ejsoni_len      ejson_valLeni
#define ejsoni_isTrue   ejson_valIsTruei

#define ejsonp          ejson_valp
#define ejsonp_valI     ejson_valIp
#define ejsonp_valF     ejson_valFp
#define ejsonp_valS     ejson_valSp
#define ejsonp_valP     ejson_valPp
#define ejsonp_valR     ejson_valRp
#define ejsonp_type     ejson_valTypep
#define ejsonp_typeS    ejson_valTypeSp
#define ejsonp_len      ejson_valLenp
#define ejsonp_isTrue   ejson_valIsTruep


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

estr ejson_toSk(eobj o, constr rawk, estr* out, eopts opts);
estr ejson_toSi(eobj o, u32    idx , estr* out, eopts opts);
estr ejson_toSp(eobj o, constr keys, estr* out, eopts opts);

#define ejsonk_toS  ejson_toSk
#define ejsoni_toS  ejson_toSi
#define ejsonp_toS  ejson_toSp


/** -----------------------------------------------------
 *
 *  ejson take and free
 *
 *  -----------------------------------------------------
 */
eobj ejson_takeH(eobj r);               // for EOBJ, EARR
eobj ejson_takeT(eobj r);               // for EOBJ, EARR
eobj ejson_takeO(eobj r, eobj      o);  // for EOBJ, EARR
eobj ejson_takeK(eobj r, constr rawk);  // for EOBJ, EARR
eobj ejson_takeI(eobj r, int     idx);  // for EARR
eobj ejson_takeP(eobj r, constr path);  // for EOBJ, EARR

int  ejson_freeH(eobj r);               // for EOBJ, EARR
int  ejson_freeT(eobj r);               // for EOBJ, EARR
int  ejson_freeO(eobj r, eobj    obj);  // for EOBJ, EARR
int  ejson_freeK(eobj r, constr rawk);  // for EOBJ, EARR
int  ejson_freeI(eobj r, int     idx);  // for EARR
int  ejson_freeP(eobj r, constr path);  // for EOBJ, EARR

int  ejson_freeHEx(eobj r,              eobj_rls_ex_cb rls, eval prvt);
int  ejson_freeTEx(eobj r,              eobj_rls_ex_cb rls, eval prvt);
int  ejson_freeOEx(eobj r, eobj    obj, eobj_rls_ex_cb rls, eval prvt);
int  ejson_freeKEx(eobj r, constr rawk, eobj_rls_ex_cb rls, eval prvt);
int  ejson_freeIEx(eobj r, int     idx, eobj_rls_ex_cb rls, eval prvt);
int  ejson_freePEx(eobj r, constr pkey, eobj_rls_ex_cb rls, eval prvt);

#define ejsonk_free     ejson_freeK
#define ejsoni_free     ejson_freeI
#define ejsonp_free     ejson_freeP

#define ejsonk_freeEx   ejson_freeKEx
#define ejsoni_freeEx   ejson_freeIEx
#define ejsonp_freeEx   ejson_freePEx

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

int  ejson_cmpIk(eobj r, constr rawk, i64    val);
int  ejson_cmpFk(eobj r, constr rawk, f64    val);
int  ejson_cmpSk(eobj r, constr rawk, constr str);

int  ejson_cmpIi(eobj r, u32    idx , i64    val);
int  ejson_cmpFi(eobj r, u32    idx , f64    val);
int  ejson_cmpSi(eobj r, u32    idx , constr str);

int  ejson_cmpIp(eobj r, constr keys, i64    val);
int  ejson_cmpFp(eobj r, constr keys, f64    val);
int  ejson_cmpSp(eobj r, constr keys, constr str);

#define ejsonk_cmpI     ejson_cmpIk
#define ejsonk_cmpF     ejson_cmpFk
#define ejsonk_cmpS     ejson_cmpSk

#define ejsoni_cmpI     ejson_cmpIi
#define ejsoni_cmpF     ejson_cmpFi
#define ejsoni_cmpS     ejson_cmpSi

#define ejsonp_cmpI     ejson_cmpIp
#define ejsonp_cmpF     ejson_cmpFp
#define ejsonp_cmpS     ejson_cmpSp

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

eobj  ejson_firstr(eobj r, constr rawk);
eobj  ejson_lastr (eobj r, constr rawk);

eobj  ejson_firstp(eobj r, constr keys);
eobj  ejson_lastp (eobj r, constr keys);

#define ejsonr_first    ejson_firstr
#define ejsonr_last     ejson_lastr

#define ejsonp_first    ejson_firstp
#define ejsonp_last     ejson_lastp

#define ejson_foreach( r,     itr)   for(itr = ejson_first (r    ); (itr); itr = ejson_next(itr))
#define ejson_foreachr(r, rk, itr)   for(itr = ejson_firstr(r, rk); (itr); itr = ejson_next(itr))
#define ejson_foreachk(r, ks, itr)   for(itr = ejson_firstp(r, ks); (itr); itr = ejson_next(itr))

#define ejson_foreach_s( r,     itr) for(eobj _INNER_ = ejson_first (r    ), itr; (itr = _INNER_, _INNER_ = ejson_next(_INNER_), itr); )
#define ejson_foreachr_s(r, rk, itr) for(eobj _INNER_ = ejson_firstr(r, rk), itr; (itr = _INNER_, _INNER_ = ejson_next(_INNER_), itr); )
#define ejson_foreachk_s(r, ks, itr) for(eobj _INNER_ = ejson_firstp(r, ks), itr; (itr = _INNER_, _INNER_ = ejson_next(_INNER_), itr); )

#define ejsonr_foreach      ejson_foreachr
#define ejsonr_foreach_s    ejson_foreachr_s

#define ejsonp_foreach      ejson_foreachp
#define ejsonp_foreach_s    ejson_foreachp_s

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
eobj ejson_setTk(eobj r, constr rawk, etypeo   t);
eobj ejson_setIk(eobj r, constr rawk, i64    val);
eobj ejson_setFk(eobj r, constr rawk, f64    val);
eobj ejson_setSk(eobj r, constr rawk, constr str);
eobj ejson_setPk(eobj r, constr rawk, constr ptr);
eobj ejson_setRk(eobj r, constr rawk, uint   len);

eobj ejson_setTi(eobj r, u32    idx , etypeo   t);
eobj ejson_setIi(eobj r, u32    idx , i64    val);
eobj ejson_setFi(eobj r, u32    idx , f64    val);
eobj ejson_setSi(eobj r, u32    idx , constr str);
eobj ejson_setPi(eobj r, u32    idx , constr ptr);
eobj ejson_setRi(eobj r, u32    idx , uint   len);

eobj ejson_setTp(eobj r, constr rawk, etypeo   t);
eobj ejson_setIp(eobj r, constr rawk, i64    val);
eobj ejson_setFp(eobj r, constr rawk, f64    val);
eobj ejson_setSp(eobj r, constr rawk, constr str);
eobj ejson_setPp(eobj r, constr rawk, constr ptr);
eobj ejson_setRp(eobj r, constr rawk, uint   len);

#define ejsonk_setT     ejson_setTk
#define ejsonk_setI     ejson_setIk
#define ejsonk_setF     ejson_setFk
#define ejsonk_setS     ejson_setSk
#define ejsonk_setP     ejson_setPk
#define ejsonk_setR     ejson_setRk

#define ejsoni_setT     ejson_setTi
#define ejsoni_setI     ejson_setIi
#define ejsoni_setF     ejson_setFi
#define ejsoni_setS     ejson_setSi
#define ejsoni_setP     ejson_setPi
#define ejsoni_setR     ejson_setRi

#define ejsonp_setT     ejson_setTp
#define ejsonp_setI     ejson_setIp
#define ejsonp_setF     ejson_setFp
#define ejsonp_setS     ejson_setSp
#define ejsonp_setP     ejson_setPp
#define ejsonp_setR     ejson_setRp

/// -----------------------------------------------------
//! @brief ejson substitute string
///
/// @return the modified obj if setted
///
eobj ejson_subSr(eobj root, constr keys, constr from, constr to);
eobj ejson_subSp(eobj root, constr rawk, constr from, constr to);

#define ejsonk_subS     ejson_subSk
#define ejsonp_subS     ejson_subSp

/// -----------------------------------------------------
/// @brief ejson counter
///
/// @note:
///     1. if NUM obj not exsit in EOBJ, will create automatically
///     2. only support NUM obj if target obj exist
///     3. return LLONG_MIN if failed
///
i64  ejson_pp  (eobj o);                // increase 1
i64  ejson_mm  (eobj o);                // decrease 1
i64  ejson_incr(eobj o, i64 v);         // increase v
i64  ejson_decr(eobj o, i64 v);         // decrease v

i64  ejsonk_pp  (eobj r, constr rawk);
i64  ejsonk_mm  (eobj r, constr rawk);
i64  ejsonk_incr(eobj r, constr rawk, i64 v);
i64  ejsonk_decr(eobj r, constr rawk, i64 v);

i64  ejsoni_pp  (eobj r, u32    idx );
i64  ejsoni_mm  (eobj r, u32    idx );
i64  ejsoni_incr(eobj r, u32    idx , i64 v);
i64  ejsoni_decr(eobj r, u32    idx , i64 v);

i64  ejsonp_pp  (eobj r, constr keys);
i64  ejsonp_mm  (eobj r, constr keys);
i64  ejsonp_incr(eobj r, constr keys, i64 v);
i64  ejsonp_decr(eobj r, constr keys, i64 v);

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
eobj  ejson_sortp(eobj r, constr keys, eobj_cmp_cb cmp);

#define ejsonr_sort  ejson_sortr
#define ejsonp_sort  ejson_sortp

/// -- ejson version
constr ejson_version();

#ifdef __cplusplus
}
#endif

#endif
