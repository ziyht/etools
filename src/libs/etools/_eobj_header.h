/// =====================================================================================
///
///       Filename:  _eobj_header.h
///
///    Description:  this is a eobj header file for internal using
///
///        Version:  1.0
///        Created:  02/28/2017 08:51:34 PM
///       Revision:  none
///       Compiler:  gcc
///         Needed:  eobj.h
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#ifndef __EOBJ_HEADER_H__
#define __EOBJ_HEADER_H__

#include "etype.h"
#include "ealloc.h"
#include "eobj.h"
#include "eutils.h"

//! ================================================
//! eobj header type
//! ================================================

#pragma pack(1)

typedef union {
    struct {
        uint t_c    : 4;         // class type: ejson elist erb edict ...
        uint t_o    : 4;         // obj   type: FALSE TRUE NUM ...
        uint t_n    : 1;         // num   type: int float
        uint t_k    : 2;         // key   type: key_s key_i no_key
    }__1;

    struct {
        uint t_c    :  4;
        uint t_on   :  5;
        uint keys   :  1;       // is a str key ?
        uint hkey   :  1;       // have a key   ?
    }__2;

    uint     t_co   : 8;
    uint     t_con  : 9;
    uint     t_all  : 11;

    struct {
        uint __     : 11;
        uint linked :  1;       // linked?

        uint lentype:  4;       // 0: no len
                                // 1: 16 len
                                // 2: 48 len
                                // 3:  8 len  8 cap
                                // 4: 24 len 24 cap
                                // 5:  8 len  8 ref
                                // 6: 24 len 24 ref
                                // 7: 32 len 32 ref
    }__3;

}_ehdt_t, * _ehdt_p;

#pragma pack()

#define _EHDT_TYPE_OFFSET_O         4 // offsetof(_ehdt_t, __1.t_o) // 4 object type offset
#define _EHDT_TYPE_OFFSET_N_F       8 // offsetof(_ehdt_t, __1.t_n) // 8 float type offset

//! _ehdt_type_k()
#define _EHDT_NOKEY         0   // 00
#define _EHDT_KEYI          2   // 10
#define _EHDT_KEYS          3   // 11

#define _EHDT_NUMI          ENUM
#define _EHDT_NUMF          (ENUM | 1 << 4)

#define _ehdt_type_c(t)     (t).__1.t_c
#define _ehdt_type_o(t)     (t).__1.t_o
#define _ehdt_type_n(t)     (t).__1.t_n
#define _ehdt_type_k(t)     (t).__1.t_k
#define _ehdt_type_co(t)    (t).t_co
#define _ehdt_type_on(t)    (t).__2.t_on
#define _ehdt_type_con(t)   (t).t_con

#define _ehdt_keys(t)       (t).__2.keys
#define _ehdt_hkey(t)       (t).__2.hkey
#define _ehdt_linked(t)     (t).__3.linked
#define _ehdt_refed(t)      (t).__3.refed

//! ================================================
//! eobj header
//! ================================================
#pragma pack(push, 1)
typedef struct _eobj_header_s{
    uint        _len;       // 32
    uint        _ref: 16;   // 16
    _ehdt_t     _typ;       // 16
}_ehdr_t, * _ehdr_p;
#pragma pack(pop)

#define _ehdr_len(h)       (h)._len
#define _ehdr_ref(h)       (h)._ref
#define _ehdr_typ(h)       (h)._typ

#define _ehdr_type_c(h)    _ehdt_type_c(_ehdr_typ(h))
#define _ehdr_type_o(h)    _ehdt_type_o(_ehdr_typ(h))
#define _ehdr_type_n(h)    _ehdt_type_n(_ehdr_typ(h))
#define _ehdr_type_k(h)    _ehdt_type_k(_ehdr_typ(h))
#define _ehdr_type_co(h)   _ehdt_type_co(_ehdr_typ(h))
#define _ehdr_type_on(h)   _ehdt_type_on(_ehdr_typ(h))
#define _ehdr_type_con(h)  _ehdt_type_con(_ehdr_typ(h))

