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
///

#include "eutils.h"

#include "evar.h"
#include "estr.h"
#include "estr_p.h"

#define EVAR_VERISON "evar 0.2.1"

static const u8 _len_map[] = __EVAR_ITEM_LEN_MAP;

#define _vp_pptr(vp, esize, i) ((char*)((vp->type & __ETYPEV_PTR_MASK) ? vp->v.p : &vp->v) + (esize * i))

evar  evar_gen (etypev t, int cnt, int size)
{
    int len; bool p, a;

    a = t & __ETYPEV_ARR_MASK;
    p = t & __ETYPEV_PTR_MASK;
    t = t & __ETYPEV_VAR_MASK;

    is1_ret(t == E_NAV, EVAR_NAV);

    if     (t    <= E_RAW)       size = _len_map[t];     // get internal size
    else if(size <  1    )       size = 1;               // recorrect size
    if     (cnt  <  1    )       cnt  = 1;

    len = cnt * size + (size == 1 ? 1 : 0);
    if(len > 8) p = 1;

    if(p)
    {
        return (evar){0,t | __ETYPEV_PTR_MASK,
                        size,
                        cnt,
                        EVAL_P(ecalloc(cnt, size))
                      };
    }

    if(cnt > 1 || a)
    {
        return (evar){0,t | __ETYPEV_ARR_MASK,
                        size,
                        cnt,
                        EVAL_ZORE
                      };
    }

    return (evar){0,t,
                    size,
                    1,
                    EVAL_ZORE
                  };
}

static evarp __evar_newa(etypev t, int cnt, int esize)
{
    evarp v;

    v = ecalloc(cnt * esize + sizeof(*v) - sizeof(eval) + (esize == 1 ? 1 : 0), 1);

    v->type  = t | __ETYPEV_NEW_MASK | __ETYPEV_ARR_MASK;
    v->esize = esize;
    v->cnt   = cnt;

    return v;
}

static evarp __evar_newp(etypev t, int cnt, int esize)
{
    evarp v;

    v = emalloc(sizeof(*v));

    v->type  = t | __ETYPEV_NEW_MASK | __ETYPEV_PTR_MASK;
    v->esize = esize;
    v->cnt   = cnt;
    v->v.p   = ecalloc(cnt, esize);

    return v;
}

evarp evar_new(etypev t, int cnt, int size)
{
    int len; bool a, p;

    a = t & __ETYPEV_ARR_MASK;
    p = t & __ETYPEV_PTR_MASK;
    t = t & __ETYPEV_VAR_MASK;

    is1_ret(t == E_NAV, 0);

    if     (t    <= E_RAW)       size = _len_map[t];     // get internal size
    else if(size <  1    )       size = 1;               // recorrect size
    if     (cnt  <  1    )       cnt  = 1;

    if(a)   return __evar_newa(t, cnt, size);
    if(p)   return __evar_newp(t, cnt, size);

    //! auto mode
    len = cnt * size + (size == 1 ? 1 : 0);
    if(len > 8)
    {
        return __evar_newp(t, cnt, size);
    }
    else
    {
        evarp v = emalloc(sizeof(*v));

        v->v = EVAL_ZORE;

        if(cnt == 1)
        {
            v->type = t | __ETYPEV_NEW_MASK;
            v->cnt  = 0;
        }
        else
        {
            v->type = t | __ETYPEV_NEW_MASK | __ETYPEV_ARR_MASK;
            v->cnt  = cnt;
        }

        return v;
    }

    return 0;
}

uint evar_clear(evarp vp)
{
    estr* base; uint cnt;

    is0_ret(vp || vp->type == E_NAV, 0);

    cnt = 1;

    if((vp->type & __ETYPEV_VAR_MASK) == __E_EVAR)
    {
        cnt = evar_free(vp->v.p);
        *vp = EVAR_NAV;
    }
    else if(vp->type & __ETYPEV_PTR_MASK)
    {
        cnt  = vp->cnt;
        base = vp->v.p;
    }
    else if(vp->type & __ETYPEV_ARR_MASK)
    {
        cnt  = vp->cnt;
        base = vp->v.sa;
    }

    if((vp->type & __ETYPEV_VAR_MASK) == E_STR || (vp->type & __ETYPEV_VAR_MASK) == E_RAW)
    {
        uint i;

        for(i = 0; i <  vp->cnt; i++)
            _s_free(base[i]);
    }

    memset(base, 0, vp->cnt * vp->esize);

    return cnt;
}

uint evar_free(evarp vp)
{
    estr* base; uint cnt;

    is0_ret(vp || vp->type == E_NAV, 0);

    cnt = 1;

    if((vp->type & __ETYPEV_VAR_MASK) == __E_EVAR)
    {
        cnt = evar_free(vp->v.p);
        *vp = EVAR_NAV;
    }
    else if(vp->type & __ETYPEV_PTR_MASK)
    {
        cnt  = vp->cnt;
        base = vp->v.p;
    }
    else if(vp->type & __ETYPEV_ARR_MASK)
    {
        cnt  = vp->cnt;
        base = vp->v.sa;
    }

    if((vp->type & __ETYPEV_VAR_MASK) == E_STR || (vp->type & __ETYPEV_VAR_MASK) == E_RAW)
    {
        uint i;

        for(i = 0; i <  vp->cnt; i++)
            _s_free(base[i]);
    }

    if(vp->type & __ETYPEV_PTR_MASK)
        efree(vp->v.p);

    if(vp->type & __ETYPEV_NEW_MASK)
        efree(vp);
    else
        *vp = EVAR_NAV;

    return cnt;
}

