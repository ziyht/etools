#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>
#include <inttypes.h>

#include "estr.h"

#define exe_ret(expr, ret) {expr;}     return ret
#define is0_ret(cond, ret) if(!(cond)) return ret
#define is1_ret(cond, ret) if( (cond)) return ret

#define is0_exeret(cond, expr, ret) if(!(cond)){ expr; return ret;}
#define is1_exeret(cond, expr, ret) if( (cond)){ expr; return ret;}
#define is0_elsret(cond, expr, ret) if(!(cond)){expr;} else return ret
#define is1_elsret(cond, expr, ret) if( (cond)){expr;} else return ret

/// -- sds from redis -----------------------------

#define s_malloc  malloc
#define s_realloc realloc
#define s_free    free

#define SDS_MAX_PREALLOC (1024*1024)

struct __attribute__ ((__packed__)) sdshdr5 {
    uint8_t flags;  /* 3 lsb of type, and 5 msb of string length */
    char    buf[];
};
struct __attribute__ ((__packed__)) sdshdr8 {
    uint8_t len;    /* used */
    uint8_t alloc;  /* excluding the header and null terminator */
    uint8_t flags;  /* 3 lsb of type, 5 unused bits */
    char    buf[];
};
struct __attribute__ ((__packed__)) sdshdr16 {
    uint16_t len;   /* used */
    uint16_t alloc; /* excluding the header and null terminator */
    uint8_t  flags; /* 3 lsb of type, 5 unused bits */
    char     buf[];
};
struct __attribute__ ((__packed__)) sdshdr32 {
    uint32_t len;   /* used */
    uint32_t alloc; /* excluding the header and null terminator */
    uint8_t  flags; /* 3 lsb of type, 5 unused bits */
    char     buf[];
};
struct __attribute__ ((__packed__)) sdshdr64 {
    uint64_t len;   /* used */
    uint64_t alloc; /* excluding the header and null terminator */
    uint8_t  flags; /* 3 lsb of type, 5 unused bits */
    char     buf[];
};

typedef char *sds;

#define SDS_TYPE_5    0
#define SDS_TYPE_8    1
#define SDS_TYPE_16   2
#define SDS_TYPE_32   3
#define SDS_TYPE_64   4
#define SDS_TYPE_MASK 7
#define SDS_TYPE_BITS 3
#define SDS_HDR_VAR(T,s)  struct sdshdr##T *sh = (void*)((s)-(sizeof(struct sdshdr##T)));
#define SDS_HDR(T,s)      ((struct sdshdr##T *)((s)-(sizeof(struct sdshdr##T))))
#define SDS_TYPE_5_LEN(f) ((f)>>SDS_TYPE_BITS)

static inline size_t _sdslen(const sds s) {
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5 : return SDS_TYPE_5_LEN(flags);
        case SDS_TYPE_8 : return SDS_HDR( 8,s)->len;
        case SDS_TYPE_16: return SDS_HDR(16,s)->len;
        case SDS_TYPE_32: return SDS_HDR(32,s)->len;
        case SDS_TYPE_64: return SDS_HDR(64,s)->len;
    }
    return 0;
}

static inline size_t _sdsavail(const sds s) {
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5 : {                   return 0;                  }
        case SDS_TYPE_8 : { SDS_HDR_VAR( 8,s);return sh->alloc - sh->len;}
        case SDS_TYPE_16: { SDS_HDR_VAR(16,s);return sh->alloc - sh->len;}
        case SDS_TYPE_32: { SDS_HDR_VAR(32,s);return sh->alloc - sh->len;}
        case SDS_TYPE_64: { SDS_HDR_VAR(64,s);return sh->alloc - sh->len;}
    }
    return 0;
}

static inline void _sdssetlen(sds s, size_t newlen) {
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5 : {unsigned char *fp = ((unsigned char*)s)-1;*fp = SDS_TYPE_5 | (newlen << SDS_TYPE_BITS);}break;
        case SDS_TYPE_8 : SDS_HDR( 8,s)->len = newlen; break;
        case SDS_TYPE_16: SDS_HDR(16,s)->len = newlen; break;
        case SDS_TYPE_32: SDS_HDR(32,s)->len = newlen; break;
        case SDS_TYPE_64: SDS_HDR(64,s)->len = newlen; break;
    }
}