#define _ehdr_keys(h)      _ehdt_keys(_ehdr_typ(h))
#define _ehdr_hkey(h)      _ehdt_hkey(_ehdr_typ(h))
#define _ehdr_linked(h)    _ehdt_linked(_ehdr_typ(h))
#define _ehdr_refed(h)     _ehdt_refed(_ehdr_typ(h))


//! ================================================
//! types definition
//! ================================================
#undef  _XX
#define _XX(ectype)                                                           \
    _ ## ectype ## _CO_FALSE  = ((ectype) | (EFALSE << _EHDT_TYPE_OFFSET_O)), \
    _ ## ectype ## _CO_TRUE   = ((ectype) | (ETRUE  << _EHDT_TYPE_OFFSET_O)), \
    _ ## ectype ## _CO_NULL   = ((ectype) | (ENULL  << _EHDT_TYPE_OFFSET_O)), \
    _ ## ectype ## _CO_NUM    = ((ectype) | (ENUM   << _EHDT_TYPE_OFFSET_O)), \
    _ ## ectype ## _CO_PTR    = ((ectype) | (EPTR   << _EHDT_TYPE_OFFSET_O)), \
    _ ## ectype ## _CO_STR    = ((ectype) | (ESTR   << _EHDT_TYPE_OFFSET_O)), \
    _ ## ectype ## _CO_RAW    = ((ectype) | (ERAW   << _EHDT_TYPE_OFFSET_O)), \
    _ ## ectype ## _CO_OBJ    = ((ectype) | (EOBJ   << _EHDT_TYPE_OFFSET_O)), \
    _ ## ectype ## _CON_FALSE = ((ectype) | (EFALSE << _EHDT_TYPE_OFFSET_O)), \
    _ ## ectype ## _CON_TRUE  = ((ectype) | (ETRUE  << _EHDT_TYPE_OFFSET_O)), \
    _ ## ectype ## _CON_NULL  = ((ectype) | (ENULL  << _EHDT_TYPE_OFFSET_O)), \
    _ ## ectype ## _CON_NUM_I = ((ectype) | (ENUM   << _EHDT_TYPE_OFFSET_O)), \
    _ ## ectype ## _CON_NUM_F = ((ectype) | (ENUM   << _EHDT_TYPE_OFFSET_O) | (1 << _EHDT_TYPE_OFFSET_N_F)), \
    _ ## ectype ## _CON_PTR   = ((ectype) | (EPTR   << _EHDT_TYPE_OFFSET_O)), \
    _ ## ectype ## _CON_STR   = ((ectype) | (ESTR   << _EHDT_TYPE_OFFSET_O)), \
    _ ## ectype ## _CON_RAW   = ((ectype) | (ERAW   << _EHDT_TYPE_OFFSET_O)), \
    _ ## ectype ## _CON_OBJ   = ((ectype) | (EOBJ   << _EHDT_TYPE_OFFSET_O))  \

typedef enum _eo_type_map{

    _EFALSE = EFALSE ,         // 0
    _ETRUE  = ETRUE,           // 1
    _ENULL  = ENULL,           // 2
    _ENUM   = ENUM,            // 3
    _EPTR   = EPTR,            // 4
    _ESTR   = ESTR,            // 5
    _ERAW   = ERAW,            // 6
    _EOBJ   = EOBJ,            // 7

    _ENUM_I = _EHDT_NUMI,
    _ENUM_F = _EHDT_NUMF,

    _EO_NOKEY = _EHDT_NOKEY,
    _EO_KEYI  = _EHDT_KEYI,
    _EO_KEYS  = _EHDT_KEYS,

    _XX(ELL),
    _XX(EDICT),
    _XX(ERB),
    _XX(ESL),
    _XX(EQL),

}_eotype;


//! ================================================
//! macro definition
//! ================================================
#pragma pack(1)

typedef struct _eroot_s{
    _ehdt_t     typ;
    eobj        obj;
}_eroot_t, * _eroot_p;

typedef struct _enode_s{
    _ehdr_t     hdr;
    eobj        obj;
}_enode_t;

#pragma pack()

#define _eobj_newm(len)       emalloc((len))
#define _eobj_newc(len)       ecalloc((len), 1)
#define _eobj_newr(o, len)    erealloc(o, (len))
#define _eobj_free(eo)        efree(eo)

