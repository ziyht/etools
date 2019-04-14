/// =====================================================================================
///
///       Filename:  ejson.c
///
///    Description:  a easier way to handle json, you can also using it as a simple dic
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

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define EJSON_VERSION "ejson 0.9.2"     // fix bugs of ejson_take

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <fcntl.h>

#include "ecompat.h"
#include "eerr.h"
#include "etype.h"
#include "eutils.h"
#include "estr.h"

#include "erb.h"
#include "eobj_p.h"

#include "ejson.h"

typedef struct _ejson_node_s* _ejsn;
typedef struct _ejson_root_s* _ejsr;

/** -----------------------------------------------------
 *
 *  ejson dict
 *
 *      rebuild from redis, is a more simple version vs
 *  edict
 *
 *  -----------------------------------------------------
 */
#pragma pack(1)
typedef struct dictht_s{
    _ejsn*  table;       // to save data
    ulong   size;
    ulong   sizemask;
    ulong   used;
}dictht_t, dictht;

typedef struct dict_s {
    dictht_t ht[2];
    long     rehashidx;
}dict_t, * dict;

typedef struct dictLink_s{
    _ejsn* _pos;
    ulong* _used;
}dictLink_t, *L;
#pragma pack()

// -- pre definitions
#define DICT_OK                   0
#define DICT_ERR                  1
#define DICT_HT_INITIAL_SIZE      4

#define _dict_can_resize          1
#define _dict_force_resize_ratio  5

// -- API
static inline dict  _dict_new();
static inline void  _dict_clear  (dict d);
static inline void  _dict_free   (dict d);
static inline _ejsn _dict_add    (dict d, constr k, int k_len, _ejsn n);
static inline _ejsn _dict_find   (dict d, constr k, int k_len);
static inline _ejsn _dict_find_ex(dict d, constr k, int k_len, bool rm);
static inline _ejsn _dict_findS  (dict d, constr k);
static inline _ejsn _dict_del    (dict d, _ejsn del);
static inline int   _dict_getL   (dict d, constr k, int k_len, L l);

#define _dict_link(l, n)        { _n_dnext(n) = *((l)._pos); *((l)._pos) = n; *((l)._used) += 1;}
#define _dictHashKey(k, l)      __djbHashS(k)

// -- micros
#define _dict_htreset(ht)       memset(ht, 0, sizeof(dictht_t));
#define _dict_resetHt(d)        memset(d->ht, 0, sizeof(dictht_t) * 2);
#define _dictIsRehashing(d)     ((d)->rehashidx != -1)
#define _dictSize(d)            ((d)->ht[0].used+(d)->ht[1].used)


/** -----------------------------------------------------
 *
 *  ejson data struct definitions
 *
 *  -----------------------------------------------------
 */
#pragma pack(1)

typedef struct _ejson_link_s{
    _ejsn           lp,     // prev node, used for double link
                    ln,     // next node, used for double link
                    dn;     // next node, used for obj's dic
}_ejsl_t, * _ejsl;

typedef struct _ejson_node_s{
    _ejsl_t         link;
    ekey_t          key;
    _ehdr_t         hdr;

    eobj_t          obj;
}_ejsn_t;

typedef struct _ejson_s{

    _ejsn h,      // head
          t;      // tail

    union{
        dict_t*     dict;
        cptr        list;
    }hd;
}_ejson_t;

#define _R_OLEN (sizeof(cptr) * 3)

typedef struct _ejson_root_s{
    _ejsl_t         link;
    ekey_t          key;
    _ehdr_t         hdr;

    _ejson_t        ejson;
}_ejsr_t;

#pragma pack()

static constr err_p;

/// ---------------------- eobj adaptor ---------------------------

#define _CUR_C_TYPE             EJSON

#define _DNODE_TYPE             _ejsn_t
#define _DNODE_LNK_FIELD        link
#define _DNODE_KEY_FIELD        key
#define _DNODE_HDR_FIELD        hdr
#define _DNODE_OBJ_FIELD        obj

#define _RNODE_TYPE             _ejsr_t
#define _RNODE_OBJ_FIELD        ejson

#define _n_newO(n)      n = _n_newc(sizeof(_RNODE_TYPE) - sizeof(void*)); _n_typecoe(n) = _cur_type(_CUR_C_TYPE, COE_OBJ );
#define _n_newA(n)      n = _n_newc(sizeof(_RNODE_TYPE));                 _n_typecoe(n) = _cur_type(_CUR_C_TYPE, COE_ARR );

#define _n_lprev(n)             _n_l(n).lp
#define _n_lnext(n)             _n_l(n).ln
#define _n_dnext(n)             _n_l(n).dn

#define _r_dict(r)              (*_r_o(r)).hd.dict
#define _r_list(r)              (*_r_o(r)).hd.list
#define _r_head(r)              (*_r_o(r)).h
#define _r_tail(r)              (*_r_o(r)).t

#define _r_keyS(r)              (r)->key.s
#define _r_freekeyS(r)          _cur_freekeyS(_r_keyS(r))

#define _cur_newkeyS(l)         emalloc(l + 1)
#define _cur_dupkeyS            strdup
#define _cur_cmpkeyS            strcmp
#define _cur_freekeyS           efree
#define _cur_lenkeyS            strlen

/// -------------------------- managers ---------------------------

// -- obj's manager
#define             _obj_hd(     r)          _r_dict(r)
#define             _obj_init(  r)           _obj_hd((_ejsr)(r)) = _dict_new()
#define             _obj_clear(  r)          _dict_clear(_obj_hd(r))
#define             _obj_bzero( r)           memset(_r_o(r), 0, _R_OLEN)
#define             _obj_find(   r,k,l)      (_obj_hd(r) ? _dict_find(_obj_hd(r), k, l) : 0)
#define             _obj_find_ex(r,k,l,rm)   _dict_find_ex(_obj_hd(r), k, l, rm)
#define             _obj_getL(   r,k,l,lp)   _dict_getL(_obj_hd(r), k, l, lp)
#define             _obj_free(   r)          _dict_free(_obj_hd(r))
static inline _ejsn _obj_add  (_ejsr r, cstr key, _ejsn n);
static inline void  _obj_link (_ejsr r, _ejsn n, L l);
static inline eobj  _obj_popH (_ejsr r);
static inline eobj  _obj_popT (_ejsr r);
static inline eobj  _obj_takeN(_ejsr r, _ejsn n);

// -- arr's manager
#define             _arr_hd(    r)   _r_list(r)
#define             _arr_init(  r)   _arr_hd((_ejsr)(r)) = ecalloc(3, sizeof(uint))
#define             _arr_clear( r)   memset( _arr_hd(r), 0, sizeof(uint) * 3)
#define             _arr_bzero( r)   memset(_r_o(r), 0, _R_OLEN)
#define             _arr_free(  r)   efree(_arr_hd(r))
static inline void  _arr_appd (_ejsr r, _ejsn n);
static inline _ejsn _arr_push (_ejsr r, _ejsn n);
static inline eobj  _arr_popH (_ejsr r);
static inline eobj  _arr_popT (_ejsr r);
static inline _ejsn _arr_find (_ejsr r, uint  idx);
static inline eobj  _arr_takeI(_ejsr r, uint  idx);
static inline eobj  _arr_takeN(_ejsr r, _ejsn n);

/// -------------------- check settings ---------------------------

#define _key_is_valid(  k) (k)
#define _key_is_invalid(k) (!k)

#if 0

#define _getObjByKeys(r, keys) __objByKeys(r, keys, 0, 0)
#define _getObjByRawk(r, rawk) __objByKeys(r, rawk, 1, 0)
#define _rmObjByKeys( r, keys) __objByKeys(r, keys, 0, 1)
#define _rmObjByRawk( r, rawk) __objByKeys(r, rawk, 1, 1)

static __always_inline cstr split   (cstr key, char c) { cstr p = strchr(key,   c); is1_elsret(p, *p = '\0';return p + 1;, NULL); }
static __always_inline cstr splitdot(cstr key        ) { cstr p = strchr(key, '.'); is1_elsret(p, *p = '\0';return p + 1;, NULL); }
static eobj __objByKeys(_ejsr r, constr keys_, bool raw, bool rm)
{
    char  keys[512]; cstr fk, sk, last_fk;    // first key, second key, last first key
    _ejsn n;
    cstr  _idx; uint idx;

    is1_ret(!_r_o(r) || _r_typec(r) != EJSON, 0);

    if(raw)
    {
        n = _obj_find_ex(r, keys_, strlen(keys_), rm);
        is0_exeret(n, eerrfmt("can not find %s in %s", keys_, "."), NULL);
        return _n_o(n);
    }

    strncpy(keys, keys_, 512);
    fk = keys;
    sk = splitdot(fk);

    do{
        _idx = split(fk, '[');

        if(*fk)
        {
            n = _obj_find(r, fk, strlen(fk));
            is0_exeret(n, eerrfmt("can not find %s in %s", fk, fk == keys ? "." : keys), NULL); // not found, return
        }
        else
            n = (_ejsn)r;

        while( _idx )
        {
            is0_exeret(_n_typeo(n) == EARR, eerrfmt("%s is %s obj", keys, __eobj_typeS(_n_o(n), true));, NULL);

            *(_idx - 1) = '[';          // restore
            is1_exeret(*_idx < '0' || *_idx > '9', eerrfmt("invalid keys: %s", keys), NULL);

            idx  = atoi(_idx);
            _idx = split(_idx, '[');

            r = (_ejsr)n;
            n = _arr_find(r, idx);
            is0_exeret(n, eerrfmt("can not find %s in %s", fk, fk == keys ? "." : keys), NULL);
        }

        is0_exeret(n, eerrfmt("can not find %s in %s", fk, fk == keys ? "." : keys), NULL);   // not found, return

        // -- found and return it
        is0_exeret(sk, is1_exeret(rm, switch (_r_typeo(r)) {
                                      case EOBJ: _obj_takeN(r, n); break;
                                      case EARR: _arr_takeN(r, n); break;
                                      default       : return 0;                   }, _n_o(n)), _n_o(n));

        r = (_ejsr)n;
        last_fk = fk;
        fk      = sk;
        sk      = splitdot(fk);
        if(last_fk != keys) *(last_fk - 1) = '.';
    }while(_n_typeo(r) == EOBJ);

    eerrfmt("%s is %s obj", keys, __eobj_typeS(_n_o(n), true));

    return 0;
}


#else

#define _getObjByKeys(r, keys) __objByKeys(r, keys, 0)
#define _rmObjByKeys( r, keys) __objByKeys(r, keys, 1)
#define _getObjByRawk(r, rawk) __objByRawk(r, rawk, 0)
#define _rmObjByRawk( r, rawk) __objByRawk(r, rawk, 1)

static __always_inline cstr split   (cstr key, char c) { cstr p = strchr(key,   c); is1_elsret(p, *p = '\0';return p + 1;, NULL); }
static __always_inline cstr splitdot(cstr key        ) { cstr p = strchr(key, '.'); is1_elsret(p, *p = '\0';return p + 1;, NULL); }

static eobj __objByRawk(_ejsr r, constr keys_, bool rm)
{
    _ejsn n;

    is1_ret(!_r_o(r) || _r_typec(r) != EJSON, 0);

    n = _obj_find_ex(r, keys_, strlen(keys_), rm);
    is0_exeret(n, eerrfmt("can not find %s in %s", keys_, "."), NULL);
    return _n_o(n);
}

int __getAKey(constr p, cstr key, constr* _p)
{
    int i = 0;

    if(*p == '.')
    {
        p++;

        while(*p && *p != '.' && *p != '[')
        {
            key[i++] = *p++;
        }

        *_p = p;
    }
    else if(*p == '[')
    {
        p++;

        while(*p && *p != ']')
        {
            key[i++] = *p++;
        }

        if(*p != ']')
            return -1;

        *_p = p + 1;
    }
    else
    {
        while(*p && *p != '.' && *p != '[')
        {
            key[i++] = *p++;
        }

        *_p = p;
    }

    key[i] = '\0';


    return i;
}

static eobj __objByKeys2(_ejsr r, constr keys_, bool rm)
{
    char key[256]; constr p; int len; int id;

    _ejsn n;

    is1_ret(!_r_o(r) || _r_typec(r) != EJSON, 0);

    p = keys_;

    n = (_ejsn)r;

    do{

        len = __getAKey(p, key, &p);

        is1_ret(len < 0, 0);

        r = (_ejsr)n;

        switch (_r_typeo(r))
        {
            case EOBJ: n = _obj_find(r, key, len);

                       is0_ret(n, 0);

                       break;


            case EARR: id = atoi(key);

                        n = _arr_find(r, id);

                        is0_ret(n, 0);

                        break;
        }

    }while(*p);

    is1_exe(rm, switch (_r_typeo(r)) {
                                          case EOBJ: _obj_takeN(r, n); break;
                                          case EARR: _arr_takeN(r, n); break;
                                          default  : return 0;              })

    return _n_o(n);
}