static inline void _sdsinclen(sds s, size_t inc) {
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5:{unsigned char *fp = ((unsigned char*)s)-1;unsigned char newlen = SDS_TYPE_5_LEN(flags)+inc;*fp = SDS_TYPE_5 | (newlen << SDS_TYPE_BITS);}break;
        case SDS_TYPE_8 : SDS_HDR( 8,s)->len += inc;break;
        case SDS_TYPE_16: SDS_HDR(16,s)->len += inc;break;
        case SDS_TYPE_32: SDS_HDR(32,s)->len += inc;break;
        case SDS_TYPE_64: SDS_HDR(64,s)->len += inc;break;
    }
}

static inline void _sdsdeclen(sds s, size_t dec) {
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5:{unsigned char *fp = ((unsigned char*)s)-1;unsigned char newlen = SDS_TYPE_5_LEN(flags)-dec;*fp = SDS_TYPE_5 | (newlen << SDS_TYPE_BITS);}break;
        case SDS_TYPE_8 : SDS_HDR( 8,s)->len -= dec;break;
        case SDS_TYPE_16: SDS_HDR(16,s)->len -= dec;break;
        case SDS_TYPE_32: SDS_HDR(32,s)->len -= dec;break;
        case SDS_TYPE_64: SDS_HDR(64,s)->len -= dec;break;
    }
}

static inline void _sdsincrlen(sds s, int incr) {
    unsigned char flags = s[-1];
    size_t len;
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5: {
            unsigned char *fp = ((unsigned char*)s)-1;
            unsigned char oldlen = SDS_TYPE_5_LEN(flags);
            assert((incr > 0 && oldlen+incr < 32) || (incr < 0 && oldlen >= (unsigned int)(-incr)));
            *fp = SDS_TYPE_5 | ((oldlen+incr) << SDS_TYPE_BITS);
            len = oldlen+incr;
            break;
        }
        case SDS_TYPE_8: {
            SDS_HDR_VAR(8,s);
            assert((incr >= 0 && sh->alloc-sh->len >= incr) || (incr < 0 && sh->len >= (unsigned int)(-incr)));
            len = (sh->len += incr);
            break;
        }
        case SDS_TYPE_16: {
            SDS_HDR_VAR(16,s);
            assert((incr >= 0 && sh->alloc-sh->len >= incr) || (incr < 0 && sh->len >= (unsigned int)(-incr)));
            len = (sh->len += incr);
            break;
        }
        case SDS_TYPE_32: {
            SDS_HDR_VAR(32,s);
            assert((incr >= 0 && sh->alloc-sh->len >= (unsigned int)incr) || (incr < 0 && sh->len >= (unsigned int)(-incr)));
            len = (sh->len += incr);
            break;
        }
        case SDS_TYPE_64: {
            SDS_HDR_VAR(64,s);
            assert((incr >= 0 && sh->alloc-sh->len >= (uint64_t)incr) || (incr < 0 && sh->len >= (uint64_t)(-incr)));
            len = (sh->len += incr);
            break;
        }
        default: len = 0; /* Just to avoid compilation warnings. */
    }
    s[len] = '\0';
}

/* sdsalloc() = sdsavail() + sdslen() */
static inline size_t _sdsalloc(const sds s) {
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5 : return SDS_TYPE_5_LEN(flags);
        case SDS_TYPE_8 : return SDS_HDR(8 ,s)->alloc;
        case SDS_TYPE_16: return SDS_HDR(16,s)->alloc;
        case SDS_TYPE_32: return SDS_HDR(32,s)->alloc;
        case SDS_TYPE_64: return SDS_HDR(64,s)->alloc;
    }
    return 0;
}

static inline void _sdssetalloc(sds s, size_t newlen) {
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5 : break; /* Nothing to do, this type has no total allocation info. */
        case SDS_TYPE_8 : SDS_HDR( 8,s)->alloc = newlen;break;
        case SDS_TYPE_16: SDS_HDR(16,s)->alloc = newlen;break;
        case SDS_TYPE_32: SDS_HDR(32,s)->alloc = newlen;break;
        case SDS_TYPE_64: SDS_HDR(64,s)->alloc = newlen;break;
    }
}

static inline int _sdsHdrSize(char type) {
    switch(type&SDS_TYPE_MASK) {
        case SDS_TYPE_5 : return sizeof(struct sdshdr5 );
        case SDS_TYPE_8 : return sizeof(struct sdshdr8 );
        case SDS_TYPE_16: return sizeof(struct sdshdr16);
        case SDS_TYPE_32: return sizeof(struct sdshdr32);
        case SDS_TYPE_64: return sizeof(struct sdshdr64);
    }
    return 0;
}