#define __cur_type(t, post)   _ ## t ## _ ## post
#define _cur_type(t, post)    __cur_type(t, post)

#define _k_keyS(k)          (k).s
#define _k_keyU(k)          (k).u
#define _k_keyI(k)          (k).i
#define _k_lenS(k)          sizeof(_k_keyS(k))
#define _k_lenU(k)          sizeof(_k_keyU(k))
#define _k_lenI(k)          sizeof(_k_keyI(k))

//! -- root node macros --
//!
#define _r_new()            _eobj_newc(sizeof(_RNODE_TYPE))
#define _r_free(r)          _eobj_free(r)

#define _r_o(r)            &(r)->_RNODE_OBJ_FIELD
#define _r_h(r)             (r)->hdr
#define _r_typeco(r)        _ehdr_type_co(_r_h(r))
#define _r_typeco_set(r)    _ehdr_type_co(_r_h(r)) = _cur_type(_CUR_C_TYPE, CO_OBJ)
#define _r_keys(r)          _ehdr_keys(_r_h(r))

//! -- data node macros --
//!
#define _n_newm(l)          _eobj_newm(   sizeof(_DNODE_TYPE) - sizeof(eobj_t) + l)
#define _n_newc(l)          _eobj_newc(   sizeof(_DNODE_TYPE) - sizeof(eobj_t) + l)
#define _n_newr(n, l)       _eobj_newr(n, sizeof(_DNODE_TYPE) - sizeof(eobj_t) + l)
#define _n_init(n)          memset(n,  0, sizeof(_DNODE_TYPE) - sizeof(eobj_t));
#define _n_newT(n)          n = _n_newc((0));
#define _n_newNm(n)         n = _n_newm(8);             _n_init(n);
#define _n_newNc(n)         n = _n_newc(8);

#define _n_newTF(n)         _n_newT(n);                                            _n_typecon(n) = _cur_type(_CUR_C_TYPE, CON_FALSE);
#define _n_newTT(n)         _n_newT(n);                                            _n_typecon(n) = _cur_type(_CUR_C_TYPE, CON_TRUE );
#define _n_newTN(n)         _n_newT(n);                                            _n_typecon(n) = _cur_type(_CUR_C_TYPE, CON_NULL );
#define _n_newIc(n)         _n_newNc(n);                                           _n_typecon(n) = _cur_type(_CUR_C_TYPE, CON_NUM_I);
#define _n_newFc(n)         _n_newNc(n);                                           _n_typecon(n) = _cur_type(_CUR_C_TYPE, CON_NUM_F);
#define _n_newPc(n)         n = _n_newc(sizeof(void*));                            _n_typecon(n) = _cur_type(_CUR_C_TYPE, CON_PTR  );
#define _n_newSc(n, l)      n = _n_newc((l) + 1);                   _n_len(n) = l; _n_typecon(n) = _cur_type(_CUR_C_TYPE, CON_STR  );
#define _n_newRc(n, l)      n = _n_newc((l) + 1);                   _n_len(n) = l; _n_typecon(n) = _cur_type(_CUR_C_TYPE, CON_RAW  );
#define _n_newOc(n)         n = _n_newc(sizeof(_RNODE_TYPE));                      _n_typecon(n) = _cur_type(_CUR_C_TYPE, CON_OBJ  );

#define _n_newN(n, v, t)    n = _n_newm(8);             _n_init(n);                _n_typecon(n) = t;              _n_setV(n, v);
#define _n_newI(n, v)       n = _n_newm(8);             _n_init(n);                _n_typecon(n) = _cur_type(_CUR_C_TYPE, CON_NUM_I); _n_setI(n, v);
#define _n_newF(n, v)       n = _n_newm(8);             _n_init(n);                _n_typecon(n) = _cur_type(_CUR_C_TYPE, CON_NUM_F); _n_setF(n, v);
#define _n_newP(n, p)       n = _n_newm(sizeof(void*)); _n_init(n);                _n_typecon(n) = _cur_type(_CUR_C_TYPE, CON_PTR  ); _n_setP(n, p);
#define _n_newS(n, s)       do{ int l = s ? strlen(s) : 0; n = _n_newm((l) + 1);   _n_init(n); _n_typecon(n) = _cur_type(_CUR_C_TYPE, CON_STR); _n_setS(n, s, l); }while(0)
#define _n_newR(n, r, l)    n = _n_newm((l) + 1);       _n_init(n); _n_len(n) = l; _n_typecon(n) = _cur_type(_CUR_C_TYPE, CON_RAW  ); _n_setR(n, r, l); _n_valR(n)[l] = '\0';

