/*
   estr(easy str) - a easier way to handle str

   Author: ziyht


*/

#ifndef __ESTR_H__
#define __ESTR_H__

#define ESTR_VERSION "0.1.4"        // add some needed API statement, do it next time

#include <sys/types.h>
#include <stdarg.h>

#ifndef __DEF_STR__
#define __DEF_STR__
typedef const char* constr;
typedef const void* conptr;
typedef char* cstr;
typedef void* cptr;
#endif

#ifndef __DEF_INT__
#define __DEF_INT__
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef long long          s64;
#else
#include <stdint.h>
typedef uint32_t uint;
typedef uint64_t u64;
typedef int64_t  s64;
#endif
#endif

/// -------------------------------
/// estr - estr for heap using
/// -------------------------------
typedef char* estr;

/// -- estr new --
estr estr_new(constr src);
estr estr_newLen(conptr ptr, size_t len);

estr estr_fromS64(s64 val);
estr estr_fromU64(u64 val);

estr estr_dup(estr s);

/// -- estr clear or free --
void estr_clear(estr s);
void estr_wipe (estr s);
void estr_free (estr s);

/// -- estr len --
u64  estr_len (estr es);
u64  estr_cap (estr es);

/// -- estr show --
void estr_shows(estr s);
void estr_showr(estr s);