static eobj __objByKeys(_ejsr r, constr keys_, bool rm)
{
    char  keys[512]; cstr fk, sk, last_fk;    // first key, second key, last first key
    _ejsn n;
    cstr  _idx; uint idx;

    is1_ret(!_r_o(r) || _r_typec(r) != EJSON, 0);

    strncpy(keys, keys_, 512);
    fk = keys;
    sk = splitdot(fk);

    do{
        _idx = split(fk, '[');

        if(*fk)
        {
            n = _obj_find(r, fk, strlen(fk));
            is0_exeret(n, eerrfmt("can not find %s in %s", fk, fk == keys ? "." : keys), NULL); // not found, return
        }
        else
            n = (_ejsn)r;

        while( _idx )
        {
            is0_exeret(_n_typeo(n) == EARR, eerrfmt("%s is %s obj", keys, __eobj_typeS(_n_o(n), true));, NULL);

            *(_idx - 1) = '[';          // restore
            is1_exeret(*_idx < '0' || *_idx > '9', eerrfmt("invalid keys: %s", keys), NULL);

            idx  = atoi(_idx);
            _idx = split(_idx, '[');

            r = (_ejsr)n;
            n = _arr_find(r, idx);
            is0_exeret(n, eerrfmt("can not find %s in %s", fk, fk == keys ? "." : keys), NULL);
        }

        is0_exeret(n, eerrfmt("can not find %s in %s", fk, fk == keys ? "." : keys), NULL);   // not found, return

        // -- found and return it
        is0_exeret(sk, is1_exeret(rm, switch (_r_typeo(r)) {
                                      case EOBJ: _obj_takeN(r, n); break;
                                      case EARR: _arr_takeN(r, n); break;
                                      default       : return 0;                   }, _n_o(n)), _n_o(n));

        r = (_ejsr)n;
        last_fk = fk;
        fk      = sk;
        sk      = splitdot(fk);
        if(last_fk != keys) *(last_fk - 1) = '.';
    }while(_n_typeo(r) == EOBJ);

    eerrfmt("%s is %s obj", keys, __eobj_typeS(_n_o(n), true));

    return 0;
}

#endif
/// -------------------- str strip helper -------------------------

#define _lstrip1(s) do{ while(*s && (unsigned char)*s <= 32) s++;}while(0)

#define NEW 0

#if NEW
#else

typedef constr (*__lstrip_cb)(constr);

static __always_inline constr __lstrip1(constr str)
{
    _lstrip1(str);
    return str;
}

static __always_inline constr __lstrip2(constr str)
{
    do{
        if(!*str || (*str > 32 && (unsigned char)*str != '/' && (unsigned char)*str != '#'))
            break;

        while (*str && (unsigned char)*str <= 32)
            str++;

        switch (*str) {
            case '/': if     (*(str+1) == '*') {while (*str && !(*str=='*' && str[1]=='/')) str++; if(*str) str += 2; else { goto return_;}}
                      else if(*(str+1) == '/') {while (*str && *str!='\n'                 ) str++; if(*str) str += 1; else { goto return_;}}
                      break;
            case '#':                          {while (*str && *str!='\n'                 ) str++; if(*str) str += 1; else { goto return_;}}
                      break;
            default : break;
        }
    }while(str);

return_:
    return str;
}

#endif


/** -----------------------------------------------------
 *
 *  ejson newer
 *
 *  -----------------------------------------------------
 */
eobj   ejson_new(etypeo type, uint len)
{
    _ejsn n;

    switch (type) {
        case EFALSE :  _n_newTF(n);              break;
        case ETRUE  :  _n_newTT(n);              break;
        case ENULL  :  _n_newTN(n);              break;
        case ENUM   :  _n_newIc(n);              break;
        case EPTR   :  _n_newPc(n);              break;
        case ESTR   :  _n_newSc(n, len);         break;
        case ERAW   :  _n_newRc(n, len);         break;
        case EOBJ   :  _n_newO(n); _obj_init(n); break;
        case EARR   :  _n_newA(n);               break;

        default     :  eerrset("invalid type"); return 0;
    }

    return _n_o(n);
}

/** -----------------------------------------------------
 *
 *  ejson parsing
 *
 *  -----------------------------------------------------
 */
#define KEY 1

static __always_inline int __scan_str_len(constr s);

static cstr  __parse_str(constr s, int len, constr* _err, cstr out);
static cstr  __parse_KEY(constr* _src, constr* _err, __lstrip_cb lstrip);

static _ejsn __parse_NUM(cstr   key, constr* _src, constr* _err, __lstrip_cb lstrip);
static _ejsn __parse_STR(cstr* _key, constr* _src, constr* _err, __lstrip_cb lstrip);
static _ejsn __parse_OBJ(cstr   key, constr* _src, constr* _err, __lstrip_cb lstrip);
static _ejsn __parse_ARR(cstr   key, constr* _src, constr* _err, __lstrip_cb lstrip);

static _ejsn __parse_obj(cstr* _key, constr* _src, constr* _err, __lstrip_cb lstrip);

static int   __ejson_free(_ejsr r);

eobj ejson_parseS  (constr json) { return ejson_parseSEx(json, &eerrget(), ENDCHECK);}
eobj ejson_parseSEx(constr json, constr* _err, eopts opts)
{
    _ejsn n; __lstrip_cb lstrip; constr err;

    is0_exeret(json, err = "null", 0);

    lstrip = opts & COMMENT ? __lstrip2 : __lstrip1;

    json = lstrip(json);

    if(is_eq(*json, '\"'))
    {
        int len = __scan_str_len(json);

        is1_ret(len < 0, 0);

        constr obj_p = lstrip(json + len + 2);

        if(*obj_p == ':')
        {
            cstr key = _cur_newkeyS(len);

            if(!__parse_str(json, len, &err, key))
            {
                goto failed;
            }

            obj_p++;

            n = __parse_obj(&key, &obj_p, &err, lstrip);

            is0_exe(n, _cur_freekeyS(key); goto failed);

            json = obj_p;
        }
        else
        {
            _n_newSc(n, len);

            if(!__parse_str(json, len, _err, _n_valS(n)))
            {
                _n_free(n);
                goto failed;
            }

            json = lstrip(json + len +2);

        }
    }
    else
        n = __parse_obj(0, &json, &err, lstrip);

    if(n && opts & ENDCHECK)
    {
        is1_exe(*json, ejson_free(_n_o(n));
                       goto failed);
    }

    return n ? _n_o(n) : 0;

failed:
    if(_err) *_err = err;

    return 0;
}

eobj ejson_parseF  (constr path) { return ejson_parseFEx(path, &eerrget(), COMMENT | ENDCHECK);}
eobj ejson_parseFEx(constr path, constr* _err, eopts opts)
{
    estr s; constr err;

    s = estr_fromFile(path, 10 * 1024 * 1024 );  // 10M
    if(s)
    {
        __lstrip_cb lstrip = opts & COMMENT ? __lstrip2 : __lstrip1;

        constr json = lstrip(s);

        _ejsn n = __parse_obj(0, &json, &err, lstrip);

        estr_free(s);

        if(n) return _n_o(n);
    }

    if(_err) *_err = err;

    return 0;
}

// todo : check s if have effect
static _ejsn __parse_obj(cstr* _key,  constr* _src, constr* _err, __lstrip_cb lstrip)
{
    _ejsn n;

    switch (**_src)
    {
        case 'n' :  is0_exeret(strncmp(*_src, "null" , 4), _n_newTN(n); is1_exeret(n, _n_keyS(n) = _key ? *_key : 0; *_src = lstrip(*_src + 4), n), 0); break;
        case 'f' :  is0_exeret(strncmp(*_src, "false", 5), _n_newTF(n); is1_exeret(n, _n_keyS(n) = _key ? *_key : 0; *_src = lstrip(*_src + 5), n), 0); break;
        case 't' :  is0_exeret(strncmp(*_src, "true" , 4), _n_newTT(n); is1_exeret(n, _n_keyS(n) = _key ? *_key : 0; *_src = lstrip(*_src + 4), n), 0); break;
        case '\"':  return __parse_STR(_key            , _src, _err, lstrip);
        case '[' :  return __parse_ARR(_key ? *_key : 0, _src, _err, lstrip);
        case '{' :  return __parse_OBJ(_key ? *_key : 0, _src, _err, lstrip);
        default  :  if(**_src == '-' || (**_src >= '0' && **_src <= '9'))
                    {
                        return __parse_NUM(_key ? *_key : 0, _src, _err, lstrip);
                    }
    }

    *_err = *_src;

    return 0;
}

static cstr __parse_KEY(constr* _src, constr* _err, __lstrip_cb lstrip)
{
    int len = __scan_str_len(*_src);

    if(len >= 0)
    {
        cstr s = _cur_newkeyS(len);

        if(__parse_str(*_src, len, _err, s))
        {
            *_src = lstrip(*_src + len + 2);   // len + two quotation '\"'
            return s;
        }

        _cur_freekeyS(s);

        return 0;
    }

    *_err = *_src - len;

    return 0;
}

static _ejsn __parse_NUM(cstr key, constr* _src, constr* _err, __lstrip_cb lstrip)
{
    double v = 0, sign = 1, scale = 0;  int subscale = 0, signsubscale = 1; bool is_float = 0; constr s  = *_src; _ejsn n;

    E_UNUSED(_err);

    _n_newNc(n); is1_elsret(n, _n_keyS(n) = key;, 0);

    if (*s == '-')                     sign = -1, s++;      // Has sign?
    if (*s == '0')                                s++;      // is zero
    if (*s >= '1' && *s   <= '9')                {      do v = (v * 10.0) + (*s++ - '0');          while (*s >= '0' && *s <= '9'); }    // Number?
    if (*s == '.' && s[1] >= '0' && s[1] <= '9') { s++; do v = (v * 10.0) + (*s++ - '0'), scale--; while (*s >= '0' && *s <= '9'); is_float = 1;}	// Fractional part?
    if (*s == 'e' || *s   == 'E')                           // Exponent?
    {
        s++;

        if      (*s == '+') s++;
        else if (*s == '-') signsubscale = -1, s++;     // With sign?

        while (*s >= '0' && *s <= '9') subscale = (subscale * 10) + (*s++ - '0');	// Number?
    }

    v = sign * v * pow(10.0, (scale + subscale * signsubscale));     // number = +/- number.fraction * 10^+/- exponent

    if(is_float) { _n_typecoe(n) = _cur_type(_CUR_C_TYPE, COE_NUM_F); _n_valF(n)    = v; }
    else         { _n_typecoe(n) = _cur_type(_CUR_C_TYPE, COE_NUM_I); _n_valI(n)    = v;}

    *_src       = lstrip(s);         // save new pos

    return n;
}

/**
 * @brief __scan_str_len
 * @param _src
 * @return >= 0 ok, the return value is the str len scaned
 *         <  0 err occured, abs() to get the err pos
 */
static __always_inline int __scan_str_len(constr s)
{
    constr end_p = s + 1;

// todo

#if 1
    //int len = 0;
    while (*end_p !='\"' && *end_p) if (*end_p++ == '\\') end_p++;
#else
    while (*end_p) if(*end_p !='\"') end_p++; else if (end_p[-1] != '\\') break;
#endif

    return *end_p == '\"' ? end_p - s     - 1
                          : s     - end_p - 1;
}

static __always_inline unsigned __parse_hex4(constr str)
{
    unsigned h=0;
    if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
    h=h<<4;str++;
    if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
    h=h<<4;str++;
    if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
    h=h<<4;str++;
    if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
    return h;
}