#define _n_free(n)          _eobj_free(n)
#define _n_freeK(n)         _cur_freekeyS(_n_keyS(n))

#define _n_l(n)             (n)->_DNODE_LNK_FIELD

#define _n_key(n)           (n)->_DNODE_KEY_FIELD
#define _n_keyS(n)          _k_keyS(_n_key(n))
#define _n_keyI(n)          _k_keyI(_n_key(n))
#define _n_keyU(n)          _k_keyU(_n_key(n))

//! -- link filed macros --
//!
#define _l_n(l)             container_of(l, _DNODE_TYPE, _DNODE_LNK_FIELD)
#define _l_o(l)             _n_o(_l_n(l))
#define _l_type(l)          _n_type(_l_n(l))
#define _l_typeo(l)         _n_typeo(_l_n(l))
#define _l_typeon(l)        _n_typeon(_l_n(l))

#define _l_len(l)           _n_len(_l_n(l))

#define _l_keyI(l)          _n_keyI(_l_n(l))
#define _l_keyS(l)          _n_keyS(_l_n(l))
#define _l_valI(l)          _eo_valI(_l_o(l))
#define _l_valF(l)          _eo_valF(_l_o(l))
#define _l_valP(l)          _eo_valP(_l_o(l))
#define _l_valS(l)          _eo_valS(_l_o(l))
#define _l_valR(l)          _eo_valR(_l_o(l))
#define _l_valV(l)          _eo_valV(_l_o(l))

//! -- obj field macros
//!
#define _eo_en(o)           container_of(o, _enode_t, obj)
#define _eo_h(o)            _eo_en(o)->hdr
#define _eo_t(o)            _eo_h(o)._typ

#define _eo_dn(o)           container_of(o, _DNODE_TYPE, _DNODE_OBJ_FIELD)
#define _eo_rn(o)           container_of(o, _RNODE_TYPE, _RNODE_OBJ_FIELD)

#define _eo_free(o)         _eobj_free(_eo_dn(o))
#define _eo_freeK(o)        _cur_freekeyS(_eo_keyS(o))
#define _eo_l(o)            _n_l(_eo_dn(o))

#define _eo_typec(o)        _ehdt_type_c(_eo_t(o))      // class type
#define _eo_typeo(o)        _ehdt_type_o(_eo_t(o))      // obj   type
#define _eo_typen(o)        _ehdt_type_n(_eo_t(o))      // num   type
#define _eo_typek(o)        _ehdt_type_k(_eo_t(o))      // key   type
#define _eo_typeco(o)       _ehdt_type_co(_eo_t(o))     // class and obj
#define _eo_typeon(o)       _ehdt_type_on(_eo_t(o))
#define _eo_typecon(n)      _ehdt_type_con(_eo_t(n))

#define _eo_keys(n)         _ehdt_keys(_eo_t(n))
#define _eo_hkey(n)         _ehdt_hkey(_eo_t(n))
#define _eo_linked(n)       _ehdt_linked(_eo_t(n))
#define _eo_refed(n)        _ehdt_refed(_eo_t(n))

#define _eo_len(n)          _ehdr_len(_eo_h(n))

#define _eo_keyI(o)         ( (i64* )(o))[-2]
#define _eo_keyS(o)         ( (cstr*)(o))[-2]
#define _eo_valI(o)         (*(i64* )(o))
#define _eo_valF(o)         (*(f64* )(o))
#define _eo_valP(o)         (*(cptr*)(o))
#define _eo_valS(o)         ( (cstr )(o))
#define _eo_valR(o)         ( (cptr )(o))
#define _eo_valV(o)         (*(eval*)(o))

