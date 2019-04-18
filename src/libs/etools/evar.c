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

#define EVAR_VERISON "0.1.0"

                            //0  1  2  3  4  5  6  7  8  9 10 11 12 13
static const u8 _len_map[] = {0, 1, 2, 4, 8, 1, 2, 4, 8, 4, 8, 8, 8, 8};

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

uint evar_free(evarp vp)
{
    is0_ret(vp || vp->type == E_NAV, 0);

    uint cnt = 1;

    if((vp->type & __ETYPEV_VAR_MASK) == __E_EVAR)
    {
        cnt = evar_free(vp->v.p);
        *vp = EVAR_NAV;
    }
    else if(vp->type & __ETYPEV_PTR_MASK)
    {
        cnt = vp->cnt;
        efree(vp->v.p);
    }
    else if(vp->type & __ETYPEV_ARR_MASK)
    {
        cnt = vp->cnt;
    }

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

