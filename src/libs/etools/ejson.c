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
#include "ejson.h"

static constr g_err;
static constr g_errp;
static char   g_err_buf[1024];

#define exe_ret(expr, ret ) { expr;      return ret;}
#define is0_ret(cond, ret ) if(!(cond)){ return ret;}
#define is1_ret(cond, ret ) if( (cond)){ return ret;}
#define is0_exe(cond, expr) if(!(cond)){ expr;}
#define is1_exe(cond, expr) if( (cond)){ expr;}

#define is0_exeret(cond, expr, ret) if(!(cond)){ expr;        return ret;}
#define is1_exeret(cond, expr, ret) if( (cond)){ expr;        return ret;}
#define is0_elsret(cond, expr, ret) if(!(cond)){ expr;} else{ return ret;}
#define is1_elsret(cond, expr, ret) if( (cond)){ expr;} else{ return ret;}

#if defined(_WIN32)
#define __unused
#define inline
#define strdup _strdup
typedef _In_ int (__cdecl* __compar_fn_t)(void const*, void const*);
#if defined(_MSC_VER) && _MSC_VER >= 1600
#include <stdint.h>
#elif defined(__WATCOMC__) || defined(__MINGW32__) || defined(__CYGWIN__)
#include <stdint.h>
#else
typedef unsigned int  uint32_t, u32;
typedef unsigned char uint8_t , u8;
#endif
typedef unsigned long  int ulong;
typedef unsigned short int ushort, u16;
typedef unsigned int       uint,   u32;
#include <io.h>
#define memccpy  _memccpy
#define open     _open
#define close    _close
#define lseek    _lseek
#define read     _read
#elif defined(__GNUC__) && !defined(__VXWORKS__)
#include <stdint.h>
#include <unistd.h>
#define __unused __attribute__((unused))
#else
#define __unused __attribute__((unused))
typedef unsigned int  uint32_t, u32;
typedef unsigned char uint8_t , u8;
#endif


// ------------------------- obj header -----------------------
typedef struct obj_header_s{
    uint type     :  4;
    uint reserved : 10;
    uint is_array :  1;
    uint is_ref   :  1;
    uint _ref     : 16;
    uint _len     : 32;
}obj_header_t, * OBJ;

// -- helper used for micros --
static void* obj__new(size_t size, uint _len);
static cstr  obj__str(uint _len);

// -- micros --
#define OBJ_size            sizeof(obj_header_t)
#define obj_to_val(o)       (void*)(o + 1)
#define val_to_obj(v)       ((char*)v - OBJ_size)
#define val_type(v)         val_to_obj(v)->type
#define obj_new(T)          (T*)obj__new(sizeof(T), 1)
//#define val_free(v)         free(val_to_obj(v));
//#define obj_newArr(T, len)  (T*)obj__new(sizeof(T), (len))
//#define obj_newStr(len)     obj__str(len)
//#define val_arrLen(v)       ((OBJ)val_to_obj(v))->_len
//#define val_strLen(v)       ((OBJ)val_to_obj(v))->_len

// --------------------- str for ejson ----------------------------

#define COMPAT_ESTR 0

typedef struct __attribute__ ((__packed__)) _s_s{
    uint cap;
    uint len;
#if COMPAT_ESTR
    u8   type;
#endif
    char s[];
}_s_t, * _s;

// -- API --
static cstr _snew1(constr src);
static cstr _snew2(conptr ptr, uint len);

// -- micros --
#define _s2hdr(s)    ((_s)(s) - 1)
#define _hdr2s(h)    ((cstr)((_s)(h) + 1))
#define _snewc(l)    calloc(sizeof(_s_t) + l + 1, 1)
#define _snewm(l)    malloc(sizeof(_s_t) + l + 1)
#define _snewr(h, l) realloc(h, sizeof(_s_t) + l + 1)
#define _sfree(s)    free(_s2hdr(s))
#if COMPAT_ESTR
#define _slen(s)     ((uint*)((cstr)s-1))[-1]
#define _scap(s)     ((uint*)((cstr)s-1))[-2]
#else
#define _slen(s)     ((uint*)(s))[-1]
#define _scap(s)     ((uint*)(s))[-2]
#endif

static inline cstr _snew1(constr src)       // create a _s from cstr
{
    is0_ret(src, 0);
    return _snew2(src, strlen(src));
}

static inline cstr _snew2(conptr ptr, uint len) // create a _s from ptrs
{
    _s hdr; cstr s;

    is0_ret(hdr  = _snewm(len), 0);
    hdr->cap = len;
    s        = hdr->s;

    if(ptr){ hdr->len = len; memcpy(s, ptr, len    ); s[len] = 0;}
//  else   { hdr->len = 0  ; memset(s, 0  , len + 1);            }      // we do not need this now
    else   { hdr->len = len; memset(s, 0  , len + 1);            }

    return s;
}

static inline cstr _swrb(cstr s, conptr ptr, uint len)  // write banary data to _s, from begining
{
    _s hhdr, nhdr;

    if(!s)
    {
        is0_ret(nhdr  = _snewm(len), 0);
        nhdr->cap = len;
        nhdr->len = len;
        s         = nhdr->s;
        memcpy(s, ptr, len);
        s[len] = 0;
        return s;
    }

    if(len > _scap(s))
    {

        hhdr = _s2hdr(s);
        nhdr = _snewr(hhdr, len);
        is0_ret(nhdr, 0);

        if(hhdr != nhdr) s = nhdr->s;
        nhdr->cap = len;
    }

    _slen(s) = len;
    memcpy(s, ptr, len);
    s[len] = '\0';

    return s;
}

static inline cstr _srel(cstr s, uint len)      // reset len
{
    if(len > _scap(s))
    {
        _s hhdr, nhdr;
        hhdr = _s2hdr(s);
        nhdr = _snewr(hhdr, len);
        is0_ret(nhdr, 0);

        if(hhdr != nhdr) s = nhdr->s;
        nhdr->cap = len;
    }

    _slen(s) = len;
    return s;
}

static inline cstr _sens(cstr s, uint len)
{
    return s;
}

/// \brief __cstr_replace - replace str @param from in s to str @param to
///
/// \param s       : a _s type str, we assume it is valid
/// \param end     : a cstr pointer, point to a pointer who point to the end '\0' of @param s now
/// \param last    : a cstr pointer, point to a pointer who point to the end '\0' of @param s when finish replacing, this ptr is predicted before call this func
/// \param from    : the cstr you want to be replaced, we assume it is valid
/// \param fromlen : the length of @param from, it must < tolen
/// \param to      : the cstr you want to replace to, we assume it is valid
/// \param tolen   : the length of @param to, it must > fromlen
///
/// @note:
///     1. fromlen < tolen needed, or you will not get the correct result, and you do not need this func if fromlen <= tolen
///     2. s must have enough place to hold the whole str when after
///
/// from: aaa        fromlen: 3
/// to  : abcdefg    tolen  : 7
/// s   : 11111aaa111aaa11111111_____________________
///                             |       |-- last = end + (7 - 3)*2
///                             |---------- end
///
///
static inline void __cstr_replace(cstr s, cstr* end, cstr* last, constr from, size fromlen, constr to, size tolen)
{
    cstr fd;

    if((fd = strstr(s, from)))
    {
        cstr mv_from, mv_to; size mv_len;

        __cstr_replace(fd + 1, end, last, from, fromlen, to, tolen);

        mv_from = fd + fromlen;
        mv_len  = *end  - fd - fromlen;
        mv_to   = *last - mv_len;

        memmove(mv_to, mv_from, mv_len);

        *last = mv_to - tolen;
        memcpy(*last, to, tolen);

        *end = fd;
    }

    return ;
}

/// \brief _ssub - replace str @param from in s to str @param to
///
/// \param s    : a _s type str, we assume it is valid
/// \param from : the cstr you want to be replaced, we assume it is valid
/// \param to   : the cstr you want to replace to, we assume it is valid
/// \return : NULL - if replace faild, this will be happen only when the new buf is alloc faild
///           cstr - if replace ok, but the returned s maybe not the same as s
/// @note:
///     1. new str, note: we do not free the old s if replace faild
///
static inline cstr _ssub(cstr s, constr from, constr to)
{
    int subLen, newLen, offLen, offNow; cstr fd_s, cp_s, end_p;

    subLen = strlen(from);
    newLen = strlen(to);
    offLen = newLen - subLen;

    if(offLen < 0)
    {
        offLen = -offLen;
        offNow = 0;
        fd_s   = s;
        end_p  = s + _slen(s);

        if((fd_s = strstr(fd_s, from)))
        {
            memcpy(fd_s, to, newLen);     // replace it

            cp_s = (fd_s += subLen);        // record the pos of str need copy
            offNow += offLen;               // record the off of str need copy

            while((fd_s = strstr(fd_s, from)))
            {
                memmove(cp_s - offNow, cp_s, fd_s - cp_s);   // move the str-need-copy ahead

                memcpy(fd_s - offNow, to, newLen);
                cp_s = (fd_s += subLen);
                offNow += offLen;
            }

            memmove(cp_s - offNow, cp_s, end_p - cp_s);
            _slen(s) -= offNow;
            *(end_p - offNow) = '\0';
        }
    }
    else if(offLen == 0)
    {
        fd_s = strstr(s, from);

        while(fd_s)
        {
            memcpy(fd_s, to, newLen);
            fd_s += subLen;
            fd_s = strstr(fd_s, from);
        }
    }
    else
    {
        offNow = _slen(s);
        fd_s   = s;

        if((fd_s  = strstr(fd_s, from)))
        {
            end_p = s + offNow;

            // -- get len need to expand
            offNow += offLen; fd_s += subLen;
            while((fd_s = strstr(fd_s, from)))
            {
                offNow += offLen; fd_s += subLen;
            }

            // -- have enough place, let's do it, we set the up limit of stack call is 4096
            if((size)offNow <= _scap(s) && (offNow - (end_p - s)) / offLen <= 4096 )
            {
                cstr last = s + offNow;
                __cstr_replace(s, &end_p, &last, from, subLen, to, newLen);
                _slen(s) = offNow;
                s[offNow] = '\0';
                return s;
            }
            else
            {
                cstr new_s, new_p; int len;

                is0_ret(new_s = _snew2(0, offNow * 1.2), 0);

                // -- to new str
                cp_s  = fd_s = s;
                new_p = new_s;
                while((fd_s = strstr(fd_s, from)))
                {
                    memcpy(new_p, cp_s, (len = fd_s - cp_s)); new_p += len;
                    memcpy(new_p, to, newLen);                new_p += newLen;

                    cp_s = (fd_s += subLen);
                }

                memcpy(new_p, cp_s, end_p - cp_s);

                _sfree(s);
                _slen(new_s) = offNow;

                return new_s;
            }
        }
    }

    return s;
}


// --------------------- dict hash table ---------------------------
typedef struct dictht_s{
    ejson*  table;       // to save data
    ulong   size;
    ulong   sizemask;
    ulong   used;
}dictht_t, dictht;

#define dictht_reset(ht)   memset(ht, 0, sizeof(dictht_t));

// ------------------------------ dict -----------------------------
typedef struct dict_s {
//  void*    privdata;
    dictht_t ht[2];
    long     rehashidx;
    int      iterators;
}dict_t, * dict;

typedef struct dictIterator_s {
    dict  d;
    long  index;
    int   table, safe;
    ejson entry, nextEntry;
    long long fingerprint;
}dictIterator_t, * dictIterator;

typedef struct dictLink_s{
    ejson* _pos;
    ulong* _used;
}dictLink_t, *L;


// -- pre definitions
#define DICT_OK   0
#define DICT_ERR  1
#define DICT_HT_INITIAL_SIZE     4

static int  dict_can_resize = 1;
static uint dict_force_resize_ratio = 5;

// -- API
static dict  dict_new();
static void  dict_free(dict d);
static ejson dict_add (dict d, const void*k, int k_len, ejson obj);
static ejson dict_find(dict d, const void*k, int k_len);
static ejson dict_del (dict d, ejson del);
static int   dict_getL(dict d, const void* k, int k_len, L l);

#define dict_link(l, o) {(o)->next = *((l)._pos); *((l)._pos) = o; *((l)._used) += 1;}

// -- micros
#define dict_resetHt(d)    memset(d->ht, 0, sizeof(dictht_t) * 2);
#define dictIsRehashing(d) ((d)->rehashidx != -1)
#define dictSize(d)        ((d)->ht[0].used+(d)->ht[1].used)

// ---------------------------- ejson ------------------------

// -- ejson struct --
typedef struct __attribute__ ((__packed__)) ejson_s{
    obj_header_t h;      // head info
    union{
        cstr     s;      // str key if obj in object
        uint     i;		 // not used now
    }k;

    ejson        p, n;   // pre, next, used for double link
    void*        next;   // next node, used for EJSON_OBJ obj's dic

    union{
        s64      i;      // int   value of NUM
        double   f[2];   // float value of NUM, f[1] is the used value

        cstr     s;      // str   ptr   of STR
        void*    p;      // ptr   value of RAW/PTR, point to raw data alloced by _addR()/_addP()

        void*    arr;    // history info of ARR
        dict_t*  obj;    // children dic of OBJ

        struct{
        void* _;
        ejson h, t;      // head, tail link of first, last child for ARR and OBJ
        }        l;
    }v;
}ejson_t;

// -- pre definitions
typedef struct _inner_{
    obj_header_t h;
    char v[16];
}_inner_;
static _inner_ __NIL = {{0,0,1,0,0, 5}, "(nil)" };
static _inner_ __VER = {{0,0,1,0,0, sizeof(EJSON_VERSION)-1}, EJSON_VERSION};
static const cstr _NIL_ = __NIL.v;
static const cstr _VER_ = __VER.v;

// -- helpler --
typedef constr (*_lstrip)(constr str);
static inline constr lstrip1(constr str);   // not support comments
static inline constr lstrip2(constr str);   //     support comments
static ejson  parse_eval(cstr* _name, constr* _src, constr* _err, _lstrip lstrip);
static int    check_eval(             constr* _src, constr* _err, _lstrip lstrip);
static cstr   wrap_obj(ejson obj, int depth, int pretty, ejsw w);

// -- OBJ operations --
#define _obj(o)       (o)->v.obj
#define _objInit(o)   _obj(o) = dict_new()
#define _objFree(o)   if(_obj(o)) dict_free(_obj(o))
#define _objGet(o, k) (_obj(o) ? dict_find(_obj(o), k, strlen(k)) : 0)

static ejson _objAdd (ejson root, cstr  key, ejson obj);
static ejson _objPush(ejson root, cstr  key, ejson obj);
static void  _objLink(ejson root, ejson obj);
static ejson _objPop (ejson root);
static ejson _objPopT(ejson root);
static ejson _objRmO (ejson root, ejson obj);

// -- ARR operations --
#define _arr(o)       (o)->v.arr
#define _arrInit(o)   _arr(o) = calloc(3, sizeof(uint))
#define _arrFree(o)   free(_arr(o))
#define _arrGet(o, i) (i < _arrLen(o) ? _arrFind(o, i) : 0)

static ejson _arrAdd (ejson root, ejson obj);
static ejson _arrPush(ejson root, ejson obj);
static ejson _arrPop (ejson root);
static ejson _arrPopT(ejson root);
static ejson _arrFind(ejson root, uint  idx);
static ejson _arrRmI (ejson root, uint  idx);
static ejson _arrRmO (ejson root, ejson obj);

static ejson _objByKeys(ejson obj, constr keys_, int del, int raw);

// -- micros --
#define _newNil()     (ejson)calloc(1, OBJ_size + sizeof(void*)*4)                      // for FALSE, TRUE, NULL obj
#define _newStr()     (ejson)calloc(1, OBJ_size + sizeof(void*)*5)                      // for STR obj
#define _newNum()     (ejson)calloc(1, OBJ_size + sizeof(void*)*4 + sizeof(double)*2)   // for NUM obj
#define _newObj()     (ejson)calloc(1, OBJ_size + sizeof(void*)*7)                      // for ARR OBJ obj
#define _freeObj(o)   free((o))

#define _newS1(s)     _snew1(s)
#define _newS2(p, l)  _snew2(p, l)
#define _freeS(s)     _sfree(s)
#define _lenS(s)      _slen(s)
#define _capS(s)      _scap(s)
#define _wrbS(s,p,l)  _swrb(s, p, l)
#define _relS(s,l)    _srel(s, l)
#define _subS(s,f,t)  _ssub(s,f,t)