/// -- estr write --
estr estr_wrs(estr s, constr src);             // write a str         to estr s
estr estr_wre(estr s, estr   s2 );             // write a estr        to estr s
estr estr_wrb(estr s, conptr ptr, size_t  len);// write a binary date to estr s
estr estr_wrv(estr s, constr fmt, va_list ap );// write a fmt str     to estr s, gets va_list instead of being variadic
#ifdef __GNUC__
estr estr_wrp(estr s, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
estr estr_wrp(estr s, constr fmt, ...);        // write a fmt str     to estr ss, like sprintf
#endif
estr estr_wrf(estr s, constr fmt, ...);        // write a fmt str     to estr ss, using a new definition API, see definition

estr estr_cats(estr s, constr src);
estr estr_cate(estr s, estr   s2 );
estr estr_catb(estr s, conptr src, size_t  len);
estr estr_catv(estr s, constr fmt, va_list ap );
#ifdef __GNUC__
estr estr_catp(estr s, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
estr estr_catp(estr s, constr fmt, ...);
#endif
estr estr_catf(estr s, constr fmt, ...);

/// -- estr adjusting --
estr estr_trim (estr s, constr cset);
estr estr_range(estr s, int start, int end);
estr estr_lower(estr s);
estr estr_upper(estr s);
estr estr_mapc (estr s, constr form, constr to);
estr estr_mapcl(estr s, constr form, constr to, size_t len);
estr estr_subs (estr s, constr from, constr to);

/// -- estr cmp ---
int  estr_cmps(estr s, constr src);
int  estr_cmpe(estr s, estr   s2 );

/// -- Low level functions exposed to the user API --
estr estr_ensure (estr s, size_t addlen);
void estr_incrLen(estr s, size_t incr);
estr estr_shrink (estr s);

/// ----------------------------------------------
/// sstr - estr for stack using
///
/// @note:
///     1. all estr can using those following API,
///        but you may not get the correct result
///        when you using it as first @param s
///     2. do not using the above APIs to operate
///        sstrs inited by sstr_init()
/// ----------------------------------------------
typedef char* sstr;

/// -- sstr new --
sstr sstr_init(cptr buf, uint len);

/// -- sstr clear --
void sstr_clear(sstr s);
void sstr_wipe (sstr s);

/// -- sstr len --
uint sstr_len (sstr s);
uint sstr_cap (sstr s);

/// -- sstr show --
void sstr_shows(sstr s);
void sstr_showr(sstr s);

/// -- sstr write --
sstr sstr_wrs(sstr s, constr src);             // write a str         to sstr s
sstr sstr_wre(sstr s, sstr   s2 );             // write a estr        to sstr s
sstr sstr_wrb(sstr s, conptr ptr, size_t  len);// write a binary date to sstr s
sstr sstr_wrv(sstr s, constr fmt, va_list ap );// write a fmt str     to sstr s, gets va_list instead of being variadic
#ifdef __GNUC__
sstr sstr_wrp(sstr s, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
sstr sstr_wrp(sstr s, constr fmt, ...);        // write a fmt str     to sstr s, like sprintf
#endif
sstr sstr_wrf(sstr s, constr fmt, ...);        // write a fmt str     to sstr s, using a new definition API, see definition

sstr sstr_cats(sstr s, constr src);
sstr sstr_cate(sstr s, sstr   s2 );
sstr sstr_catb(sstr s, conptr src, size_t  len);
sstr sstr_catv(sstr s, constr fmt, va_list ap );
#ifdef __GNUC__
sstr sstr_catp(sstr s, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
sstr sstr_catp(sstr s, constr fmt, ...);
#endif
sstr sstr_catf(sstr s, constr fmt, ...);

/// -- estr adjusting --
sstr sstr_trim (sstr s, constr cset);
sstr sstr_range(sstr s, int start, int end);
sstr sstr_lower(sstr s);
sstr sstr_upper(sstr s);
sstr sstr_mapc (sstr s, constr form, constr to);
sstr sstr_mapcl(sstr s, constr form, constr to, size_t len);
sstr sstr_subs (sstr s, constr from, constr to);

/// -- estr cmp ---
int  sstr_cmps(sstr s, constr src );
int  sstr_cmpe(sstr s, sstr   s2);

/// -------------------------------
/// ebuf
/// -------------------------------

typedef struct ebuf_s{
    estr s;
}ebuf_t;

typedef struct ebuf_s* ebuf;

/// -- estr new --
ebuf ebuf_new(size_t len);
ebuf ebuf_newLen(conptr ptr, size_t len);

/// -- estr clear or free --
void ebuf_clear(ebuf b);
void ebuf_wipe (ebuf b);
void ebuf_free (ebuf b);

/// -- sstr len --
uint ebuf_len (ebuf b);
uint ebuf_cap (ebuf b);

/// -- estr show --
void ebuf_shows(ebuf b);
void ebuf_showr(ebuf b);

/// -- estr write --
ebuf ebuf_wrs(ebuf b, constr src);             // write a str         to estr es
ebuf ebuf_wre(ebuf b, ebuf   b2 );             // write a estr        to estr e1
ebuf ebuf_wrb(ebuf b, conptr ptr, size_t  len);// write a binary date to estr es
ebuf ebuf_wrv(ebuf b, constr fmt, va_list ap );// write a fmt str     to estr es, gets va_list instead of being variadic
#ifdef __GNUC__
ebuf ebuf_wrp(ebuf b, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
estr ebuf_wrp(ebuf b, constr fmt, ...);        // write a fmt str     to estr es, like sprintf
#endif
ebuf ebuf_wrf(ebuf b, constr fmt, ...);        // write a fmt str     to estr es, using a new definition API, see definition

ebuf ebuf_cats(ebuf b, constr src);
ebuf ebuf_cate(ebuf b, ebuf   b2 );
ebuf ebuf_catb(ebuf b, conptr src, size_t  len);
ebuf ebuf_catv(ebuf b, constr fmt, va_list ap );
#ifdef __GNUC__
ebuf ebuf_catp(ebuf b, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
ebuf estr_catp(ebuf b, constr fmt, ...);
#endif
ebuf ebuf_catf(ebuf b, constr fmt, ...);

/// -- estr adjusting --
ebuf ebuf_trim (ebuf b, constr cset);
ebuf ebuf_range(ebuf b, int start, int end);
ebuf ebuf_lower(ebuf b);
ebuf ebuf_upper(ebuf b);
ebuf ebuf_mapc (ebuf b, constr form, constr to);
ebuf ebuf_mapcl(ebuf b, constr form, constr to, size_t len);
ebuf ebuf_subs (ebuf b, constr from, constr to);

/// -- estr cmp ---
int  ebuf_cmps(ebuf s, constr src);
int  ebuf_cmpe(ebuf s, estr   s2 );

/// -- Low level functions exposed to the user API --
ebuf ebuf_ensure (ebuf s, size_t addlen);
void ebuf_incrLen(ebuf s, size_t incr);
ebuf ebuf_shrink (ebuf s);


#endif
