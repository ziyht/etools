/// =====================================================================================
///
///       Filename:  elist.c
///
///    Description:  an easier double link list
///
///        Version:  1.0
///        Created:  05/04/2017 04:50:18 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "eutils.h"
#include "elist.h"
#include "_eobj_header.h"

#define ELIST_VERSION "elist 1.2.0"      //

#pragma pack(push, 1)

typedef struct _elist_node_s*  _elistn;
typedef struct _elist_node_s{
    _elistn    prev;
    _elistn    next;
    _eobjh_t   h;

    eobj_t     obj;         // the return obj is the addr of val
}_elist_node_t;

typedef struct _elist_s{
    _eobjh_t   h;

    struct {
        _elistn     head;
        _elistn     tail;
        _elistn     hisn;
        uint        hisi;
    }           list;

    void*       type;
}_elist_t;
#pragma pack(pop)

/// -------------------------- micros helper ---------------------------------

#define _l_head(l)    (l)->list.head
#define _l_tail(l)    (l)->list.tail
#define _l_hisn(l)    (l)->list.hisn
#define _l_hisi(l)    (l)->list.hisi
#define _l_h(l)       (l)->h
#define _l_len(l)     _eobjh_len(_l_h(l))
#define _l_typeco(l)  _eobjh_type_co(_l_h(l))

#define _l_free(l)    efree(l)

#define _n_new(l)     _eobj_calloc(sizeof(_elist_node_t) - sizeof(eval_t) + (l))
#define _n_newT()     _n_new(0)
#define _n_newI()     _n_new(sizeof(i64))
#define _n_newF()     _n_new(sizeof(double))
#define _n_newR(l)    _n_new((l) + 1)
#define _n_newS(l)    _n_new((l) + 1)
#define _n_newP()     _n_new(sizeof(void*))
#define _n_free(n)    _eobj_free(n)

#define _n_type(n)    _eobjh_type_co((n)->h)
#define _n_typec(n)   _eobjh_type_c((n)->h)
#define _n_typeo(n)   _eobjh_type_o((n)->h)
#define _n_typen(n)   _eobjh_type_con((n)->h)
#define _n_len(n)     _eobjh_len((n)->h)
#define _n_isChild(n) _eobjh_linked((n)->h)
#define _n_prev(n)    (n)->prev
#define _n_next(n)    (n)->next

#define _n_o(n)      &(n)->obj
#define _n_valI(n)    (n)->obj.i
#define _n_valF(n)    (n)->obj.f
#define _n_valS(n)    (n)->obj.r
#define _n_valR(n)    (n)->obj.r
#define _n_valP(n)    (n)->obj.p

#define _o_n(o)       container_of(o, _elist_node_t, obj)
#define _o_free(o)    _n_free(_o_n(o))
#define _o_valI(o)    _eobj_valI(o)
#define _o_valF(o)    _eobj_valF(o)
#define _o_valS(o)    _eobj_valS(o)
#define _o_valP(o)    _eobj_valP(o)
#define _o_valR(o)    _eobj_valR(o)

/// _elist_init(elist l)
#define _elist_init(l)                              \
do{                                                 \
    memset(l, 0, sizeof(_elist_t));                 \
                                                    \
    _l_typeco(l) = _ELIST_CO_OBJ;                   \
}while(0)

#define _elist_push(l, n)                                               \
do{                                                                     \
    _n_isChild(n) = 1;                                                  \
                                                                        \
    if(!_l_head(l)){_l_head(l) =         _l_tail(l)  = n;}              \
                                                                        \
    else           {_n_next(n) =         _l_head(l)     ;               \
                    _l_head(l) = _n_prev(_l_head(l)) = n;               \
                    if(_l_hisi(l)) _l_hisi(l)++         ;}              \
        \
    _l_len(l)++;    \
}while(0)

#define _elist_takeH(l, n)                                              \
do{                                                                     \
    n = _l_head(l);                                                     \
                                                                        \
    if(1 == _l_len(l)){_l_head(l) = _l_tail(l) = NULL      ;}           \
                                                                        \
    else              {_l_head(l)              = _n_next(n);            \
                       _n_prev(_n_next(n))     = NULL      ;            \
                       if(_l_hisi(l)) _l_hisi(l)--         ;            \
                      }                                                 \
                                                                        \
    _l_len(l)--;                                                        \
                                                                        \
    _n_next(n)    = NULL;                                               \
    _n_isChild(n) = 0;                                                  \
}while(0)