#define _TYPE(o)      (o)->h.type
#define _setFALSE(o)  (o)->h.type = EJSON_FALSE
#define _setTRUE(o)   (o)->h.type = EJSON_TRUE
#define _setNULL(o)   (o)->h.type = EJSON_NULL
#define _setNUM(o)    (o)->h.type = EJSON_NUM
#define _setSTR(o)    (o)->h.type = EJSON_STR
#define _setPTR(o)    (o)->h.type = EJSON_PTR
#define _setRAW(o)    (o)->h.type = EJSON_RAW
#define _setARR(o)    (o)->h.type = EJSON_ARR
#define _setOBJ(o)    (o)->h.type = EJSON_OBJ

#define _isFALSE(o)   ((o) && (o)->h.type == EJSON_FALSE)
#define _isTRUE(o)    ((o) && (o)->h.type == EJSON_TRUE )
#define _isNULL(o)    ((o) && (o)->h.type == EJSON_NULL )
#define _isNUM(o)     ((o) && (o)->h.type == EJSON_NUM  )
#define _isSTR(o)     ((o) && (o)->h.type == EJSON_STR  )
#define _isRAW(o)     ((o) && (o)->h.type == EJSON_RAW  )
#define _isPTR(o)     ((o) && (o)->h.type == EJSON_PTR  )
#define _isARR(o)     ((o) && (o)->h.type == EJSON_ARR  )
#define _isOBJ(o)     ((o) && (o)->h.type == EJSON_OBJ  )

#define _isParent(o)  ((o) && ((o)->h.type > EJSON_RAW))
#define _isChild(o)   (o)->h.is_array

#define _keyS(o)      (o)->k.s
#define _valI(o)      (o)->v.i
#define _valF(o)      (o)->v.f[1]
#define _valS(o)      (o)->v.s
#define _valR(o)      (o)->v.p
#define _valP(o)      (o)->v.p

#define _objLen(o)    (o)->h._len
#define _arrLen(o)    (o)->h._len
#define _objPrev(o)   (o)->p
#define _objNext(o)   (o)->n
#define _objHead(o)   (o)->v.l.h
#define _objTail(o)   (o)->v.l.t

#define _getObjByKeys(obj, keys) _objByKeys(obj, keys, 0, 0)
#define _getObjByRawk(obj, keys) _objByKeys(obj, keys, 0, 1)
#define _rmObjByKeys( obj, keys) _objByKeys(obj, keys, 1, 0)
#define _rmObjByRawk( obj, keys) _objByKeys(obj, keys, 1, 1)

#define errset(err)  g_err = err
#define errfmt(...)  {snprintf(g_err_buf, 1024, ##__VA_ARGS__);g_err = g_err_buf;}

// -- ejson API definition --
ejson ejso_new(type type)
{
    ejson out;
    switch (type) {
        case EJSON_FALSE: is0_exeret(out = _newNil(), errset("alloc err"), 0); _setFALSE(out); break;
        case EJSON_TRUE : is0_exeret(out = _newNil(), errset("alloc err"), 0); _setTRUE (out); break;
        case EJSON_NULL : is0_exeret(out = _newNil(), errset("alloc err"), 0); _setNULL (out); break;
        case EJSON_NUM  : is0_exeret(out = _newNum(), errset("alloc err"), 0); _setNUM  (out); break;
        case EJSON_STR  : is0_exeret(out = _newStr(), errset("alloc err"), 0); _setSTR  (out); break;
        case EJSON_PTR  : is0_exeret(out = _newStr(), errset("alloc err"), 0); _setPTR  (out); break;
        case EJSON_RAW  : is0_exeret(out = _newStr(), errset("alloc err"), 0); _setRAW  (out); break;
        case EJSON_ARR  : is0_exeret(out = _newObj(), errset("alloc err"), 0); _setARR  (out); break;
        case EJSON_OBJ  : is0_exeret(out = _newObj(), errset("alloc err"), 0); _setOBJ  (out); break;
        default         : out = 0;  errset("invalid type");  break;
    }
    return out;
}

static _lstrip e_lstrip = lstrip1;
static _lstrip f_lstrip = lstrip2;
static int     e_check  = 0;
static int     f_check  = 0;
static ejson parse_STR(cstr* _name, constr* _src, constr* _err, _lstrip lstrip);
inline ejson ejss_eval(constr src)
{
    ejson obj; cstr key; _lstrip lstrip;

    is0_exeret(src, g_errp = _NIL_, 0);

    lstrip = e_lstrip;  src = lstrip(src);
    if(*src == '\"')
    {
        key = NULL;
        parse_STR(&key, &src, &g_errp, lstrip);
        is0_ret(key, 0);
        if(*src == ':')
        {
            src = lstrip(src+1);
            obj = parse_eval(&key, &src, &g_errp, lstrip);
        }
        else
        {
            if((obj = _newStr()))
            {
                _setSTR(obj);
                _valS(obj) = key;
            }
            else _sfree(key);
        }
    }
    else
        obj = parse_eval(0, &src, &g_errp, lstrip);

    if (e_check && obj) {src=lstrip(src);if (*src) {ejso_free(obj);g_errp=src;return 0;}}

    return obj;
}

ejson ejss_evalOpts(constr src, constr* err_pos, opts opt)
{
    _lstrip lstrip; ejson obj; cstr key; constr* err;

    is0_exeret(src, g_errp = _NIL_, 0);

    err    = err_pos     ? err_pos : &g_errp;
    lstrip = opt&CMMT_ON ? lstrip2 : lstrip1;

    src = lstrip(src);
    if(*src == '\"')
    {
        key = NULL;
        parse_STR(&key, &src, &g_errp, lstrip);
        if(*src == ':')
        {
            src = lstrip(src+1);
            obj = parse_eval(&key, &src, err, lstrip);
        }
        else
        {
            if((obj = _newStr()))
            {
                _setSTR(obj);
                _valS(obj) = key;
            }
            else _sfree(key);
        }
    }
    else
        obj = parse_eval(0, &src, err, lstrip);

    // -- check for a null terminator
    if (obj && opt&ENDCHK_ON) {src=lstrip(src);if (*src) {ejso_free(obj);*err=src;return 0;}}

    return obj;
}

int ejss_check(constr src)
{
    _lstrip lstrip; int ret;

    is0_exeret(src, g_errp = _NIL_, 0);

    lstrip = e_lstrip;
    src    = lstrip(src);

    is1_exeret(*src != '{', g_errp = src, 0);
    ret = check_eval(&src, &g_errp, lstrip);

    if(e_check && ret) {src=lstrip(src);if (*src) {g_errp=src;return 0;}}

    return ret;
}

int ejss_checkOpts(constr src, constr* err_pos, opts opt)
{
    _lstrip lstrip; constr* err; int ret;

    is0_exeret(src, g_errp = _NIL_, 0);

    err    = err_pos     ? err_pos : &g_errp;
    lstrip = opt&CMMT_ON ? lstrip2 : lstrip1;

    src = lstrip(src);
    is1_exeret(*src != '{', g_errp = src, 0);
    ret = check_eval(&src, err, lstrip);

    if(ret && opt&ENDCHK_ON) {src=lstrip(src);if (*src) {*err=src;return 0;}}

    return ret;
}

ejson  ejsf_eval(constr file)
{
    static char _errp_buf[512];
    _lstrip lstrip; int fd; long len; char* data; constr src; ejson obj;

    is0_exeret(file, g_errp = _NIL_, 0);

    is1_exeret((fd = open(file, O_RDONLY)) == -1, g_errp = 0; errfmt("ejsf_eval err: %s", strerror(errno));, 0);

    len=lseek(fd, 0L, SEEK_END); lseek(fd, 0L, SEEK_SET);

    is0_exeret(len,                close(fd); g_errp = 0; errset("ejsf_eval err: empty file");, 0);
    is0_exeret(data=malloc(len+1), close(fd); g_errp = 0; errset("ejsf_eval err: alloc failed");, 0);

    read(fd, data, len);data[len]='\0'; close(fd);
    lstrip = f_lstrip;

    src = lstrip(data);
    is1_exeret(*src != '{', memccpy(_errp_buf, src, 0, 511);g_errp = _errp_buf;free(data);, 0);
    if(!(obj = parse_eval(0, &src, &g_errp, lstrip))){                                 memccpy(_errp_buf, g_errp, '\0', 511);g_errp = _errp_buf;}
    if(f_check && obj) {src=lstrip(src);if (*src)    {ejso_free(obj);obj=0;g_errp=src; memccpy(_errp_buf, g_errp, '\0', 511);g_errp = _errp_buf;}}

    free(data);
    return obj;
}

ejson  ejsf_evalOpts(constr file, constr* err_pos, opts opt)
{
    static char _errp_buf[512];
    _lstrip lstrip; int fd; long len;  char* data; constr src; ejson obj; constr* err;

    is0_exeret(file, g_errp = _NIL_, 0);

    is1_exeret((fd = open(file, O_RDONLY)) == -1, g_errp = 0; errfmt("ejsf_evalOpts err: %s", strerror(errno));, 0);

    len=lseek(fd, 0L, SEEK_END); lseek(fd, 0L, SEEK_SET);

    is0_exeret(len,                close(fd); g_errp = 0; errset("ejsf_evalOpts err: empty file");, 0);
    is0_exeret(data=malloc(len+1), close(fd); g_errp = 0; errset("ejsf_evalOpts err: alloc failed");, 0);

    read(fd, data, len);data[len]='\0'; close(fd);
    err    = err_pos      ? err_pos : &g_errp;
    lstrip = opt&CMMT_ON  ? lstrip2 : lstrip1;

    src = (char*)lstrip(data);
    is1_exeret(*src != '{', memccpy(_errp_buf, src, 0, 511);g_errp = _errp_buf;free(data);, 0);
    if(!(obj = parse_eval(0, &src, err, lstrip)))      {                                 memccpy(_errp_buf, *err, '\0', 511);g_errp = _errp_buf;}
    if(obj && opt&ENDCHK_ON) {src=lstrip(src);if (*src){ejso_free(obj);obj=0;g_errp=src; memccpy(_errp_buf, *err, '\0', 511);g_errp = _errp_buf;}}

    free(data);
    return obj;
}

inline void   ejse_set(opts opt) {   e_lstrip = opt&CMMT_ON ? lstrip2 : lstrip1; e_check = opt&ENDCHK_ON ? 1 : 0;}
inline void   ejsf_set(opts opt) {   f_lstrip = opt&CMMT_ON ? lstrip2 : lstrip1; f_check = opt&ENDCHK_ON ? 1 : 0;}

inline constr ejse_str()    {   return g_err;    }
inline constr ejse_pos()    {   return g_errp;   }

/// ------------------------  macro err ---------------------------
#define _validS(s)      ((s) && *(s))

#define _invalidS(s)    (!(s) || !*(s))
#define _invalidO(o)    (!(o) || !_keyS(o))
#define _canotAdd(r, o) (!(o) || (r)==(o) || _isChild(o))
#define _canotRm(o)     (!(o) || !_isChild(o))              // todo: should not need this operation

#define _invalidSErr " is NULL or EMPTY"
#define _isOBJErr    " is not a OBJ obj"
#define _isParentErr " is not a ARR/OBJ obj"
#define _canotAddErr " is NULL or is the root obj self or is a child obj of other obj"
#define _canotRmErr  " is NULL or is not a child obj"

#define _checkOBJ(o)        is0_exeret(_isOBJ(o)     , errset("invalid" #o " (not a OBJ object)"), 0)
#define _checkNULL(o)       is0_exeret(o             , errset("invalid" #o " (nullptr)"), 0)
#define _checkZERO(i)       is0_exeret(i             , errset("invalid" #i " (= 0)"), 0)
#define _checkSRC(s)        is1_exeret(_invalidS(s)  , errset("invalid" #s " (nullptr or empty)");g_errp = _NIL_, 0)
#define _checkParent(o)     is0_exeret(_isParent(o)  , errset("invalid" #o " (nullptr or not in ARR/OBJ type)"), 0)
#define _checkInvldS(s)     is1_exeret(_invalidS(s)  , errset("invalid" #s " (nullptr or empty)"), 0)
#define _checkCanAdd(r,o)   is1_exeret(_canotAdd(r,o), errset("invalid" #o " (nullptr or is the root obj self or is in another ejson already)"), 0)

#define _ERRSTR_NOKEY       "found no key in param and obj"
#define _ERRSTR_PARSE       "src parsing error"
#define _ERRSTR_TYPEDF      "invalid type (root obj is not in OBJ/ARR type)"

#define _ERRSTR_ALLOC(tag)      "alloc faild for" #tag
#define _ERRSTR_KEYINPRM(key)   "key \"%s\" in param is already exist in root obj", key
#define _ERRSTR_KEYINOBJ(key)   "key \"%s\" in obj is already exist in root obj", key

static inline ejson __ejso_addObj(ejson root, constr key, ejson  obj)
{
    cstr nk, hk; uint len; dictLink_t l;

    switch (_TYPE(root)) {
        case EJSON_OBJ: if(!_obj(root)) is0_exeret(_objInit(root), errset(_ERRSTR_ALLOC(dict));, 0);

                        hk = _keyS(obj);
                        if(_validS(key))            // have a key in param
                        {
                            len = strlen(key);
                            is0_exeret(dict_getL(_obj(root), key, len, &l), errfmt(_ERRSTR_KEYINPRM(key));, 0);
                            is0_exeret(nk = _wrbS(hk, key, len)           , errset(_ERRSTR_ALLOC(key));   , 0);

                            if(nk != hk) _keyS(obj) = nk;
                            dict_link(l, obj);_objLink(root, obj);
                        }
                        else
                        {
                            is0_exeret(hk,                     errset(_ERRSTR_NOKEY)        , 0);
                            is0_exeret(_objAdd(root, hk, obj), errfmt(_ERRSTR_KEYINOBJ(hk));, 0);
                        }
                        break;
        case EJSON_ARR: if(!_arr(root)) is0_exeret(_arrInit(root), errset(_ERRSTR_ALLOC(list));, 0);
                        _arrAdd(root, obj);
                        break;
        default       : errset(_ERRSTR_TYPEDF); return 0;
    }

    return obj;
}

static inline ejson __ejso_addEval(ejson root, constr key, constr src)
{
    cstr nk, hk; ejson obj; uint len; _lstrip lstrip; dictLink_t l;


    hk = NULL;  lstrip = e_lstrip;  src = lstrip(src);

    if(*src == '\"')
        parse_STR(&hk, &src, &g_errp, lstrip);

    switch (_TYPE(root)) {
        case EJSON_OBJ: if(!_obj(root)) is0_exeret(_objInit(root), errset(_ERRSTR_ALLOC(dict));, 0);
                        if(!hk)                     // have no key in src
                        {
                            is1_exeret(_invalidS(key)                            , errset(_ERRSTR_NOKEY);            , 0); len = strlen(key);
                            is0_exeret(dict_getL(_obj(root), key, len, &l)       , errfmt(_ERRSTR_KEYINPRM(key));    , 0);
                            is0_exeret(nk  = _newS2(key, len)                    , errset(_ERRSTR_ALLOC(key));       , 0);
                            is0_exeret(obj = parse_eval(0, &src, &g_errp, lstrip), errset(_ERRSTR_PARSE); _freeS(nk);, 0);
                            dict_link(l, obj); _objLink(root, obj); _keyS(obj) = nk;
                        }
                        else
                        {
                            if(*src != ':')         // have no key in src, hk is a str val
                            {
                                if (e_check) {if (*src) {_freeS(hk);g_errp=src;return 0;}}
                                is1_exeret(_invalidS(key),                      errset(_ERRSTR_NOKEY)         ;_sfree(hk);           , 0);len = strlen(key);
                                is0_exeret(dict_getL(_obj(root), key, len, &l), errfmt(_ERRSTR_KEYINPRM(key)) ;_sfree(hk);           , 0);
                                is0_exeret(nk  = _newS2(key, len),              errset(_ERRSTR_ALLOC(key))    ;_sfree(hk);           , 0);
                                is0_exeret(obj = _newStr(),                     errset(_ERRSTR_ALLOC(STR obj));_sfree(nk);_freeS(hk);, 0);
                                dict_link(l, obj); _objLink(root, obj); _keyS(obj) = nk; _valS(obj) = hk; _setSTR(obj);
                            }
                            else if(!_invalidS(key))  // hk is a key in src, and has a key in param
                            {
                                src = lstrip(src + 1);
                                len = strlen(key);
                                is0_exeret(dict_getL(_obj(root), key, len, &l), errfmt(_ERRSTR_KEYINPRM(key));_freeS(hk);, 0);
                                is0_exeret(nk = _wrbS(hk, key, len)           , errset(_ERRSTR_ALLOC(key))   ;_freeS(hk);, 0);
                                is0_exeret(obj = parse_eval(0, &src, &g_errp, lstrip), errset(_ERRSTR_PARSE) ;_freeS(hk);, 0);
                                if (e_check) {if (*src) {_freeS(hk); ejso_free(obj); g_errp=src;return 0;}}
                                dict_link(l, obj); _objLink(root, obj); _keyS(obj) = nk;
                            }
                            else                    // hk is a key in src, and has no key in param
                            {
                                src = lstrip(src + 1);
                                is0_exeret(dict_getL(_obj(root), hk, _lenS(hk), &l)  , errfmt(_ERRSTR_KEYINOBJ(hk));_freeS(hk);, 0);
                                is0_exeret(obj = parse_eval(0, &src, &g_errp, lstrip), errset(_ERRSTR_PARSE)       ;_freeS(hk);, 0);
                                dict_link(l, obj); _objLink(root, obj); _keyS(obj) = hk;
                            }
                        }
                        break;
        case EJSON_ARR: if(!_arr(root)) is0_exeret(_arrInit(root), errset(_ERRSTR_ALLOC(list)); if(hk) _freeS(hk);, 0);
                        if(!hk)                 // have no key in src
                        {
                            is0_exeret(obj = parse_eval(0, &src, &g_errp, lstrip), errset(_ERRSTR_PARSE);, 0);
                        }
                        else
                        {
                            if(*src != ':')     // have no key in src, hk is a str val
                            {
                                if (e_check) {if (*src) {_freeS(hk);g_errp=src;return 0;}}
                                is0_exeret(obj = _newStr(), errset(_ERRSTR_ALLOC(obj));_freeS(hk);, 0);
                                _valS(obj) = hk; _setSTR(obj);
                            }
                            else                // hk is a key in src, we do not check key in param when root is a ARR
                            {
                                src = lstrip(src + 1);
                                is0_exeret(obj = parse_eval(&hk, &src, &g_errp, lstrip), errset(_ERRSTR_PARSE); _freeS(hk);, 0);
                            }
                        }
                        _arrAdd(root, obj);
                        break;
        default       : errset(_ERRSTR_TYPEDF); if(hk) _freeS(hk); return 0;
    }

    return obj;
}

