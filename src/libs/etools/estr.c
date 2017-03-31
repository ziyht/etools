/// =====================================================================================
///
///       Filename:  estr.c
///
///    Description:  a easier way to handle string in C, rebuild based on sds from redis,
///                  including three tools:
///                     estr - for heap using
///                     sstr - for stack using
///                     ebuf - using estr with a fixed handle
///
///        Version:  1.0
///        Created:  02/25/2017 08:51:34 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>
#include <inttypes.h>
#include <unistd.h>
#include <termios.h>        // tcsetattr()
#include <errno.h>

#include "estr.h"

#define exe_ret(expr, ret ) { expr;      return ret;}
#define is0_ret(cond, ret ) if(!(cond)){ return ret;}
#define is1_ret(cond, ret ) if( (cond)){ return ret;}
#define is0_exe(cond, expr) if(!(cond)){ expr;}
#define is1_exe(cond, expr) if( (cond)){ expr;}

#define is0_exeret(cond, expr, ret) if(!(cond)){ expr;        return ret;}
#define is1_exeret(cond, expr, ret) if( (cond)){ expr;        return ret;}
#define is0_elsret(cond, expr, ret) if(!(cond)){ expr;} else{ return ret;}
#define is1_elsret(cond, expr, ret) if( (cond)){ expr;} else{ return ret;}

/// -- sds from redis -----------------------------

#define s_malloc  malloc
#define s_calloc  calloc
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

#define SDS_TYPE_5     0
#define SDS_TYPE_8     1
#define SDS_TYPE_16    2
#define SDS_TYPE_32    3
#define SDS_TYPE_64    4
#define SDS_TYPE_BITS  3
#define SDS_TYPE_MASK  7    // 0000 0111
#define SDS_STACK_MASK 8    // 0000 1000

#define SDS_TYPE(s)       (s)[-1]
#define SDS_HDR_VAR(T,s)  struct sdshdr##T *sh = (void*)((s)-(sizeof(struct sdshdr##T)));
#define SDS_HDR(T,s)      ((struct sdshdr##T *)((s)-(sizeof(struct sdshdr##T))))
#define SDS_TYPE_5_LEN(f) ((f)>>SDS_TYPE_BITS)

static inline size_t _sdslen(const sds s) {
    unsigned char flags = SDS_TYPE(s);
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
    unsigned char flags = SDS_TYPE(s);
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
    unsigned char flags = SDS_TYPE(s);
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5 : {unsigned char *fp = ((unsigned char*)s)-1;*fp = SDS_TYPE_5 | (newlen << SDS_TYPE_BITS);}break;
        case SDS_TYPE_8 : SDS_HDR( 8,s)->len = newlen; break;
        case SDS_TYPE_16: SDS_HDR(16,s)->len = newlen; break;
        case SDS_TYPE_32: SDS_HDR(32,s)->len = newlen; break;
        case SDS_TYPE_64: SDS_HDR(64,s)->len = newlen; break;
    }
}

static inline void _sdsinclen(sds s, size_t inc) {
    unsigned char flags = SDS_TYPE(s);
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5:{unsigned char *fp = ((unsigned char*)s)-1;unsigned char newlen = SDS_TYPE_5_LEN(flags)+inc;*fp = SDS_TYPE_5 | (newlen << SDS_TYPE_BITS);}break;
        case SDS_TYPE_8 : SDS_HDR( 8,s)->len += inc;break;
        case SDS_TYPE_16: SDS_HDR(16,s)->len += inc;break;
        case SDS_TYPE_32: SDS_HDR(32,s)->len += inc;break;
        case SDS_TYPE_64: SDS_HDR(64,s)->len += inc;break;
    }
}

static inline void _sdsdeclen(sds s, size_t dec) {
    unsigned char flags = SDS_TYPE(s);
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5:{unsigned char *fp = ((unsigned char*)s)-1;unsigned char newlen = SDS_TYPE_5_LEN(flags)-dec;*fp = SDS_TYPE_5 | (newlen << SDS_TYPE_BITS);}break;
        case SDS_TYPE_8 : SDS_HDR( 8,s)->len -= dec;break;
        case SDS_TYPE_16: SDS_HDR(16,s)->len -= dec;break;
        case SDS_TYPE_32: SDS_HDR(32,s)->len -= dec;break;
        case SDS_TYPE_64: SDS_HDR(64,s)->len -= dec;break;
    }
}