static cstr  __parse_str(constr s, int len, constr* _err, cstr out)
{
#if 0
    memcpy(out, s + 1, len); out[len] = '\0';
#else
    static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

    constr   end_p = s + 1 +len,
             ptr1  = s + 1;
    cstr     ptr2  = out;
    unsigned uc, uc2;

    while (ptr1 < end_p)
    {
        if (*ptr1!='\\') *ptr2++ = *ptr1++;
        else
        {
            ptr1++;
            switch (*ptr1)
            {
                case 'b': *ptr2++='\b';	break;
                case 'f': *ptr2++='\f';	break;
                case 'n': *ptr2++='\n';	break;
                case 'r': *ptr2++='\r';	break;
                case 't': *ptr2++='\t';	break;
                case 'u':	 // transcode utf16 to utf8
                    uc = __parse_hex4(ptr1 + 1); ptr1 += 4;	// get the unicode char
                    is1_exeret(ptr1 >= end_p,                       *_err = s, 0);     // invalid
                    is1_exeret((uc>=0xDC00 && uc<=0xDFFF) || uc==0, *_err = s, 0);     // check for invalid

                    if (uc>=0xD800 && uc<=0xDBFF)       // UTF16 surrogate pairs
                    {
                        is1_exeret(ptr1 + 6 > end_p,               *_err = s, 0);   // invalid
                        is1_exeret(ptr1[1]!='\\' || ptr1[2]!='u',  *_err = s, 0);	// missing second-half of surrogate
                        uc2 = __parse_hex4(ptr1 + 3); ptr1 += 6;
                        is1_exeret(uc2 < 0xDC00  || uc2 > 0xDFFF,  *_err = s, 0);	// invalid second-half of surrogate
                        uc = 0x10000 + (((uc&0x3FF)<<10) | (uc2&0x3FF));
                    }

                    if      (uc < 0x80   ) len = 1;
                    else if (uc < 0x800  ) len = 2;
                    else if (uc < 0x10000) len = 3;
                    else                   len = 4;
                    ptr2 += len;

                    switch (len) {
                        case 4: *--ptr2 =((uc | 0x80              ) & 0xBF); uc >>= 6;
                        case 3: *--ptr2 =((uc | 0x80              ) & 0xBF); uc >>= 6;
                        case 2: *--ptr2 =((uc | 0x80              ) & 0xBF); uc >>= 6;
                        case 1: *--ptr2 =( uc | firstByteMark[len]);
                        default:;
                    }
                    ptr2+=len;
                    break;
                default:  *ptr2++ = *ptr1; break;
            }
            ptr1++;
        }
    }
    *ptr2 = 0;
#endif

    return out;
}

/**
 * @brief __parse_STR
 * @param _key : a pointer to the name of obj
 *                   if  _key == NULL, create new obj and parse str to obj.v.s
 *                   if *_key == NULL, parse str to *_key;
 *                   else              create new obj, set *_key to obj.k and parse str to obj.v.s
 * @param _src : src string pointer
 * @param _err : error str pointer
 * @return
 */
static _ejsn __parse_STR(cstr* _key, constr* _src, constr* _err, __lstrip_cb lstrip)
{
    static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

    int len; cstr ptr2, k_v; unsigned uc, uc2;

    constr end_p = *_src + 1, ptr1 = end_p;

// todo
#if 1
    while (*end_p !='\"' && *end_p) if (*end_p++ == '\\') end_p++;
#else
    while (*end_p) if(*end_p !='\"') end_p++; else if (end_p[-1] != '\\') break;
#endif

    len = end_p - *_src - 1;

    is0_ret(k_v = _cur_newkeyS(len), 0);   // mem fail

#if 0
    memcpy(k_v, s + 1, len);
    ptr = end_p;
#else

    ptr2 = k_v;
    while (ptr1 < end_p)
    {
        if (*ptr1!='\\') *ptr2++ = *ptr1++;
        else
        {
            ptr1++;
            switch (*ptr1)
            {
                case 'b': *ptr2++='\b';	break;
                case 'f': *ptr2++='\f';	break;
                case 'n': *ptr2++='\n';	break;
                case 'r': *ptr2++='\r';	break;
                case 't': *ptr2++='\t';	break;
                case 'u':	 // transcode utf16 to utf8
                    uc = __parse_hex4(ptr1 + 1); ptr1 += 4;	// get the unicode char
                    is1_exeret(ptr1 >= end_p,                       *_err = ptr1, 0);     // invalid
                    is1_exeret((uc>=0xDC00 && uc<=0xDFFF) || uc==0, *_err = ptr1, 0);     // check for invalid

                    if (uc>=0xD800 && uc<=0xDBFF)       // UTF16 surrogate pairs
                    {
                        is1_exeret(ptr1 + 6 > end_p,               *_err = ptr1, 0);   // invalid
                        is1_exeret(ptr1[1]!='\\' || ptr1[2]!='u',  *_err = ptr1, 0);	// missing second-half of surrogate
                        uc2 = __parse_hex4(ptr1 + 3); ptr1 += 6;
                        is1_exeret(uc2 < 0xDC00  || uc2 > 0xDFFF,  *_err = ptr1, 0);	// invalid second-half of surrogate
                        uc = 0x10000 + (((uc&0x3FF)<<10) | (uc2&0x3FF));
                    }

                    if      (uc < 0x80   ) len = 1;
                    else if (uc < 0x800  ) len = 2;
                    else if (uc < 0x10000) len = 3;
                    else                   len = 4;
                    ptr2 += len;

                    switch (len) {
                        case 4: *--ptr2 =((uc | 0x80              ) & 0xBF); uc >>= 6;
                        case 3: *--ptr2 =((uc | 0x80              ) & 0xBF); uc >>= 6;
                        case 2: *--ptr2 =((uc | 0x80              ) & 0xBF); uc >>= 6;
                        case 1: *--ptr2 =( uc | firstByteMark[len]);
                        default:;
                    }
                    ptr2+=len;
                    break;
                default:  *ptr2++ = *ptr1; break;
            }
            ptr1++;
        }
    }
    *ptr2 = 0;
#endif

    if (*ptr1 =='\"') *_src = lstrip(ptr1 + 1);

    if(!_key || *_key != NULL)
    {
         _ejsn n;

        _n_newSl(n, k_v, len); _cur_freekeyS(k_v);
        is0_ret(n, 0);

        if(_key)
            _n_keyS(n) = *_key;

        return n;
    }
    else
        *_key = k_v;

    return 0;
}

static int __ejson_free(_ejsr r);
static _ejsn __parse_OBJ(cstr key, constr* _src, constr* _err, __lstrip_cb lstrip)
{
    _ejsr r; _ejsn c_n; cstr c_key;

    //! check err format
    *_src = lstrip(*_src + 1);
    if(**_src != '\"' && **_src != '}')
    {
        *_err = *_src;
        return 0;
    }

    //! creat obj
    {
        _n_newO(r); _n_keyS(r) = key;
        _obj_init(r);

        is1_exeret(**_src == '}', *_src = lstrip(*_src + 1), (_ejsn)r);
    }

    //! parse first child
    {
        // -- parse key
        c_key = 0;
        __parse_STR(&c_key, _src, _err, lstrip);
        is0_exe(c_key    , goto rls_ret);

        // -- parse child
        is1_exe(**_src != ':', goto err_set);
        *_src = lstrip(*_src + 1);
        c_n   = __parse_obj(&c_key, _src, _err, lstrip);
        is0_exe(c_n, _cur_freekeyS(c_key); goto rls_ret);
        _n_keyS(c_n) = c_key;

        // -- add child to obj
        _obj_add(r, c_key, c_n);
    }

    //! parse next children
    while(**_src == ',')
    {
        *_src = lstrip(*_src + 1);

        // -- parse key
        is1_exe(**_src != '\"', break);
        c_key = 0;
        __parse_STR(&c_key, _src, _err, lstrip);
        is0_exe(c_key     , goto rls_ret);

        // -- parse child
        is1_exe(**_src != ':', goto err_set);
        *_src = lstrip(*_src + 1);
        c_n   = __parse_obj(&c_key, _src, _err, lstrip);
        is0_exe(c_n, _cur_freekeyS(c_key); goto rls_ret);
        _n_keyS(c_n) = c_key;

        // -- link to dict
        is0_exe(_obj_add(r, c_key, c_n), __ejson_free(r); __ejson_free((_ejsr)c_n); goto rls_ret);
    }

    //! check complited
    is1_exeret(**_src == '}', *_src = lstrip(*_src + 1), (_ejsn)r);

err_set:
    *_err = *_src;

rls_ret:
    __ejson_free(r);

    return 0;
}

static _ejsn __parse_ARR(cstr key, constr* _src, constr* _err, __lstrip_cb lstrip)
{
    _ejsr r; _ejsn c_n;

    _n_newA(r); _n_keyS(r) = key;

    *_src = lstrip(*_src + 1);
    is1_exeret(**_src == ']', *_src = lstrip(*_src + 1), (_ejsn)r);

    _arr_init(r);

    do{
        c_n   = __parse_obj(0, _src, _err, lstrip);
        is0_exeret(c_n, __ejson_free(r), 0);
        _arr_appd(r, c_n);

    }while(**_src == ',' && (*_src = lstrip(*_src + 1)));

    //! check complited
    is1_exeret(**_src == ']', *_src = lstrip(*_src + 1), (_ejsn)r);

    *_err = *_src;
    __ejson_free(r);

    return 0;
}

/** -----------------------------------------------------
 *
 *  ejson clear and free
 *
 *  -----------------------------------------------------
 */

static int __ejson_free   (_ejsr r);
static int __ejson_free_ex(_ejsr r, eobj_rls_ex_cb rls, eval prvt);

int    ejson_clear  (eobj _r)
{
    int cnt; _ejsr r;

    is0_ret(_r, 0);

    cnt = 0;
    r   = _eo_rn(_r);

    switch (_r_typeo(r)) {
        case EOBJ : if(_r_head(r)){ cnt += __ejson_free((_ejsr)_r_head(r));} _obj_clear(r); break;
        case EARR : if(_r_head(r)){ cnt += __ejson_free((_ejsr)_r_head(r));} _arr_clear(r); break;
        default   : return 0;
    }

    _r_len (r) = 0;
    _r_head(r) = _r_tail(r) = 0;

    return cnt;
}

int    ejson_clearEx(eobj _r, eobj_rls_ex_cb rls, eval prvt)
{
    int cnt; _ejsr r;

    if(!rls) return ejson_clear(_r);

    is0_ret(_r, 0);

    cnt = 0;
    r   = _eo_rn(_r);

    switch (_r_typeo(r)) {
        case EPTR :
        case ERAW : rls(_r, prvt); break;
        case EOBJ : if(_r_head(r)){ cnt += __ejson_free_ex(r, rls, prvt);} _obj_clear(r); break;
        case EARR : if(_r_head(r)){ cnt += __ejson_free_ex(r, rls, prvt);} _arr_clear(r); break;
        default   : return 0;
    }

    _r_len (r) = 0;
    _r_head(r) = _r_tail(r) = 0;

    return cnt;
}

int  ejson_free(eobj o)
{
    is1_ret(!o || _eo_linked(o), 0);
    return __ejson_free(_eo_rn(o));
}

int  ejson_freeEx(eobj o, eobj_rls_ex_cb rls, eval prvt)
{
    if(!rls) return ejson_free(o);

    is1_ret(!o || _eo_linked(o), 0);
    return __ejson_free_ex(_eo_rn(o), rls, prvt);
}

static int __ejson_free(_ejsr r)
{
    _ejsn itr; int cnt = 0;

    do{
        switch (_r_typeo(r))
        {
            case EOBJ : if(_r_tail(r)) {_n_lnext(_r_tail(r)) = _n_lnext(r);_n_lnext(r)= _r_head(r);} _obj_free(r);break;
            case EARR : if(_r_tail(r)) {_n_lnext(_r_tail(r)) = _n_lnext(r);_n_lnext(r)= _r_head(r);} _arr_free(r);break;
            default   : break;
        }
        if(_r_keyS(r)) _r_freekeyS(r);
        itr = _n_lnext(r);
        _r_free(r);
        r = (_ejsr)itr;

        cnt ++;
    }while(r);

    return cnt;
}

static int __ejson_free_ex(_ejsr r, eobj_rls_ex_cb rls, eval prvt)
{
    _ejsn itr; int cnt = 0;

    do{
        switch (_r_typeo(r))
        {
            case EPTR :
            case ERAW : rls((eobj)_r_o(r), prvt); break;
            case EOBJ : if(_r_tail(r)) {_n_lnext(_r_tail(r)) = _n_lnext(r);_n_lnext(r)= _r_head(r);} _obj_free(r);break;
            case EARR : if(_r_tail(r)) {_n_lnext(_r_tail(r)) = _n_lnext(r);_n_lnext(r)= _r_head(r);} _arr_free(r);break;
            default   : break;
        }
        if(_r_keyS(r)) _r_freekeyS(r);
        itr = _n_lnext(r);
        _r_free(r);
        r = (_ejsr)itr;

        cnt ++;
    }while(r);

    return cnt;
}


etypeo ejson_type   (eobj o) { _eo_retT(o); }
uint   ejson_len    (eobj o) { _eo_retL(o); }
bool   ejson_isEmpty(eobj o) { return _ec_isEmpty(o); }

constr ejson_errp() { return err_p    ; }
constr ejson_err () { return eerrget(); }

/** -----------------------------------------------------
 *
 *  ejson checking
 *
 *  -----------------------------------------------------
 */
static bool __check_KEY(constr* _src, constr* _err, erb     set);
static bool __check_NUM(constr* _src, constr* _err);
static bool __check_STR(constr* _src, constr* _err);
static bool __check_ARR(constr* _src, constr* _err, __lstrip_cb lstrip);
static bool __check_OBJ(constr* _src, constr* _err, __lstrip_cb lstrip);

static bool __check_obj(constr* _src, constr* _err, __lstrip_cb lstrip);

