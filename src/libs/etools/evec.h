#ifndef __EVEC_H__
#define __EVEC_H__

typedef struct evec_s * evec;

#include "eobj.h"
#include "evar.h"

#ifdef __cplusplus
extern "C" {
#endif

/** -----------------------------------------------------
 *
 *  evec basic
 *
 * ------------------------------------------------------
 */
evec evec_new(etypev type);
evec evec_newEx(int size);

bool evec_reserve(evec v);

uint evec_len (evec v);
uint evec_cap (evec v);
uint evec_size(evec v);

int  evec_clear  (evec v);
int  evec_clearEx(evec v, eobj_rls_ex_cb rls, eval prvt);

int  evec_free  (evec v);
int  evec_freeEx(evec v, eobj_rls_ex_cb rls, eval prvt);

/** -----------------------------------------------------
 *
 *  evec add
 *
 * ------------------------------------------------------
 */
bool evec_pushV(evec v, evar   var);
bool evec_pushI(evec v, i64    val);
bool evec_pushF(evec v, f64    val);
bool evec_pushS(evec v, constr str);
bool evec_pushP(evec v, conptr ptr);
bool evec_pushR(evec v, uint   len);

bool evec_appdV(evec v, evar   var);
bool evec_appdI(evec v, i64    val);
bool evec_appdF(evec v, f64    val);
bool evec_appdS(evec v, constr str);
bool evec_appdP(evec v, conptr ptr);
bool evec_appdR(evec v, uint   len);

bool evec_addV(evec v, uint i, evar   var);
bool evec_addI(evec v, uint i, i64    str);
bool evec_addF(evec v, uint i, f64    str);
bool evec_addS(evec v, uint i, constr str);     // hold as ptr actually
bool evec_addP(evec v, uint i, conptr str);     // hold as ptr actually
bool evec_addR(evec v, uint i, uint   len);     // hold as ptr actually

bool evec_addVEx(evec v, uint i, eval val, int len);

/** -----------------------------------------------------
 *
 *  evec val
 *
 * ------------------------------------------------------
 */
cptr evec_at  (evec v, uint idx);

cptr evec_val (evec v, uint idx);
i64  evec_valI(evec v, uint idx);
f64  evec_valF(evec v, uint idx);
cstr evec_valS(evec v, uint idx);
cptr evec_valP(evec v, uint idx);
cptr evec_valR(evec v, uint idx);


#ifdef __cplusplus
}
#endif

#endif