static inline void _sdsincrlen(sds s, int incr) {
    unsigned char flags = SDS_TYPE(s);
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
    unsigned char flags = SDS_TYPE(s);
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
    unsigned char flags = SDS_TYPE(s);
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

static sds _sdsNewRoom(size_t len)
{
    cstr sh; sds s; char type; int hdrlen;

    if(len < 1024) len = len * 1.2;

    type = _sdsReqType(len);

    /* Don't use type 5: the user is appending to the string and type 5 is
     * not able to remember empty space, so sdsMakeRoomFor() must be called
     * at every appending operation. */

    if (type == SDS_TYPE_5) type = SDS_TYPE_8;
    hdrlen = _sdsHdrSize(type);

    is0_ret(sh = s_calloc(1, hdrlen+len+1), 0);

    s = sh + hdrlen;
    SDS_TYPE(s) = type;
    _sdssetalloc(s, len);

    return s;
}

static sds _sdsMakeRoomFor(sds s, size_t addlen) {
    void *sh, *newsh; size_t len, newlen, avail; char type, oldtype; int hdrlen;

    avail   = _sdsavail(s);
    oldtype = SDS_TYPE(s) & SDS_TYPE_MASK;

    /* Return ASAP if there is enough space left. */
    if (avail >= addlen) return s;

    len = _sdslen(s);
    sh = (char*)s-_sdsHdrSize(oldtype);
    newlen = (len+addlen);
    if (newlen < SDS_MAX_PREALLOC) newlen *= 2;
    else                           newlen += SDS_MAX_PREALLOC;

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
        SDS_TYPE(s) = type;
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

/// =====================================================================================
/// estr
/// =====================================================================================

/// -- estr adapter -------------------------------------

#define _estr_new(l)       _sdsNewRoom(l)
#define _estr_reqT(s)      _sdsReqType(s)
#define _estr_lenH(t)      _sdsHdrSize(t)
#define _estr_len(s)       _sdslen(s)
#define _estr_rest(s)      _sdsavail(s)
#define _estr_cap(s)       _sdsalloc(s)
#define _estr_setLen(s,l)  _sdssetlen(s,l)
#define _estr_setCap(s,l)  _sdssetalloc(s,l)
#define _estr_incLen(s,l)  _sdsinclen(s,l)      //  for inner using
#define _estr_decLen(s,l)  _sdsdeclen(s,l)
#define _estr_incrLen(s,l) _sdsincrlen(s,l)     // for Low level functions using
#define _estr_ensure(s,l)  _sdsMakeRoomFor(s,l)
#define _estr_setStack(s)  SDS_TYPE(s) |= SDS_STACK_MASK
#define _estr_isStack(s)   ((SDS_TYPE(s) & SDS_STACK_MASK) && (SDS_TYPE_5 != (SDS_TYPE(s)&SDS_TYPE_MASK)))

static inline void _show(constr tag, estr s)
{
    size_t len; u8 flags;

    if(!s)
    {
        printf("(%s: nullptr)", tag);fflush(stdout);
        return;
    }

    tag = tag[1] == 'b' ? _estr_isStack(s) ? "ebuf: sstr" : "ebuf: estr"
                        : _estr_isStack(s) ? "sstr" : "estr" ;

    len   = _estr_len(s);
    flags = SDS_TYPE(s);
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5 : printf("(%s: e05 %"PRIi64"/%"PRIi64"):[", tag, len, _estr_cap(s)); break;
        case SDS_TYPE_8 : printf("(%s: e08 %"PRIi64"/%"PRIi64"):[", tag, len, _estr_cap(s)); break;
        case SDS_TYPE_16: printf("(%s: e16 %"PRIi64"/%"PRIi64"):[", tag, len, _estr_cap(s)); break;
        case SDS_TYPE_32: printf("(%s: e32 %"PRIi64"/%"PRIi64"):[", tag, len, _estr_cap(s)); break;
        case SDS_TYPE_64: printf("(%s: e64 %"PRIi64"/%"PRIi64"):[", tag, len, _estr_cap(s)); break;
    }
    fflush(stdout);

    write(STDOUT_FILENO, s, len);

    printf("]\n");
    fflush(stdout);
}

/// -- estr creator and destroyer -----------------------

inline estr estr_new(constr src)
{
    size_t initlen = (src == NULL) ? 0 : strlen(src);
    return estr_newLen(src, initlen);
}

estr estr_newLen(conptr ptr, size initlen) {
    cstr sh; sds  s; size_t datalen; char type; int hdrlen; unsigned char *fp; /* flags pointer. */

    datalen = ptr ? initlen : 0;
    type    = _estr_reqT(initlen);

    /* Empty strings are usually created in order to append. Use type 8
     * since type 5 is not good at this. */
    if (type == SDS_TYPE_5 && datalen == 0 ) type = SDS_TYPE_8;
    hdrlen = _estr_lenH(type);
    sh = s_malloc(hdrlen + initlen + 1);
    is0_ret(sh, 0);

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

/// @brief set_disp_mode
/// @param fd     : STDIN_FILENO for stdin
/// @param option : 0: off, 1: on ;
/// @return [0] - set ok
///         [1] - set err
///
#define ECHOFLAGS (ECHO | ECHOE | ECHOK | ECHONL)
static inline int __set_disp_mode(int fd, int option)
{
   int err; struct termios term;

   if(tcgetattr(fd, &term) == -1)
   {
        perror("Cannot get the attribution of the terminal");
        return 1;
   }
   if(option) term.c_lflag |=  ECHOFLAGS;
   else       term.c_lflag &= ~ECHOFLAGS;

   err = tcsetattr(fd, TCSAFLUSH, &term);

   if(err == -1 && err == EINTR){
        perror("Cannot set the attribution of the terminal");
        return 1;
   }

   return 0;
}

estr estr_fromInput(constr tag, int code)
{
    char c; estr input;

    input = estr_newLen(0, 16);

    is0_ret(input, 0);

    if(tag)  write(STDOUT_FILENO, tag, strlen(tag));
    if(code) __set_disp_mode(STDIN_FILENO, 0);

    do{
       c = getchar();
       if (c != '\n' && c !='\r'){
         input = estr_catB(input, &c, 1);

       }
    }while(c != '\n' && c !='\r');

    if(code)
    {
        __set_disp_mode(STDIN_FILENO, 1);
        write(STDOUT_FILENO, "\n", 1);
    }

    return input;
}

inline estr estr_dup(estr s) { return estr_newLen(s, _estr_len(s)); }

inline void estr_free(estr s) { if(s){ if(!_estr_isStack(s))s_free(s - _estr_lenH(SDS_TYPE(s)));} }

/// -- estr writer --------------------------------------

inline estr estr_wrt (estr s, estr   s2 ) { return s2  ? estr_wrtB(s, s2 , _estr_len(s2)) : s; }
inline estr estr_wrtS(estr s, constr src) { return src ? estr_wrtB(s, src, strlen(src)  ) : s; }

estr estr_wrtB(estr s, conptr ptr, size len)
{
    is0_exe(s, is0_ret(s = _estr_new(len), 0));

    if (_estr_cap(s) < len) {
        s = _estr_ensure(s,len - _estr_len(s));
        if (s == NULL) return NULL;
    }
    memcpy(s, ptr, len);
    s[len] = '\0';
    _estr_setLen(s, len);
    return s;
}

estr estr_wrtV(estr s, constr fmt, va_list ap)
{
    va_list cpy; size_t wr_len;
    char staticbuf[1024], *buf = staticbuf, *t;

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
    t = wr_len > 0 ? estr_wrtB(s, buf, wr_len) : estr_wrtS(s, buf);
    if (buf != staticbuf) s_free(buf);
    return t;
}

estr estr_wrtP(estr s, constr fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    s = estr_wrtV(s, fmt, ap);
    va_end(ap);

    return s;
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
estr estr_wrtF(estr s, constr fmt, ...)     // todo : using ptr, not using index i
{
    constr f; size_t i; va_list ap;

    is0_exe(s, is0_ret(s = _estr_new(strlen(fmt) * 2), 0));

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

inline estr estr_cat (estr s, estr   s2 ) { return s2  ? estr_catB(s, s2, _estr_len(s2)) : s; }
inline estr estr_catS(estr s, constr src) { return src ? estr_catB(s, src, strlen(src) ) : s; }


estr estr_catB(estr s, conptr ptr, size len)
{
    is0_exe(s, is0_ret(s = _estr_new(len), 0));

    size_t curlen = _estr_len(s);

    s = _estr_ensure(s, len);
    if (s == NULL) return NULL;
    memcpy(s+curlen, ptr, len);
    _estr_setLen(s, curlen+len);
    s[curlen+len] = '\0';
    return s;
}

estr estr_catV(estr s, constr fmt, va_list ap)
{
    va_list cpy; size_t wr_len;
    char staticbuf[1024], *buf = staticbuf, *t;

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
    t = wr_len > 0 ? estr_catB(s, buf, wr_len) : estr_catS(s, buf);
    if (buf != staticbuf) s_free(buf);
    return t;
}

estr estr_catP(estr s, constr fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    s = estr_catV(s, fmt, ap);
    va_end(ap);

    return s;
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
estr estr_catF(estr s, constr fmt, ...)
{
    constr f; size_t i; va_list ap;

    is0_exe(s, is0_ret(s = _estr_new(strlen(fmt) * 2), 0));

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

estr estr_setT(estr s, char    c)
{
    int pos;

    is0_ret(s, NULL);

    pos = _estr_len(s);
    if(pos)
    {
        s[pos - 1] = c;

        if(c == '\0')
            _estr_decLen(s, 1);
    }

    return s;
}

inline void estr_clear(estr s) { if(s){ s[0] = '\0';                _estr_setLen(s, 0);} }
inline void estr_wipe (estr s) { if(s){ memset(s, 0, _estr_len(s)); _estr_setLen(s, 0);} }

/// -- estr utils ---------------------------------------
inline void estr_show(estr s) { _show("estr", s); }

inline size estr_len (estr s) { return s ? _estr_len(s) : 0; }
inline size estr_cap (estr s) { return s ? _estr_cap(s) : 0; }

int  estr_cmp (estr s, estr   s2)
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

int  estr_cmpS(estr s, constr src )
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
inline estr estr_trim(estr s, constr cset)
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

inline estr estr_lower(estr s)
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

inline estr estr_upper(estr s)
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
inline estr estr_range(estr s, int start, int end)
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

/* Modify the string substituting all the occurrences of the set of
 * characters specified in the 'from' string to the corresponding character
 * in the 'to' array.
 *
 * For instance: estr_mapc(mystring, "ho", "01")
 * will have the effect of turning the string "hello" into "0ell1".
 *
 * The function returns the sds string pointer, that is always the same
 * as the input pointer since no resize is needed. */
inline estr estr_mapc (estr s, constr from, constr to)
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

inline estr estr_mapcl(estr s, constr from, constr to, size_t len)
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

/// @note: tolen > fromlen needed
static void __str_replace(cstr s, cstr* end, cstr* last, constr from, size fromlen, constr to, size tolen)
{
    cstr fd;

    if((fd = strstr(s, from)))
    {
        cstr mv_from, mv_to; size mv_len;

        __str_replace(fd + 1, end, last, from, fromlen, to, tolen);

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
            end_p = s + offNow;

            // -- get len need to expand
            offNow += offLen; fd_s += subLen;
            while((fd_s = strstr(fd_s, from)))
            {
                offNow += offLen; fd_s += subLen;
            }

            // -- have enough place, let's do it, we set the up limit of stack call is 4096
            if((size)offNow <= _estr_cap(s) && ((offNow - (end_p - s)) / offLen) <= 4096)
            {
                cstr last = s + offNow;
                __str_replace(s, &end_p, &last, from, subLen, to, newLen);
                _estr_setLen(s, offNow);
                s[offNow] = '\0';
                return s;
            }
            else
            {
                cstr new_s, new_p; int len;

                //is0_exeret(new_s = estr_newLen(0, offNow), s_free((char*)s - _estr_lenH(SDS_TYPE(s)));, 0);  // new str
                is0_exeret(new_s = _estr_new(offNow), s_free((char*)s - _estr_lenH(SDS_TYPE(s)));, 0);  // new str

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

                s_free((char*)s - _estr_lenH(SDS_TYPE(s)));
                _estr_setLen(new_s, offNow);

                return new_s;
            }
        }
    }

    return s;
}

estr estr_join (char** argv, int argc, constr sep)
{
    int i, seplen; estr join = 0;

    seplen = sep ? strlen(sep) : 0;

    for (i = 0; i < argc; i++)
    {
        join = estr_catS(join, argv[i]);

        if (seplen && i != argc-1) join = estr_catB(join, sep, seplen);
    }
    return join;
}

typedef struct __split_s{
    estr src;
     int cnt;
    estr list[];
}__split_t;

#define KEEP_ADJOINING 1
#define SKIP_ADJOINING 0

///
/// @note:
///     need len > 0
///
static estr* __estr_splitbin(conptr s, int len, conptr sep, int seplen, int* _cnt, int keep_same)
{
    estr s_dup; estr o_buf; int cnt, prev_pos, j, search_len;

    is0_ret(s_dup = estr_newLen(s, len), NULL);
    o_buf = estr_newLen(0, sizeof(__split_t) + 5 * sizeof(estr));
    is0_exeret(o_buf, estr_free(s_dup), 0);

    _estr_incLen(o_buf, sizeof(__split_t));

    search_len = (len - (seplen - 1));
    prev_pos   = 0;
    cnt        = 0;

    if(seplen == 1)
    {
        char c = *(cstr)sep;
        for (j = 0; j < search_len; j++)
        {
            if (s_dup[j] == c) {
                o_buf = _estr_ensure(o_buf, sizeof(estr) * 2);
                is0_exeret(o_buf, estr_free(s_dup), 0);

                if(!keep_same)
                {
                    if(j != prev_pos)
                    {
                        ((__split_t*)o_buf)->list[cnt++] = s_dup + prev_pos;
                        _estr_incLen(o_buf, sizeof(estr));
                    }
                }
                else
                {
                    ((__split_t*)o_buf)->list[cnt++] = s_dup + prev_pos;
                    _estr_incLen(o_buf, sizeof(estr));
                }

                s_dup[j] = '\0';
                prev_pos = j + 1;
            }
        }
    }
    else if(seplen > 1)
    {
        for (j = 0; j < search_len; j++)
        {
            if (0 == memcmp(s_dup + j, sep, seplen)) {
                o_buf = _estr_ensure(o_buf, sizeof(estr) * 2);
                is0_exeret(o_buf, estr_free(s_dup), 0);

                if(!keep_same)
                {
                    if(j != prev_pos)
                    {
                        ((__split_t*)o_buf)->list[cnt++] = s_dup + prev_pos;
                        _estr_incLen(o_buf, sizeof(estr));
                    }
                }
                else
                {
                    ((__split_t*)o_buf)->list[cnt++] = s_dup + prev_pos;
                    _estr_incLen(o_buf, sizeof(estr));
                }

                s_dup[j] = '\0';
                j        = j + seplen -1;
                prev_pos = j + 1;

            }
        }
    }

    // -- Add the final element. We are sure there is room in the tokens array.
    ((__split_t*)o_buf)->list[cnt] = s_dup + prev_pos;
    ((__split_t*)o_buf)->src = s_dup;
    ((__split_t*)o_buf)->cnt = *_cnt = cnt + 1;

    return ((__split_t*)o_buf)->list;
}

// todo:  macro SKIP_SAME is not good
estr*estr_split(estr s, constr sep, int* _cnt)
{
    int len, seplen, cnt; estr* split_list;

    len    = s   ? _estr_len(s): 0;
    seplen = sep ? strlen(sep) : 0;

    is0_exeret(len, is1_exe(_cnt, *_cnt = 0), NULL);

    split_list = __estr_splitbin(s, len, sep, seplen, &cnt, SKIP_ADJOINING);

    is1_exe(_cnt, *_cnt = cnt);

    return split_list;
}

estr*estr_splitS(constr src, constr sep, int* _cnt)
{
    int len, seplen, cnt; estr* split_list;

    len    = src ? strlen(src) : 0;
    seplen = sep ? strlen(sep) : 0;

    is0_exeret(len, is1_exe(_cnt, *_cnt = 0), NULL);

    split_list = __estr_splitbin(src, len, sep, seplen, &cnt, SKIP_ADJOINING);

    is1_exe(_cnt, *_cnt = cnt);

    return split_list;
}

estr*estr_splitLen(conptr s, int len, conptr sep, int seplen, int* _cnt)
{
    estr* split_list; int cnt;

    is1_exeret(len <= 0 || seplen < 0, is1_exe(_cnt, *_cnt = 0), NULL);

    split_list = __estr_splitbin(s, len, sep, seplen, &cnt, KEEP_ADJOINING);

    is1_exe(_cnt, *_cnt = cnt);

    return split_list;
}

static inline int __is_hex_digit(char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
static inline int __hex_digit_to_int(char c) {
    switch(c) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': case 'A': return 10;
    case 'b': case 'B': return 11;
    case 'c': case 'C': return 12;
    case 'd': case 'D': return 13;
    case 'e': case 'E': return 14;
    case 'f': case 'F': return 15;
    default: return 0;
    }
}

static estr* __estr_splitArgsLine(constr src, int len, int argc, int* _cnt, int dup_src)
{
    estr s_dup; estr o_buf; constr p; cstr c_arg, wr_p; int cnt; int inq, insq, done;

    is0_ret(s_dup = dup_src ? estr_newLen(src, len) : (estr)src, NULL);
    if(argc == 0) argc = 1;

    o_buf = estr_newLen(0, sizeof(__split_t) + argc * sizeof(estr));
    is0_exeret(o_buf, estr_free(s_dup), 0);

    _estr_incLen(o_buf, sizeof(__split_t));

    cnt = 0;
    p   = s_dup;
    while(1)
    {
        /* skip blanks */
        while(*p && isspace(*p)) p++;
        if (*p) {
            c_arg = wr_p = (cstr)p;

            if(*c_arg == '\"' || *c_arg == '\'') c_arg++;

            /* get a token */
            inq  = 0;  /* set to 1 if we are in "quotes" */
            insq = 0;  /* set to 1 if we are in 'single quotes' */
            done = 0;

            while(!done)
            {
                if (inq) {
                    if (*p == '\\' && *(p+1) == 'x' && __is_hex_digit(*(p+2)) &&
                                                       __is_hex_digit(*(p+3)))
                    {
                        unsigned char byte;

                        byte = (__hex_digit_to_int(*(p+2))*16)+
                                __hex_digit_to_int(*(p+3));

                        *wr_p = byte;
                        p += 3;
                    } else if (*p == '\\' && *(p+1)) {
                        char c;

                        p++;
                        switch(*p) {
                            case 'n': c = '\n'; break;
                            case 'r': c = '\r'; break;
                            case 't': c = '\t'; break;
                            case 'b': c = '\b'; break;
                            case 'a': c = '\a'; break;
                            default : c = *p  ; break;
                        }
                        *wr_p = c;
                        //current = sdscatlen(current,&c,1);
                    } else if (*p == '"') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        if(wr_p != p) *wr_p = *p;
                        //current = sdscatlen(current,p,1);
                    }
                } else if (insq) {
                    if (*p == '\\' && *(p+1) == '\'') {
                        p++;
                        *wr_p = '\'';
                        //current = sdscatlen(current,"'",1);
                    } else if (*p == '\'') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        if(wr_p != p) *wr_p = *p;
                        //current = sdscatlen(current,p,1);
                    }
                } else {
                    switch(*p) {
                        case ' ' :
                        case '\n':
                        case '\r':
                        case '\t':
                        case '\0': done = 1; break;
                        case '"' : inq  = 1; break;
                        case '\'': insq = 1; break;
                        default  : break;
                    }
                }

                if (*p)
                {
                    p++;
                    if(done) *wr_p = '\0';
                    else      wr_p++;
                }
            }

            /* add the token to the vector */
            o_buf = _estr_ensure(o_buf, sizeof(estr) * 1);
            is0_exeret(o_buf, estr_free(s_dup), NULL);

            ((__split_t*)o_buf)->list[cnt++] = c_arg;
            _estr_incLen(o_buf, sizeof(estr));
        }
        else
            break;
    }

    ((__split_t*)o_buf)->src = s_dup;
    ((__split_t*)o_buf)->cnt = *_cnt = cnt;

    return ((__split_t*)o_buf)->list;

err:
    estr_free(s_dup);
    estr_free(o_buf);
    return NULL;
}

estr*estr_splitArgv(char** argv, int argc, int* _cnt)
{
    estr line; int cnt; estr* split_list;

    line = estr_join(argv, argc, " ");
    is0_ret(line, NULL);

    split_list = __estr_splitArgsLine(line, _estr_len(line), argc, &cnt, 0);

    is1_exe(_cnt, *_cnt = cnt);

    return split_list;
}

estr*estr_splitArgs(constr line, int*argc)
{
    int cnt, len; estr* split_list;

    is0_ret(len = line ? strlen(line) : 0, NULL);

    split_list = __estr_splitArgsLine(line, len, 0, &cnt, 1);

    is1_exe(argc, *argc = cnt);

    return split_list;
}

void estr_showSplit(estr* _s, int max)
{
    __split_t* _split; int i, cnt;

    is0_exeret(_s, printf("(estr_split 0/0): nullptr\n"); fflush(stdout), );

    _split = (__split_t*)_s - 1;

    if(max == -1)   cnt = _split->cnt;
    else            cnt = max < _split->cnt ? max : _split->cnt;

    //_show("estr", _split);
    printf("(estr_split %d/%d):\n", cnt, _split->cnt); fflush(stdout);
    for(i = 0; i < cnt; i++)
    {
        printf("%2d: %s\n", i + 1, _split->list[i]); fflush(stdout);
    }
}

void estr_freeSplit(estr* _s, int cnt)
{
    __split_t* _split;

    is0_ret(_s, );

    _split = (__split_t*)_s - 1;

    if(cnt)
    {
        assert(_split->cnt == cnt);
    }

    estr_free(_split->src);
    estr_free((estr)_split);
}

/// -- Low level functions exposed to the user API ------
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
    char type, oldtype = SDS_TYPE(s) & SDS_TYPE_MASK;
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
        SDS_TYPE(s) = type;
        _estr_setLen(s, len);
    }
    _estr_setCap(s, len);
    return s;
}

/// =====================================================================================
/// sstr
/// =====================================================================================

/// -- sstr initializer ---------------------------------
sstr sstr_init(cptr buf, uint len)
{
    cstr sh; sds  s; size_t cap; char type; unsigned char *fp; /* flags pointer. */

    is0_ret(buf, 0); is0_ret(len > 4, 0);

    type    = _estr_reqT(len);

    if (type == SDS_TYPE_5) type = SDS_TYPE_8;
    int hdrlen = _estr_lenH(type);
    cap = len - hdrlen;

    s  = (cstr)buf + hdrlen;
    fp = ((unsigned char*)s)-1;
    switch(type) {
        case SDS_TYPE_8 : {SDS_HDR_VAR( 8,s); sh->len = 0; sh->alloc = cap - 1; *fp = type ; break; }
        case SDS_TYPE_16: {SDS_HDR_VAR(16,s); sh->len = 0; sh->alloc = cap - 1; *fp = type; break; }
        case SDS_TYPE_32: {SDS_HDR_VAR(32,s); sh->len = 0; sh->alloc = cap - 1; *fp = type; break; }
        case SDS_TYPE_64: {SDS_HDR_VAR(64,s); sh->len = 0; sh->alloc = cap - 1; *fp = type; break; }
    }

    //memset(s, 0, cap);
    _estr_setStack(s);
    s[0] = 0;

    return s;
}

/// -- sstr writer --------------------------------------
inline sstr sstr_wrt (sstr s, sstr   s2 ) { return s2  ? sstr_wrtB(s, s2 , _estr_len(s2)) : s; }
inline sstr sstr_wrtS(sstr s, constr src) { return src ? sstr_wrtB(s, src, strlen(src)  ) : s; }

sstr sstr_wrtB(sstr s, conptr ptr, size    len)
{
    size_t cap;

    is0_ret(s, 0);

    if((cap = _estr_cap(s)) < len) len = cap;
    memcpy(s, ptr, len);
    s[len] = '\0';
    _estr_setLen(s, len);

    return s;
}

sstr sstr_wrtV(sstr s, constr fmt, va_list ap )
{
    va_list cpy; size_t wr_len;
    char staticbuf[1024], *buf = staticbuf, *t;

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
    t = wr_len > 0 ? sstr_wrtB(s, buf, wr_len) : sstr_wrtS(s, buf);
    if (buf != staticbuf) s_free(buf);
    return t;
}

sstr sstr_wrtP(sstr s, constr fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    s = sstr_wrtV(s, fmt, ap);
    va_end(ap);

    return s;
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
sstr sstr_wrtF(sstr s, constr fmt, ...)
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

        if (0 == _estr_rest(s))
            return s;

        switch(*f) {
        case '%':
            next = *(f+1);
            f++;
            switch(next) {
            case 's':
            case 'S':
                str = va_arg(ap, char*);
                l = (next == 's') ? strlen(str) : _estr_len(str);
                if (_estr_rest(s) < l)
                    return s;

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
                        if (_estr_rest(s) < l)
                            return s;

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
                    if (_estr_rest(s) < l)
                        return s;

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

inline sstr sstr_cat (sstr s, sstr   s2 ) { return s2  ? sstr_catB(s, s2, _estr_len(s2)) : s;}
inline sstr sstr_catS(sstr s, constr src) { return src ? sstr_catB(s, src, strlen(src) ) : s;}


sstr sstr_catB(sstr s, conptr ptr, size   len)
{
    size_t curlen, rest;

    is0_ret(s, 0);

    curlen = _estr_len(s);
    rest   = _estr_rest(s);
    if(rest < len) len = rest;

    memcpy(s+curlen, ptr, len);
    _estr_setLen(s, curlen+len);
    s[curlen+len] = '\0';
    return s;
}

sstr sstr_catV(sstr s, constr fmt, va_list ap )
{
    va_list cpy; size_t wr_len;
    char staticbuf[1024], *buf = staticbuf, *t;

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
    t = wr_len > 0 ? sstr_catB(s, buf, wr_len) : sstr_catS(s, buf);
    if (buf != staticbuf) s_free(buf);
    return t;
}

sstr sstr_catP(sstr s, constr fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    s = sstr_catV(s, fmt, ap);
    va_end(ap);

    return s;
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
sstr sstr_catF(sstr s, constr fmt, ...)
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
        if (0 == _estr_rest(s))
            return s;

        switch(*f) {
        case '%':
            next = *(f+1);
            f++;
            switch(next) {
            case 's':
            case 'S':
                str = va_arg(ap, char*);
                l = (next == 's') ? strlen(str) : _estr_len(str);
                if (_estr_rest(s) < l)
                    return s;

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
                        if (_estr_rest(s) < l)
                            return s;

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
                        if (_estr_rest(s) < l)
                            return s;

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

inline void sstr_clear(sstr s) { estr_clear(s);}
inline void sstr_wipe (sstr s) { estr_wipe(s);}

/// -- sstr utils ---------------------------------------
inline void sstr_show(sstr s)  { _show("sstr", s); }

inline size sstr_len (sstr s)  { return s ? _estr_len(s) : 0; }
inline size sstr_cap (sstr s)  { return s ? _estr_cap(s) : 0; }

int  sstr_cmp (sstr s, sstr   s2  ) { return estr_cmp (s, s2 ); }
int  sstr_cmpS(sstr s, constr src ) { return estr_cmpS(s, src); }

inline sstr sstr_lower(sstr s) { return estr_lower(s); }
inline sstr sstr_upper(sstr s) { return estr_upper(s); }

inline sstr sstr_range(sstr s, int   start, int end) { return estr_range(s, start, end);}
inline sstr sstr_trim (sstr s, constr cset)          { return estr_trim (s, cset); }

inline sstr sstr_mapc (sstr s, constr from, constr to)             { return estr_mapc(s, from, to); }
inline sstr sstr_mapcl(sstr s, constr from, constr to, size_t len) { return estr_mapcl(s, from, to, len); }

sstr sstr_subs (sstr s, constr from, constr to)
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
            end_p = s + _estr_len(s);

            // -- get len need to expand
            offNow += offLen; fd_s += subLen;
            while((fd_s = strstr(fd_s, from)))
            {
                offNow += offLen; fd_s += subLen;
            }

            // -- have enough place, let's do it
            if((size)offNow <= _estr_cap(s))
            {
                cstr last = s + offNow;
                __str_replace(s, &end_p, &last, from, subLen, to, newLen);
                _estr_setLen(s, offNow);
                s[offNow] = '\0';
            }
        }
    }

    return s;
}


/// =====================================================================================
/// ebuf
/// =====================================================================================

typedef struct ebuf_s{
    estr s;
}ebuf_t;

/// -- ebuf creator and destroyer -----------------------
inline ebuf ebuf_new(constr src)
{
    size_t initlen = (src == NULL) ? 0 : strlen(src);
    return ebuf_newLen(src, initlen);
}

ebuf ebuf_newLen(conptr ptr, size len)
{
    ebuf eb; size_t initlen; cstr sh; sds  s;  char type; unsigned char *fp; /* flags pointer. */

    is0_ret(eb = s_malloc(sizeof(*eb)), 0);

    if(!ptr)
    {
        if(len < 8)                  len = 8;
        is0_exeret(eb->s = _estr_new(len), s_free(eb);, 0);

        return eb;
    }

    if((initlen = len * 1.2) < 8) initlen = 8;
    type    = _estr_reqT(initlen);

    /* Empty strings are usually created in order to append. Use type 8
     * since type 5 is not good at this. */
    if (type == SDS_TYPE_5) type = SDS_TYPE_8;
    int hdrlen = _estr_lenH(type);
    sh = s_malloc(hdrlen+initlen+1);
    is0_exeret(sh, s_free(eb);, 0);

    s  = eb->s = sh + hdrlen;
    fp = ((unsigned char*)s)-1;
    switch(type) {
        case SDS_TYPE_8 : {SDS_HDR_VAR( 8,s); sh->len = len; sh->alloc = initlen; *fp = type; break; }
        case SDS_TYPE_16: {SDS_HDR_VAR(16,s); sh->len = len; sh->alloc = initlen; *fp = type; break; }
        case SDS_TYPE_32: {SDS_HDR_VAR(32,s); sh->len = len; sh->alloc = initlen; *fp = type; break; }
        case SDS_TYPE_64: {SDS_HDR_VAR(64,s); sh->len = len; sh->alloc = initlen; *fp = type; break; }
    }
    memcpy(s, ptr, len);
    s[len] = '\0';

    return eb;
}

inline void ebuf_free (ebuf b) { if(b) { estr_free (b->s); s_free(b); } }

/// -- ebuf base ----------------------------------------
inline cptr ebuf_base(ebuf b) { return b ? b->s : 0; }

/// -- ebuf writer --------------------------------------
inline size ebuf_wrt (ebuf b, ebuf   b2 ) { if(b && b2) { return estr_len((b->s = estr_wrt (b->s, b2->s)));} return 0; }
inline size ebuf_wrtE(ebuf b, estr   s  ) { if(b      ) { return estr_len((b->s = estr_wrt (b->s, s    )));} return 0; }
inline size ebuf_wrtS(ebuf b, constr src) { if(b      ) { return estr_len((b->s = estr_wrtS(b->s, src  )));} return 0; }
inline size ebuf_wrtB(ebuf b, conptr ptr, size    len)  { if(b) { return estr_len((b->s = estr_wrtB(b->s, ptr, len)));} return 0; }
inline size ebuf_wrtV(ebuf b, constr fmt, va_list ap )  { if(b) { return estr_len((b->s = estr_wrtV(b->s, fmt, ap )));} return 0; }
inline size ebuf_wrtP(ebuf b, constr fmt, ...)
{
    size len; va_list ap;

    va_start(ap, fmt);
    len = ebuf_wrtV(b, fmt, ap);
    va_end(ap);

    return len;
}

size ebuf_wrf(ebuf b, constr fmt, ...)
{
    constr f; size_t i; va_list ap; estr s;

    is0_ret(b, 0); is0_ret((s = b->s), 0);

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
    b->s = s;
    return estr_len(s);
}

inline size ebuf_cat (ebuf b, ebuf   b2 )
{
    if(b && b2)
    {
        size_t hlen = estr_len(b->s);
        b->s = estr_cat(b->s, b2->s);

        return b->s ? _estr_len(b->s) - hlen : 0;
    }

    return 0;
}

inline size ebuf_catS(ebuf b, constr src)
{
    if(b)
    {
        size_t hlen = estr_len(b->s);
        b->s = estr_catS(b->s, src);

        return b->s ? _estr_len(b->s) - hlen : 0;
    }

    return 0;
}

inline size ebuf_catB(ebuf b, conptr ptr, size len)
{
    if(b)
    {
        size_t hlen = estr_len(b->s);
        b->s = estr_catB(b->s, ptr, len);

        return b->s ? _estr_len(b->s) - hlen : 0;
    }

    return 0;
}

inline size ebuf_catV(ebuf b, constr fmt, va_list ap )
{
    if(b)
    {
        size_t hlen = estr_len(b->s);
        b->s = estr_catV(b->s, fmt, ap);

        return b->s ? _estr_len(b->s) - hlen : 0;
    }

    return 0;
}

size ebuf_catP(ebuf b, constr fmt, ...)
{
    size_t len; va_list ap;

    va_start(ap, fmt);
    len = ebuf_catV(b, fmt, ap);
    va_end(ap);

    return len;
}

size ebuf_catF(ebuf b, constr fmt, ...)
{
    constr f; size_t i, hlen; va_list ap; estr s;

    is0_ret(b, 0); is0_ret((s = b->s), 0);

    va_start(ap,fmt);
    f = fmt;                    /* Next format specifier byte to process. */
    i = hlen = _estr_len(s);     /* Position of the next byte to write to dest str. */
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
    b->s = s;
    return s ? _estr_len(s) - hlen : 0;
}

inline void ebuf_clear(ebuf b) { if(b) { estr_clear(b->s);} }
inline void ebuf_wipe (ebuf b) { if(b) { estr_wipe (b->s);} }

/// -- ebuf utils ---------------------------------------
inline void ebuf_show(ebuf b) { _show("ebuf", b->s); }

inline size ebuf_len (ebuf b) { return b ? estr_len(b->s) : 0; }
inline size ebuf_cap (ebuf b) { return b ? estr_cap(b->s) : 0; }

inline int  ebuf_cmp (ebuf b, ebuf   b2 ) { if(b && b2) { return estr_cmp (b->s, b2->s); } else return -2; }
inline int  ebuf_cmpE(ebuf b, estr   s  ) { if(b      ) { return estr_cmp (b->s, s    ); } else return -2; }
inline int  ebuf_cmpS(ebuf b, constr src) { if(b      ) { return estr_cmpS(b->s, src  ); } else return -2; }

inline ebuf ebuf_lower(ebuf b) { if(b) { estr_lower(b->s); } return b->s ? b : 0; }
inline ebuf ebuf_upper(ebuf b) { if(b) { estr_upper(b->s); } return b->s ? b : 0; }

inline ebuf ebuf_range(ebuf b, int start, int end) { if(b) { estr_range(b->s, start, end); } return b->s ? b : 0; }
inline ebuf ebuf_trim (ebuf b, constr cset)        { if(b) { estr_trim(b->s, cset);        } return b->s ? b : 0; }

inline ebuf ebuf_mapc (ebuf b, constr from, constr to)             { if(b) {estr_mapc (b->s, from, to     ); } return b->s ? b : 0; }
inline ebuf ebuf_mapcl(ebuf b, constr from, constr to, size_t len) { if(b) {estr_mapcl(b->s, from, to, len); } return b->s ? b : 0; }

inline ebuf ebuf_subs (ebuf b, constr from, constr to) { if(b) { b->s = estr_subs(b->s, from, to); } return b->s ? b : 0; }

/// -- Low level functions exposed to the user API --
ebuf ebuf_ensure (ebuf b, size_t addlen) { if(b) { b->s = estr_ensure (b->s, addlen); } return b->s ? b : 0; }
void ebuf_incrLen(ebuf b, size_t incr  ) { if(b) {        estr_incrLen(b->s, incr  ); } }
ebuf ebuf_shrink (ebuf b) { if(b){ b->s = estr_shrink(b->s); } return b->s ? b : 0; }