#define _eo_setV(o, v)       _eo_valV(o) = v
#define _eo_setN(o, v)       _eo_setV(o, *(eval*)&v)
#define _eo_setI(o, v)       _eo_setV(o, *(eval*)&v)
#define _eo_setF(o, v)       _eo_setV(o, *(eval*)&v)
#define _eo_setP(o, v)       _eo_setV(o, *(eval*)&v)
#define _eo_setS(o, s, l)    _eo_setR(o, s, l); _eo_valS(o)[l] = '\0';
#define _eo_setR(o, r, l)    memcpy((o), r, l); _eo_len(o) = l

#define _eo_wipeR(o, l)      memset((o), 0, l); _eo_len(o) = l

#define _eo_retT(o)         return o ? _eo_typeo(o) : EOBJ_UNKNOWN
#define _eo_retI(o)         if(o){ switch(_eo_typeon(o)){ case _ENUM_I: return _eo_valI(o); case _ENUM_F: return _eo_valF(o); }} return   0
#define _eo_retF(o)         if(o){ switch(_eo_typeon(o)){ case _ENUM_I: return _eo_valI(o); case _ENUM_F: return _eo_valF(o); }} return 0.0
#define _eo_retP(o)         if(o && EPTR == _eo_typeo(o)) return _eo_valP(o); return 0
#define _eo_retS(o)         if(o && ESTR == _eo_typeo(o)) return _eo_valS(o); return 0
#define _eo_retR(o)         if(o && ERAW == _eo_typeo(o)) return _eo_valR(o); return 0

#define _eo_retL(o)         if(o){ switch(_eo_typeo(o)){ case _ESTR: case _ERAW: return _eo_len(o); }} return 0

#define _n_h(n)             (n)->_DNODE_HDR_FIELD
#define _n_type(n)          _ehdr_type_co(_n_h(n))
#define _n_typec(n)         _ehdr_type_c(_n_h(n))
#define _n_typeo(n)         _ehdr_type_o(_n_h(n))
#define _n_typen(n)         _ehdr_type_n(_n_h(n))
#define _n_typeon(n)        _ehdr_type_on(_n_h(n))
#define _n_typecon(n)       _ehdr_type_con(_n_h(n))
#define _n_len(n)           _ehdr_len(_n_h(n))
#define _n_linked(n)        _ehdr_linked(_n_h(n))

#define _n_o(n)            &(n)->_DNODE_OBJ_FIELD
#define _n_valI(n)          (n)->_DNODE_OBJ_FIELD.i
#define _n_valF(n)          (n)->_DNODE_OBJ_FIELD.f
#define _n_valS(n)          (n)->_DNODE_OBJ_FIELD.r
#define _n_valR(n)          (n)->_DNODE_OBJ_FIELD.r
#define _n_valP(n)          (n)->_DNODE_OBJ_FIELD.p
#define _n_valV(n)          (n)->_DNODE_OBJ_FIELD.v

#define _n_setV(n, v)       _n_valV(n) = v
#define _n_setN(n, v)       _n_setV(n, *(eval*)&v)
#define _n_setI(n, v)       _n_setV(n, *(eval*)&v)
#define _n_setF(n, v)       _n_setV(n, *(eval*)&v)
#define _n_setP(n, v)       _n_setV(n, *(eval*)&v)
#define _n_setS(n, s, l)    _n_setR(n, s, l); _n_valS(n)[l] = '\0'
#define _n_setR(n, r, l)    memcpy(_n_o(n), r, l); _n_len(n) = l

#define _n_wipeR(n, l)      memset(_n_o(n), 0, l)


static __always_inline bool __eobj_isTrue(eobj o)
{
    if(o)
    {
        switch (_eo_typeo(o)) {
            case ETRUE:   return true;
            case ENUM :   return _eo_valI(o)    != 0;
            case ESTR :   return _eo_valS(o)[0] != '\0';
            case EPTR :   return _eo_valP(o)    != NULL;
            case ERAW :   return _eo_len(o)     != 0;
        }
    }

    return false;
}



#endif