static inline ejson __ejso_addType(ejson root, constr key, int type)
{
    ejson obj; int len; cstr nk; dictLink_t l;

    switch (_TYPE(root)) {
        case EJSON_OBJ: _checkInvldS(key); len = strlen(key);
                        if(!_obj(root)) is0_exeret(_objInit(root)     , errset(_ERRSTR_ALLOC(dict));  , 0);
                        is0_exeret(dict_getL(_obj(root), key, len, &l), errfmt(_ERRSTR_KEYINPRM(key));, 0);
                        switch (type)   {case EJSON_FALSE:case EJSON_TRUE:case EJSON_NULL: obj = _newNil();break; case EJSON_ARR:case EJSON_OBJ: obj = _newObj();break; default: errset("ejso_addT err: not supported type");return 0;}
                        is0_exeret(obj,                  errset(_ERRSTR_ALLOC(NIL obj));              , 0);
                        is0_exeret(nk = _newS2(key,len), errset(_ERRSTR_ALLOC(key));ejso_free(obj);   , 0);
                        dict_link(l, obj);_objLink(root, obj); _keyS(obj) = nk; _TYPE(obj) = type;
                        break;
        case EJSON_ARR: if(!_arr(root)) is0_exeret(_arrInit(root), errset(_ERRSTR_ALLOC(list));, 0);
                        switch (type)   {case EJSON_FALSE:case EJSON_TRUE:case EJSON_NULL: obj = _newNil();break; case EJSON_ARR:case EJSON_OBJ: obj = _newObj();break; default: errset("ejso_addT err: not supported type");return 0;}
                        is0_exeret(obj,                  errset(_ERRSTR_ALLOC(obj));           , 0);
                        _arrAdd(root, obj); _TYPE(obj) = type;
                        break;
        default       : errset(_ERRSTR_TYPEDF); return 0;
    }

    return obj;
}

static inline ejson __ejso_addStr(ejson root, constr key, constr str)
{
    ejson obj; int len; cstr nk, nv; dictLink_t l;

    switch (_TYPE(root)) {
        case EJSON_OBJ: _checkInvldS(key); len = strlen(key);
                        if(!_obj(root)) is0_exeret(_objInit(root)      , errset(_ERRSTR_ALLOC(dict))   ;                         , 0);
                        is0_exeret(dict_getL(_obj(root), key, len, &l) , errfmt(_ERRSTR_KEYINPRM(key)) ;                         , 0);
                        is0_exeret(obj = _newStr()                     , errset(_ERRSTR_ALLOC(STR obj));                         , 0);
                        is0_exeret(nk  = _newS2(key, len)              , errset(_ERRSTR_ALLOC(key))    ;_freeObj(obj);           , 0);
                        is0_exeret(nv  = _newS1(str)                   , errset(_ERRSTR_ALLOC(str))    ;_freeObj(obj);_freeS(nk);, 0);
                        dict_link(l, obj);_objLink(root, obj); _setSTR(obj); _keyS(obj) = nk; _valS(obj) = nv;
                        break;
        case EJSON_ARR: if(!_arr(root)) is0_exeret(_arrInit(root)      , errset(_ERRSTR_ALLOC(list))   ;              , 0);
                        is0_exeret(obj = _newStr()                     , errset(_ERRSTR_ALLOC(STR obj));              , 0);
                        is0_exeret(nv  = _newS1(str)                   , errset(_ERRSTR_ALLOC(str))    ;_freeObj(obj);, 0);
                        _arrAdd(root, obj); _setSTR(obj); _valS(obj) = nv;
                        break;
        default       : errset(_ERRSTR_TYPEDF); return 0;
    }

    return obj;
}

static inline ejson __ejso_addNum(ejson root, constr key, double val)
{
    ejson obj; int len; cstr nk; dictLink_t l;

    switch (_TYPE(root)) {
        case EJSON_OBJ: _checkInvldS(key); len = strlen(key);
                        if(!_obj(root)) is0_exeret(_objInit(root)      , errset(_ERRSTR_ALLOC(dict))   ;              , 0);
                        is0_exeret(dict_getL(_obj(root), key, len, &l) , errfmt(_ERRSTR_KEYINPRM(key)) ;              , 0);
                        is0_exeret(obj = _newNum()                     , errset(_ERRSTR_ALLOC(NUM obj));              , 0);
                        is0_exeret(nk  = _newS2(key, len)              , errset(_ERRSTR_ALLOC(key))    ;_freeObj(obj);, 0);
                        dict_link(l, obj); _objLink(root, obj); _setNUM(obj); _keyS(obj) = nk; _valI(obj) = (s64)(_valF(obj) = val);
                        break;
        case EJSON_ARR: if(!_arr(root)) is0_exeret(_arrInit(root)      , errset(_ERRSTR_ALLOC(list));   , 0);
                        is0_exeret(obj = _newNum()                     , errset(_ERRSTR_ALLOC(NUM obj));, 0);
                        _arrAdd(root, obj); _setNUM(obj); _valI(obj) = (s64)(_valF(obj) = val);
                        break;
        default       : errset(_ERRSTR_TYPEDF); return 0;
    }

    return obj;
}

static inline ejson __ejso_addPtr(ejson root, constr key, void*  ptr)
{
    ejson obj; int len; cstr nk; dictLink_t l;

    switch (_TYPE(root)) {
        case EJSON_OBJ: _checkInvldS(key); len = strlen(key);
                        if(!_obj(root)) is0_exeret(_objInit(root)      , errset(_ERRSTR_ALLOC(dict))   ;              , 0);
                        is0_exeret(dict_getL(_obj(root), key, len, &l) , errfmt(_ERRSTR_KEYINPRM(key)) ;              , 0);
                        is0_exeret(obj = _newStr()                     , errset(_ERRSTR_ALLOC(PTR obj));              , 0);
                        is0_exeret(nk  = _newS2(key, len)              , errset(_ERRSTR_ALLOC(key))    ;_freeObj(obj);, 0);
                        dict_link(l, obj); _objLink(root, obj); _setPTR(obj); _keyS(obj) = nk; _valP(obj) = ptr;
                        break;
        case EJSON_ARR: if(!_arr(root)) is0_exeret(_arrInit(root)      , errset(_ERRSTR_ALLOC(list));   , 0);
                        is0_exeret(obj = _newStr()                     , errset(_ERRSTR_ALLOC(PTR obj));, 0);
                        _arrAdd(root, obj); _setPTR(obj); _valP(obj) = ptr;
                        break;
        default       : errset(_ERRSTR_TYPEDF); return 0;
    }

    return obj;
}

static inline ejson __ejso_addRaw(ejson root, constr key, int _len)
{
    ejson obj; int len; cstr nk; void* nv; dictLink_t l;

    switch (_TYPE(root)) {
        case EJSON_OBJ: _checkInvldS(key); len = strlen(key);
                        if(!_obj(root)) is0_exeret(_objInit(root)      , errset(_ERRSTR_ALLOC(dict))   ;                         , 0);
                        is0_exeret(dict_getL(_obj(root), key, len, &l) , errfmt(_ERRSTR_KEYINPRM(key)) ;                         , 0);
                        is0_exeret(obj = _newStr()                     , errset(_ERRSTR_ALLOC(RAW obj));                         , 0);
                        is0_exeret(nk  = _newS2(key, len)              , errset(_ERRSTR_ALLOC(key))    ;_freeObj(obj);           , 0);
                        is0_exeret(nv  = _newS2(0,  _len)              , errset(_ERRSTR_ALLOC(raw))    ;_freeObj(obj);_freeS(nk);, 0);
                        dict_link(l, obj); _objLink(root, obj); _setRAW(obj); _keyS(obj) = nk;
                        return _valR(obj) = nv;
        case EJSON_ARR: if(!_arr(root)) is0_exeret(_arrInit(root)      , errset(_ERRSTR_ALLOC(list))   ;              , 0);
                        is0_exeret(obj = _newStr()                     , errset(_ERRSTR_ALLOC(RAW obj));              , 0);
                        is0_exeret(nv  = _newS2(0,  _len)              , errset(_ERRSTR_ALLOC(raw))    ;_freeObj(obj);, 0);
                        _arrAdd(root, obj); _setRAW(obj);
                        return _valR(obj) = nv;
        default       : errset(_ERRSTR_TYPEDF); return 0;
    }

    return 0;
}

ejson  ejso_addO(ejson root, constr key, ejson  obj) { _checkNULL(root); _checkCanAdd(root, obj); return __ejso_addObj (root, key, obj); }
ejson  ejso_addE(ejson root, constr key, constr src) { _checkNULL(root); _checkSRC(src);          return __ejso_addEval(root, key, src); }
ejson  ejso_addT(ejson root, constr key, int   type) { _checkNULL(root);                          return __ejso_addType(root, key,type); }
ejson  ejso_addS(ejson root, constr key, constr val) { _checkNULL(root); _checkNULL(val);         return __ejso_addStr (root, key, val); }
ejson  ejso_addN(ejson root, constr key, double val) { _checkNULL(root);                          return __ejso_addNum (root, key, val); }
ejson  ejso_addP(ejson root, constr key, void*  ptr) { _checkNULL(root);                          return __ejso_addStr (root, key, ptr); }
void*  ejso_addR(ejson root, constr key, int    len) { _checkNULL(root); _checkZERO(len);         return __ejso_addRaw (root, key, len); }

ejson  ejsk_addO(ejson root, constr keys, constr key, ejson  obj) { _checkParent(root); _checkInvldS(keys); _checkCanAdd(root, obj); root = _getObjByKeys(root, keys); _checkParent(root); return __ejso_addObj (root, key, obj); }
ejson  ejsk_addE(ejson root, constr keys, constr key, constr src) { _checkParent(root); _checkInvldS(keys); _checkSRC(src);          root = _getObjByKeys(root, keys); _checkParent(root); return __ejso_addEval(root, key, src); }
ejson  ejsk_addT(ejson root, constr keys, constr key, int   type) { _checkParent(root); _checkInvldS(keys);                          root = _getObjByKeys(root, keys); _checkParent(root); return __ejso_addType(root, key,type); }
ejson  ejsk_addS(ejson root, constr keys, constr key, constr val) { _checkParent(root); _checkInvldS(keys); _checkNULL(val);         root = _getObjByKeys(root, keys); _checkParent(root); return __ejso_addStr (root, key, val); }
ejson  ejsk_addN(ejson root, constr keys, constr key, double val) { _checkParent(root); _checkInvldS(keys);                          root = _getObjByKeys(root, keys); _checkParent(root); return __ejso_addNum (root, key, val); }
ejson  ejsk_addP(ejson root, constr keys, constr key, void*  ptr) { _checkParent(root); _checkInvldS(keys);                          root = _getObjByKeys(root, keys); _checkParent(root); return __ejso_addPtr (root, key, ptr); }
void*  ejsk_addR(ejson root, constr keys, constr key, int    len) { _checkParent(root); _checkInvldS(keys); _checkZERO(len);         root = _getObjByKeys(root, keys); _checkParent(root); return __ejso_addRaw (root, key, len); }

ejson  ejsr_addO(ejson root, constr rawk, constr key, ejson  obj) { _checkOBJ(root);    _checkInvldS(rawk); _checkCanAdd(root, obj); root = _getObjByRawk(root, rawk); _checkParent(root); return __ejso_addObj (root, key, obj); }
ejson  ejsr_addE(ejson root, constr rawk, constr key, constr src) { _checkOBJ(root);    _checkInvldS(rawk); _checkSRC(src);          root = _getObjByRawk(root, rawk); _checkParent(root); return __ejso_addEval(root, key, src); }
ejson  ejsr_addT(ejson root, constr rawk, constr key, int   type) { _checkOBJ(root);    _checkInvldS(rawk);                          root = _getObjByRawk(root, rawk); _checkParent(root); return __ejso_addType(root, key,type); }
ejson  ejsr_addS(ejson root, constr rawk, constr key, constr val) { _checkOBJ(root);    _checkInvldS(rawk); _checkNULL(val);         root = _getObjByRawk(root, rawk); _checkParent(root); return __ejso_addStr (root, key, val); }
ejson  ejsr_addN(ejson root, constr rawk, constr key, double val) { _checkOBJ(root);    _checkInvldS(rawk);                          root = _getObjByRawk(root, rawk); _checkParent(root); return __ejso_addNum (root, key, val); }
ejson  ejsr_addP(ejson root, constr rawk, constr key, void*  ptr) { _checkOBJ(root);    _checkInvldS(rawk);                          root = _getObjByRawk(root, rawk); _checkParent(root); return __ejso_addPtr (root, key, ptr); }
void*  ejsr_addR(ejson root, constr rawk, constr key, int    len) { _checkOBJ(root);    _checkInvldS(rawk); _checkZERO(len);         root = _getObjByRawk(root, rawk); _checkParent(root); return __ejso_addRaw (root, key, len); }

void   ejso_free(ejson obj)
{
    ejson itr;

    is1_ret(!obj || _isChild(obj), );

    do{
        switch (_TYPE(obj)) {
            case EJSON_RAW: _freeS(_valR(obj)); break;
            case EJSON_STR: _freeS(_valS(obj)); break;
            case EJSON_ARR: if(_objTail(obj)) {_objNext(_objTail(obj)) = _objNext(obj);_objNext(obj)= _objHead(obj);} _arrFree(obj);break;
            case EJSON_OBJ: if(_objTail(obj)) {_objNext(_objTail(obj)) = _objNext(obj);_objNext(obj)= _objHead(obj);} _objFree(obj);break;
        }
        if(_keyS(obj)) _freeS(_keyS(obj));
        obj = _objNext(itr = obj);
        _freeObj(itr);
    }while(obj);

}

void   ejsk_free (ejson root, constr keys)
{
    is0_exeret(_isParent(root), errset("ejso_freeK err: root obj" _isParentErr);, );
    is1_exeret(_invalidS(keys), errset("ejso_freeK err: keys"     _invalidSErr);, );

    ejso_free(_rmObjByKeys(root, keys));
}
void   ejsr_free (ejson root, constr rawk)
{
    is0_exeret(_isOBJ(root),    errset("ejso_freeR err: root obj" _isOBJErr   );, );
    is1_exeret(_invalidS(rawk), errset("ejso_freeR err: keys"     _invalidSErr);, );

    ejso_free(_rmObjByRawk(root, rawk));
}