#define _elist_appd(l, n)                                               \
do{                                                                     \
    _n_isChild(n) = 1;                                                  \
                                                                        \
    if(!_l_head(l)){_l_head(l) =         _l_tail(l)  = n;}              \
                                                                        \
    else           {_n_prev(n) =         _l_tail(l)     ;               \
                    _l_tail(l) = _n_next(_l_tail(l)) = n;}              \
                                                                        \
    _l_len(l)++;                                                        \
}while(0)


#define _elist_takeT(l, n)                                              \
do{                                                                     \
    n = _l_tail(l);                                                     \
                                                                        \
    if(1 == _l_len(l)){_l_head(l) = _l_tail(l) = NULL;}                 \
                                                                        \
    else              {_l_tail(l)              = _n_prev(n);            \
                       _n_next(_n_prev(n))     = NULL      ;            \
                       if(_l_hisi(l) && _l_hisn(l) == n)                \
                       {_l_hisn(l) = _n_prev(n); _l_hisi(l)--;}         \
                      }                                                 \
                                                                        \
    _l_len(l)--;                                                        \
                                                                        \
    _n_prev(n)    = NULL;                                               \
    _n_isChild(n) = 0;                                                  \
}while(0)

/// _elist_at(elist l, int idx, _elistn n)  // the n is the output search node
#define _elist_at(l, idx, n)                                            \
do{                                                                     \
    uint i;                                                             \
                                                                        \
    if(idx >= _l_hisi(l))   { if(idx - _l_hisi(l) < _l_len(l) - 1 - idx) { n = _l_hisi(l) ? _l_hisn(l) : _l_head(l); for(i = _l_hisi(l)   ; i != idx; i++) n = _n_next(n);}     \
                              else                                       { n = _l_tail(l);                           for(i = _l_len(l) - 1; i != idx; i--) n = _n_prev(n);}     \
                            }                                           \
    else                    { if(_l_hisi(l) - idx > idx)                 { n = _l_head(l);                           for(i = 0            ; i != idx; i++) n = _n_next(n);}     \
                              else                                       { n = _l_hisn(l);                           for(i = _l_hisi(l)   ; i != idx; i--) n = _n_prev(n);}     \
                            }                                           \
    _l_hisi(l) = idx;                                                   \
    _l_hisn(l) = n;                                                     \
                                                                        \
}while(0)

#define _elist_find(l, o, i)                                            \
do{                                                                     \
    uint cnt;                                                           \
                                                                        \
    o = (eobj)_o_n(o);                                                  \
                                                                        \
    if(_l_hisi(l)){        for(i = _l_hisi(l)    , n =         _l_hisn(l) , cnt = 0; n && n != (_elistn)o; n = _n_next(n), i++) if(cnt++ > 5) break;        \
                    if(!n) for(i = _l_hisi(l) - 1, n = _n_prev(_l_hisn(l)), cnt = 0; n && n != (_elistn)o; n = _n_prev(n), i--) if(cnt++ > 5) break; }      \
    else          {        for(i = 0             , n = _l_head(l)         , cnt = 0; n && n != (_elistn)o; n = _n_next(n), i++); }                          \
                                                                                                                                                            \
}while(0)

/// _elist_take(elist l, _elistn n, int idx)
#define _elist_take(l, n, idx)                                                                          \
do{                                                                                                     \
    if   (_n_prev(n)) _n_next(_n_prev(n)) = _n_next(n);                                                 \
    else              _l_head(l)          = _n_next(n);                                                 \
                                                                                                        \
    if   (_n_next(n)){_n_prev(_l_hisn(l) = _n_next(n)) = _n_prev(n);            _l_hisi(l) = idx    ;}  \
    else             {        _l_hisn(l) = _l_tail(l)  = _n_prev(n);    if(idx) _l_hisi(l) = idx - 1;}  \
                                                                                                        \
    _l_len(l)--;                                                                                        \
                                                                                                        \
    _n_prev(n)      =                                                                                   \
    _n_next(n)      = NULL;                                                                             \
    _n_isChild(n)   = 0;                                                                                \
                                                                                                        \
}while(0)

/// -----------------------  inline compat ------------------------
#if defined(WIN32) && !defined(__cplusplus)
#define inline
#endif


/// -------------------------- elist ------------------------------
elist elist_new()
{
    elist l = emalloc(sizeof(_elist_t));

    _elist_init(l);

    return l;
}

eobj elist_newO(etypev type, uint len)
{
    _elistn n;

    switch (type) {
        case EFALSE :
        case ETRUE  :
        case ENULL  :   n = _n_new(0);  break;
        case ENUM   :
        case EPTR   :   n = _n_new(8);  break;
        case ESTR   :
        case ERAW   :
        case EOBJ   :   n = _n_new(len);  _n_len(n) = len; break;

        default     :   // todo: set err
                            return 0;

    }

    _n_typec(n) = ELL;
    _n_typeo(n) = type;

    return _n_o(n);
}