bool   ejson_checkS  (constr json){ return ejson_checkSEx(json, &eerrget(), ENDCHECK); }
bool   ejson_checkSEx(constr json, constr* _err, eopts opts)
{
    constr err; __lstrip_cb lstrip;

    is0_exe(json, err = "NULL"; goto failed);

    lstrip = opts & COMMENT ? __lstrip2 : __lstrip1;

    is0_exe(__check_obj(&json, &err, lstrip), goto failed);

    if(opts & ENDCHECK)
    {
        json = lstrip(json);

        is1_exe(*json, goto failed;);
    }

    return true;

failed:
    if(_err) *_err = json;

    return false;
}

static bool __check_obj(constr* _src, constr* _err, __lstrip_cb lstrip)
{
    constr s  = lstrip(*_src);

    switch (*s) {
        case 'n' :  is0_exeret(strncmp(s,"null", 4), *_src = s + 4, 1); break;
        case 'f' :  is0_exeret(strncmp(s,"false",5), *_src = s + 5, 1); break;
        case 't' :  is0_exeret(strncmp(s,"true", 4), *_src = s + 4, 1); break;
        case '\"':  return __check_STR(_src, _err);
        case '[' :  return __check_ARR(_src, _err, lstrip);
        case '{' :  return __check_OBJ(_src, _err, lstrip);
        default  :  if(*s == '-' || (*s >= '0' && *s <= '9'))
                        return __check_NUM(_src, _err);
    }

    *_src = s;  return 0;	// failure: err src
}

static bool __check_NUM(constr* _src, constr* _err)
{
    E_UNUSED(_err);

    constr s  = *_src;

    if (*s == '-')                                 s++;               // Has sign?
    if (*s == '0')                                 s++;               // is zero
    if (*s >= '1' && *s   <= '9')                       do s++; while (*s >= '0' && *s <= '9');	  // Number?
    if (*s == '.' && s[1] >= '0' && s[1] <= '9') { s++; do s++; while (*s >= '0' && *s <= '9'); } // Fractional part?
    if (*s == 'e' || *s   == 'E')         // Exponent?
    {
        s++;

        if      (*s == '+') s++;
        else if (*s == '-') s++;	// With sign?

        while (*s >= '0' && *s <= '9') s++;  // Number?
    }

    *_src = s;

    return 1;
}

static bool __check_STR(constr* _src, constr* _err)
{
    E_UNUSED(_err);

    int len = __scan_str_len(*_src);

    if(len < 0)
    {
        *_src = *_src - len;

        return 0;
    }

    return 1;
}

static bool __check_ARR(constr* _src, constr* _err, __lstrip_cb lstrip)
{
    constr s = lstrip(*_src + 1);

    is1_exeret(*s == ']', *_src = s + 1, 1);

    do{
        *_src = s;

        is0_ret(__check_obj(_src, _err, lstrip), 0);

        s = lstrip(*_src);

    }while(*s == ',' && s++);

    is1_exeret(*s == ']', *_src = s + 1, 1);

    *_src = s;

    return 0;
}

static bool __check_KEY(constr* _src, constr* _err, erb     set)
{
    E_UNUSED(_err);

    int len = __scan_str_len(*_src);

    if(len < 0)
    {
        *_src = *_src - len;

        return 0;
    }
    else
    {
        if(len <= 128)
        {
            char key[129];

            memcpy(key, *_src + 1, len);

            return erb_addMI(set, ekey_s(key), 1);
        }
        else
        {
            bool ret;
            cstr key = emalloc(len + 1);

            memcpy(key, *_src + 1, len);

            ret = erb_addMI(set, ekey_s(key), 1);

            efree(key);

            return ret;
        }
    }

    return 1;
}

static bool __check_OBJ(constr* _src, constr* _err, __lstrip_cb lstrip)
{
    erb keyset; constr s;

    keyset = erb_new(EKEY_S);
    s      = lstrip(*_src + 1);

    while(*s == '\"')
    {
        // -- check key
        is0_exe(__check_KEY(_src, _err, keyset), goto err_set);

        // -- check obj
        s   = lstrip(*_src);
        is1_exe(*s != ':', goto err_set);
        *_src = s + 1;
        is0_exe(__check_obj(_src, _err, lstrip), goto rls_ret);

        // -- strip to next
        s   = lstrip(*_src);
        if(*s == ',') s++;
        s   = lstrip(s + 1);
    }
    is1_exeret(*s == '}', erb_free(keyset), 1);

err_set:
    *_src = s;

rls_ret:
    erb_free(keyset);

    return 0;
}

/** -----------------------------------------------------
 *
 *  ejson add
 *
 *  -----------------------------------------------------
 */

#define __ __always_inline
static     eobj __ejson_makeRoom(_ejsr r, eobj    in, bool overwrite, bool find);
static  __ eobj __ejson_addJson (_ejsr r, constr key, constr src);
static  __ eobj __ejson_addO    (_ejsr r, constr key, eobj   in );
#undef  __

#define _eo_setT(o) if(type == EOBJ){ _obj_bzero(_eo_rn(o)); _obj_init(_eo_rn(o));} else if(type == EARR) { _arr_bzero(_eo_rn(o)); }
#define _t_olen(t)  (t < EOBJ ? 0 : _R_OLEN )
#define _s_olen(s)  (s ? strlen(s) + 1 : 1)

eobj   ejson_addJ(eobj r, constr key, constr json) { is0_ret(r, 0); return __ejson_addJson(_eo_rn(r), key, json); }
eobj   ejson_addT(eobj r, constr key, etypeo type) { _ejsn_t b = {{0}, {.s = (cstr)key}, {._len = _t_olen(type), ._typ = {.__1 = {EJSON, type, 0, 0}}}, {0}}; eobj o = __ejson_makeRoom(_eo_rn(r), _n_o(&b), 0, 0);  if(o) {               _eo_setT (o     );              _eo_typeco (o) = _n_typeco(&b)   ; } return o; }
eobj   ejson_addI(eobj r, constr key, i64    val ) { _ejsn_t b = {{0}, {.s = (cstr)key}, {._len =             8, ._typ = {.t_coe = _EJSON_COE_NUM_I }}, {0}}; eobj o = __ejson_makeRoom(_eo_rn(r), _n_o(&b), 0, 0);  if(o) {               _eo_setI (o, val);              _eo_typecoe(o) = _EJSON_COE_NUM_I; } return o; }
eobj   ejson_addF(eobj r, constr key, f64    val ) { _ejsn_t b = {{0}, {.s = (cstr)key}, {._len =             8, ._typ = {.t_coe = _EJSON_COE_NUM_F }}, {0}}; eobj o = __ejson_makeRoom(_eo_rn(r), _n_o(&b), 0, 0);  if(o) {               _eo_setF (o, val);              _eo_typecoe(o) = _EJSON_COE_NUM_F; } return o; }
eobj   ejson_addS(eobj r, constr key, constr str ) { _ejsn_t b = {{0}, {.s = (cstr)key}, {._len =  _s_olen(str), ._typ = {.t_coe = _EJSON_COE_STR   }}, {0}}; eobj o = __ejson_makeRoom(_eo_rn(r), _n_o(&b), 0, 0);  if(o) { _n_len(&b)--; _eo_setS (o, str, _n_len(&b));  _eo_typeco (o) = _EJSON_CO_STR   ; } return o; }
eobj   ejson_addP(eobj r, constr key, conptr ptr ) { _ejsn_t b = {{0}, {.s = (cstr)key}, {._len =             8, ._typ = {.t_coe = _EJSON_COE_PTR   }}, {0}}; eobj o = __ejson_makeRoom(_eo_rn(r), _n_o(&b), 0, 0);  if(o) {               _eo_setP (o, ptr);              _eo_typeco (o) = _EJSON_CO_PTR   ; } return o; }
eobj   ejson_addR(eobj r, constr key, uint   len ) { _ejsn_t b = {{0}, {.s = (cstr)key}, {._len =       len + 1, ._typ = {.t_coe = _EJSON_COE_RAW   }}, {0}}; eobj o = __ejson_makeRoom(_eo_rn(r), _n_o(&b), 0, 0);  if(o) {               _eo_wipeR(o, len);              _eo_typeco (o) = _EJSON_CO_RAW   ; } return o; }
eobj   ejson_addO(eobj r, constr key, eobj   o   ) { is0_ret(r, 0); return __ejson_addO(_eo_rn(r), key, o); }

eobj   ejson_addrJ(eobj r, constr rawk, constr key, constr json) { return ejson_addJ(_getObjByRawk(_eo_rn(r), rawk), key, json); }
eobj   ejson_addrT(eobj r, constr rawk, constr key, etypeo type) { return ejson_addT(_getObjByRawk(_eo_rn(r), rawk), key, type); }
eobj   ejson_addrI(eobj r, constr rawk, constr key, i64    val ) { return ejson_addI(_getObjByRawk(_eo_rn(r), rawk), key, val ); }
eobj   ejson_addrF(eobj r, constr rawk, constr key, f64    val ) { return ejson_addF(_getObjByRawk(_eo_rn(r), rawk), key, val ); }
eobj   ejson_addrS(eobj r, constr rawk, constr key, constr str ) { return ejson_addS(_getObjByRawk(_eo_rn(r), rawk), key, str ); }
eobj   ejson_addrP(eobj r, constr rawk, constr key, conptr ptr ) { return ejson_addP(_getObjByRawk(_eo_rn(r), rawk), key, ptr ); }
eobj   ejson_addrR(eobj r, constr rawk, constr key, uint   len ) { return ejson_addR(_getObjByRawk(_eo_rn(r), rawk), key, len ); }
eobj   ejson_addrO(eobj r, constr rawk, constr key, eobj   o   ) { return ejson_addO(_getObjByRawk(_eo_rn(r), rawk), key, o   ); }

eobj   ejson_addkJ(eobj r, constr keys, constr key, constr json) { return ejson_addJ(_getObjByKeys(_eo_rn(r), keys), key, json); }
eobj   ejson_addkT(eobj r, constr keys, constr key, etypeo type) { return ejson_addT(_getObjByKeys(_eo_rn(r), keys), key, type); }
eobj   ejson_addkI(eobj r, constr keys, constr key, i64    val ) { return ejson_addI(_getObjByKeys(_eo_rn(r), keys), key, val ); }
eobj   ejson_addkF(eobj r, constr keys, constr key, f64    val ) { return ejson_addF(_getObjByKeys(_eo_rn(r), keys), key, val ); }
eobj   ejson_addkS(eobj r, constr keys, constr key, constr str ) { return ejson_addS(_getObjByKeys(_eo_rn(r), keys), key, str ); }
eobj   ejson_addkP(eobj r, constr keys, constr key, conptr ptr ) { return ejson_addP(_getObjByKeys(_eo_rn(r), keys), key, ptr ); }
eobj   ejson_addkR(eobj r, constr keys, constr key, uint   len ) { return ejson_addR(_getObjByKeys(_eo_rn(r), keys), key, len ); }
eobj   ejson_addkO(eobj r, constr keys, constr key, eobj   o   ) { return ejson_addO(_getObjByKeys(_eo_rn(r), keys), key, o   ); }

static eobj __ejson_makeRoom(_ejsr r, eobj in, bool overwrite, bool find)
{
    dictLink_t l; uint len; _ejsn n;

    E_UNUSED(overwrite); E_UNUSED(find);

    is1_ret(!_r_o(r) || _key_is_invalid(_eo_keyS(in)), 0);

    switch (_r_typeo(r))
    {
        case EOBJ:  len = strlen(_eo_keyS(in));
                    if(!_obj_getL(r, _eo_keyS(in), len, &l)) return 0;

                    n = _n_newm(_eo_len(in)); _n_init(n);

                    _n_keyS(n) = _cur_dupkeyS(_eo_keyS(in));

                    _obj_link(r, n, &l);

                    return _n_o(n);

        case EARR:  n = _n_newm(_eo_len(in)); _n_init(n);

                    _arr_appd(r, n);

                    return _n_o(n);

        default  :  eerrset(_ERRSTR_TYPEDF); return 0;
    }

    return 0;
}