void ejso_freeO(ejson root, ejson  obj)
{
    is0_exeret(root, errset("ejso_freeO err: root obj is NULL"), );
    is0_exeret(obj , errset("ejso_freeO err: free obj is NULL"), );

    switch (_TYPE(root)) {
        case EJSON_OBJ: is0_exeret(_keyS(obj)        , errset("ejso_freeO err: free obj have no key" );, );
                        is0_exeret(_objRmO(root, obj), errset("ejso_freeO err: free obj is not in the root OBJ"), );
                        break;
        case EJSON_ARR: is0_exeret(_arrRmO(root, obj), errset("ejso_freeO err: free obj is not in the root ARR"), );
                        break;
        default       : errset("ejso_freeO err: root obj" _isParentErr);
    }

    ejso_free(obj);
}

inline void ejso_freeK(ejson root, constr keys)
{
    is0_exeret(_isParent(root), errset("ejso_freeK err: root obj" _isParentErr);, );
    is1_exeret(_invalidS(keys), errset("ejso_freeK err: keys"     _invalidSErr);, );

    ejso_free(_rmObjByKeys(root, keys));
}

inline void ejso_freeR(ejson root, constr rawk)
{
    is0_exeret(_isParent(root), errset("ejso_freeR err: root obj" _isParentErr);, );
    is1_exeret(_invalidS(rawk), errset("ejso_freeR err: keys"     _invalidSErr);, );

    ejso_free(_rmObjByRawk(root, rawk));
}

ejson   ejso_rmO(ejson root, ejson obj)
{
    is0_exeret(root, errset("ejso_rmv err: root obj is NULL"), 0);
    is0_exeret(obj , errset("ejso_rmO err: rm obj is NULL"), 0);

    switch (_TYPE(root)) {
        case EJSON_OBJ: is0_exeret(_keyS(obj)        , errset("ejso_rmO err: rm obj have no key" );, 0);
                        is0_exeret(_objRmO(root, obj), errset("ejso_rmO err: rm obj is not in the root OBJ"), 0);
                        break;
        case EJSON_ARR: is0_exeret(_arrRmO(root, obj), errset("ejso_rmO err: rm obj is not in the root ARR"), 0);
                        break;
        default       : errset("ejso_rmO err: root obj" _isParentErr); return 0;
    }

    return obj;
}

ejson  ejso_rmK(ejson root, constr keys)
{
    is0_exeret(_isParent(root), errset("ejso_rmK err: root obj" _isParentErr);, 0);
    is1_exeret(_invalidS(keys), errset("ejso_rmK err: keys"     _invalidSErr);, 0);

    return _rmObjByKeys(root, keys);
}

ejson  ejso_rmR(ejson root, constr rawk)
{
    is0_exeret(_isParent(root), errset("ejso_rmR err: root obj" _isParentErr);, 0);
    is1_exeret(_invalidS(rawk), errset("ejso_rmR err: rawk"     _invalidSErr);, 0);

    return _rmObjByRawk(root, rawk);
}

ejson  ejsk_rmO(ejson root, constr keys, ejson obj)
{
    is0_exeret(_isParent(root), errset("ejsk_rmO err: root obj" _isParentErr);, 0);
    is1_exeret(_invalidS(keys), errset("ejsk_rmO err: keys"     _invalidSErr);, 0);
    is1_exeret(_canotRm(obj)  , errset("ejsk_rmO err: rm obj "  _canotRmErr );, 0);

    root = _getObjByKeys(root, keys);
    switch (_TYPE(root)) {
        case EJSON_OBJ: is0_exeret(_keyS(obj)        , errset("ejsk_rmO err: rm obj have no key" );, 0);
                        is0_exeret(_objRmO(root, obj), errset("ejsk_rmO err: rm obj is not in the root OBJ"), 0);
                        break;
        case EJSON_ARR: is0_exeret(_arrRmO(root, obj), errset("ejsk_rmO err: rm obj is not in the root ARR"), 0);
                        break;
        default       : errfmt("ejsk_rmO err: keys \"%s\" in root obj" _isParentErr, keys); return 0;
    }

    return 0;
}

ejson  ejsr_rmO(ejson root, constr rawk, ejson obj)
{
    is0_exeret(_isOBJ(root)   , errset("ejsk_rmO err: root obj" _isOBJErr   );, 0);
    is1_exeret(_invalidS(rawk), errset("ejsk_rmO err: keys"     _invalidSErr);, 0);
    is1_exeret(_canotRm(obj)  , errset("ejsk_rmO err: rm obj "  _canotRmErr );, 0);

    root = _getObjByRawk(root, rawk);
    switch (_TYPE(root)) {
        case EJSON_OBJ: is0_exeret(_keyS(obj)        , errset("ejsk_rmO err: rm obj have no key" );, 0);
                        is0_exeret(_objRmO(root, obj), errset("ejsk_rmO err: rm obj is not in the root OBJ"), 0);
                        break;
        case EJSON_ARR: is0_exeret(_arrRmO(root, obj), errset("ejsk_rmO err: rm obj is not in the root ARR"), 0);
                        break;
        default       : errfmt("ejsk_rmO err: rawk \"%s\" in root obj" _isParentErr, rawk); return 0;
    }

    return 0;
}

ejson  ejso_pop(ejson root )
{
    is0_exeret(root, errset("ejso_pop err: root obj is NULL"), 0);

    switch (_TYPE(root)) {
        case EJSON_OBJ: return _objPop(root);
        case EJSON_ARR: return _arrPop(root);
        default       : errset("ejso_pop err: root obj" _isParentErr); return 0;
    }

    return 0;
}

ejson  ejsk_pop(ejson root, constr keys)
{
    is0_exeret(_isParent(root), errset("ejsk_pop err: root obj"       _isParentErr);, 0);

    if(!_invalidS(keys) && (root = _getObjByKeys(root, keys)))
    {
        switch (_TYPE(root)) {
            case EJSON_OBJ: return _objPop(root);
            case EJSON_ARR: return _arrPop(root);
            default       : errfmt("ejsk_pop err: keys \"%s\" in root obj" _isParentErr, keys); return 0;
        }
    }
    return 0;
}
ejson  ejsr_pop(ejson root, constr rawk)
{
    is0_exeret(_isOBJ(root), errset("ejsr_pop err: root obj"       _isOBJErr);, 0);

    if(!_invalidS(rawk) && (root = _getObjByRawk(root, rawk)))
    {
        switch (_TYPE(root)) {
            case EJSON_OBJ: return _objPop(root);
            case EJSON_ARR: return _arrPop(root);
            default       : errfmt("ejsr_pop err: rawk \"%s\" in root obj" _isParentErr, rawk); return 0;
        }
    }
    return 0;
}

ejson  ejso_popT(ejson root )
{
    is0_exeret(root, errset("ejso_popT err: root obj is NULL"), 0);

    switch (_TYPE(root)) {
        case EJSON_OBJ: return _objPopT(root);
        case EJSON_ARR: return _arrPopT(root);
        default       : errset("ejso_popT err: root obj" _isParentErr); return 0;
    }

    return 0;
}

ejson  ejsk_popT(ejson root, constr keys)
{
    is0_exeret(_isParent(root), errset("ejsk_popT err: root obj"       _isParentErr);, 0);

    if(!_invalidS(keys) && (root = _getObjByKeys(root, keys)))
    {
        switch (_TYPE(root)) {
            case EJSON_OBJ: return _objPopT(root);
            case EJSON_ARR: return _arrPopT(root);
            default       : errfmt("ejsk_popT err: keys \"%s\" in root obj" _isParentErr, keys); return 0;
        }
    }
    return 0;
}
ejson  ejsr_popT(ejson root, constr rawk)
{
    is0_exeret(_isOBJ(root), errset("ejsr_popT err: root obj"       _isOBJErr);, 0);

    if(!_invalidS(rawk) && (root = _getObjByRawk(root, rawk)))
    {
        switch (_TYPE(root)) {
            case EJSON_OBJ: return _objPopT(root);
            case EJSON_ARR: return _arrPopT(root);
            default       : errfmt("ejsr_popT err: rawk \"%s\" in root obj" _isParentErr, rawk); return 0;
        }
    }
    return 0;
}

ejson  ejso_clear(ejson root)
{
    ejson head;

    is0_ret(root, 0);

    switch (_TYPE(root)) {
        case EJSON_ARR: if((head = _objHead(root))){_isChild(head) = 0; ejso_free(head);} _arrFree(root);break;
        case EJSON_OBJ: if((head = _objHead(root))){_isChild(head) = 0; ejso_free(head);} _objFree(root);break;
        default       : return 0;
    }
    _obj(root)     = 0;
    _objLen(root)  = 0;
    _objHead(root) = _objTail(root) = 0;

    return root;
}

ejson  ejsk_clear(ejson root, constr keys)
{
    ejson head;

    is0_exeret(_isParent(root), errset("ejsk_clear err: root obj" _isParentErr);, 0);
    is1_exeret(_invalidS(keys), errset("ejsk_clear err: keys"     _invalidSErr);, 0);

    root = _getObjByKeys(root, keys);
    is0_exeret(_isParent(root), errfmt("ejsk_clear err: \"%s\" in root obj" _isParentErr, keys);, 0);

    switch (_TYPE(root)) {
        case EJSON_ARR: if((head = _objHead(root))){_isChild(head) = 0; ejso_free(head);} _arrFree(root);break;
        case EJSON_OBJ: if((head = _objHead(root))){_isChild(head) = 0; ejso_free(head);} _objFree(root);break;
    }
    _obj(root)     = 0;
    _objLen(root)  = 0;
    _objHead(root) = _objTail(root) = 0;

    return root;
}

ejson  ejsr_clear(ejson root, constr rawk)
{
    ejson head;

    is0_exeret(_isOBJ(root)   , errset("ejsr_clear err: root obj" _isOBJErr   );, 0);
    is1_exeret(_invalidS(rawk), errset("ejsr_clear err: keys"     _invalidSErr);, 0);

    root = _getObjByRawk(root, rawk);
    is0_exeret(_isParent(root), errfmt("ejsr_clear err: \"%s\" in root obj" _isParentErr, rawk);, 0);

    switch (_TYPE(root)) {
        case EJSON_ARR: if((head = _objHead(root))){_isChild(head) = 0; ejso_free(head);} _arrFree(root);break;
        case EJSON_OBJ: if((head = _objHead(root))){_isChild(head) = 0; ejso_free(head);} _objFree(root);break;
    }
    _obj(root)     = 0;
    _objLen(root)  = 0;
    _objHead(root) = _objTail(root) = 0;

    return root;
}

typedef struct ejsw_s{
    uint cap;
    uint len;
    cstr s;
}ejsw_t;

#define DF_WBUF_LEN 8

ejsw   ejsw_new(uint len)
{
    ejsw w;

    if(len < 8)    len = DF_WBUF_LEN;
    if(len > 1024) len = 1024;

#if COMPAT_ESTR

    is0_ret(w = calloc(1, sizeof(ejsw_t)), 0);
    is0_exeret(w->s = _snewm(len), free(w), 0);

    w->cap = len;
    w->s   = _hdr2s(w->s);

#else
    is0_ret(w = calloc(1, sizeof(ejsw_t)), 0);
    is0_exeret(w->s = (char*)malloc(OBJ_size + len), free(w), 0);

    w->cap = len;
    w->s  += OBJ_size;
#endif
    return w;
}

cstr ejso_toFStr(ejson obj)
{
    cstr s;   ejsw_t w;

#if COMPAT_ESTR
    is0_ret(obj, _NIL_);
    is0_ret(w.s = _snewm(DF_WBUF_LEN), 0);

    w.cap = DF_WBUF_LEN;
    w.len = 0;
    w.s   = _hdr2s(w.s);
#else
    is0_ret(obj, _NIL_);
    is0_ret(w.s = (char*)malloc(OBJ_size + DF_WBUF_LEN), 0);

    w.cap = DF_WBUF_LEN;
    w.len = 0;
    w.s  += OBJ_size;
#endif

    if((s = wrap_obj(obj, 0, 1, &w))) {_capS(s) = w.cap; _lenS(s) = w.len;}

    return s;
}

cstr ejso_toUStr(ejson obj)
{
    cstr s;   ejsw_t w;

#if COMPAT_ESTR

    is0_ret(obj, _NIL_);
    is0_ret(w.s = _snewm(DF_WBUF_LEN), 0);

    w.cap = DF_WBUF_LEN;
    w.len = 0;
    w.s   = _hdr2s(w.s);

#else

    is0_ret(obj, _NIL_);
    is0_ret(w.s = (char*)malloc(OBJ_size + DF_WBUF_LEN), 0);

    if(!obj)    return _NIL_;

    w.cap  = DF_WBUF_LEN;
    w.len  = 0;
    w.s   += OBJ_size;

#endif
    if((s = wrap_obj(obj, 0, 0, &w))) {_capS(s) = w.cap; _lenS(s) = w.len;}

    return s;
}

inline cstr ejsk_toFStr(ejson root, constr keys){   return ejso_toFStr(ejsk(root, keys));}
inline cstr ejsk_toUStr(ejson root, constr keys){   return ejso_toUStr(ejsk(root, keys));}
inline cstr ejsr_toFStr(ejson root, constr rawk){   return ejso_toFStr(ejsr(root, rawk));}
inline cstr ejsr_toUStr(ejson root, constr rawk){   return ejso_toUStr(ejsr(root, rawk));}

cstr   ejso_toFWra(ejson obj, ejsw w)
{
    cstr s;

    is0_ret(w, 0); is0_ret(obj, _NIL_);

    w->len = 0;
    if((s = wrap_obj(obj, 0, 1, w))) {_capS(s) = w->cap; _lenS(s) = w->len;}
    else w->len = 0;

    return s;
}

cstr   ejso_toUWra(ejson obj, ejsw w)
{
    cstr s;

    is0_ret(w, 0); is0_ret(obj, _NIL_);

    w->len = 0;
    if((s = wrap_obj(obj, 0, 0, w))) {_capS(s) = w->cap; _lenS(s) = w->len;}
    else w->len = 0;

    return s;
}

cstr   ejsk_toFWra(ejson root, constr keys, ejsw w) { return ejso_toFWra(ejsk(root, keys), w); }
cstr   ejsk_toUWra(ejson root, constr keys, ejsw w) { return ejso_toUWra(ejsk(root, keys), w); }
cstr   ejsr_toFWra(ejson root, constr rawk, ejsw w) { return ejso_toFWra(ejsr(root, rawk), w); }
cstr   ejsr_toUWra(ejson root, constr rawk, ejsw w) { return ejso_toUWra(ejsr(root, rawk), w); }

inline void ejss_show  (constr s) {if(!s) puts("(s:nil)"); else printf("(s:%d/%d):%s\n", _lenS(s), _capS(s), s   ); fflush(stdout);}
inline void ejsw_show  (ejsw   w) {if(!w) puts("(w:nil)"); else printf("(w:%d/%d):%s\n", w->len  , w->cap  , w->s); fflush(stdout);}

inline uint ejss_len   (constr s) { return s ? _lenS(s) : 0;}
inline void ejss_free  (cstr   s) { if(s && s != _NIL_) _freeS(s);}
inline cstr ejsw_valS  (ejsw   w) { return w ? w->s   : 0; }
inline uint ejsw_len   (ejsw   w) { return w ? w->len : 0; }
inline void ejsw_free  (ejsw   w) { if(w){_freeS(w->s);free(w);} }

static inline cstr split(cstr key, char c) {
    cstr p = strchr(key, c);
    is1_elsret(p, *p = '\0';return p + 1;, NULL);
}

static inline cstr splitdot(cstr key) {
    cstr p = strchr(key,'.');
    is1_elsret(p, *p = '\0';return p + 1;, NULL);
}