eobj  elist_pushI(elist l, i64    val){ _elistn n;          is0_ret(l, 0);                              n = _n_newI();    _n_typen(n) = _ELIST_CON_NUM_I;                    _n_valI(n) = val;             _elist_push(l, n); return _n_o(n);}
eobj  elist_pushF(elist l, f64    val){ _elistn n;          is0_ret(l, 0);                              n = _n_newF();    _n_typen(n) = _ELIST_CON_NUM_F;                    _n_valF(n) = val;             _elist_push(l, n); return _n_o(n);}
eobj  elist_pushS(elist l, constr str){ _elistn n; int len; is0_ret(l, 0); len = str ? strlen(str) : 0; n = _n_newS(len); _n_type(n)  = _ELIST_CO_STR;    _n_len(n)  = len;  memcpy(_n_valS(n), str, len); _elist_push(l, n); return _n_o(n);}
eobj  elist_pushP(elist l, conptr ptr){ _elistn n;          is0_ret(l, 0);                              n = _n_newP();    _n_type(n)  = _ELIST_CO_PTR;                       _n_valP(n) = (cptr)ptr;       _elist_push(l, n); return _n_o(n);}
eobj  elist_pushR(elist l, size_t len){ _elistn n;          is0_ret(l, 0);                              n = _n_newR(len); _n_type(n)  = _ELIST_CO_RAW;    _n_len(n)  = len;                                _elist_push(l, n); return _n_o(n);}
eobj  elist_pushO(elist l, eobj   obj){ _elistn n;          is1_ret(!l || !obj, 0);                     n = _o_n(obj); is1_ret(_n_isChild(n) || _n_typec(n) != ELL , 0);                                   _elist_push(l, n); return obj    ;}  // todo: 2 -> 1

eobj  elist_appdI(elist l, i64    val){ _elistn n;          is0_ret(l, 0);                              n = _n_newI();    _n_typen(n) = _ELIST_CON_NUM_I;                    _n_valI(n) = val;             _elist_appd(l, n); return _n_o(n);}
eobj  elist_appdF(elist l, f64    val){ _elistn n;          is0_ret(l, 0);                              n = _n_newF();    _n_typen(n) = _ELIST_CON_NUM_F;                    _n_valF(n) = val;             _elist_appd(l, n); return _n_o(n);}
eobj  elist_appdS(elist l, constr str){ _elistn n; int len; is0_ret(l, 0); len = str ? strlen(str) : 0; n = _n_newS(len); _n_type(n)  = _ELIST_CO_STR;    _n_len(n)  = len;  memcpy(_n_valS(n), str, len); _elist_appd(l, n); return _n_o(n);}
eobj  elist_appdP(elist l, conptr ptr){ _elistn n;          is0_ret(l, 0);                              n = _n_newP();    _n_type(n)  = _ELIST_CO_PTR;                       _n_valP(n) = (cptr)ptr;       _elist_appd(l, n); return _n_o(n);}
eobj  elist_appdR(elist l, size_t len){ _elistn n;          is0_ret(l, 0);                              n = _n_newR(len); _n_type(n)  = _ELIST_CO_RAW;    _n_len(n)  = len;                                _elist_appd(l, n); return _n_o(n);}
eobj  elist_appdO(elist l, eobj   obj){ _elistn n;          is1_ret(!l || !obj, 0);                     n = _o_n(obj); is1_ret(_n_isChild(n) || _n_typec(n) != ELL, 0);                                    _elist_appd(l, n); return obj    ;} // todo: 2 -> 1

inline int   elist_isEmpty(elist  l) { return l ? !_l_len(l) : -1 ; }
inline uint  elist_len    (elist  l) { return l ?  _l_len(l) : 0 ; }
inline uint  elist_size   (elist  l) { return l ?  _l_len(l) : 0 ; }

inline eobj  elist_first(elist  l){ return _l_head(l)         ? _n_o(_l_head(l))         : 0; }
inline eobj  elist_last (elist  l){ return _l_tail(l)         ? _n_o(_l_tail(l))         : 0; }
inline eobj  elist_next (eobj obj){ return _n_next(_o_n(obj)) ? _n_o(_n_next(_o_n(obj))) : 0; }
inline eobj  elist_prev (eobj obj){ return _n_prev(_o_n(obj)) ? _n_o(_n_prev(_o_n(obj))) : 0; }