static inline eobj __ejson_addJson(_ejsr r, constr key, constr src)
{

#undef  _opt
#define _opt __lstrip1

    cstr hk; _ejsn n; dictLink_t l; constr err;

    hk = NULL;

    _lstrip1(src);
    if(*src == '\"')        // maybe have a key in src
    {
        __parse_STR(&hk, &src, &err, _opt);

        if(hk)
        {
            if(*src == ':')
            {
                src++; _lstrip1(src);
            }
            else if(*src != '\0')  // wrong json format
            {
                _cur_freekeyS(hk);

                return 0;
            }
        }
    }

    switch (_r_typeo(r))
    {
        case EOBJ:  if(_key_is_valid(key))         // input key is valid, using it
                    {
                        is0_exe(_obj_getL(r, key, strlen(key), &l), goto err_ret;);

                        if(*src)  n = __parse_obj(0, &src, &err, _opt);
                        else      _n_newS(n, hk);       // here use hk to create ESTR obj

                        is0_exe(n, goto err_ret;);

                        _n_keyS(n) = _cur_dupkeyS(key);

                        is1_exe(hk, _cur_freekeyS(hk));
                    }
                    else
                    {
                        is1_exe(!*src || _key_is_invalid(hk) || !_obj_getL(r, hk, _cur_lenkeyS(hk), &l) || !(n = __parse_obj(0, &src, &err, _opt)), goto err_ret;);

                        _n_keyS(n) = hk;
                    }

                    _obj_link(r, n, &l);

                    return _n_o(n);

        case EARR:  if(*src)    // == ':'
                    {
                        is0_exe(n = __parse_obj(0, &src, &err, _opt), goto err_ret;);
                    }
                    else        // == '\0'
                    {
                        _n_newS(n, hk);
                        is0_exe(n, goto err_ret;);
                    }

                    _n_keyS(n) = hk;

                    _arr_appd(r, n);

                    return _n_o(n);

        default   : eerrset(_ERRSTR_TYPEDF); goto err_ret;
    }

err_ret:
    if(hk)
        _cur_freekeyS(hk);

    return 0;
}

static eobj __ejson_addO(_ejsr r, constr key, eobj   o   )
{
    cstr ok; dictLink_t l;

    is1_ret(_eo_linked(o) || un_eq(_eo_typec(o), EJSON), 0);

    switch (_r_typeo(r))
    {
        case EOBJ:  ok = _eo_keyS(o);
                    if(_key_is_valid(key))
                    {
                        is0_ret(_obj_getL(r, key, strlen(key), &l), 0);

                        if(ok) _cur_freekeyS(ok);

                        _eo_keyS(o) = _cur_dupkeyS(key);
                    }
                    else if(_key_is_valid(ok))
                    {
                        is0_ret(_obj_getL(r, ok, _cur_lenkeyS(ok), &l), 0);
                    }
                    else
                        return 0;

                    _obj_link(r, _eo_dn(o), &l);

                    return o;

        case EARR:  _arr_appd(r, _eo_dn(o));

                    return o;

        default  :  eerrset(_ERRSTR_TYPEDF); return 0;
    }

    return 0;
}

/** -----------------------------------------------------
 *
 *  ejson val
 *
 *  -----------------------------------------------------
 */

eobj   ejson_valr (eobj r, constr rawk) { return _getObjByRawk(_eo_rn(r), rawk);}
i64    ejson_valrI(eobj r, constr rawk) { r = _getObjByRawk(_eo_rn(r), rawk); _eo_retI(r); }
f64    ejson_valrF(eobj r, constr rawk) { r = _getObjByRawk(_eo_rn(r), rawk); _eo_retF(r); }
constr ejson_valrS(eobj r, constr rawk) { r = _getObjByRawk(_eo_rn(r), rawk); _eo_retS(r); }
cptr   ejson_valrP(eobj r, constr rawk) { r = _getObjByRawk(_eo_rn(r), rawk); _eo_retP(r); }
cptr   ejson_valrR(eobj r, constr rawk) { r = _getObjByRawk(_eo_rn(r), rawk); _eo_retR(r); }

etypeo ejson_valrType  (eobj r, constr rawk) { r = _getObjByRawk(_eo_rn(r), rawk); _eo_retT(r); }
constr ejson_valrTypeS (eobj r, constr rawk) { r = _getObjByRawk(_eo_rn(r), rawk); return eobj_typeoS(r); }
uint   ejson_valrLen   (eobj r, constr rawk) { r = _getObjByRawk(_eo_rn(r), rawk); _eo_retL(r); }
bool   ejson_valrIsTrue(eobj r, constr rawk) { return __eobj_isTrue(_getObjByRawk(_eo_rn(r), rawk));}


eobj   ejson_valk (eobj r, constr keys) { return _getObjByKeys(_eo_rn(r), keys);}
i64    ejson_valkI(eobj r, constr keys) { r = _getObjByKeys(_eo_rn(r), keys); _eo_retI(r); }
f64    ejson_valkF(eobj r, constr keys) { r = _getObjByKeys(_eo_rn(r), keys); _eo_retF(r); }
constr ejson_valkS(eobj r, constr keys) { r = _getObjByKeys(_eo_rn(r), keys); _eo_retS(r); }
cptr   ejson_valkP(eobj r, constr keys) { r = _getObjByKeys(_eo_rn(r), keys); _eo_retP(r); }
cptr   ejson_valkR(eobj r, constr keys) { r = _getObjByKeys(_eo_rn(r), keys); _eo_retR(r); }

etypeo ejson_valkType  (eobj r, constr keys) { r = _getObjByKeys(_eo_rn(r), keys); _eo_retT(r); }
constr ejson_valkTypeS (eobj r, constr keys) { r = _getObjByKeys(_eo_rn(r), keys); return eobj_typeoS(r); }
uint   ejson_valkLen   (eobj r, constr keys) { r = _getObjByKeys(_eo_rn(r), keys); _eo_retL(r); }
bool   ejson_valkIsTrue(eobj r, constr keys) { return __eobj_isTrue(_getObjByKeys(_eo_rn(r), keys));}

/** -----------------------------------------------------
 *
 *  ejson format
 *
 *  -----------------------------------------------------
 */

static void __wrap_str (cstr src, int _len, estr* s);

#define     __wrap_KEY(      n,       s)   __wrap_str(_n_keyS(n), _cur_lenkeyS(_n_keyS(n)), s)
#define     __wrap_STR(      n,       s)   __wrap_str(_n_valS(n), _n_len      (        n ), s)
static void __wrap_NUM(_ejsn n, estr* s);
static void __wrap_OBJ(_ejsn n, estr* s, int depth);
static void __wrap_ARR(_ejsn n, estr* s, int depth);

static estr __wrap_ejsn(_ejsn n, estr* s, int depth);

estr ejson_toSr(eobj o, constr rawk, estr* out, eopts opts) { return ejson_toS(_getObjByRawk(_eo_rn(o), rawk), out, opts); }
estr ejson_toSk(eobj o, constr keys, estr* out, eopts opts) { return ejson_toS(_getObjByKeys(_eo_rn(o), keys), out, opts); }
estr ejson_toS (eobj o,              estr* out, eopts opts)
{
    estr buf;

    is0_ret(o, 0);

    if(out)
    {
        estr_clear(*out);
    }
    else
    {
        buf = 0;
        out = &buf;
    }

    return  __wrap_ejsn(_eo_dn(o), out, opts & PRETTY ? 0 : -1);
}



static estr __wrap_ejsn(_ejsn n, estr* s, int depth)
{
    switch (_n_typeo(n))
    {
        case ENULL  :  estr_catB(*s, "null" , 4); break;
        case EFALSE :  estr_catB(*s, "false", 5); break;
        case ETRUE  :  estr_catB(*s, "true" , 4); break;
        case ENUM   :  __wrap_NUM(n, s);     break;
        case ESTR   :  __wrap_STR(n, s);     break;
        case EPTR   :  estr_catP(*s, "\"(PTR@%p)\"", _n_valP(n)); break;
        case ERAW   :  estr_catP(*s, "\"(RAW %d)\"", _n_len (n)); break;
        case EARR   :  __wrap_ARR(n, s, depth);  break;
        case EOBJ   :  __wrap_OBJ(n, s, depth);  break;

        default     : eerrlog("invalid type in __wrap_exec()"); abort();
    }

    return *s;
}

static void __wrap_NUM(_ejsn n, estr* s)
{
    if(!_n_typee(n))
    {
        if ( is_eq(_n_valI(n), 0) )
            estr_catB(*s, "0", 1);              // special case for 0.
        else
            estr_catF(*s, "%I", _n_valI(n));    // i64
    }
    else                                        // f64
    {
        f64 d = _n_valF(n);

        if      (d * 0 != 0)									    estr_catB(*s, "\"[nan]\"", 7);  /* This checks for NaN and Infinity */
        else if (fabs(floor(d)-d)<=DBL_EPSILON && fabs(d)<1.0e60)	estr_catP(*s, "%.0f",d);
        else if (fabs(d)<1.0e-6 || fabs(d)>1.0e9)					estr_catP(*s, "%e",d);
        else                                                     	estr_catP(*s, "%f",d);
    }
}

static void __wrap_str(cstr src, int _len, estr* s)
{
    constr ptr; bool flag = 0;  unsigned char u; char c;

    estr_catB(*s, "\"", 1);

    for (ptr=src; (c = *ptr);ptr++) if((c>0 && c<32)||(c=='\"')||(c=='\\')) {flag=1;break;}
    if (!flag)
    {
        estr_catB(*s, src, _len);
        estr_catB(*s, "\"", 1);

        return ;
    }

    ptr = src;
    while (*ptr)
    {
        if ((unsigned char)*ptr>31 && *ptr!='\"' && *ptr!='\\') estr_catB(*s, ptr++, 1);

        else
        {
            estr_catB(*s, "\\", 1);
            switch(u = *ptr++)
            {
                case '\\':	estr_catB(*s, "\\"   , 1);	break;
                case '\"':	estr_catB(*s, "\""   , 1);	break;
                case '\b':	estr_catB(*s, "b"    , 1);	break;
                case '\f':	estr_catB(*s, "f"    , 1);	break;
                case '\n':	estr_catB(*s, "n"    , 1);	break;
                case '\r':	estr_catB(*s, "r"    , 1);	break;
                case '\t':	estr_catB(*s, "t"    , 1);	break;
                default  :  estr_catP(*s, "u%04x", u);  break;	/* escape and print */
            }
        }
    }
    estr_catB(*s, "\"", 1);
}

static void __wrap_OBJ(_ejsn n, estr* s ,int depth)
{
    // -- Explicitly handle empty object case
    if (!_n_len(n))
    {
        estr_catB(*s, "{}", 2);
        return ;
    }

    // -- Compose the output
    n   = _r_head((_ejsr)n);
    if(depth >= 0)
    {
        depth++;

        estr_catB(*s, "{\n", 2);

        while(n)
        {
            estr_catC(*s, '\t', depth);

            __wrap_KEY(n, s);
            estr_catB(*s, ": ", 2);

            __wrap_ejsn(n, s, depth);
            estr_catB(*s, ",\n", 2);

            n = _n_lnext(n);
        }
        estr_decrLen(*s,    2);
        estr_catB(*s, "\n", 1);
        estr_catC(*s, '\t', depth - 1);
        estr_catB(*s, "}" , 1);
    }
    else
    {
        estr_catB(*s, "{\n", 1);

        while(n)
        {
            __wrap_KEY(n, s);
            estr_catB(*s, ": ", 1);

            __wrap_ejsn(n, s, depth);
            estr_catB(*s, ",\n", 1);

            n = _n_lnext(n);
        }
        estr_decrLen(*s,   1);
        estr_catB(*s, "}", 1);
    }
}

static void __wrap_ARR(_ejsn n, estr* s, int depth)
{
    // -- Explicitly handle numentries == 0
    if (!_n_len(n))
    {
        estr_catB(*s, "[]", 2);
        return ;
    }

    // -- Compose the output array.
    {
        u8 gap;

        gap = depth >= 0 ? 2 : 1;
        n   = _r_head((_ejsr)n);

        estr_catB(*s, "[", 1);
        while(n)
        {
            __wrap_ejsn(n, s, depth);
            estr_catB(*s, ", ", gap);

            n = _n_lnext(n);
        }
        estr_decrLen(*s, gap);
        estr_catB(*s, "]", 1);
    }
}

/** -----------------------------------------------------
 *
 *  ejson comparing
 *
 *  -----------------------------------------------------
 */
int ejson_cmpi(eobj o, int    val) { return o ? _eo_typeo(o) == ENUM ? __eobj_valI(o) < val ? -1 : __eobj_valI(o) > val ? 1 : 0 : -3 : -2; }
int ejson_cmpf(eobj o, double val) { return o ? _eo_typeo(o) == ENUM ? __eobj_valF(o) < val ? -1 : __eobj_valF(o) > val ? 1 : 0 : -3 : -2; }
int ejson_cmps(eobj o, constr str) { return o ? _eo_typeo(o) == ESTR ? str ? strcmp(_eo_valS(o), str) : -4 : -3 : -2;}

int ejson_rcmpi(eobj r, constr rawk, int    val) { return ejson_cmpi(_getObjByRawk(_eo_rn(r), rawk), val);}
int ejson_rcmpf(eobj r, constr rawk, double val) { return ejson_cmpf(_getObjByRawk(_eo_rn(r), rawk), val);}
int ejson_rcmps(eobj r, constr rawk, constr str) { return ejson_cmps(_getObjByRawk(_eo_rn(r), rawk), str);}