uint  evar_cnt  (evarp vp)  { return vp ? vp->cnt             : 0; }
uint  evar_esize(evarp vp)  { return vp ? vp->esize           : 0; }
uint  evar_space(evarp vp)  { return vp ? vp->cnt * vp->esize : 0; }

etypev evar_type( evarp vp) { return vp ? vp->type                     : E_NAV;}
bool   evar_isArr(evarp vp) { return vp ? vp->type & __ETYPEV_ARR_MASK : 0;  }
bool   evar_isPtr(evarp vp) { return vp ? vp->type & __ETYPEV_PTR_MASK : 0;  }

bool evar_set(evarp vp, uint idx, conptr  in, int ilen)
{
    is1_ret(!vp || vp->cnt <= idx, false);

    switch (vp->type & __ETYPEV_VAR_MASK)
    {

        case E_I8 :
        case E_I16:
        case E_I32:
        case E_I64:

        case E_U8 :
        case E_U16:
        case E_U32:
        case E_U64:

        case E_F32:
        case E_F64:

        case E_PTR:
        case E_USER:
                    is1_ret(ilen != vp->esize, false);

                    memcpy(_vp_pptr(vp, vp->esize, idx), in, vp->esize);

                    return true;

        case E_STR:
        case E_RAW: __estr_wrtB((estr*)_vp_pptr(vp, vp->esize, idx), in, ilen);

                    return true;

    }

    return false;
}

bool evar_setI(evarp vp, uint idx, i64    val)          { is1_ret(!vp || (vp->type & __ETYPEV_VAR_MASK) != E_I64 || vp->cnt <= idx, false);            *(i64   *)_vp_pptr(vp, 8, idx) = val; return true; }
bool evar_setF(evarp vp, uint idx, f64    val)          { is1_ret(!vp || (vp->type & __ETYPEV_VAR_MASK) != E_F64 || vp->cnt <= idx, false);            *(f64   *)_vp_pptr(vp, 8, idx) = val; return true; }
bool evar_setS(evarp vp, uint idx, constr str)          { is1_ret(!vp || (vp->type & __ETYPEV_VAR_MASK) != E_STR || vp->cnt <= idx, false); __estr_wrtS((estr  *)_vp_pptr(vp, 8, idx), str); return true; }
bool evar_setP(evarp vp, uint idx, conptr ptr)          { is1_ret(!vp || (vp->type & __ETYPEV_VAR_MASK) != E_PTR || vp->cnt <= idx, false);            *(conptr*)_vp_pptr(vp, 8, idx) = ptr; return true; }
bool evar_setR(evarp vp, uint idx, conptr  in, int ilen){ is1_ret(!vp || (vp->type & __ETYPEV_VAR_MASK) != E_RAW || vp->cnt <= idx, false); __estr_wrtB((estr * )_vp_pptr(vp, 8, idx), in, ilen); return true; }

evar evar_at  (evarp vp, uint idx)
{
    evar out;

    is1_ret(!vp || vp->cnt <= idx, EVAR_NAV);

    out.type  = (vp->type & __ETYPEV_VAR_MASK) | __ETYPEV_PTR_MASK;
    out.esize = vp->esize;
    out.cnt   = 1;
    out.v.p   = _vp_pptr(vp, vp->esize, idx);

    return out;
}

evar evar_val (evarp vp, uint idx)
{
    evar out;

    is1_ret(!vp || vp->cnt <= idx, EVAR_NAV);

    out.type  = (vp->type & __ETYPEV_VAR_MASK) | __ETYPEV_PTR_MASK;
    out.esize = vp->esize;
    out.cnt   = 1;
    out.v.p   = _vp_pptr(vp, vp->esize, idx);

    return out;

#if 0
    switch (vp->type & __ETYPEV_VAR_MASK)
    {
        case E_I8  :
        case E_I16 :
        case E_I32 :
        case E_I64 :

        case E_U8  :
        case E_U16 :
        case E_U32 :
        case E_U64 :

        case E_F32 :
        case E_F64 :

        case E_PTR :
        case E_USER: return EVAR_NAV;

    }

#endif
}

i64  evar_valI(evarp vp, uint idx) { is1_ret(!vp || (vp->type & __ETYPEV_VAR_MASK) != E_I64 || vp->cnt <= idx, 0); return *(i64   *)_vp_pptr(vp, 8, idx); }
f64  evar_valF(evarp vp, uint idx) { is1_ret(!vp || (vp->type & __ETYPEV_VAR_MASK) != E_F64 || vp->cnt <= idx, 0); return *(f64   *)_vp_pptr(vp, 8, idx); }
cstr evar_valS(evarp vp, uint idx) { is1_ret(!vp || (vp->type & __ETYPEV_VAR_MASK) != E_STR || vp->cnt <= idx, 0); return *(cstr  *)_vp_pptr(vp, 8, idx); }
cptr evar_valP(evarp vp, uint idx) { is1_ret(!vp || (vp->type & __ETYPEV_VAR_MASK) != E_PTR || vp->cnt <= idx, 0); return *(cptr  *)_vp_pptr(vp, 8, idx); }
cptr evar_valR(evarp vp, uint idx) { is1_ret(!vp || (vp->type & __ETYPEV_VAR_MASK) != E_RAW || vp->cnt <= idx, 0); return *(cptr  *)_vp_pptr(vp, 8, idx); }