// assume that the obj and keys_ is valid, please check before calling
static inline ejson _objByKeys(ejson obj, constr keys_, int rm, int raw)
{
    char  keys[MAX_KEY_LEN]; cstr fk, sk, last_fk = 0;    // first key, second key, last first key
    ejson root;
    cstr  _idx; uint idx;

    if(raw)
    {
        root = obj;
        obj  = _objGet(root, keys_);
        is0_exeret(obj, errfmt("can not find %s in %s", keys_, "."), NULL);
        if(rm) _objRmO(root, obj);
        return obj;
    }

    strncpy(keys, keys_, MAX_KEY_LEN);
    fk = keys;
    sk = splitdot(fk);

    do
    {
        _idx = split(fk, '[');
        root = obj;
        if(*fk) {
            obj = _objGet(root, fk);
            is0_exeret(obj, errfmt("can not find %s in %s", fk, fk == keys ? "." : keys), NULL); // not found, return
        }

        while( _idx )
        {
            is0_exeret(_TYPE(obj) == EJSON_ARR,
                       switch (_TYPE(obj)) {
                           case EJSON_FALSE:  errfmt("%s is [FLASE] obj",  keys); break;
                           case EJSON_TRUE :  errfmt("%s is [TRUE] obj",   keys); break;
                           case EJSON_NULL :  errfmt("%s is [NULL] obj",   keys); break;
                           case EJSON_NUM  :  errfmt("%s is [NUMBER] obj", keys); break;
                           case EJSON_STR  :  errfmt("%s is [STRING] obj", keys); break;
                           case EJSON_OBJ  :  errfmt("%s is [OBJ] obj",    keys); break;
                       }, NULL);

            *(_idx - 1) = '[';          // restore
            is1_exeret(*_idx < '0' || *_idx > '9', errfmt("invalid keys: %s", keys), NULL);

            idx  = atoi(_idx);
            _idx = split(_idx, '[');

            root = obj;
            obj = _arrGet(root, idx);
            is0_exeret(obj, errfmt("can not find %s in %s", fk, fk == keys ? "." : keys), NULL);
        }

        is0_exeret(obj, errfmt("can not find %s in %s", fk, fk == keys ? "." : keys), NULL);   // not found, return

        // -- found and return it
        is0_exeret(sk, is1_exeret(rm, switch (_TYPE(root)) {
                                      //case EJSON_ARR: _arrRmI(root, idx); break;
                                      case EJSON_ARR: _arrRmO(root, obj); break;
                                      case EJSON_OBJ: _objRmO(root, obj); break;}, obj), obj);
        last_fk = fk;
        fk = sk;
        sk = splitdot(fk);
        if(last_fk != keys) *(last_fk - 1) = '.';
    }while(_TYPE(obj) == EJSON_OBJ);

    switch (_TYPE(obj)) {
        case EJSON_FALSE:  errfmt("%s is [FLASE] obj",  keys); break;
        case EJSON_TRUE :  errfmt("%s is [TRUE] obj",   keys); break;
        case EJSON_NULL :  errfmt("%s is [NULL] obj",   keys); break;
        case EJSON_NUM  :  errfmt("%s is [NUMBER] obj", keys); break;
        case EJSON_STR  :  errfmt("%s is [STRING] obj", keys); break;
        case EJSON_ARR  :  errfmt("%s is [ARRAY] obj",  keys); break;
    }

    return 0;
}

/// -- ejson get --

inline constr ejso_keyS (ejson obj){   return obj ? _keyS(obj) : _NIL_;  }
inline type   ejso_type (ejson obj){   return obj ? _TYPE(obj) : -1   ;  }
inline constr ejso_typeS(ejson obj)
{
    static _inner_ t_false = {{0,0,1,0,0, 5}, "false" };
    static _inner_ t_true  = {{0,0,1,0,0, 4}, "true"  };
    static _inner_ t_null  = {{0,0,1,0,0, 4}, "null"  };
    static _inner_ t_num   = {{0,0,1,0,0, 6}, "number"};
    static _inner_ t_str   = {{0,0,1,0,0, 6}, "string"};
    static _inner_ t_arr   = {{0,0,1,0,0, 5}, "array" };
    static _inner_ t_obj   = {{0,0,1,0,0, 3}, "obj"   };

    is0_ret(obj, _NIL_);

    switch (_TYPE(obj)) {
        case EJSON_FALSE:  return t_false.v;
        case EJSON_TRUE :  return t_true.v ;
        case EJSON_NULL :  return t_null.v ;
        case EJSON_NUM  :  return t_num.v  ;
        case EJSON_STR  :  return t_str.v  ;
        case EJSON_ARR  :  return t_arr.v  ;
        case EJSON_OBJ  :  return t_obj.v  ;
    }
    return 0;
}
inline int    ejso_is(ejson obj , int type) { return obj ? _TYPE(obj) == type : 0; }

inline ejson  ejsk      (ejson root, constr keys)          { _checkParent(root);_checkInvldS(keys);return _getObjByKeys(root, keys);}
inline type   ejsk_type (ejson root, constr keys)          { _checkParent(root);_checkInvldS(keys);return (root = _getObjByKeys(root, keys)) ? _TYPE(root)        : -1;}
inline constr ejsk_typeS(ejson root, constr keys)          { _checkParent(root);_checkInvldS(keys);return ejso_typeS(_getObjByKeys(root, keys));}
inline int    ejsk_is   (ejson root, constr keys, int type){ _checkParent(root);_checkInvldS(keys);return (root = _getObjByKeys(root, keys)) ? _TYPE(root) == type : 0;}

inline ejson  ejsr      (ejson root, constr rawk)          { _checkOBJ(root)   ;_checkInvldS(rawk);return _getObjByRawk(root, rawk);}
inline type   ejsr_type (ejson root, constr rawk)          { _checkOBJ(root)   ;_checkInvldS(rawk);return (root = _getObjByRawk(root, rawk)) ? _TYPE(root)        : -1;}
inline constr ejsr_typeS(ejson root, constr rawk)          { _checkOBJ(root)   ;_checkInvldS(rawk);return ejso_typeS(_getObjByRawk(root, rawk));                       }
inline int    ejsr_is   (ejson root, constr rawk, int type){ _checkOBJ(root)   ;_checkInvldS(rawk);return (root = _getObjByRawk(root, rawk)) ? _TYPE(root) == type : 0;}

/// -- ejson value --
inline int    ejso_valB(ejson obj) {if(obj){switch(_TYPE(obj)) {case EJSON_TRUE: return 1; case EJSON_STR : return _lenS(_valS(obj)) ? 1 : 0; case EJSON_NUM : return _valI(obj) ? 1 : 0;}}return 0;}
inline s64    ejso_valI(ejson obj) {return _isNUM(obj) ? _valI(obj) : 0 ;}
inline double ejso_valF(ejson obj) {return _isNUM(obj) ? _valF(obj) : 0 ;}
inline constr ejso_valS(ejson obj) {return _isSTR(obj) ? _valS(obj) : 0 ;}
inline void*  ejso_valP(ejson obj) {return _isPTR(obj) ? _valP(obj) : 0 ;}
inline void*  ejso_valR(ejson obj) {return _isRAW(obj) ? _valR(obj) : 0 ;}

inline int    ejsk_valB(ejson root, constr keys){_checkParent(root);_checkInvldS(keys);if((root = _getObjByKeys(root, keys))){switch(_TYPE(root)){case EJSON_TRUE: return 1; case EJSON_STR : return _lenS(_valS(root)) ? 1 : 0; case EJSON_NUM : return _valI(root) ? 1 : 0;}}return 0;}
inline s64    ejsk_valI(ejson root, constr keys){_checkParent(root);_checkInvldS(keys);if((root = _getObjByKeys(root, keys))){return _TYPE(root) == EJSON_NUM ? _valI(root) : 0;}return 0;}
inline double ejsk_valF(ejson root, constr keys){_checkParent(root);_checkInvldS(keys);if((root = _getObjByKeys(root, keys))){return _TYPE(root) == EJSON_NUM ? _valF(root) : 0;}return 0;}
inline constr ejsk_valS(ejson root, constr keys){_checkParent(root);_checkInvldS(keys);if((root = _getObjByKeys(root, keys))){return _TYPE(root) == EJSON_STR ? _valS(root) : 0;}return 0;}
inline void*  ejsk_valP(ejson root, constr keys){_checkParent(root);_checkInvldS(keys);if((root = _getObjByKeys(root, keys))){return _TYPE(root) == EJSON_PTR ? _valP(root) : 0;}return 0;}
inline void*  ejsk_valR(ejson root, constr keys){_checkParent(root);_checkInvldS(keys);if((root = _getObjByKeys(root, keys))){return _TYPE(root) == EJSON_RAW ? _valR(root) : 0;}return 0;}

inline int    ejsr_valB(ejson root, constr rawk){_checkOBJ(root)   ;_checkInvldS(rawk);if((root = _getObjByRawk(root, rawk))){switch(_TYPE(root)){case EJSON_TRUE: return 1; case EJSON_STR : return _lenS(_valS(root)) ? 1 : 0; case EJSON_NUM : return _valI(root) ? 1 : 0;}}return 0;}
inline s64    ejsr_valI(ejson root, constr rawk){_checkOBJ(root)   ;_checkInvldS(rawk);if((root = _getObjByRawk(root, rawk))){return _TYPE(root) == EJSON_NUM ? _valI(root) : 0;}return 0;}
inline double ejsr_valF(ejson root, constr rawk){_checkOBJ(root)   ;_checkInvldS(rawk);if((root = _getObjByRawk(root, rawk))){return _TYPE(root) == EJSON_NUM ? _valF(root) : 0;}return 0;}
inline constr ejsr_valS(ejson root, constr rawk){_checkOBJ(root)   ;_checkInvldS(rawk);if((root = _getObjByRawk(root, rawk))){return _TYPE(root) == EJSON_STR ? _valS(root) : 0;}return 0;}
inline void*  ejsr_valP(ejson root, constr rawk){_checkOBJ(root)   ;_checkInvldS(rawk);if((root = _getObjByRawk(root, rawk))){return _TYPE(root) == EJSON_PTR ? _valP(root) : 0;}return 0;}
inline void*  ejsr_valR(ejson root, constr rawk){_checkOBJ(root)   ;_checkInvldS(rawk);if((root = _getObjByRawk(root, rawk))){return _TYPE(root) == EJSON_RAW ? _valR(root) : 0;}return 0;}

/// -- ejson len --
inline uint   ejso_len (ejson obj){ return _isParent(obj) ? _objLen(obj)           : 0;}
inline uint   ejso_lenS(ejson obj){ return _isSTR(obj)    ? _lenS(_valS(obj)) : 0;}
inline uint   ejso_lenR(ejson obj){ return _isRAW(obj)    ? _lenS(_valR(obj)) : 0;}

inline uint   ejsk_len (ejson root, constr keys){ is1_elsret(root = ejsk(root, keys), return _TYPE(root) >  EJSON_RAW ? _objLen( root)          : 0;, 0); return 0;}
inline uint   ejsk_lenR(ejson root, constr keys){ is1_elsret(root = ejsk(root, keys), return _TYPE(root) == EJSON_STR ? _lenS(_valS(root)) : 0;, 0); return 0;}
inline uint   ejsk_lenS(ejson root, constr keys){ is1_elsret(root = ejsk(root, keys), return _TYPE(root) == EJSON_STR ? _lenS(_valR(root)) : 0;, 0); return 0;}

inline uint   ejsr_len (ejson root, constr rawk){ is1_elsret(root = ejsr(root, rawk), return _TYPE(root) >  EJSON_RAW ? _objLen( root)          : 0;, 0); return 0;}
inline uint   ejsr_lenS(ejson root, constr rawk){ is1_elsret(root = ejsr(root, rawk), return _TYPE(root) == EJSON_STR ? _lenS(_valS(root)) : 0;, 0); return 0;}
inline uint   ejsr_lenR(ejson root, constr rawk){ is1_elsret(root = ejsr(root, rawk), return _TYPE(root) == EJSON_STR ? _lenS(_valR(root)) : 0;, 0); return 0;}

/// -- ejson iterating --
inline ejson  ejso_first(ejson obj){   return _isParent(obj) ? _objHead(obj) : 0;}
inline ejson  ejso_last (ejson obj){   return _isParent(obj) ? _objTail(obj) : 0;}
inline ejson  ejso_next (ejson obj){   return obj ? _objNext(obj) : 0;  }
inline ejson  ejso_prev (ejson obj){   return obj ? _objPrev(obj) : 0;  }

inline ejson  ejsk_first(ejson obj, constr keys){ is1_elsret(obj = ejsk(obj, keys), switch(_TYPE(obj)){case EJSON_ARR: case EJSON_OBJ: return _objHead(obj);}, 0); return 0;}
inline ejson  ejsk_last (ejson obj, constr keys){ is1_elsret(obj = ejsk(obj, keys), switch(_TYPE(obj)){case EJSON_ARR: case EJSON_OBJ: return _objTail(obj);}, 0); return 0;}

inline ejson  ejsr_first(ejson obj, constr rawk){ is1_elsret(obj = ejsr(obj, rawk), switch(_TYPE(obj)){case EJSON_ARR: case EJSON_OBJ: return _objHead(obj);}, 0); return 0;}
inline ejson  ejsr_last (ejson obj, constr rawk){ is1_elsret(obj = ejsr(obj, rawk), switch(_TYPE(obj)){case EJSON_ARR: case EJSON_OBJ: return _objTail(obj);}, 0); return 0;}

/// -- ejson compare --
inline int ejso_cmpi(ejson obj, int    val){ return obj ? _TYPE(obj) == EJSON_NUM ? _valI(obj) < val ? -1 : _valI(obj) > val ? 1 : 0 : -3 : -2;}
inline int ejso_cmpf(ejson obj, double val){ return obj ? _TYPE(obj) == EJSON_NUM ? _valF(obj) - val < 0.000001 ? -1 : _valF(obj) - val > 0.000001 ? 1 : 0 : -3 : -2;}
inline int ejso_cmps(ejson obj, cstr   val){ return obj ? _TYPE(obj) == EJSON_STR ? val ? strcmp(_valS(obj), val) : -4 : -3 : -2;}

inline int ejsk_cmpi(ejson obj, constr keys, int    val){return (obj = ejsk(obj, keys)) ? _TYPE(obj) == EJSON_NUM ? _valI(obj) < val ? -1 : _valI(obj) > val ? 1 : 0 : -3 : -2;}
inline int ejsk_cmpf(ejson obj, constr keys, double val){return (obj = ejsk(obj, keys)) ? _TYPE(obj) == EJSON_NUM ? _valF(obj) - val < 0.000001 ? -1 : _valF(obj) - val > 0.000001 ? 1 : 0 : -3 : -2;}
inline int ejsk_cmps(ejson obj, constr keys, cstr   val){return (obj = ejsk(obj, keys)) ? _TYPE(obj) == EJSON_STR ? val ? strcmp(_valS(obj), val) : -4 : -3 : -2;}

/// -- ejson set val --
ejson ejso_setk(ejson obj, constr key)
{
    cstr hk, nk;

    is1_ret(_isChild(obj), 0); _checkInvldS(key);

    hk  = _keyS(obj);
    is0_exeret(nk = _wrbS(hk, key, strlen(key)), errset("ejso_setk err: alloc for new key failed"), 0);
    if(nk != hk)  _keyS(obj) = nk;

    return obj;
}

/// -- ejson set val --
ejson ejso_setT(ejson obj, uint  type)
{
    is0_ret(obj, 0); is1_ret(type > EJSON_NULL, 0);

    if(_TYPE(obj) <= EJSON_NULL)
        _TYPE(obj) = type;

    return obj;
}

ejson ejso_setF(ejson obj, double val)
{
    is0_ret(obj, 0); is1_ret(_TYPE(obj) != EJSON_NUM, 0);

    _valI(obj) = (s64)(_valF(obj) = val);

    return obj;
}

ejson ejso_setS(ejson obj, constr val)
{
    cstr hv, nv;

    is0_ret(obj, 0); is0_ret(val, 0); is1_ret(_TYPE(obj) != EJSON_STR, 0);

    hv  = _valS(obj);
    is0_exeret(nv = _wrbS(hv, val, strlen(val)), errset("ejso_setS err: alloc for new val failed"), 0);
    if(nv != hv) _valS(obj) = nv;

    return obj;
}

void* ejso_setR(ejson obj, uint   len)
{
    void* hr, * nr;

    is0_ret(obj, 0); is0_ret(len, 0);

    switch (_TYPE(obj)) {
        case EJSON_RAW: hr = _valR(obj);
                        is0_exeret(nr = _relS(hr, len), errset("ejso_setR err: alloc for new raw failed"), 0);
                        if(nr != hr) _valR(obj) = nr;
                        return nr;
        case EJSON_PTR: is0_exeret(nr = _newS2(0, len), errset("ejso_setR err: alloc for new failed"), 0);
                        _valR(obj) = nr; _setRAW(obj);
                        return nr;
    }

    return 0;
}

ejson ejso_setP(ejson obj, void*  ptr)
{
    is0_ret(obj, 0);

    switch (_TYPE(obj)) {
        case EJSON_RAW: _freeS(_valR(obj)); _valP(obj) = ptr; _setPTR(obj); break;
        case EJSON_PTR:                     _valP(obj) = ptr;               break;
        default       : return 0;
    }

    return obj;
}