int ejson_kcmpi(eobj r, constr keys, int    val) { return ejson_cmpi(_getObjByKeys(_eo_rn(r), keys), val);}
int ejson_kcmpf(eobj r, constr keys, double val) { return ejson_cmpi(_getObjByKeys(_eo_rn(r), keys), val);}
int ejson_kcmps(eobj r, constr keys, constr str) { return ejson_cmps(_getObjByKeys(_eo_rn(r), keys), str);}

/** -----------------------------------------------------
 *
 *  ejson iterating
 *
 *  -----------------------------------------------------
 */

#define _o_is_parent(r) (r && (_eo_typeco(r) == _EJSON_CO_OBJ || _eo_typeco(r) == _EJSON_CO_ARR))

eobj  ejson_first(eobj r) { return _o_is_parent(r) ? _n_o(_r_head(_eo_rn(r))) : 0; }
eobj  ejson_last (eobj r) { return _o_is_parent(r) ? _n_o(_r_tail(_eo_rn(r))) : 0; }
eobj  ejson_next (eobj o) { return (o && _n_lnext(_eo_dn(o))) ? _n_o(_n_lnext(_eo_dn(o))) : 0; }
eobj  ejson_prev (eobj o) { return (o && _n_lprev(_eo_dn(o))) ? _n_o(_n_lprev(_eo_dn(o))) : 0; }

eobj  ejson_rfirst(eobj r, constr rawk) { return ejson_first(_getObjByRawk(_eo_rn(r), rawk)); }
eobj  ejspn_rlast (eobj r, constr rawk) { return ejson_last (_getObjByRawk(_eo_rn(r), rawk)); }

eobj  ejson_kfirst(eobj r, constr keys) { return ejson_first(_getObjByKeys(_eo_rn(r), keys)); }
eobj  ejson_klast (eobj r, constr keys) { return ejson_last (_getObjByKeys(_eo_rn(r), keys)); }

/** -----------------------------------------------------
 *
 *  ejson take and free
 *
 *  -----------------------------------------------------
 */
eobj ejson_takeH(eobj r)              { if(r){ switch(_eo_typeco(r)) { case _EJSON_CO_OBJ: return _obj_popH (_eo_rn(r)           ); case _EJSON_CO_ARR: return _arr_popH(_eo_rn(r)            ); }} return 0;}
eobj ejson_takeT(eobj r)              { if(r){ switch(_eo_typeco(r)) { case _EJSON_CO_OBJ: return _obj_popT (_eo_rn(r)           ); case _EJSON_CO_ARR: return _arr_popT(_eo_rn(r)            ); }} return 0;}
eobj ejson_takeO(eobj r, eobj      o) { if(r){ switch(_eo_typeco(r)) { case _EJSON_CO_OBJ: return _obj_takeN(_eo_rn(r), _eo_dn(o)); case _EJSON_CO_ARR: return _arr_takeN(_eo_rn(r), _eo_dn(o)); }} return 0;}
eobj ejson_takeK(eobj r, constr keys) { return _rmObjByKeys(_eo_rn(r), keys); }
eobj ejson_takeR(eobj r, constr rawk) { return _rmObjByRawk(_eo_rn(r), rawk); }
eobj ejson_takeI(eobj r, int     idx) { return (r &&  _eo_typeco(r) == _EJSON_CO_ARR) ? _arr_takeI(_eo_rn(r), idx) : 0; }

int  ejson_freeH(eobj r)              { return ejson_free(ejson_takeH(r     )); }
int  ejson_freeT(eobj r)              { return ejson_free(ejson_takeH(r     )); }
int  ejson_freeO(eobj r, eobj      o) { return ejson_free(ejson_takeO(r,   o)); }
int  ejson_freeK(eobj r, constr keys) { return ejson_free(_rmObjByKeys(_eo_rn(r), keys)); }
int  ejson_freeR(eobj r, constr rawk) { return ejson_free(_rmObjByRawk(_eo_rn(r), rawk)); }
int  ejson_freeI(eobj r, int     idx) { return ejson_free(ejson_takeI(r, idx)); }

int  ejson_freeHEx(eobj r,              eobj_rls_ex_cb rls, eval prvt) { return ejson_freeEx(ejson_takeH(r     ), rls, prvt); }
int  ejson_freeTEx(eobj r,              eobj_rls_ex_cb rls, eval prvt) { return ejson_freeEx(ejson_takeT(r     ), rls, prvt); }
int  ejson_freeOEx(eobj r, eobj      o, eobj_rls_ex_cb rls, eval prvt) { return ejson_freeEx(ejson_takeO(r,   o), rls, prvt); }
int  ejson_freeKEx(eobj r, constr keys, eobj_rls_ex_cb rls, eval prvt) { return ejson_freeEx(_rmObjByKeys(_eo_rn(r), keys), rls, prvt);}
int  ejson_freeREx(eobj r, constr rawk, eobj_rls_ex_cb rls, eval prvt) { return ejson_freeEx(_rmObjByRawk(_eo_rn(r), rawk), rls, prvt);}
int  ejson_freeIEx(eobj r, int     idx, eobj_rls_ex_cb rls, eval prvt) { return ejson_freeEx(ejson_takeI(r, idx), rls, prvt); }



/// -- ejson set val --
///

#if 0

ejson ejso_setT(ejson obj, uint  type)
{
    is0_ret(obj, 0); is1_ret(type > ENULL, 0);

    if(_TYPE(obj) <= ENULL)
        _TYPE(obj) = type;

    return obj;
}

ejson ejso_setF(ejson obj, double val)
{
    is0_ret(obj, 0); is1_ret(_TYPE(obj) != ENUM, 0);

    _valI(obj) = (i64)(_valF(obj) = val);

    return obj;
}

ejson ejso_setS(ejson obj, constr val)
{
    cstr hv, nv;

    is0_ret(obj, 0); is0_ret(val, 0); is1_ret(_TYPE(obj) != ESTR, 0);

    hv  = _valS(obj);
    is0_exeret(nv = _wrbS(hv, val, strlen(val)), errset(_ERRSTR_ALLOC(str));, 0);
    if(nv != hv) _valS(obj) = nv;

    return obj;
}

void* ejso_setR(ejson obj, uint   len)
{
    void* hr, * nr;

    is0_ret(obj, 0); is0_ret(len, 0);

    switch (_TYPE(obj)) {
        case ERAW: hr = _valR(obj);
                        is0_exeret(nr = _relS(hr, len), errset(_ERRSTR_ALLOC(raw));, 0);
                        if(nr != hr) _valR(obj) = nr;
                        return nr;
        case EPTR: is0_exeret(nr = _newS2(0, len), errset(_ERRSTR_ALLOC(raw));, 0);
                        _valR(obj) = nr; _setRAW(obj);
                        return nr;
        default       : return 0;
    }

    return 0;
}

ejson ejso_setP(ejson obj, void*  ptr)
{
    is0_ret(obj, 0);

    switch (_TYPE(obj)) {
        case ERAW: _freeS(_valR(obj)); _valP(obj) = ptr; _setPTR(obj); break;
        case EPTR:                     _valP(obj) = ptr;               break;
        default       : return 0;
    }

    return obj;
}

ejson ejsk_setT(ejson r, constr keys, uint  type)
{
    _checkParent(r);_checkInvldS(keys); is1_ret(type > ENULL, 0);
    is0_ret(r = _getObjByKeys(r, keys), 0);

    if(_TYPE(r) <= ENULL)
        _TYPE(r) = type;

    return r;
}

ejson ejsk_setF(ejson r, constr keys, double val)
{
    _checkParent(r);_checkInvldS(keys);
    is0_ret(r = _getObjByKeys(r, keys), 0);

    is1_ret(_TYPE(r) != ENUM, 0);

    _valI(r) = (i64)(_valF(r) = val);

    return r;
}

ejson ejsk_setS(ejson r, constr keys, constr val)
{
    cstr hv, nv;

    _checkParent(r);_checkInvldS(keys); is0_ret(val, 0);
    is0_ret(r = _getObjByKeys(r, keys), 0);
    is1_ret(_TYPE(r) != ESTR, 0);

    hv  = _valS(r);
    is0_exeret(nv = _wrbS(hv, val, strlen(val)), errset(_ERRSTR_ALLOC(str));, 0);
    if(nv != hv) _valS(r) = nv;

    return r;
}

void* ejsk_setR(ejson r, constr keys, uint   len)
{
    void* hr, * nr;

    _checkParent(r);_checkInvldS(keys); is0_ret(len, 0);
    is0_ret(r = _getObjByKeys(r, keys), 0);

    switch (_TYPE(r)) {
        case ERAW: hr = _valR(r);
                        is0_exeret(nr = _relS(hr, len), errset(_ERRSTR_ALLOC(raw));, 0);
                        if(nr != hr) _valR(r) = nr;
                        return nr;
        case EPTR: is0_exeret(nr = _newS2(0, len), errset(_ERRSTR_ALLOC(raw));, 0);
                        _valR(r) = nr; _setRAW(r);
                        return nr;
        default       : return 0;
    }

    return 0;
}

ejson ejsk_setP(ejson r, constr keys, void*  ptr)
{
    _checkParent(r);_checkInvldS(keys);
    is0_ret(r = _getObjByKeys(r, keys), 0);

    switch (_TYPE(r)) {
        case ERAW: _freeS(_valR(r)); _valP(r) = ptr; _setPTR(r); break;
        case EPTR:                      _valP(r) = ptr;                break;
        default       : return 0;
    }

    return r;
}

ejson ejsr_setT(ejson r, constr rawk, uint  type)
{
    _checkOBJ(r);_checkInvldS(rawk); is1_ret(type > ENULL, 0);
    is0_ret(r = _getObjByRawk(r, rawk), 0);

    if(_TYPE(r) <= ENULL)
        _TYPE(r) = type;

    return r;
}

ejson ejsr_setF(ejson r, constr rawk, double val)
{
    _checkOBJ(r);_checkInvldS(rawk);
    is0_ret(r = _getObjByRawk(r, rawk), 0);

    is1_ret(_TYPE(r) != ENUM, 0);

    _valI(r) = (i64)(_valF(r) = val);

    return r;
}

ejson ejsr_setS(ejson r, constr rawk, constr val)
{
    cstr hv, nv;

    _checkOBJ(r);_checkInvldS(rawk); is0_ret(val, 0);
    is0_ret(r = _getObjByRawk(r, rawk), 0);
    is1_ret(_TYPE(r) != ESTR, 0);

    hv  = _valS(r);
    is0_exeret(nv = _wrbS(hv, val, strlen(val)), errset(_ERRSTR_ALLOC(str));, 0);
    if(nv != hv) _valS(r) = nv;

    return r;
}

void* ejsr_setR(ejson r, constr rawk, uint   len)
{
    void* hr, * nr;

    _checkOBJ(r);_checkInvldS(rawk); is0_ret(len, 0);
    is0_ret(r = _getObjByRawk(r, rawk), 0);

    switch (_TYPE(r)) {
        case ERAW: hr = _valR(r);
                        is0_exeret(nr = _relS(hr, len), errset(_ERRSTR_ALLOC(raw));, 0);
                        if(nr != hr) _valR(r) = nr;
                        return nr;
        case EPTR: is0_exeret(nr = _newS2(0, len), errset(_ERRSTR_ALLOC(raw));, 0);
                        _valR(r) = nr; _setRAW(r);
                        return nr;
        default       : return 0;
    }

    return 0;
}

ejson ejsr_setP(ejson r, constr rawk, void*  ptr)
{
    _checkOBJ(r);_checkInvldS(rawk);
    is0_ret(r = _getObjByRawk(r, rawk), 0);

    switch (_TYPE(r)) {
        case ERAW: _freeS(_valR(r)); _valP(r) = ptr; _setPTR(r); break;
        case EPTR:                      _valP(r) = ptr;                break;
        default       : return 0;
    }

    return r;
}

#endif

/// -- ejson substitute string --

#if 0

ejson ejso_subk(ejson obj , constr subS, constr newS)
{
    cstr hk, nk;

    is1_ret(_isChild(obj), 0); is0_ret((hk = _keyS(obj)), obj); is1_ret(_invalidS(subS), obj); is0_ret(newS, obj);

    is0_exeret(nk = _subS(hk, subS, newS), errset(_ERRSTR_ALLOC(key));, 0);
    if(nk != hk) _keyS(obj) = nk;

    return obj;
}

ejson ejso_subS(ejson obj , constr subS, constr newS)
{
    cstr hs, ns;

    is0_ret(_isSTR(obj), 0); is1_ret(_invalidS(subS), obj); is0_ret(newS, obj);

    hs = _valS(obj);
    is0_exeret(ns = _subS(hs, subS, newS), errset(_ERRSTR_ALLOC(str));, 0);
    if(ns != hs) _valS(obj) = ns;

    return obj;
}