eobj elist_takeH (elist l)           { _elistn n;                  is1_ret(!l || !_l_len(l), 0);                               _elist_takeH(l, n);                     return _n_o(n); }
eobj elist_takeT (elist l)           { _elistn n;                  is1_ret(!l || !_l_len(l), 0);                               _elist_takeT(l, n);                     return _n_o(n); }
eobj elist_takeAt(elist l, uint idx) { _elistn n;                  is1_ret(!l || idx >= _l_len(l), 0); _elist_at  (l, idx, n); _elist_take(l, n, idx);                 return _n_o(n); }
eobj elist_takeO (elist l, eobj obj) { _elistn n; register uint i; is1_ret(!l || !obj, 0);             _elist_find(l, obj, i); is1_elsret(n, _elist_take(l, n, i), 0); return _n_o(n); }

eobj  elist_at  (elist l, uint idx) { _elistn n; is1_ret(!l || idx >= _l_len(l), 0); _elist_at(l, idx, n); return _n_o(n); }
eobj  elist_val (elist l, uint idx) { _elistn n; is1_ret(!l || idx >= _l_len(l), 0); _elist_at(l, idx, n); return _n_o(n); }
i64   elist_valI(elist l, uint idx) { eobj o = elist_at(l, idx); if(o){ switch (_n_typen(_o_n(o))) { case _ELIST_CON_NUM_I: return _o_valI(o); case _ELIST_CON_NUM_F: return _o_valF(o); } } return 0; }
f64   elist_valF(elist l, uint idx) { eobj o = elist_at(l, idx); if(o){ switch (_n_typen(_o_n(o))) { case _ELIST_CON_NUM_I: return _o_valI(o); case _ELIST_CON_NUM_F: return _o_valF(o); } } return 0; }
cstr  elist_valS(elist l, uint idx) { eobj o = elist_at(l, idx); return (o && _n_typeo(_o_n(o)) == ESTR) ? _o_valS(o) : 0; }
cptr  elist_valP(elist l, uint idx) { eobj o = elist_at(l, idx); return (o && _n_typeo(_o_n(o)) == EPTR) ? _o_valP(o) : 0; }
cptr  elist_valR(elist l, uint idx) { eobj o = elist_at(l, idx); return (o && _n_typeo(_o_n(o)) == ERAW) ? _o_valR(o) : 0; }

etypev elist_valType(elist l, uint idx) { eobj o = elist_at(l, idx); return o ? _n_typeo(_o_n(o)) : EVAL_END; }
uint   elist_valLen (elist l, uint idx) { eobj o = elist_at(l, idx); return o ? _n_len  (_o_n(o)) : 0       ; }

int   elist_freeH(elist l)          { cptr o = elist_takeH (l     ); if(o) { _n_free(_o_n(o)); return 1; } return 0; }
int   elist_freeT(elist l)          { cptr o = elist_takeT (l     ); if(o) { _n_free(_o_n(o)); return 1; } return 0; }
int   elist_freeI(elist l, uint idx){ cptr o = elist_takeAt(l, idx); if(o) { _n_free(_o_n(o)); return 1; } return 0; }
int   elist_freeO(elist l, eobj obj)
{
    is0_ret(obj, 0);

    if(!l)
    {
        is1_ret(_n_isChild(_o_n(obj)) || _n_typec(_o_n(obj)) != ELL, 0);
        exe_ret(_o_free(obj), 1);
    }

    is1_exeret(obj = elist_takeO(l, obj), _o_free(obj), 1);

    return 0;
}

elist elist_clear(elist l)
{
    _elistn n, itr;

    is0_ret(l, 0);

    for(n = _l_head(l); (itr = n); )
    {
        n = _n_next(n);

        _n_free(itr);
    }

    _elist_init(l);

    return l;
}

elist elist_clearEx(elist l, eobj_rls_cb rls)
{
    _elistn n, itr;

    is0_ret(rls, elist_clear(l));
    is0_ret(l  , 0);

    for(n = _l_head(l); (itr = n); )
    {
        n = _n_next(n);

        rls(_n_o(itr));
        _n_free(itr);
    }

    _elist_init(l);

    return l;
}

int elist_free(elist l)
{
    _elistn n, itr;

    is0_ret(l, 0);

    for(n = _l_head(l); (itr = n); )
    {
        n = _n_next(n);
        _n_free(itr);
    }

    _l_free(l);

    return 1;
}

int   elist_freeEx(elist l, eobj_rls_cb rls)
{
    _elistn n, itr;

    is0_ret(rls, elist_free(l));
    is0_ret(l  , 0);

    for(n = _l_head(l); (itr = n); )
    {
        n = _n_next(n);

        rls(_n_o(itr));
        _n_free(itr);
    }

    _l_free(l);

    return 1;
}

constr elist_version()
{
    return ELIST_VERSION;
}