ejson ejsk_setT(ejson root, constr keys, uint  type)
{
    _checkParent(root);_checkInvldS(keys); is1_ret(type > EJSON_NULL, 0);
    is0_ret(root = _getObjByKeys(root, keys), 0);

    if(_TYPE(root) <= EJSON_NULL)
        _TYPE(root) = type;

    return root;
}

ejson ejsk_setF(ejson root, constr keys, double val)
{
    _checkParent(root);_checkInvldS(keys);
    is0_ret(root = _getObjByKeys(root, keys), 0);

    is1_ret(_TYPE(root) != EJSON_NUM, 0);

    _valI(root) = (s64)(_valF(root) = val);

    return root;
}

ejson ejsk_setS(ejson root, constr keys, constr val)
{
    cstr hv, nv;

    _checkParent(root);_checkInvldS(keys); is0_ret(val, 0);
    is0_ret(root = _getObjByKeys(root, keys), 0);
    is1_ret(_TYPE(root) != EJSON_STR, 0);

    hv  = _valS(root);
    is0_exeret(nv = _wrbS(hv, val, strlen(val)), errset("ejsk_setS err: alloc for val failed"), 0);
    if(nv != hv) _valS(root) = nv;

    return root;
}

void* ejsk_setR(ejson root, constr keys, uint   len)
{
    void* hr, * nr;

    _checkParent(root);_checkInvldS(keys); is0_ret(len, 0);
    is0_ret(root = _getObjByKeys(root, keys), 0);

    switch (_TYPE(root)) {
        case EJSON_RAW: hr = _valR(root);
                        is0_exeret(nr = _relS(hr, len), errset("ejsk_setR err: alloc for new raw failed"), 0);
                        if(nr != hr) _valR(root) = nr;
                        return nr;
        case EJSON_PTR: is0_exeret(nr = _newS2(0, len), errset("ejsk_setR err: alloc for new failed"), 0);
                        _valR(root) = nr; _setRAW(root);
                        return nr;
    }

    return 0;
}

ejson ejsk_setP(ejson root, constr keys, void*  ptr)
{
    _checkParent(root);_checkInvldS(keys);
    is0_ret(root = _getObjByKeys(root, keys), 0);

    switch (_TYPE(root)) {
        case EJSON_RAW: _freeS(_valR(root)); _valP(root) = ptr; _setPTR(root); break;
        case EJSON_PTR:                      _valP(root) = ptr;                break;
        default       : return 0;
    }

    return root;
}

ejson ejsr_setT(ejson root, constr rawk, uint  type)
{
    _checkOBJ(root);_checkInvldS(rawk); is1_ret(type > EJSON_NULL, 0);
    is0_ret(root = _getObjByRawk(root, rawk), 0);

    if(_TYPE(root) <= EJSON_NULL)
        _TYPE(root) = type;

    return root;
}

ejson ejsr_setF(ejson root, constr rawk, double val)
{
    _checkOBJ(root);_checkInvldS(rawk);
    is0_ret(root = _getObjByRawk(root, rawk), 0);

    is1_ret(_TYPE(root) != EJSON_NUM, 0);

    _valI(root) = (s64)(_valF(root) = val);

    return root;
}

ejson ejsr_setS(ejson root, constr rawk, constr val)
{
    cstr hv, nv;

    _checkOBJ(root);_checkInvldS(rawk); is0_ret(val, 0);
    is0_ret(root = _getObjByRawk(root, rawk), 0);
    is1_ret(_TYPE(root) != EJSON_STR, 0);

    hv  = _valS(root);
    is0_exeret(nv = _wrbS(hv, val, strlen(val)), errset("ejsk_setS err: alloc for val failed"), 0);
    if(nv != hv) _valS(root) = nv;

    return root;
}

void* ejsr_setR(ejson root, constr rawk, uint   len)
{
    void* hr, * nr;

    _checkOBJ(root);_checkInvldS(rawk); is0_ret(len, 0);
    is0_ret(root = _getObjByRawk(root, rawk), 0);

    switch (_TYPE(root)) {
        case EJSON_RAW: hr = _valR(root);
                        is0_exeret(nr = _relS(hr, len), errset("ejsr_setR err: alloc for new raw failed"), 0);
                        if(nr != hr) _valR(root) = nr;
                        return nr;
        case EJSON_PTR: is0_exeret(nr = _newS2(0, len), errset("ejsr_setR err: alloc for new failed"), 0);
                        _valR(root) = nr; _setRAW(root);
                        return nr;
    }

    return 0;
}

ejson ejsr_setP(ejson root, constr rawk, void*  ptr)
{
    _checkOBJ(root);_checkInvldS(rawk);
    is0_ret(root = _getObjByRawk(root, rawk), 0);

    switch (_TYPE(root)) {
        case EJSON_RAW: _freeS(_valR(root)); _valP(root) = ptr; _setPTR(root); break;
        case EJSON_PTR:                      _valP(root) = ptr;                break;
        default       : return 0;
    }

    return root;
}

/// -- ejson substitute string --

ejson ejso_subk(ejson obj , constr subS, constr newS)
{
    cstr hk, nk;

    is1_ret(_isChild(obj), 0); is0_ret((hk = _keyS(obj)), obj); is1_ret(_invalidS(subS), obj); is0_ret(newS, obj);

    is0_exeret(nk = _subS(hk, subS, newS), errset("ejso_subk err: alloc for new key failed"), 0);
    if(nk != hk) _keyS(obj) = nk;

    return obj;
}

ejson ejso_subS(ejson obj , constr subS, constr newS)
{
    cstr hs, ns;

    is0_ret(_isSTR(obj), 0); is1_ret(_invalidS(subS), obj); is0_ret(newS, obj);

    hs = _valS(obj);
    is0_exeret(ns = _subS(hs, subS, newS), errset("ejso_subS err: alloc for new str failed"), 0);
    if(ns != hs) _valS(obj) = ns;

    return obj;
}

ejson ejsk_subS(ejson root, constr keys, constr subS, constr newS)
{
    cstr hs, ns;

    _checkParent(root);_checkInvldS(keys);
    root = _getObjByKeys(root, keys);

    is0_ret(_isSTR(root), 0); is1_ret(_invalidS(subS), root); is0_ret(newS, root);

    hs = _valS(root);
    is0_exeret(ns = _subS(hs, subS, newS), errset("ejsk_subS err: alloc for new str failed"), 0);
    if(ns != hs) _valS(root) = ns;

    return root;
}

ejson ejsr_subS(ejson root, constr rawk, constr subS, constr newS)
{
    cstr hs, ns;

    _checkOBJ(root);_checkInvldS(rawk);
    root = _getObjByRawk(root, rawk);

    is0_ret(_isSTR(root), 0); is1_ret(_invalidS(subS), root); is0_ret(newS, root);

    hs = _valS(root);
    is0_exeret(ns = _subS(hs, subS, newS), errset("ejsr_subS err: alloc for new str failed"), 0);
    if(ns != hs) _valS(root) = ns;

    return root;
}

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

ejson  ejsk_cntpp(ejson root, constr keys)
{
    ejson obj;
    _checkParent(root);_checkInvldS(keys);
    if((obj = _getObjByKeys(root, keys)))
    {
        if(_TYPE(obj) == EJSON_NUM) _valF(obj) = (double)(++_valI(obj));
        else return 0;
    }
    else
    {
        obj = ejso_addN(root, keys, 1);
    }

    return obj;
}

ejson  ejsk_cntmm(ejson root, constr keys)
{
    ejson obj;
    _checkParent(root);_checkInvldS(keys);
    if((obj = _getObjByKeys(root, keys)))
    {
        if(_TYPE(obj) == EJSON_NUM){ if(_valI(obj)) _valF(obj) = (double)(--_valI(obj));}
        else return 0;
    }
    else
    {
        obj = ejso_addN(root, keys, 0);
    }

    return obj;
}

ejson  ejsr_cntpp(ejson root, constr rawk)
{
    ejson obj;
    _checkOBJ(root);_checkInvldS(rawk);
    if((obj = _getObjByRawk(root, rawk)))
    {
        if(_TYPE(obj) == EJSON_NUM) _valF(obj) = (double)(++_valI(obj));
        else return 0;
    }
    else
    {
        obj = ejso_addN(root, rawk, 1);
    }

    return obj;
}
ejson  ejsr_cntmm(ejson root, constr rawk)
{
    ejson obj;
    _checkOBJ(root);_checkInvldS(rawk);
    if((obj = _getObjByRawk(root, rawk)))
    {
        if(_TYPE(obj) == EJSON_NUM){ if(_valI(obj)) _valF(obj) = (double)(--_valI(obj));}
        else return 0;
    }
    else
    {
        obj = ejso_addN(root, rawk, 0);
    }

    return obj;
}

/// -- ejson counter --

#define DF_SORTBASE_LEN 128

ejson  ejso_sort(ejson root, __ecompar_fn fn)
{
    ejson* base; ejson itr; int i, j, len; ejson _base[DF_SORTBASE_LEN];

    is0_ret(_isParent(root), 0); is0_ret(fn, 0);
    is1_ret(_objLen(root) < 2, root);

    if(_objLen(root) <= DF_SORTBASE_LEN)
        base = _base;
    else
        is0_ret(base = malloc(_objLen(root) * sizeof(ejson)), 0);

    for(itr = _objHead(root), i = 0; itr; itr = _objNext(itr), i++)
        base[i] = itr;

    qsort(base, len = i, sizeof(ejson), (__compar_fn_t)fn);

    _objHead(root) = _objTail(root) = 0;
    for(i = 0; i < len - 1; i++)
    {
        j = i + 1;
        _objNext(base[i]) = base[j];
        _objPrev(base[j]) = base[i];
    }
    _objHead(root) = base[0];       _objPrev(base[0])       = 0;
    _objTail(root) = base[len - 1]; _objNext(base[len - 1]) = 0;

    if(base != _base) free(base);
    return root;
}

ejson  ejsk_sort(ejson root, constr keys, __ecompar_fn fn)
{
    ejson obj;
    _checkParent(root);_checkInvldS(keys);
    if((obj = _getObjByKeys(root, keys)))
    {
        return ejso_sort(obj, fn);
    }

    return obj;
}

ejson  ejsr_sort(ejson root, constr rawk, __ecompar_fn fn)
{
    ejson obj;
    _checkParent(root);_checkInvldS(rawk);
    if((obj = _getObjByRawk(root, rawk)))
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
    if(_TYPE(e1) == EJSON_NUM)
    {
        if(_TYPE(e2) == EJSON_NUM)
            return _valI(e1) - _valI(e2) > 0;   // swap when return val > 0
        else
            return 0;
    }

    return _TYPE(e2) == EJSON_NUM ? 1 : 0;
}

int    __VALI_DES(ejson* _e1, ejson* _e2)
{
    ejson e1, e2; e1 = *_e1; e2 = *_e2;
    if(_TYPE(e1) == EJSON_NUM)
    {
        if(_TYPE(e2) == EJSON_NUM)
            return _valI(e2) - _valI(e1) > 0;
        else
            return 0;
    }

    return _TYPE(e2) == EJSON_NUM ? 1 : 0;
}

// ------------------------- obj header difinitions -----------------------

static inline void* obj__new(size_t size, uint _len)
{
    OBJ _out = calloc(1, OBJ_size + size * _len);

    if(_len > 1)
    {
        _out->is_array = 1;
        _out->_len     = _len;
    }

    return obj_to_val(_out);
}

static inline cstr obj__str(uint _len)
{
    OBJ  _head = calloc(1, OBJ_size + _len + 1);

    is0_ret(_head, 0);

    _head->is_array = 1;
    _head->_len     = _len;

    return obj_to_val(_head);
}


// --------------------------- dict definition -----------------------
#define DICT_HASH_FUNCTION_SEED 5381;
uint dictHashKey(const void *key, int len) {
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
    };

    /* Do a few final mixes of the hash to ensure the last few
     * bytes are well-incorporated. */
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return (unsigned int)h;
}

static inline int dict_init(dict d)
{
    dict_resetHt(d);
    d->rehashidx = -1;
    d->iterators = 0;

    return DICT_OK;
}

static inline dict dict_new()
{
    dict d = malloc(sizeof(*d));

    dict_init(d);
    return d;
}

static inline void dict_free(dict d)
{
    free(d->ht[0].table);
    free(d->ht[1].table);
    free(d);
}