ejson ejsk_subS(ejson r, constr keys, constr subS, constr newS)
{
    cstr hs, ns;

    _checkParent(r);_checkInvldS(keys);
    r = _getObjByKeys(r, keys);

    is0_ret(_isSTR(r), 0); is1_ret(_invalidS(subS), r); is0_ret(newS, r);

    hs = _valS(r);
    is0_exeret(ns = _subS(hs, subS, newS), errset(_ERRSTR_ALLOC(str));, 0);
    if(ns != hs) _valS(r) = ns;

    return r;
}

ejson ejsr_subS(ejson r, constr rawk, constr subS, constr newS)
{
    cstr hs, ns;

    _checkOBJ(r);_checkInvldS(rawk);
    r = _getObjByRawk(r, rawk);

    is0_ret(_isSTR(r), 0); is1_ret(_invalidS(subS), r); is0_ret(newS, r);

    hs = _valS(r);
    is0_exeret(ns = _subS(hs, subS, newS), errset(_ERRSTR_ALLOC(str));, 0);
    if(ns != hs) _valS(r) = ns;

    return r;
}

#endif

#if 0

/// -- ejson counter --
ejson  ejso_cntpp(ejson obj)
{
    is1_ret(!_isNUM(obj), 0);
    _valF(obj) = (double)(++_valI(obj));

    return obj;
}
ejson  ejso_cntmm(ejson obj)
{
    is1_ret(!_isNUM(obj), 0);
    if(_valI(obj)) _valF(obj) = (double)(--_valI(obj));
    return obj;
}

ejson  ejsk_cntpp(ejson r, constr keys)
{
    ejson obj;
    _checkParent(r);_checkInvldS(keys);
    if((obj = _getObjByKeys(r, keys)))
    {
        if(_TYPE(obj) == ENUM) _valF(obj) = (double)(++_valI(obj));
        else return 0;
    }
    else
    {
        obj = ejso_addN(r, keys, 1);
    }

    return obj;
}

ejson  ejsk_cntmm(ejson r, constr keys)
{
    ejson obj;
    _checkParent(r);_checkInvldS(keys);
    if((obj = _getObjByKeys(r, keys)))
    {
        if(_TYPE(obj) == ENUM){ if(_valI(obj)) _valF(obj) = (double)(--_valI(obj));}
        else return 0;
    }
    else
    {
        obj = ejso_addN(r, keys, 0);
    }

    return obj;
}

ejson  ejsr_cntpp(ejson r, constr rawk)
{
    ejson obj;
    _checkOBJ(r);_checkInvldS(rawk);
    if((obj = _getObjByRawk(r, rawk)))
    {
        if(_TYPE(obj) == ENUM) _valF(obj) = (double)(++_valI(obj));
        else return 0;
    }
    else
    {
        obj = ejso_addN(r, rawk, 1);
    }

    return obj;
}
ejson  ejsr_cntmm(ejson r, constr rawk)
{
    ejson obj;
    _checkOBJ(r);_checkInvldS(rawk);
    if((obj = _getObjByRawk(r, rawk)))
    {
        if(_TYPE(obj) == ENUM){ if(_valI(obj)) _valF(obj) = (double)(--_valI(obj));}
        else return 0;
    }
    else
    {
        obj = ejso_addN(r, rawk, 0);
    }

    return obj;
}

#endif

/// -- ejson counter --

#if 0

#define DF_SORTBASE_LEN 128

ejson  ejso_sort(ejson r, __ecompar_fn fn)
{
    ejson* base; ejson itr; int i, j, len; ejson _base[DF_SORTBASE_LEN];

    is0_ret(_isParent(r), 0); is0_ret(fn, 0);
    is1_ret(_objLen(r) < 2, r);

    if(_objLen(r) <= DF_SORTBASE_LEN)
        base = _base;
    else
        is0_ret(base = malloc(_objLen(r) * sizeof(ejson)), 0);

    for(itr = _objHead(r), i = 0; itr; itr = _n_lnext(itr), i++)
        base[i] = itr;

    qsort(base, len = i, sizeof(ejson), (__compar_fn_t)fn);

    _objHead(r) = _objTail(r) = 0;
    for(i = 0; i < len - 1; i++)
    {
        j = i + 1;
        _n_lnext(base[i]) = base[j];
        _n_lprev(base[j]) = base[i];
    }
    _objHead(r) = base[0];       _n_lprev(base[0])       = 0;
    _objTail(r) = base[len - 1]; _n_lnext(base[len - 1]) = 0;

    if(base != _base) free(base);
    return r;
}

ejson  ejsk_sort(ejson r, constr keys, __ecompar_fn fn)
{
    ejson obj;
    _checkParent(r);_checkInvldS(keys);
    if((obj = _getObjByKeys(r, keys)))
    {
        return ejso_sort(obj, fn);
    }

    return obj;
}

ejson  ejsr_sort(ejson r, constr rawk, __ecompar_fn fn)
{
    ejson obj;
    _checkParent(r);_checkInvldS(rawk);
    if((obj = _getObjByRawk(r, rawk)))
    {
        return ejso_sort(obj, fn);
    }

    return obj;
}

int    __KEYS_ACS(ejson* _e1, ejson* _e2)
{
    cstr _c1, _c2; char c1, c2;

    _c1 = _keyS(*_e1);
    _c2 = _keyS(*_e2);

    if(_c1)
    {
        if(_c2)
        {
            c1 = *_c1; c2 = *_c2;
            while( c1 && c2 )
            {
                if(c1 > c2) return 1;
                if(c1 < c2) return 0;

                c1 = *(++_c1); c2 = *(++_c2);
            }

            return c1 > c2;
        }
        else
            return 0;
    }

    return _c2 ? 1 : 0;
}

int    __KEYS_DES(ejson* _e1, ejson* _e2)
{
    cstr _c1, _c2; char c1, c2;

    _c1 = _keyS(*_e1);
    _c2 = _keyS(*_e2);

    if(_c1)
    {
        if(_c2)
        {
            c1 = *_c1; c2 = *_c2;
            while( c1 && c2 )
            {
                if(c2 > c1) return 1;
                if(c2 < c1) return 0;

                c1 = *(++_c1); c2 = *(++_c2);
            }

            return c2 > c1;
        }
        else
            return 0;
    }

    return _c2 ? 1 : 0;
}

int    __VALI_ACS(ejson* _e1, ejson* _e2)
{
    ejson e1, e2; e1 = *_e1; e2 = *_e2;
    if(_TYPE(e1) == ENUM)
    {
        if(_TYPE(e2) == ENUM)
            return _valI(e1) - _valI(e2) > 0;   // swap when return val > 0
        else
            return 0;
    }

    return _TYPE(e2) == ENUM ? 1 : 0;
}

int    __VALI_DES(ejson* _e1, ejson* _e2)
{
    ejson e1, e2; e1 = *_e1; e2 = *_e2;
    if(_TYPE(e1) == ENUM)
    {
        if(_TYPE(e2) == ENUM)
            return _valI(e2) - _valI(e1) > 0;
        else
            return 0;
    }

    return _TYPE(e2) == ENUM ? 1 : 0;
}

#endif

// --------------------------- dict definition -----------------------
#define DICT_HASH_FUNCTION_SEED 5381;
uint __MurmurHash2(const void *key, int len) {
    /* 'm' and 'r' are mixing constants generated offline.
     They're not really 'magic', they just happen to work well.  */
    static uint32_t seed = DICT_HASH_FUNCTION_SEED;
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    /* Initialize the hash to a 'random' value */
    uint32_t h = seed ^ len;

    /* Mix 4 bytes at a time into the hash */
    const unsigned char *data = (const unsigned char *)key;

    while(len >= 4) {
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    /* Handle the last few bytes of the input array  */
    switch(len) {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0]; h *= m;
    default:;
    };

    /* Do a few final mixes of the hash to ensure the last few
     * bytes are well-incorporated. */
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return (unsigned int)h;
}

static inline uint __djbHashB(constr buf, int len) {
    register uint hash = (uint)5381;

    while (len--)
        hash += (hash << 5) + *buf++;   // hash * 33 + c

    return hash & ~(1u << 31);
}

static inline uint __djbHashS(constr buf) {
    register uint hash = (uint)5381;

    while (*buf)
        hash += (hash << 5) + *buf++;   // hash * 33 + c

    return hash & ~(1u << 31);
}

static inline int _dict_init(dict d)
{
    _dict_resetHt(d);
    d->rehashidx = -1;

    return DICT_OK;
}

static inline dict _dict_new()
{
    dict d = malloc(sizeof(*d));

    _dict_init(d);
    return d;
}

static inline void _dict_clear(dict d)
{
    if(d->ht[1].size > 0)
    {
        free(d->ht[0].table);
        d->ht[0] = d->ht[1];
        _dict_htreset(&d->ht[1]);
    }
    else
    {
        memset(d->ht[0].table, 0, sizeof(ejson*) * d->ht[0].size);
    }

    d->ht[0].used = 0;
    d->rehashidx = -1;
}

static inline void _dict_free(dict d)
{
    free(d->ht[0].table);
    free(d->ht[1].table);
    free(d);
}

static inline int _dictRehash(dict d, int n)
{
    int empty_visits = n * 10;
    is0_ret(_dictIsRehashing(d), 0);

    while(n-- && d->ht[0].used != 0)
    {
        _ejsn de, nextde;

        while(d->ht[0].table[d->rehashidx] == NULL) {
            d->rehashidx++;
            if (--empty_visits == 0) return 1;
        }

        de = d->ht[0].table[d->rehashidx];

         while(de) {
             unsigned int h;
             nextde = _n_dnext(de);
             h = _dictHashKey(_n_keyS(de), _cur_lenkeyS(_n_keyS(de))) & d->ht[1].sizemask;
             _n_dnext(de) = d->ht[1].table[h];
             d->ht[1].table[h] = de;
             d->ht[0].used--;
             d->ht[1].used++;
             de = nextde;
         }
         d->ht[0].table[d->rehashidx] = NULL;
         d->rehashidx++;
    }

    if (d->ht[0].used == 0) {
        free(d->ht[0].table);
        d->ht[0] = d->ht[1];
        _dict_htreset(&d->ht[1]);
        d->rehashidx = -1;
        return DICT_OK;
    }

    return DICT_ERR;
}

static inline void _dictRehashPtrStep(dict d)
{
    _dictRehash(d, 1);
}

static inline ulong _dictNextPower(ulong size)
{
    ulong i = DICT_HT_INITIAL_SIZE;

    if (size >= LONG_MAX) return LONG_MAX;
    while(1) {
        if (i >= size)
            return i;
        i *= 2;
    }
}

static inline int _dictExpand(dict d, ulong size)
{
    dictht n;
    ulong realsize = _dictNextPower(size);

    is1_ret(_dictIsRehashing(d) || d->ht[0].used > size, DICT_ERR);
    is1_ret(realsize == d->ht[0].size,                  DICT_ERR);

    n.size     = realsize;
    n.sizemask = realsize - 1;
    n.table    = calloc(realsize * sizeof(void*), 1);
    n.used     = 0;

    is1_exeret(d->ht[0].table == NULL, d->ht[0] = n, DICT_OK);

    d->ht[1] = n;
    d->rehashidx = 0;
    return DICT_OK;
}

static inline int _dictExpandIfNeeded(dict d)
{
    /* Incremental rehashing already in progress. Return. */
    if (_dictIsRehashing(d)) return DICT_OK;

    /* If the hash table is empty expand it to the initial size. */
    if (d->ht[0].size == 0) return _dictExpand(d, DICT_HT_INITIAL_SIZE);

    /* If we reached the 1:1 ratio, and we are allowed to resize the hash
     * table (global setting) or we should avoid it but the ratio between
     * elements/buckets is over the "safe" threshold, we resize doubling
     * the number of buckets. */
    if (d->ht[0].used >= d->ht[0].size &&
        (_dict_can_resize ||
         d->ht[0].used/d->ht[0].size > _dict_force_resize_ratio))
    {
        return _dictExpand(d, d->ht[0].used*2);
    }
    return DICT_OK;
}

static int _dictKeyIndex(dict d, const void* key, int key_len)
{
    uint h, idx, table;
    _ejsn he;

    /* Expand the hash table if needed */
    if (_dictExpandIfNeeded(d) == DICT_ERR)
        return -1;
    /* Compute the key hash value */
    h = _dictHashKey(key, key_len);
    for (table = 0; table <= 1; table++) {
        idx = h & d->ht[table].sizemask;
        /* Search if this slot does not already contain the given key */
        he = d->ht[table].table[idx];
        while(he) {
            if ( !_cur_cmpkeyS(key, _n_keyS(he)) )
                return -1;
            he = _n_dnext(he);
        }
        if (!_dictIsRehashing(d)) break;
    }
    return idx;
}