static inline char _sdsReqType(size_t string_size) {
    if (string_size < 1<<5)    return SDS_TYPE_5;
    if (string_size < 1<<8)    return SDS_TYPE_8;
    if (string_size < 1<<16)   return SDS_TYPE_16;
#if (LONG_MAX == LLONG_MAX)
    if (string_size < 1ll<<32) return SDS_TYPE_32;
#endif
    return SDS_TYPE_64;
}

/// -- helper ------------------------------

static sds _sdsMakeRoomFor(sds s, size_t addlen) {
    void *sh, *newsh;
    size_t avail = _sdsavail(s);
    size_t len, newlen;
    char type, oldtype = s[-1] & SDS_TYPE_MASK;
    int hdrlen;

    /* Return ASAP if there is enough space left. */
    if (avail >= addlen) return s;

    len = _sdslen(s);
    sh = (char*)s-_sdsHdrSize(oldtype);
    newlen = (len+addlen);
    if (newlen < SDS_MAX_PREALLOC)
        newlen *= 2;
    else
        newlen += SDS_MAX_PREALLOC;

    type = _sdsReqType(newlen);

    /* Don't use type 5: the user is appending to the string and type 5 is
     * not able to remember empty space, so sdsMakeRoomFor() must be called
     * at every appending operation. */
    if (type == SDS_TYPE_5) type = SDS_TYPE_8;

    hdrlen = _sdsHdrSize(type);
    if (oldtype==type) {
        newsh = s_realloc(sh, hdrlen+newlen+1);
        if (newsh == NULL) return NULL;
        s = (char*)newsh+hdrlen;
    } else {
        /* Since the header size changes, need to move the string forward,
         * and can't use realloc */
        newsh = s_malloc(hdrlen+newlen+1);
        if (newsh == NULL) return NULL;
        memcpy((char*)newsh+hdrlen, s, len+1);
        s_free(sh);
        s = (char*)newsh+hdrlen;
        s[-1] = type;
        _sdssetlen(s, len);
    }
    _sdssetalloc(s, newlen);
    return s;
}

/* Helper for sdscatlonglong() doing the actual number -> string
 * conversion. 's' must point to a string with room for at least
 * SDS_LLSTR_SIZE bytes.
 *
 * The function returns the length of the null-terminated string
 * representation stored at 's'. */