static int dictRehash(dict d, int n)
{
    int empty_visits = n * 10;
    is0_ret(dictIsRehashing(d), 0);

    while(n-- && d->ht[0].used != 0)
    {
        ejson de, nextde;

        while(d->ht[0].table[d->rehashidx] == NULL) {
            d->rehashidx++;
            if (--empty_visits == 0) return 1;
        }

        de = d->ht[0].table[d->rehashidx];

         while(de) {
             unsigned int h;
             nextde = de->next;
             h = dictHashKey(_keyS(de), _lenS(_keyS(de))) & d->ht[1].sizemask;
             de->next = d->ht[1].table[h];
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
        dictht_reset(&d->ht[1]);
        d->rehashidx = -1;
        return DICT_OK;
    }

    return DICT_ERR;
}

static inline void dictRehashPtrStep(dict d)
{
    if (d->iterators == 0) dictRehash(d, 1);
}

static inline ulong dictNextPower(ulong size)
{
    ulong i = DICT_HT_INITIAL_SIZE;

    if (size >= LONG_MAX) return LONG_MAX;
    while(1) {
        if (i >= size)
            return i;
        i *= 2;
    }
}

static int dictExpand(dict d, ulong size)
{
    dictht n;
    ulong realsize = dictNextPower(size);

    is1_ret(dictIsRehashing(d) || d->ht[0].used > size, DICT_ERR);
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

static int dictExpandIfNeeded(dict d)
{
    /* Incremental rehashing already in progress. Return. */
    if (dictIsRehashing(d)) return DICT_OK;

    /* If the hash table is empty expand it to the initial size. */
    if (d->ht[0].size == 0) return dictExpand(d, DICT_HT_INITIAL_SIZE);

    /* If we reached the 1:1 ratio, and we are allowed to resize the hash
     * table (global setting) or we should avoid it but the ratio between
     * elements/buckets is over the "safe" threshold, we resize doubling
     * the number of buckets. */
    if (d->ht[0].used >= d->ht[0].size &&
        (dict_can_resize ||
         d->ht[0].used/d->ht[0].size > dict_force_resize_ratio))
    {
        return dictExpand(d, d->ht[0].used*2);
    }
    return DICT_OK;
}

static int dictKeyIndex(dict d, const void* key, int key_len)
{
    uint h, idx, table;
    ejson he;

    /* Expand the hash table if needed */
    if (dictExpandIfNeeded(d) == DICT_ERR)
        return -1;
    /* Compute the key hash value */
    h = dictHashKey(key, key_len);
    for (table = 0; table <= 1; table++) {
        idx = h & d->ht[table].sizemask;
        /* Search if this slot does not already contain the given key */
        he = d->ht[table].table[idx];
        while(he) {
            if ( !strcmp(key, _keyS(he)) )
                return -1;
            he = he->next;
        }
        if (!dictIsRehashing(d)) break;
    }
    return idx;
}

static inline ejson dict_add(dict d, const void* k, int k_len, ejson obj)
{
    int     idx;
    dictht* ht;

    if(dictIsRehashing(d)) dictRehashPtrStep(d);

    is1_ret((idx = dictKeyIndex(d, k, k_len)) == -1, NULL); // already exist

    ht = dictIsRehashing(d) ? &d->ht[1] : &d->ht[0];
    obj->next      = ht->table[idx];
    ht->table[idx] = obj;
    ht->used++;

    return obj;
}

static ejson dict_find(dict d, const void* k, int k_len)
{
    ejson he;
    unsigned int h, idx, table;

    if(d->ht[0].size == 0) return NULL; /* We don't have a table at all */
    if(dictIsRehashing(d)) dictRehashPtrStep(d);
    h = dictHashKey(k, k_len);
    for (table = 0; table <= 1; table++) {
        idx = h & d->ht[table].sizemask;
        he = d->ht[table].table[idx];
        while(he) {
            if ( !strcmp(k, _keyS(he)) )
                return he;
            he = he->next;
        }
        if (!dictIsRehashing(d)) return NULL;
    }
    return NULL;
}

static int dict_getL(dict d, const void* k, int k_len, L l)
{
    int     idx;
    dictht* ht;

    if(dictIsRehashing(d)) dictRehashPtrStep(d);

    is1_ret((idx = dictKeyIndex(d, k, k_len)) == -1, 0); // already exist

    ht = dictIsRehashing(d) ? &d->ht[1] : &d->ht[0];
    l->_pos  = &ht->table[idx];
    l->_used = &ht->used;

    return 1;
}


static ejson dict_del(dict d, ejson del)
{
    unsigned int h, idx;
    ejson he, prevHe;
    int table;

    if (d->ht[0].size == 0) return NULL; /* d->ht[0].table is NULL */
    if (dictIsRehashing(d)) dictRehashPtrStep(d);
    h = dictHashKey(_keyS(del), _lenS(_keyS(del)));

    for (table = 0; table <= 1; table++) {
        idx = h & d->ht[table].sizemask;
        he = d->ht[table].table[idx];
        prevHe = NULL;
        while(he) {
            if (!strcmp(_keyS(del), _keyS(he))) {
                if(he != del)   return NULL;    // not match

                /* Unlink the element from the list */
                if (prevHe) prevHe->next = he->next;
                else        d->ht[table].table[idx] = he->next;
                d->ht[table].used--;
                return he;
            }
            prevHe = he;
            he = he->next;
        }
        if (!dictIsRehashing(d)) break;
    }
    return NULL; /* not found */
}

// ---------------------------- ejson helpler ------------------------

static inline int   pow2gt(int x)	{	--x;	x|=x>>1;	x|=x>>2;	x|=x>>4;	x|=x>>8;	x|=x>>16;	return x+1;	}
static inline char* ensure(ejsw w, uint needed)
{
    cstr ns; int ncap;

    needed += w->len;
    is1_ret(needed <= w->cap, w->s + w->len);

    ncap   = pow2gt(needed);
#if COMPAT_ESTR
    ns = _snewr(_s2hdr(w->s), ncap);
    is0_ret(ns, 0);
    w->cap = ncap;
    if(ns != (cstr)_s2hdr(w->s))
        w->s = _hdr2s(ns);
#else
    ns = realloc(val_to_obj(w->s), OBJ_size + ncap);
    is0_ret(ns, 0);
    w->cap = ncap;
    if(ns != val_to_obj(w->s))
        w->s = ns + OBJ_size;
#endif


    return w->s + w->len;
}

#define wlenInc( w, l)   (w)->len+=(l)
#define wlenDec( w, l)   (w)->len-=(l)
#define wlenPP(w)        (w)->len++
#define wlenMM(w)        (w)->len--

static inline constr lstrip1(constr str)
{
    while (*str && (unsigned char)*str <= 32)
        str++;

    return str;
}

static inline constr lstrip2(constr str)
{
    do{
        if(!*str || (*str > 32 && (unsigned char)*str != '/' && (unsigned char)*str != '#'))
            break;

        while (*str && (unsigned char)*str <= 32)
            str++;

        switch (*str) {
            case '/': if     (*(str+1) == '*') {while (*str && !(*str=='*' && str[1]=='/')) str++; if(*str) str += 2; else { errset("connot find \"*/\""); goto return_;}}
                      else if(*(str+1) == '/') {while (*str && *str!='\n'                 ) str++; if(*str) str += 1; else { errset("connot find '\\n'" ); goto return_;}}
                      break;
            case '#':                          {while (*str && *str!='\n'                 ) str++; if(*str) str += 1; else { errset("connot find '\\n'" ); goto return_;}}
                      break;
        }
    }while(str);

return_:
    return str;
}

/// -- parse operations -------------------------------------------------

static ejson parse_NUM(cstr   name, constr* _src,               _lstrip lstrip);
//static ejson parse_STR(cstr* _name, constr* _src, constr* _err, _lstrip lstrip);
static ejson parse_ARR(cstr   name, constr* _src, constr* _err, _lstrip lstrip);
static ejson parse_OBJ(cstr   name, constr* _src, constr* _err, _lstrip lstrip);

static inline ejson parse_eval(cstr* _name, constr* _src, constr* _err, _lstrip lstrip)
{
    constr src  = *_src; ejson _out;

    //is0_ret(src, NULL);    // Fail on null
    is1_ret(*src == '-' || (*src>='0' && *src<='9'), parse_NUM(_name ? *_name : 0, _src, lstrip));

    switch (*src) {
        case 'n' :  is0_exeret(strncmp(src,"null", 4), is1_exeret(_out = _newNil(), _keyS(_out) = _name ? *_name : 0; _setNULL(_out) ; *_src = lstrip(src + 4), _out), 0); break;
        case 'f' :  is0_exeret(strncmp(src,"false",5), is1_exeret(_out = _newNil(), _keyS(_out) = _name ? *_name : 0; _setFALSE(_out); *_src = lstrip(src + 5), _out), 0); break;
        case 't' :  is0_exeret(strncmp(src,"true", 4), is1_exeret(_out = _newNil(), _keyS(_out) = _name ? *_name : 0; _setTRUE(_out) ; *_src = lstrip(src + 4), _out), 0); break;
        case '\"':  return parse_STR(_name             , _src, _err, lstrip);
        case '[' :  return parse_ARR(_name ? *_name : 0, _src, _err, lstrip);
        case '{' :  return parse_OBJ(_name ? *_name : 0, _src, _err, lstrip);
        default:    if(_name && *_name) _freeS(*_name);  break;
    }

    *_err = src; *_src = 0; return 0;	/* failure: err src */
}

static ejson parse_NUM(cstr name, constr* _src, _lstrip lstrip)
{
    double n = 0, sign = 1, scale = 0;  int subscale = 0, signsubscale = 1; constr src  = *_src; ejson _out;

    is0_ret(_out = _newNum(), 0);       // mem fail

    if (*src=='-') sign = -1, src++;	// Has sign?
    if (*src=='0') src++;               // is zero
    if (*src>='1' && *src<='9')                          do	n=(n*10.0)+(*src++ -'0');         while (*src>='0' && *src<='9');	// Number?
    if (*src=='.' && src[1]>='0' && src[1]<='9') {src++; do	n=(n*10.0)+(*src++ -'0'),scale--; while (*src>='0' && *src<='9');}	// Fractional part?
    if (*src=='e' || *src=='E')         // Exponent?
    {
        src++;if (*src=='+') src++;	else if (*src=='-') signsubscale=-1,src++;	// With sign?
        while (*src>='0' && *src<='9') subscale=(subscale*10)+(*src++ - '0');	// Number?
    }

    n = sign*n*pow(10.0,(scale+subscale*signsubscale));     // number = +/- number.fraction * 10^+/- exponent

    *_src           = lstrip(src);         // save new pos
    _keyS(_out)     = name;
    _valF(_out)     = n;
    _valI(_out)     = (long)n;
    _setNUM(_out);

    return _out;
}

static unsigned parse_hex4(constr str)
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

/**
 * @brief parse_string
 * @param _name a pointer to the name of obj
 *      if  _name == NULL, create new obj and parse str to obj.v.s
 *      if *_name == NULL, parse str to *_name;
 *      else               create new obj, set *_name to obj.k and parse str to obj.v.s
 * @param _src  src string pointer
 * @param _err  error str pointer
 * @return
 */
static ejson parse_STR(cstr* _name, constr* _src, constr* _err, _lstrip lstrip)
{
    static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
    constr   src     = *_src;        ejson out;
    constr   end_ptr = src + 1, ptr; cstr  ptr2, k_v;
    unsigned uc, uc2;                int   len = 0;

    while (*end_ptr!='\"' && *end_ptr && ++len) if (*end_ptr++ == '\\') end_ptr++;
    is0_ret(k_v = _newS2(0, len), 0);   // mem fail

#if 0
    memcpy(k_v, src + 1, len);
    ptr = end_ptr;
#else
    ptr = src + 1; ptr2 = k_v;
    while (ptr < end_ptr)
    {
        if (*ptr!='\\') *ptr2++=*ptr++;
        else
        {
            ptr++;
            switch (*ptr)
            {
                case 'b': *ptr2++='\b';	break;
                case 'f': *ptr2++='\f';	break;
                case 'n': *ptr2++='\n';	break;
                case 'r': *ptr2++='\r';	break;
                case 't': *ptr2++='\t';	break;
                case 'u':	 // transcode utf16 to utf8
                    uc = parse_hex4(ptr+1); ptr += 4;	// get the unicode char
                    is1_exeret(ptr >= end_ptr,                      _freeS(k_v); *_err = src; *_src = 0, 0);  // invalid
                    is1_exeret((uc>=0xDC00 && uc<=0xDFFF) || uc==0, _freeS(k_v); *_err = src; *_src = 0, 0);	// check for invalid

                    if (uc>=0xD800 && uc<=0xDBFF)       // UTF16 surrogate pairs
                    {
                        is1_exeret(ptr+6 > end_ptr,             _freeS(k_v); *_err = src; *_src = 0, 0);  // invalid
                        is1_exeret(ptr[1]!='\\' || ptr[2]!='u', _freeS(k_v); *_err = src; *_src = 0, 0);	// missing second-half of surrogate
                        uc2 = parse_hex4(ptr+3); ptr += 6;
                        is1_exeret(uc2<0xDC00 || uc2>0xDFFF,    _freeS(k_v); *_err = src; *_src = 0, 0);	// invalid second-half of surrogate
                        uc = 0x10000 + (((uc&0x3FF)<<10) | (uc2&0x3FF));
                    }

                    len=4;if (uc<0x80) len=1;else if (uc<0x800) len=2;else if (uc<0x10000) len=3; ptr2+=len;

                    switch (len) {
                        case 4: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
                        case 3: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
                        case 2: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
                        case 1: *--ptr2 =(uc | firstByteMark[len]);
                    }
                    ptr2+=len;
                    break;
                default:  *ptr2++=*ptr; break;
            }
            ptr++;
        }
    }
    *ptr2=0;
#endif
    if (*ptr=='\"') *_src = lstrip(ptr + 1);

    if(!_name || *_name)        // _name == NULL || *_name != NULL
    {
        is0_exeret(out  = _newStr(), _freeS(k_v), 0);  // mem fail
        _setSTR(out);
        _valS(out)           = k_v;
        if(_name) _keyS(out) = *_name;
    }
    else
    {
        out    = 0;
        *_name = k_v;
    }

    return out;
}

static ejson parse_ARR(cstr name, constr* _src, constr* _err, _lstrip lstrip)
{
    constr src = *_src; ejson out = 0; ejson  elem;

    is1_elsret( out  = _newObj(), _keyS(out) =name; _setARR(out);, 0);          // mem fail
    is1_exeret(*(src = lstrip(src + 1)) == ']', *_src = lstrip(src + 1), out);  // empty array

    // -- parse element
    *_src = lstrip(src);
    is0_exeret(elem = parse_eval(0, _src, _err, lstrip), _freeObj(out), 0);

    // -- init dict for father obj and add elem
    is0_exeret(_arrInit(out), ejso_free(elem); _freeObj(out);, 0);
    _arrAdd(out, elem);

    while (*(src = *_src) == ',')
    {
        *_src = lstrip(src + 1);
        is0_exeret(elem = parse_eval(0, _src, _err, lstrip), ejso_free(out), 0);
        _arrAdd(out, elem);
    }

    is1_exeret(**_src == ']', *_src = lstrip(*_src + 1), out);      // end of array
    *_err = *_src; *_src = 0;                // malformed
    ejso_free(out); return 0;
}

static ejson parse_OBJ(cstr name, constr* _src, constr* _err, _lstrip lstrip)
{
    cstr  c_name = 0; ejson out = 0;ejson child;

    is1_elsret(out = _newObj(), _keyS(out) =name; _setOBJ(out);, 0);                  // mem fail
    is1_exeret(*(*_src = lstrip(*_src + 1)) == '}', *_src = lstrip(*_src + 1), out);// empty obj

    // -- parse key of child
    *_src = lstrip(*_src);
    is0_exeret(**_src == '\"', *_err = *_src; *_src = 0, 0);
    parse_STR(&c_name, _src, _err, lstrip);
    is0_exeret(c_name, _freeObj(out), 0);

    // -- parse child obj
    is1_exeret(**_src != ':', _freeObj(out); *_err = *_src; *_src = 0, 0);     // not a obj
    *_src = lstrip(*_src + 1);
    is0_exeret(child = parse_eval(&c_name, _src, _err, lstrip), _freeObj(out), 0);

    // -- init dict for father obj and add child
    is0_exeret(_objInit(out), ejso_free(child); _freeObj(out);, 0);
    _objAdd(out, _keyS(child), child);

    // -- parse next
    while(**_src == ',')
    {
        // -- parse key
        c_name = 0;
        *_src = lstrip(*_src + 1);
        is0_exeret(**_src == '\"', *_err = *_src; *_src = 0, 0);
        parse_STR(&c_name, _src, _err, lstrip);
        is0_exeret(c_name, ejso_free(out), 0);

        // -- parse child obj
        is1_exeret(**_src != ':', ejso_free(out); *_err = *_src; *_src = 0, 0);     // not a obj
        *_src = lstrip(*_src + 1);
        is0_exeret(child = parse_eval(&c_name, _src, _err, lstrip), ejso_free(out);, 0);

        // -- add child
        is0_exeret(_objAdd(out, _keyS(child), child), _freeObj(child); ejso_free(out);, 0);    // already have the same key
    }

    is1_exeret(**_src == '}', *_src = lstrip(*_src + 1), out);      // end of obj
    *_err = *_src; *_src = 0;                // malformed
    ejso_free(out); return 0;
}

/// -- check operations -------------------------------------------------

static int check_NUM(constr* _src,               _lstrip lstrip);
static int check_STR(constr* _src, constr* _err, _lstrip lstrip);
static int check_ARR(constr* _src, constr* _err, _lstrip lstrip);
static int check_OBJ(constr* _src, constr* _err, _lstrip lstrip);

static int check_eval(constr* _src, constr* _err, _lstrip lstrip)
{
    constr src  = *_src;

    is1_ret(*src == '-' || (*src>='0' && *src<='9'), check_NUM(_src, lstrip));

    switch (*src) {
        case 'n' :  is0_exeret(strncmp(src,"null", 4), *_src = lstrip(src + 4), 1); break;
        case 'f' :  is0_exeret(strncmp(src,"false",5), *_src = lstrip(src + 5), 1); break;
        case 't' :  is0_exeret(strncmp(src,"true", 4), *_src = lstrip(src + 4), 1); break;
        case '\"':  return check_STR(_src, _err, lstrip);
        case '[' :  return check_ARR(_src, _err, lstrip);
        case '{' :  return check_OBJ(_src, _err, lstrip);
        default  :  break;
    }

    *_err = src; *_src = 0; return 0;	// failure: err src
}

static int check_NUM(constr* _src, _lstrip lstrip)
{
    constr src  = *_src;

    if (*src=='-') src++;               // Has sign?
    if (*src=='0') src++;               // is zero
    if (*src>='1' && *src<='9')                          do	src++; while (*src>='0' && *src<='9');	// Number?
    if (*src=='.' && src[1]>='0' && src[1]<='9') {src++; do	src++; while (*src>='0' && *src<='9');}	// Fractional part?
    if (*src=='e' || *src=='E')         // Exponent?
    {
        src++;if (*src=='+') src++;	else if (*src=='-') src++;	// With sign?
        while (*src>='0' && *src<='9') src++;                   // Number?
    }
    *_src = lstrip(src);
    return 1;
}

static int check_STR(constr* _src, constr* _err, _lstrip lstrip)
{
    constr src = *_src+1;

    while (*src!='\"' && *src) if (*src++ == '\\') src++;

    is1_exeret(*src != '\"', *_err = src, 0);
    *_src = lstrip(src + 1);
    return 1;
}

static int check_ARR(constr* _src, constr* _err, _lstrip lstrip)
{
    constr src = *_src;

    is1_exeret(*(src = lstrip(src + 1)) == ']', *_src = lstrip(src + 1), 1);    // empty array

    // -- check element
    *_src = lstrip(src);
    is0_ret(check_eval(_src, _err, lstrip), 0);

    // -- check next
    while (**_src == ',')
    {
        *_src = lstrip(*_src + 1);
        is0_ret(check_eval(_src, _err, lstrip), 0);
    }
    is1_exeret(**_src == ']', *_src = lstrip(*_src + 1), 1);
    *_err = src; *_src = 0;                // malformed
    return 0;
}

static int check_OBJ(constr* _src, constr* _err, _lstrip lstrip)
{
    is1_exeret(*(*_src = lstrip(*_src + 1)) == '}', *_src = lstrip(*_src + 1), 1);// empty obj

    // -- check key of child
    *_src = lstrip(*_src);
    is1_exeret(**_src != '\"', *_err = *_src; *_src = 0, 0);
    is0_ret(check_STR(_src, _err, lstrip), 0);

    // -- check child obj
    is1_exeret(**_src != ':', *_err = *_src; *_src = 0, 0);
    *_src = lstrip(*_src + 1);
    is0_ret(check_eval(_src, _err, lstrip), 0);

    // -- check next
    while(**_src == ',')
    {
        // -- check key
        *_src = lstrip(*_src + 1);
        is1_exeret(**_src != '\"', *_err = *_src; *_src = 0, 0);
        is0_ret(check_STR(_src, _err, lstrip), 0);

        // -- check obj
        is1_exeret(**_src != ':', *_err = *_src; *_src = 0, 0);
        *_src = lstrip(*_src + 1);
        is0_ret(check_eval(_src, _err, lstrip), 0);
    }

    is1_exeret(**_src == '}', *_src = lstrip(*_src + 1), 1);      // end of obj
    *_err = *_src; *_src = 0;                // malformed
    return 0;
}

/// -- wrap operations -------------------------------------------------

static cstr wrap_str(constr src, ejsw p);
#define wrap_key(o, w) wrap_str(_keyS(o), w)
#define wrap_STR(o, w) wrap_str(_valS(o), w)

static cstr wrap_NUM(ejson  obj, ejsw w);
static cstr wrap_ARR(ejson  obj, int depth, int pretty, ejsw w);
static cstr wrap_OBJ(ejson  obj, int depth, int pretty, ejsw w);

static cstr wrap_obj(ejson obj, int depth, int pretty, ejsw w)
{
    cstr _out = 0;

    switch (_TYPE(obj))
    {
        case EJSON_NULL : _out = ensure(w, 5);	if(_out){ strcpy(_out,"null"  );wlenInc(w, 4);}   break;
        case EJSON_FALSE: _out = ensure(w, 6);	if(_out){ strcpy(_out,"false" );wlenInc(w, 5);}   break;
        case EJSON_TRUE : _out = ensure(w, 5);	if(_out){ strcpy(_out,"true"  );wlenInc(w, 4);}   break;
        case EJSON_NUM  : _out = wrap_NUM(obj, w);                                                 break;
        case EJSON_STR  : _out = wrap_STR(obj, w);                                                 break;
        case EJSON_ARR  : _out = wrap_ARR(obj, depth, pretty, w);                                  break;
        case EJSON_OBJ  : _out = wrap_OBJ(obj, depth, pretty, w);                                  break;
        case EJSON_RAW  : _out = ensure(w, 26);	if(_out){ wlenInc(w, snprintf(_out, 26, "\"(RAW %d/%d)\"", _lenS(_valR(obj)), _capS(_valR(obj))));} break;
        case EJSON_PTR  : _out = ensure(w, 24); if(_out){ wlenInc(w, snprintf(_out, 24, "\"(PTR@%p)\""   , _valP(obj))                          );} break;
    }

    return _out;
}

static cstr wrap_NUM(ejson obj, ejsw w)
{
    int len = 0; char* str = 0; double d = _valF(obj);

    if (d == 0)
    {
        if ((str = ensure(w, 2))) {*str='0';*(str+1)=0;}    // special case for 0.
        len = 1;
    }
    else if (fabs(((double)_valI(obj)) - d) <= DBL_EPSILON && d <= INT_MAX && d >= INT_MIN)
    {
        if ((str = ensure(w, 21))) len = snprintf(str, 21, "%"PRIi64"", _valI(obj));    // 2^64+1 can be represented in 21 chars.
    }
    else
    {
        if ((str = ensure(w,64)))   // This is a nice tradeoff.
        {
            if      (d*0 != 0)											len = snprintf(str,21,"null");	/* This checks for NaN and Infinity */
            else if (fabs(floor(d)-d)<=DBL_EPSILON && fabs(d)<1.0e60)	len = snprintf(str,21,"%.0f",d);
            else if (fabs(d)<1.0e-6 || fabs(d)>1.0e9)					len = snprintf(str,21,"%e",d);
            else                                                     	len = snprintf(str,21,"%f",d);
        }
    }

    wlenInc(w, len);

    return str;
}

static cstr wrap_ARR(ejson obj, int depth, int pretty, ejsw w)
{
    cstr   out  = 0, ptr;   int len = 5;
    ejson  elem = obj->next;
    int    numentries = 0, i = 0, num = 0;

    // -- How many entries in the array?
    numentries = _arrLen(obj);

    // -- Explicitly handle numentries == 0
    if (!numentries)
    {
        if ((out = ensure(w, 3)))  {out[0] = '['; out[1] = ']'; out[2] = 0;wlenInc(w, 2);}
        return    out;
    }

    // -- Compose the output array.
    i   = w->len;
    is0_ret(ptr = ensure(w, 1), 0); 	*ptr='[';	wlenPP(w);

    elem = _objHead(obj);
    while (num++ < numentries)
    {
        wrap_obj(elem, depth+1, pretty, w);
        len=pretty?2:1;ptr=ensure(w,len+1);if (!ptr) return 0;*ptr++=',';if(pretty)*ptr++=' ';*ptr=0; wlenInc(w,len);
        elem = _objNext(elem);
    }
    if (pretty)	{ *(ptr-2) = '0';  wlenDec(w,2); }
    else        { *(ptr-1) = 0  ;  wlenMM (w);}
    ptr=ensure(w,2);if (!ptr) return 0;	*ptr++=']';*ptr=0;
    out=(w->s)+i;
    wlenPP(w);

    return out;
}

static cstr wrap_OBJ(ejson obj, int depth, int pretty, ejsw w)
{
    cstr  out     = 0, ptr; int len = 7, i = 0, j;
    ejson child;
    int   numentries = _objLen(obj), num = 0;

    // -- Explicitly handle empty object case
    if (!numentries)
    {
        is0_ret(out = ensure(w, pretty ? depth+4 : 3), 0);
        ptr = out; *ptr++ = '{';
        if (pretty) {*ptr++='\n';for (i=0;i<depth;i++) *ptr++='\t';}
        *ptr++='}';*ptr=0;
        wlenInc(w, ptr-out);
        return out;
    }

    // -- Compose the output
    i   = w->len;
    len = pretty?2:1;	ptr=ensure(w,len+1);	if (!ptr) return 0;
    *ptr++='{';	if (pretty) *ptr++='\n';	*ptr=0; wlenInc(w, len);

    child = _objHead(obj);
    depth++;
    while (num++ < numentries)
    {
        if (pretty)
        {
            is0_ret(ptr = ensure(w, depth), 0);
            for (j=0; j<depth; j++) *ptr++='\t';
            wlenInc(w, depth);
        }
        wrap_key(child, w);

        len = pretty ? 2 : 1;
        is0_ret(ptr = ensure(w, len), 0);
        *ptr++=':';if (pretty) *ptr++='\t';
        wlenInc(w, len);
        wrap_obj(child, depth, pretty, w);

        len=(pretty?1:0) + 1;
        is0_ret(ptr = ensure(w, len + 1), 0);
        *ptr++=',';
        if (pretty) *ptr++='\n';
        *ptr=0;
        wlenInc(w, len);
        child=_objNext(child);
    }
    if (pretty)	{ *(ptr-2) = '\n'; wlenMM(w);}
    else        { *(ptr-1) = 0;    wlenMM(w);}

    is0_ret(ptr = ensure(w, pretty?(depth+1):2), 0);
    if (pretty)	for (i=0;i<depth-1;i++) *ptr++='\t';
    *ptr++='}';*ptr=0;
    out=(w->s)+i;
    wlenInc(w, pretty?(depth):1);

    return out;
}

static cstr wrap_str(constr src, ejsw w)
{
    constr ptr; cstr ptr2, out;  int len = 0, flag = 0;  unsigned char u; char c;

    if (!src)
    {
        is0_ret(out = ensure(w, 3), 0);
        out[0] = out[1] = '\"'; out[2] = 0;
        wlenInc(w, 2);
        return out;
    }

    for (ptr=src; (c = *ptr);ptr++) if((c>0 && c<32)||(c=='\"')||(c=='\\')) {flag=1;break;}
    if (!flag)
    {
        len = ptr - src;
        is0_ret(out = ensure(w, len + 3), 0);
        ptr2=out;*ptr2++='\"';
        memcpy(ptr2,src,len);
        ptr2[len]  = '\"';
        ptr2[len+1]= 0;
        wlenInc(w, len + 2);
        return out;
    }

    len += (ptr - src);
    while ((u=*ptr)) {++len;if (strchr("\"\\\b\f\n\r\t",u)) len++; else if (u<32) len+=5;ptr++;}

    is0_ret(out = ensure(w, len + 3), 0);
    ptr=src;    ptr2=out;*ptr2++='\"';
    while (*ptr)
    {
        if ((unsigned char)*ptr>31 && *ptr!='\"' && *ptr!='\\') *ptr2++=*ptr++;

        else
        {
            *ptr2++='\\';
            switch (u=*ptr++)
            {
                case '\\':	*ptr2++='\\';	break;
                case '\"':	*ptr2++='\"';	break;
                case '\b':	*ptr2++='b';	break;
                case '\f':	*ptr2++='f';	break;
                case '\n':	*ptr2++='n';	break;
                case '\r':	*ptr2++='r';	break;
                case '\t':	*ptr2++='t';	break;
                default: sprintf(ptr2,"u%04x",u);ptr2+=5;	break;	/* escape and print */
            }
        }
    }
    *ptr2++='\"';*ptr2=0;
    wlenInc(w, (ptr2 - out));

    return out;
}

/// --------------------------- inner OBJ operation

static ejson _objAdd(ejson root, cstr key, ejson obj)
{
    is0_ret(dict_add(_obj(root), key, _lenS(key), obj), 0);
    _isChild(obj) = 1;

    if(!_objHead(root)){_objHead(root) =          _objTail(root)  = obj;}

    else               {_objPrev(obj ) =          _objTail(root)       ;
                        _objTail(root) = _objNext(_objTail(root)) = obj;}

    _objLen(root)++;

    return obj;
}

static void _objLink(ejson root, ejson obj)
{
    _isChild(obj) = 1;

    if(!_objHead(root)){_objHead(root) =          _objTail(root)  = obj;}

    else               {_objPrev(obj ) =          _objTail(root)       ;
                        _objTail(root) = _objNext(_objTail(root)) = obj;}

    _objLen(root)++;
}

static ejson _objPush(ejson root, cstr key, ejson obj)
{
    is0_ret(dict_add(_obj(root), key, _lenS(key), obj), 0);
    _isChild(obj) = 1;

    if(!_objHead(root)){_objHead(root) =          _objTail(root) = obj;}

    else               {_objNext(obj ) =          _objHead(root)       ;
                        _objHead(root) = _objPrev(_objHead(root)) = obj;}

    _objLen(root)++;

    return obj;
}

static ejson _objPop(ejson root)
{
    ejson out;

    is0_ret(_objLen(root), 0);

    dict_del(_obj(root), (out = _objHead(root)));

    if(1 == _objLen(root)){_objHead(root) = _objTail(root)          = NULL;}

    else                  {_objHead(root)                           = _objNext(out);
                                            _objPrev(_objNext(out)) = NULL;}

    _objPrev(out) = _objNext(out) = NULL;
    _isChild(out) = 0;

    _objLen(root)--;

    return out;
}

static ejson _objPopT(ejson root)
{
    ejson out;

    is0_ret(_objLen(root), 0);

    dict_del(_obj(root), (out = _objTail(root)));

    if(1 == _objLen(root)){_objHead(root) = _objTail(root)          = NULL         ;}

    else                  {_objTail(root)                           = _objPrev(out);
                                            _objNext(_objPrev(out)) = NULL         ;}

    _objPrev(out) = _objNext(out) = NULL;
    _isChild(out) = 0;

    _objLen(root)--;

    return out;
}

static ejson  _objRmO(ejson root, ejson obj)
{
    if(!dict_del(_obj(root), obj))  return NULL;

    if   (_objPrev(obj))  _objNext(_objPrev(obj)) = _objNext(obj);
    else                  _objHead(root)          = _objNext(obj);

    if   (_objNext(obj))  _objPrev(_objNext(obj)) = _objPrev(obj);
    else                  _objTail(root)          = _objPrev(obj);

    _objPrev(obj) = _objNext(obj) = NULL;
    _isChild(obj) = 0;

    _objLen(root)--;

    return obj;
}

/// -------------------------------- inner ARR operation

typedef struct { ejson o; uint  i;}* H;

// -- add in tail
static ejson _arrAdd(ejson root, ejson obj)
{
    _isChild(obj) = 1;

    if(!_objHead(root)){_objHead(root) =          _objTail(root)  = obj;}

    else               {_objPrev(obj)  =          _objTail(root)       ;
                        _objTail(root) = _objNext(_objTail(root)) = obj;}

    _arrLen(root)++;

    return obj;
}

// -- add in head
static ejson _arrPush(ejson root, ejson obj)
{
    _isChild(obj) = 1;

    if(!_objHead(root)){_objHead(root) =          _objTail(root)  = obj;}

    else               {_objNext(obj)  =          _objHead(root)       ;
                        _objHead(root) = _objPrev(_objHead(root)) = obj;
                        if(((H)_arr(root))->i) ((H)_arr(root))->i++;    }

    _arrLen(root)++;

    return obj;
}

// -- remove in head
static ejson _arrPop(ejson root)
{
    ejson out;

    is0_ret(_objLen(root), 0);

    out = _objHead(root);

    if(1 == _arrLen(root)){_objHead(root) = _objTail(root)          = NULL;}

    else                  {_objHead(root)                           = _objNext(out);
                                            _objPrev(_objNext(out)) = NULL;
                           if(((H)_arr(root))->i) ((H)_arr(root))->i--;}

    _arrLen(root)--;

    _objPrev(out) = _objNext(out) = NULL;
    _isChild(out) = 0;

    return out;
}

// -- remove in tail
static ejson _arrPopT(ejson root)
{
    ejson out; H h;

    is0_ret(_objLen(root), 0);

    out = _objTail(root);
    h   = _arr(root);

    if(1 == _arrLen(root)){_objHead(root) = _objTail(root)          = NULL;}

    else                  {_objTail(root)                           = _objPrev(out);
                                            _objNext(_objPrev(out)) = NULL         ;
                           if(h->i && h->o == out) {h->o = _objPrev(out); h->i--;} ;}

    _arrLen(root)--;

    _objPrev(out) = _objNext(out) = NULL;
    _isChild(out) = 0;

    return out;
}

static ejson _arrFind(ejson root, uint idx)
{
    ejson out; uint i; H h;

    h = _arr(root);

    if(idx >= (i = h->i)) { out = i ? h->o : _objHead(root); for(; i != idx; i++) out = _objNext(out); }
    else                  { out =     h->o                 ; for(; i != idx; i--) out = _objPrev(out); }

    h->i = idx;
    h->o = out;

    return out;
}

static ejson _arrRmI(ejson root, uint idx)
{
    ejson out; uint i; H h;

    h = _arr(root);

    if(idx >= (i = h->i)) { out = i ? h->o : _objHead(root); for(; i != idx; i++) out = _objNext(out); }
    else                  { out =     h->o                 ; for(; i != idx; i--) out = _objPrev(out); }

    if   (_objPrev(out)) _objNext(_objPrev(out)) = _objNext(out);
    else                 _objHead(root)          = _objNext(out);

    if   (_objNext(out)){_objPrev(h->o = _objNext(out)) = _objPrev(out);            h->i = idx    ;}
    else                {         h->o = _objTail(root) = _objPrev(out);    if(idx) h->i = idx - 1;}

    _arrLen(root)--;

    _objPrev(out) = _objNext(out) = NULL;
    _isChild(out) = 0;

    return out;
}

static ejson _arrRmO(ejson root, ejson obj)
{
    ejson out; uint i; H h;

    h = _arr(root);

    if((i = h->i))  {          for(              out =          h->o ; out && out != obj; out = _objNext(out), i++);
                      if(!out) for(i = h->i - 1, out = _objPrev(h->o); out && out != obj; out = _objPrev(out), i--); }
    else            { for(i = 0, out = _objHead(root); out && out != obj; out = _objNext(out), i++);}

    is0_ret(out, 0);

    if   (_objPrev(out)) _objNext(_objPrev(out)) = _objNext(out);
    else                 _objHead(root)          = _objNext(out);

    if   (_objNext(out)){_objPrev(h->o = _objNext(out)) = _objPrev(out);          h->i = i    ;}
    else                {         h->o = _objTail(root) = _objPrev(out);    if(i) h->i = i - 1;}

    _arrLen(root)--;

    _objPrev(out) = _objNext(out) = NULL;
    _isChild(out) = 0;

    return out;
}

inline constr ejson_version() {   return _VER_;    }