static inline _ejsn _dict_add(dict d, constr k, int k_len, _ejsn n)
{
    int     idx;
    dictht* ht;

    if(_dictIsRehashing(d)) _dictRehashPtrStep(d);

    is1_ret((idx = _dictKeyIndex(d, k, k_len)) == -1, NULL); // already exist

    ht = _dictIsRehashing(d) ? &d->ht[1] : &d->ht[0];
    _n_dnext(n)    = ht->table[idx];
    ht->table[idx] = n;
    ht->used++;

    return n;
}

static _ejsn _dict_find(dict d, constr k, int k_len)
{
    _ejsn he;
    unsigned int h, idx, table;

    if(d->ht[0].size == 0) return NULL; /* We don't have a table at all */
    if(_dictIsRehashing(d)) _dictRehashPtrStep(d);
    h = _dictHashKey(k, k_len);
    for (table = 0; table <= 1; table++) {
        idx = h & d->ht[table].sizemask;
        he = d->ht[table].table[idx];
        while(he) {
            if ( !_cur_cmpkeyS(k, _n_keyS(he)) )
                return he;
            he = _n_dnext(he);
        }
        if (!_dictIsRehashing(d)) return NULL;
    }
    return NULL;
}

static _ejsn _dict_findS(dict d, constr k)
{
    _ejsn he;
    unsigned int h, idx, table;

    if(d->ht[0].size == 0) return NULL; /* We don't have a table at all */
    if(_dictIsRehashing(d)) _dictRehashPtrStep(d);
    h = _dictHashKey(k, strlen(k));
    for (table = 0; table <= 1; table++) {
        idx = h & d->ht[table].sizemask;
        he = d->ht[table].table[idx];
        while(he) {
            if ( !_cur_cmpkeyS(k, _n_keyS(he)) )
                return he;
            he = _n_dnext(he);
        }
        if (!_dictIsRehashing(d)) return NULL;
    }
    return NULL;
}

static _ejsn _dict_find_ex(dict d, constr k, int k_len, bool rm)
{
    unsigned int h, idx;
    _ejsn he, prevHe;
    int table;

    if (d->ht[0].size == 0) return NULL; /* d->ht[0].table is NULL */
    if (_dictIsRehashing(d)) _dictRehashPtrStep(d);
    h = _dictHashKey(k, k_len);

    for (table = 0; table <= 1; table++) {
        idx = h & d->ht[table].sizemask;
        he = d->ht[table].table[idx];
        prevHe = NULL;
        while(he) {
            if (!_cur_cmpkeyS(k, _n_keyS(he))) {
                if(rm)
                {
                    /* Unlink the element from the list */
                    if (prevHe) _n_dnext(prevHe)        = _n_dnext(he);
                    else        d->ht[table].table[idx] = _n_dnext(he);
                    d->ht[table].used--;
                }

                return he;
            }
            prevHe  = he;
            he      = _n_dnext(he);
        }
        if (!_dictIsRehashing(d)) break;
    }
    return NULL; /* not found */
}


static int _dict_getL(dict d, constr k, int k_len, L l)
{
    int     idx;
    dictht* ht;

    if(_dictIsRehashing(d)) _dictRehashPtrStep(d);

    is1_ret((idx = _dictKeyIndex(d, k, k_len)) == -1, 0); // already exist

    ht = _dictIsRehashing(d) ? &d->ht[1] : &d->ht[0];
    l->_pos  = &ht->table[idx];
    l->_used = &ht->used;

    return 1;
}


static _ejsn _dict_del(dict d, _ejsn del)
{
    unsigned int h, idx;
    _ejsn he, prevHe;
    int table;

    if (d->ht[0].size == 0) return NULL; /* d->ht[0].table is NULL */
    if (_dictIsRehashing(d)) _dictRehashPtrStep(d);
    h = _dictHashKey(_n_keyS(del), _cur_lenkeyS(_n_keyS(del)));

    for (table = 0; table <= 1; table++) {
        idx = h & d->ht[table].sizemask;
        he = d->ht[table].table[idx];
        prevHe = NULL;
        while(he) {
            if (!_cur_cmpkeyS(_n_keyS(del), _n_keyS(he))) {
                if(he != del)   return NULL;    // not match

                /* Unlink the element from the list */
                if (prevHe) _n_dnext(prevHe)        = _n_dnext(he);
                else        d->ht[table].table[idx] = _n_dnext(he);
                d->ht[table].used--;
                return he;
            }
            prevHe  = he;
            he      = _n_dnext(he);
        }
        if (!_dictIsRehashing(d)) break;
    }
    return NULL; /* not found */
}

/// --------------------------- inner OBJ operation

static inline _ejsn _obj_add(_ejsr r, cstr key, _ejsn n)
{
    is0_ret(_dict_add(_obj_hd(r), key, _cur_lenkeyS(key), n), 0);
    _n_linked(n) = 1;

    if(!_r_head(r)){_r_head(r) =           _r_tail(r)  = n;}

    else           {_n_lprev(n) =          _r_tail(r)     ;
                    _r_tail (r) = _n_lnext(_r_tail(r)) = n;}

    _r_len(r)++;

    return n;
}

static inline void _obj_link(_ejsr r, _ejsn n, L l)
{
    _n_linked(n) = 1;

    _dict_link(*l, n);

    if(!_r_head(r)){_r_head(r) =          _r_tail(r)  = n;}

    else           {_n_lprev(n ) =          _r_tail(r)     ;
                    _r_tail(r)   = _n_lnext(_r_tail(r)) = n;}

    _r_len(r)++;
}


static inline eobj _obj_popH(_ejsr r)
{
    _ejsn n;

    is0_ret(_r_len(r), 0);

    _dict_del(_obj_hd(r), (n = _r_head(r)));

    if(1 == _r_len(r)){_r_head(r) = _r_tail(r)            = NULL;}

    else              {_r_head(r)                         = _n_lnext(n);
                                    _n_lprev(_n_lnext(n)) = NULL;}

    _n_lprev(n) = _n_lnext(n) = NULL;
    _n_linked(n) = 0;

    _r_len(r)--;

    return _n_o(n);
}

static inline eobj _obj_popT(_ejsr r)
{
    _ejsn n;

    is0_ret(_r_len(r), 0);

    _dict_del(_obj_hd(r), (n = _r_tail(r)));

    if(1 == _r_len(r)){_r_head(r) = _r_tail(r)            = NULL         ;}

    else              {_r_tail(r)                         = _n_lprev(n);
                                    _n_lnext(_n_lprev(n)) = NULL         ;}

    _n_lprev(n) = _n_lnext(n) = NULL;
    _n_linked(n) = 0;

    _r_len(r)--;

    return _n_o(n);
}

static inline eobj  _obj_takeN(_ejsr r, _ejsn n)
{
    if(!_dict_del(_obj_hd(r), n))  return NULL;

    if   (_n_lprev(n))  _n_lnext(_n_lprev(n)) = _n_lnext(n);
    else                _r_head(r)            = _n_lnext(n);

    if   (_n_lnext(n))  _n_lprev(_n_lnext(n)) = _n_lprev(n);
    else                _r_tail(r)            = _n_lprev(n);

    _n_lprev(n) = _n_lnext(n) = NULL;
    _n_linked(n) = 0;

    _r_len(r)--;

    return _n_o(n);
}

/// -------------------------------- inner ARR operation

typedef struct { _ejsn o; uint  i;}* H;

// -- add in tail
static void _arr_appd(_ejsr r, _ejsn n)
{
    _n_linked(n) = 1;

    if(!_r_head(r)){_r_head(r) =          _r_tail(r)  = n;}

    else           {_n_lprev(n)=          _r_tail(r)     ;
                    _r_tail(r) = _n_lnext(_r_tail(r)) = n;}

    _r_len(r)++;
}

// -- add in head
static _ejsn _arr_push(_ejsr r, _ejsn n)
{
    _n_linked(n) = 1;

    if(!_r_head(r)){_r_head(r) =          _r_tail(r)  = n;}

    else           {_n_lnext(n)  =          _r_head(r)       ;
                    _r_head(r) = _n_lprev(_r_head(r)) = n;
                    //if(((H)_r_list(r))->i) ((H)_elist(r))->i++;
    }

    _r_len(r)++;

    return n;
}

// -- remove in head
static eobj _arr_popH(_ejsr r)
{
    _ejsn n;

    is0_ret(_r_len(r), 0);

    n = _r_head(r);

    if(1 == _r_len(r)){_r_head(r) = _r_tail(r)          = NULL;}

    else              { _r_head(r)                           = _n_lnext(n);
                        _n_lprev(_n_lnext(n)) = NULL;
                           //if(((H)_elist(r))->i) ((H)_elist(r))->i--;
    }

    _r_len(r)--;

    _n_lprev(n)  = _n_lnext(n) = NULL;
    _n_linked(n) = 0;

    return _n_o(n);
}

// -- remove in tail
static eobj _arr_popT(_ejsr r)
{
    _ejsn n; H h;

    is0_ret(_r_len(r), 0);

    n = _r_tail(r);
    h = _arr_hd(r);

    if(1 == _r_len(r)){_r_head(r) = _r_tail(r)          = NULL;}

    else              {_r_tail(r)                       = _n_lprev(n);
                                  _n_lnext(_n_lprev(n)) = NULL         ;
                       if(h->i && h->o == n) {h->o = _n_lprev(n); h->i--;} ;}

    _r_len(r)--;

    _n_lprev(n) = _n_lnext(n) = NULL;
    _n_linked(n) = 0;

    return _n_o(n);
}

static _ejsn _arr_find(_ejsr r, uint idx)
{
    _ejsn n; uint i; H h;

    is0_ret(idx < _r_len(r), 0);

    h = _arr_hd(r);

    if(idx >= (i = h->i)) { n = i ? h->o : _r_head(r); for(; i != idx; i++) n = _n_lnext(n); }
    else                  { n =     h->o             ; for(; i != idx; i--) n = _n_lprev(n); }

    h->i = idx;
    h->o = n;

    return n;
}

static _ejsn _elistFind_ex(_ejsr r, uint idx, bool rm)
{
    _ejsn n; uint i; H h;

    h = _arr_hd(r);

    if(idx >= (i = h->i)) { n = i ? h->o : _r_head(r); for(; i != idx; i++) n = _n_lnext(n); }
    else                  { n =     h->o             ; for(; i != idx; i--) n = _n_lprev(n); }

    if(rm)
    {

    if   (_n_lprev(n)) _n_lnext(_n_lprev(n)) = _n_lnext(n);
    else               _r_tail(r)            = _n_lnext(n);

    if   (_n_lnext(n)){_n_lprev(h->o = _n_lnext(n)) = _n_lprev(n);            h->i = idx    ;}
    else              {         h->o = _r_tail(r)   = _n_lprev(n);    if(idx) h->i = idx - 1;}

    _r_len(r)--;

    _n_lprev(n)  = _n_lnext(n) = NULL;
    _n_linked(n) = 0;

    }

    return n;
}


static eobj _arr_takeI(_ejsr r, uint idx)
{
    _ejsn n; uint i; H h;

    h = _arr_hd(r);

    if(idx >= (i = h->i)) { n = i ? h->o : _r_head(r); for(; i != idx; i++) n = _n_lnext(n); }
    else                  { n =     h->o             ; for(; i != idx; i--) n = _n_lprev(n); }

    if   (_n_lprev(n)) _n_lnext(_n_lprev(n)) = _n_lnext(n);
    else               _r_head(r)            = _n_lnext(n);

    if   (_n_lnext(n)){_n_lprev(h->o = _n_lnext(n)) = _n_lprev(n);            h->i = idx    ;}
    else              {         h->o = _r_tail(r)   = _n_lprev(n);    if(idx) h->i = idx - 1;}

    _r_len(r)--;

    _n_lprev(n) = _n_lnext(n) = NULL;
    _n_linked(n) = 0;

    return _n_o(n);
}

static eobj _arr_takeN(_ejsr r, _ejsn del)
{
    _ejsn n; uint i; H h;

    h = _arr_hd(r);

    if((i = h->i))  {        for(              n =          h->o ; n && n != del; n = _n_lnext(n), i++);
                      if(!n) for(i = h->i - 1, n = _n_lprev(h->o); n && n != del; n = _n_lprev(n), i--); }
    else            {        for(i = 0       , n = _r_head (r   ); n && n != del; n = _n_lnext(n), i++); }

    is0_ret(n, 0);

    if   (_n_lprev(n)) _n_lnext(_n_lprev(n)) = _n_lnext(n);
    else               _r_head(r)            = _n_lnext(n);

    if   (_n_lnext(n)){_n_lprev(h->o = _n_lnext(n)) = _n_lprev(n);          h->i = i    ;}
    else              {         h->o = _r_tail (r)  = _n_lprev(n);    if(i) h->i = i - 1;}

    _r_len(r)--;

    _n_lprev(n) = _n_lnext(n) = NULL;
    _n_linked(n) = 0;

    return _n_o(n);
}

inline constr ejson_version() {   return EJSON_VERSION;    }