#define SDS_LLSTR_SIZE 21
static int _ll2str(char *s, long long value) {
    char *p, aux;
    unsigned long long v;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    v = (value < 0) ? -value : value;
    p = s;
    do {
        *p++ = '0'+(v%10);
        v /= 10;
    } while(v);
    if (value < 0) *p++ = '-';

    /* Compute length and add null term. */
    l = p-s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while(s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

/* Identical sdsll2str(), but for unsigned long long type. */
static int _ull2str(char *s, unsigned long long v) {
    char *p, aux;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    p = s;
    do {
        *p++ = '0'+(v%10);
        v /= 10;
    } while(v);

    /* Compute length and add null term. */
    l = p-s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while(s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

/// -- estr adapter ------------

#define _estr_reqT(s)      _sdsReqType(s)
#define _estr_lenH(t)      _sdsHdrSize(t)
#define _estr_len(s)       _sdslen(s)
#define _estr_rest(s)      _sdsavail(s)
#define _estr_cap(s)       _sdsalloc(s)
#define _estr_setLen(s,l)  _sdssetlen(s,l)
#define _estr_setCap(s,l)  _sdssetalloc(s,l)
#define _estr_incLen(s,l)  _sdsinclen(s,l)
#define _estr_decLen(s,l)  _sdsdeclen(s,l)
#define _estr_incrLen(s,l) _sdsincrlen(s,l)
#define _estr_ensure(s,l)  _sdsMakeRoomFor(s,l)

/// -- estr --------------------

estr estr_new(constr src)
{
    u64 initlen = (src == NULL) ? 0 : strlen(src);
    return estr_newLen(src, initlen);
}

estr estr_newLen(conptr ptr, size_t initlen) {
    cstr sh; sds  s; size_t datalen; char type; unsigned char *fp; /* flags pointer. */

    datalen = ptr ? initlen : 0;
    type    = _estr_reqT(initlen);

    /* Empty strings are usually created in order to append. Use type 8
     * since type 5 is not good at this. */
    if (type == SDS_TYPE_5 && datalen == 0 ) type = SDS_TYPE_8;
    int hdrlen = _estr_lenH(type);

    sh = s_malloc(hdrlen+initlen+1);
    if (sh == NULL) return NULL;

    s  = sh + hdrlen;
    fp = ((unsigned char*)s)-1;
    switch(type) {
        case SDS_TYPE_5 : {*fp = type | (initlen << SDS_TYPE_BITS); break; }
        case SDS_TYPE_8 : {SDS_HDR_VAR( 8,s); sh->len = datalen; sh->alloc = initlen; *fp = type; break; }
        case SDS_TYPE_16: {SDS_HDR_VAR(16,s); sh->len = datalen; sh->alloc = initlen; *fp = type; break; }
        case SDS_TYPE_32: {SDS_HDR_VAR(32,s); sh->len = datalen; sh->alloc = initlen; *fp = type; break; }
        case SDS_TYPE_64: {SDS_HDR_VAR(64,s); sh->len = datalen; sh->alloc = initlen; *fp = type; break; }
    }
    if (datalen) { memcpy(s, ptr, initlen); s[datalen] = '\0';}
    else           memset(s, 0, initlen+1);
    return s;
}

estr estr_fromS64(s64 val)
{
    char buf[SDS_LLSTR_SIZE];
    int len = _ll2str(buf, val);

    return estr_newLen(buf, len);
}

estr estr_fromU64(u64 val)
{
    char buf[SDS_LLSTR_SIZE];
    int len = _ull2str(buf, val);

    return estr_newLen(buf, len);
}

estr estr_dup(estr s)
{
    return estr_newLen(s, _estr_len(s));
}

/// -- estr clear or free --
void estr_clear(estr s)
{
    if(s){ _estr_setLen(s, 0); s[0] = '\0'; }
}

void estr_wipe(estr s)
{
    if(s) {memset(s, 0, _estr_len(s)); _estr_setLen(s, 0);}
}

void estr_free(estr s)
{
    is0_ret(s, );
    s_free(s - _estr_lenH(s[-1]));
}

/// -- estr show --
void estr_shows(estr s)
{
    if(!s)
    {
        printf("(nullptr)");
        return;
    }

    size_t len = _estr_len(s);
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5 : printf("(e05 %"PRIi64"/%"PRIi64"):[", len, _estr_cap(s)); break;
        case SDS_TYPE_8 : printf("(e08 %"PRIi64"/%"PRIi64"):[", len, _estr_cap(s)); break;
        case SDS_TYPE_16: printf("(e16 %"PRIi64"/%"PRIi64"):[", len, _estr_cap(s)); break;
        case SDS_TYPE_32: printf("(e32 %"PRIi64"/%"PRIi64"):[", len, _estr_cap(s)); break;
        case SDS_TYPE_64: printf("(e64 %"PRIi64"/%"PRIi64"):[", len, _estr_cap(s)); break;
    }

    if(s) printf("%s", s);

    printf("]\n");
    fflush(stdout);
}

void estr_showr(estr s)
{
    if(!s)
    {
        printf("(nullptr)");
        return;
    }

    size_t len = _estr_len(s); constr p = s;
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5 : printf("(e05 %"PRIi64"/%"PRIi64"):[", len, _estr_cap(s)); break;
        case SDS_TYPE_8 : printf("(e08 %"PRIi64"/%"PRIi64"):[", len, _estr_cap(s)); break;
        case SDS_TYPE_16: printf("(e16 %"PRIi64"/%"PRIi64"):[", len, _estr_cap(s)); break;
        case SDS_TYPE_32: printf("(e32 %"PRIi64"/%"PRIi64"):[", len, _estr_cap(s)); break;
        case SDS_TYPE_64: printf("(e64 %"PRIi64"/%"PRIi64"):[", len, _estr_cap(s)); break;
    }

    while(len--) { printf("%c", *p++);}

    printf("]\n");
    fflush(stdout);
}

/// -- estr write --
estr estr_wrs(estr s, constr src) { return src ? estr_wrb(s, src, strlen(src)  ) : s;}
estr estr_wre(estr s, estr   s2 ) { return s2  ? estr_wrb(s, s2 , _estr_len(s2)) : s;}

estr estr_wrb(estr s, conptr ptr, size_t len)
{
    is0_ret(s, 0);

    if (_estr_cap(s) < len) {
        s = _estr_ensure(s,len - _estr_len(s));
        if (s == NULL) return NULL;
    }
    memcpy(s, ptr, len);
    s[len] = '\0';
    _estr_setLen(s, len);
    return s;
}

estr estr_wrp(estr s, constr fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    s = estr_wrv(s, fmt, ap);
    va_end(ap);

    return s;
}

estr estr_wrv(estr s, constr fmt, va_list ap)
{
    va_list cpy; size_t wr_len;
    char staticbuf[1024], *buf = staticbuf, *t;

    is0_ret(s, 0);
    size_t buflen = strlen(fmt)*2;

    /* We try to start using a static buffer for speed.
     * If not possible we revert to heap allocation. */
    if (buflen > sizeof(staticbuf)) {
        buf = s_malloc(buflen);
        if (buf == NULL) return NULL;
    } else {
        buflen = sizeof(staticbuf);
    }

    /* Try with buffers two times bigger every time we fail to
     * fit the string in the current buffer size. */
    while(1) {
        buf[buflen-2] = '\0';
        va_copy(cpy,ap);
        wr_len = vsnprintf(buf, buflen, fmt, cpy);
        va_end(cpy);
        if (buf[buflen-2] != '\0') {
            if (buf != staticbuf) s_free(buf);
            buflen *= 2;
            buf = s_malloc(buflen);
            if (buf == NULL) return NULL;
            continue;
        }
        break;
    }

    /* Finally concat the obtained string to the SDS string and return it. */
    t = wr_len > 0 ? estr_wrb(s, buf, wr_len) : estr_wrs(s, buf);
    if (buf != staticbuf) s_free(buf);
    return t;
}

/* This function is similar to sdscatprintf, but much faster as it does
 * not rely on sprintf() family functions implemented by the libc that
 * are often very slow. Moreover directly handling the sds string as
 * new data is concatenated provides a performance improvement.
 *
 * However this function only handles an incompatible subset of printf-alike
 * format specifiers:
 *
 * %s - C String
 * %S - estr
 * %i - signed int
 * %I - 64 bit signed integer (long long, int64_t)
 * %u - unsigned int
 * %U - 64 bit unsigned integer (unsigned long long, uint64_t)
 * %% - Verbatim "%" character.
 */
estr estr_wrf(estr s, constr fmt, ...)     // todo : using ptr, not using index i
{
    constr f; size_t i; va_list ap;

    is0_ret(s, 0);

    _estr_setLen(s, 0);
    va_start(ap,fmt);
    f = fmt;     /* Next format specifier byte to process. */
    i = 0;       /* Position of the next byte to write to dest str. */
    while(*f) {
        char next, *str;
        size_t l;
        long long num;
        unsigned long long unum;

        /* Make sure there is always space for at least 1 char. */
        if (0 == _estr_rest(s)) {
            s = _estr_ensure(s,1);
        }

        switch(*f) {
        case '%':
            next = *(f+1);
            f++;
            switch(next) {
            case 's':
            case 'S':
                str = va_arg(ap, char*);
                l = (next == 's') ? strlen(str) : _estr_len(str);
                if (_estr_rest(s) < l) {
                    s = _estr_ensure(s,l);
                }
                memcpy(s + i, str, l);
                _estr_incLen(s, l);
                i += l;
                break;
            case 'i':
            case 'I':
                if (next == 'i')
                    num = va_arg(ap,int);
                else
                    num = va_arg(ap,long long);
                    {
                        char buf[SDS_LLSTR_SIZE];
                        l = _ll2str(buf,num);
                        if (_estr_rest(s) < l) {
                            s = _estr_ensure(s,l);
                        }
                        memcpy(s+i,buf,l);
                        _estr_incLen(s,l);
                        i += l;
                    }
                break;
            case 'u':
            case 'U':
                if (next == 'u')
                    unum = va_arg(ap,unsigned int);
                else
                    unum = va_arg(ap,unsigned long long);
                {
                    char buf[SDS_LLSTR_SIZE];
                    l = _ull2str(buf,unum);
                    if (_estr_rest(s) < l) {
                        s = _estr_ensure(s,l);
                    }
                    memcpy(s+i, buf, l);
                    _estr_incLen(s,l);
                    i += l;
                }
                break;
            default: /* Handle %% and generally %<unknown>. */
                s[i++] = next;
                _estr_incLen(s,1);
                break;
            }
            break;
        default:
            s[i++] = *f;
            _estr_incLen(s, 1);
            break;
        }
        f++;
    }
    va_end(ap);

    /* Add null-term */
    s[i] = '\0';
    return s;
}

estr estr_cats(estr s, constr src) { return src ? estr_catb(s, src, strlen(src) ) : s;}
estr estr_cate(estr s, estr   s2 ) { return s2  ? estr_catb(s, s2, _estr_len(s2)) : s;}

estr estr_catb(estr s, conptr ptr, size_t len)
{
    is0_ret(s, 0);
    size_t curlen = _estr_len(s);

    s = _estr_ensure(s, len);
    if (s == NULL) return NULL;
    memcpy(s+curlen, ptr, len);
    _estr_setLen(s, curlen+len);
    s[curlen+len] = '\0';
    return s;
}

estr estr_catp(estr s, constr fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    s = estr_catv(s, fmt, ap);
    va_end(ap);

    return s;
}

estr estr_catv(estr s, constr fmt, va_list ap)
{
    va_list cpy; size_t wr_len;
    char staticbuf[1024], *buf = staticbuf, *t;

    is0_ret(s, 0);
    size_t buflen = strlen(fmt)*2;

    /* We try to start using a static buffer for speed.
     * If not possible we revert to heap allocation. */
    if (buflen > sizeof(staticbuf)) {
        buf = s_malloc(buflen);
        if (buf == NULL) return NULL;
    } else {
        buflen = sizeof(staticbuf);
    }

    /* Try with buffers two times bigger every time we fail to
     * fit the string in the current buffer size. */
    while(1) {
        buf[buflen-2] = '\0';
        va_copy(cpy,ap);
        wr_len = vsnprintf(buf, buflen, fmt, cpy);
        va_end(cpy);
        if (buf[buflen-2] != '\0') {
            if (buf != staticbuf) s_free(buf);
            buflen *= 2;
            buf = s_malloc(buflen);
            if (buf == NULL) return NULL;
            continue;
        }
        break;
    }

    /* Finally concat the obtained string to the SDS string and return it. */
    t = wr_len > 0 ? estr_catb(s, buf, wr_len) : estr_cats(s, buf);
    if (buf != staticbuf) s_free(buf);
    return t;
}

/* This function is similar to sdscatprintf, but much faster as it does
 * not rely on sprintf() family functions implemented by the libc that
 * are often very slow. Moreover directly handling the sds string as
 * new data is concatenated provides a performance improvement.
 *
 * However this function only handles an incompatible subset of printf-alike
 * format specifiers:
 *
 * %s - C String
 * %S - estr
 * %i - signed int
 * %I - 64 bit signed integer (long long, int64_t)
 * %u - unsigned int
 * %U - 64 bit unsigned integer (unsigned long long, uint64_t)
 * %% - Verbatim "%" character.
 */
estr estr_catf(estr s, constr fmt, ...)
{
    constr f; size_t i; va_list ap;

    is0_ret(s, 0);

    va_start(ap,fmt);
    f = fmt;            /* Next format specifier byte to process. */
    i = _estr_len(s);  /* Position of the next byte to write to dest str. */
    while(*f) {
        char next, *str;
        size_t l;
        long long num;
        unsigned long long unum;

        /* Make sure there is always space for at least 1 char. */
        if (0 == _estr_rest(s)) {
            s = _estr_ensure(s,1);
        }

        switch(*f) {
        case '%':
            next = *(f+1);
            f++;
            switch(next) {
            case 's':
            case 'S':
                str = va_arg(ap, char*);
                l = (next == 's') ? strlen(str) : _estr_len(str);
                if (_estr_rest(s) < l) {
                    s = _estr_ensure(s,l);
                }
                memcpy(s + i, str, l);
                _estr_incLen(s, l);
                i += l;
                break;
            case 'i':
            case 'I':
                if (next == 'i')
                    num = va_arg(ap,int);
                else
                    num = va_arg(ap,long long);
                    {
                        char buf[SDS_LLSTR_SIZE];
                        l = _ll2str(buf,num);
                        if (_estr_rest(s) < l) {
                            s = _estr_ensure(s,l);
                        }
                        memcpy(s+i,buf,l);
                        _estr_incLen(s,l);
                        i += l;
                    }
                break;
            case 'u':
            case 'U':
                if (next == 'u')
                    unum = va_arg(ap,unsigned int);
                else
                    unum = va_arg(ap,unsigned long long);
                    {
                        char buf[SDS_LLSTR_SIZE];
                        l = _ull2str(buf,unum);
                        if (_estr_rest(s) < l) {
                            s = _estr_ensure(s,l);
                        }
                        memcpy(s+i, buf, l);
                        _estr_incLen(s,l);
                        i += l;
                    }
                break;
            default: /* Handle %% and generally %<unknown>. */
                s[i++] = next;
                _estr_incLen(s,1);
                break;
            }
            break;
        default:
            s[i++] = *f;
            _estr_incLen(s, 1);
            break;
        }
        f++;
    }
    va_end(ap);

    /* Add null-term */
    s[i] = '\0';
    return s;
}

/// -- estr adjusting --
/* Remove the part of the string from left and from right composed just of
 * contiguous characters found in 'cset', that is a null terminted C string.
 *
 * After the call, the modified sds string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call.
 *
 * Example:
 *
 * s = estr_new("AA...AA.a.aa.aHelloWorld     :::");
 * s = estr_strim(s, "Aa. :");
 * printf("%s\n", s);
 *
 * Output will be just "Hello World".
 */
estr estr_trim(estr s, constr cset)
{
    char *start, *end, *sp, *ep; size_t len;

    is0_ret(s, 0);

    sp = start = s;
    ep = end   = s + _estr_len(s) -1;
    while(sp <= end && strchr(cset, *sp)) sp++;
    while(ep >  sp  && strchr(cset, *ep)) ep--;
    len = (sp > ep) ? 0 : ((ep - sp) + 1);
    if (s != sp) memmove(s, sp, len);
    s[len] = '\0';
    _estr_setLen(s,len);
    return s;
}

/* Turn the string into a smaller (or equal) string containing only the
 * substring specified by the 'start' and 'end' indexes.
 *
 * start and end can be negative, where -1 means the last character of the
 * string, -2 the penultimate character, and so forth.
 *
 * The interval is inclusive, so the start and end characters will be part
 * of the resulting string.
 *
 * The string is modified in-place.
 *
 * Example:
 *
 * s = estr_new("Hello World");
 * estr_range(s,1,-1); => "ello World"
 */
estr estr_range(estr s, int start, int end)
{
    size_t newlen, len;

    is0_ret(s, 0);

    len = _estr_len(s);
    if (len == 0) return 0;
    if (start < 0) { start = len + start; if (start < 0) start = 0;}
    if (end   < 0) {   end = len + end  ; if (end   < 0) end   = 0;}
    newlen = (start > end) ? 0 : (end - start) + 1;
    if (newlen != 0) {
        if (start >= (signed)len) {
            newlen = 0;
        } else if (end >= (signed)len) {
            end = len-1;
            newlen = (start > end) ? 0 : (end-start)+1;
        }
    } else {
        start = 0;
    }
    if (start && newlen) memmove(s, s + start, newlen);
    s[newlen] = 0;
    _estr_setLen(s, newlen);

    return s;
}

estr estr_lower(estr s)
{
    size_t len, i;

    is0_ret(s, 0);

    len = _estr_len(s);
    for (i = 0; i < len; i++)
    {
        if(s[i] >= 'A' && s[i] <= 'Z') s[i] += 0x20;
    }

    return s;
}

estr estr_upper(estr s)
{
    size_t len, i;

    is0_ret(s, 0);

    len = _estr_len(s);
    for (i = 0; i < len; i++)
    {
        if(s[i] >= 'a' && s[i] <= 'z') s[i] -= 0x20;
    }

    return s;
}

/* Modify the string substituting all the occurrences of the set of
 * characters specified in the 'from' string to the corresponding character
 * in the 'to' array.
 *
 * For instance: estr_mapc(mystring, "ho", "01")
 * will have the effect of turning the string "hello" into "0ell1".
 *
 * The function returns the sds string pointer, that is always the same
 * as the input pointer since no resize is needed. */
estr estr_mapc (estr s, constr from, constr to)
{
    size_t j, i, l, len, len2;

    is0_ret(s, 0); is1_ret(!from || !to, s);

    len  = strlen(from);
    len2 = strlen(to);

    if(len > len2) len = len2;

    l = _estr_len(s);

    for (j = 0; j < l; j++) {
        for (i = 0; i < len; i++) {
            if (s[j] == from[i]) {
                s[j] = to[i];
                break;
            }
        }
    }
    return s;
}

estr estr_mapcl(estr s, constr from, constr to, size_t len)
{
    size_t j, i, l;

    is0_ret(s, 0);

    l = _estr_len(s);

    for (j = 0; j < l; j++) {
        for (i = 0; i < len; i++) {
            if (s[j] == from[i]) {
                s[j] = to[i];
                break;
            }
        }
    }
    return s;
}

static cptr __memmem(conptr haystack, size_t haystacklen, conptr needle, size_t needlelen);

static int _memmem(unsigned char * a, int alen, unsigned char * b, int blen)
{
    int i, ja, jb, match, off; char tag[256] = {0};

    for (i = 0; i < blen; ++ i)
    {
        tag[*(b+i)] = 1;
    }

    off = alen - blen--;
    for (i = 0; i <= off;)
    {
        for(ja = i + blen, jb = blen, match = 1; jb >= 0; --ja, --jb)
        {
            if (!tag[a[ja]])
            {
                i = ja;
                match = 0;
                break;
            }
            if (match && a[ja] != b[jb])
            {
                match = 0;
            }
        }
        if (match)
        {
            return i;
        }
        ++ i;
    }
    return -1;
}

estr estr_subs (estr s, constr from, constr to)
{
    int subLen, newLen, offLen, offNow; cstr fd_s, cp_s, end_p;

    is0_ret(s, 0); is1_ret(!from || !to, s);

    subLen = strlen(from);
    newLen = strlen(to);
    offLen = newLen - subLen;

    if(offLen < 0)
    {
        offLen = -offLen;
        offNow = 0;
        fd_s   = s;
        end_p  = s + _estr_len(s);

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
            _sdsdeclen(s, offNow);;
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
        offNow = _estr_len(s);
        fd_s   = s;

        if((fd_s   = strstr(fd_s, from)))
        {
            cstr new_s, new_p; int len;

            end_p = s + _estr_len(s);

            // -- get len need to expand
            offNow += offLen; fd_s += subLen;
            while((fd_s = strstr(fd_s, from)))
            {
                offNow += offLen; fd_s += subLen;
            }

            is0_exeret(new_s = estr_newLen(0, offNow), s_free((char*)s - _estr_lenH(s[-1]));, 0);  // new str

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

            s_free((char*)s - _estr_lenH(s[-1]));
            _estr_setLen(new_s, offNow);

            return new_s;
        }
    }

    return s;
}

/// -- estr cmp ---
int  estr_cmps(estr s, constr src )
{
    size_t l1, l2, minlen;
    int cmp;

    is1_ret(s == src, 0);

    l1 = s   ? _estr_len(s) : 0;
    l2 = src ? strlen(src)  : 0;
    minlen = (l1 < l2) ? l1 : l2;
    cmp = memcmp(s, src, minlen);
    if (cmp == 0) return l1-l2;
    return cmp;
}

int  estr_cmpe(estr s, estr   s2)
{
    size_t l1, l2, minlen;
    int cmp;

    is1_ret(s == s2, 0);

    l1 = s  ? _estr_len(s ) : 0;
    l2 = s2 ? _estr_len(s2) : 0;
    minlen = (l1 < l2) ? l1 : l2;
    cmp = memcmp(s, s2, minlen);
    if (cmp == 0) return l1-l2;
    return cmp;
}

/// -- estr len --
u64  estr_len (estr s) { return s ? _estr_len(s) : 0; }
u64  estr_cap (estr s) { return s ? _estr_cap(s) : 0; }

/// -- Low level functions exposed to the user API --
estr estr_ensure (estr s, size_t addlen)
{
    return s ? _estr_ensure(s, addlen) : 0;
}

/* Increment the sds length and decrements the left free space at the
 * end of the string according to 'incr'. Also set the null term
 * in the new end of the string.
 *
 * This function is used in order to fix the string length after the
 * user calls sdsMakeRoomFor(), writes something after the end of
 * the current string, and finally needs to set the new length.
 *
 * Note: it is possible to use a negative increment in order to
 * right-trim the string.
 *
 * Usage example:
 *
 * Using sdsIncrLen() and sdsMakeRoomFor() it is possible to mount the
 * following schema, to cat bytes coming from the kernel to the end of an
 * sds string without copying into an intermediate buffer:
 *
 * oldlen = estr_len(s);
 * s = estr_ensure(s, BUFFER_SIZE);
 * nread = read(fd, s+oldlen, BUFFER_SIZE);
 * ... check for nread <= 0 and handle it ...
 * estr_incrLen(s, nread);
 */
void estr_incrLen(estr s, size_t incr)
{
    if(s) _estr_incrLen(s, incr);
}

estr estr_shrink (estr s)
{
    void *sh, *newsh;
    char type, oldtype = s[-1] & SDS_TYPE_MASK;
    int hdrlen;
    size_t len = _estr_len(s);
    sh = (char*)s - _estr_lenH(oldtype);

    type   = _estr_reqT(len);
    hdrlen = _estr_lenH(type);
    if (oldtype==type) {
        newsh = s_realloc(sh, hdrlen+len+1);
        if (newsh == NULL) return NULL;
        s = (char*)newsh+hdrlen;
    } else {
        newsh = s_malloc(hdrlen+len+1);
        if (newsh == NULL) return NULL;
        memcpy((char*)newsh+hdrlen, s, len+1);
        s_free(sh);
        s = (char*)newsh+hdrlen;
        s[-1] = type;
        _estr_setLen(s, len);
    }
    _estr_setCap(s, len);
    return s;
}
