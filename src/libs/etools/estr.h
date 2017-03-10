/// =====================================================================================
///
///       Filename:  estr.h
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


#ifndef __ESTR_H__
#define __ESTR_H__

#define ESTR_VERSION "1.0.1"        // if s = 0, create a new estr autolly in wr and cat oprerations, only support estr

#include <stdarg.h>

#include "etype.h"

#ifdef __cplusplus
extern "C" {
#endif

/// ---------------------- estr -------------------------
///
///     estr - for heap using
///
///     when using a estr, you can deal with it as a cstr,
/// but it will be more convenient and High-Performance with
/// our APIs.
///
/// @note:
///     1. as the API will auto expand(realloc) the estr, so
///        you must inherit it when you using the APIs who
///        returned estr, lg:
///           estr s = estr_new("12");
///           s = estr_wrs(s, "1234");
///     2. you must free it by using estr_free()
///

typedef char* estr;

/// -- estr new --
estr estr_new(constr src);
estr estr_newLen(conptr ptr, size len);

estr estr_fromS64(s64 val);
estr estr_fromU64(u64 val);

estr estr_dup(estr s);

/// -- estr clear or free --
void estr_clear(estr s);
void estr_wipe (estr s);
void estr_free (estr s);

/// -- estr len --
size estr_len (estr es);
size estr_cap (estr es);

/// -- estr show --
void estr_shows(estr s);
void estr_showr(estr s);

/// -- estr writer --
estr estr_wrs(estr s, constr src);             // write a str         to estr s
estr estr_wre(estr s, estr   s2 );             // write a estr        to estr s
estr estr_wrb(estr s, conptr ptr, size    len);// write a binary date to estr s
estr estr_wrv(estr s, constr fmt, va_list ap );// write a fmt str     to estr s, gets va_list instead of being variadic
#ifdef __GNUC__
estr estr_wrp(estr s, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
estr estr_wrp(estr s, constr fmt, ...);        // write a fmt str     to estr ss, like sprintf
#endif
estr estr_wrf(estr s, constr fmt, ...);        // write a fmt str     to estr ss, using a new definition API, see definition

estr estr_cats(estr s, constr src);
estr estr_cate(estr s, estr   s2 );
estr estr_catb(estr s, conptr ptr, size    len);
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
estr estr_mapc (estr s, constr from, constr to);
estr estr_mapcl(estr s, constr from, constr to, size_t len);
estr estr_subs (estr s, constr from, constr to);

/// -- estr cmp ---
int  estr_cmps(estr s, constr src);
int  estr_cmpe(estr s, estr   s2 );

/// -- Low level functions exposed to the user API --
estr estr_ensure (estr s, size_t addlen);
void estr_incrLen(estr s, size_t incr);
estr estr_shrink (estr s);

/// ---------------------- sstr -------------------------
///
///     sstr - for stack using
///
///     the using of sstr is almost the same as estr,
/// we using the passed buf to init it, so we will not
/// expand it autolly when there is no enough space to
/// do the operation, in this case, we'll do nothing and
/// always return 0;
///
/// @note:
///     1. all estr can using those following API, but
///        you may not get the correct result when you
///        using it as first @param s, and you can also
///        consider those APIs is a safety version for
///        estr.
///     2. do not using the above estr APIs to operate
///        sstrs inited by sstr_init()
///

typedef char* sstr;

/// -- sstr new --
sstr sstr_init(cptr buf, uint len);

/// -- sstr clear --
void sstr_clear(sstr s);
void sstr_wipe (sstr s);

/// -- sstr len --
size sstr_len (sstr s);
size sstr_cap (sstr s);

/// -- sstr show --
void sstr_shows(sstr s);
void sstr_showr(sstr s);

/// -- sstr writer --
sstr sstr_wrs(sstr s, constr src);             // write a str         to sstr s
sstr sstr_wre(sstr s, sstr   s2 );             // write a estr        to sstr s
sstr sstr_wrb(sstr s, conptr ptr, size    len);// write a binary date to sstr s
sstr sstr_wrv(sstr s, constr fmt, va_list ap );// write a fmt str     to sstr s, gets va_list instead of being variadic
#ifdef __GNUC__
sstr sstr_wrp(sstr s, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
sstr sstr_wrp(sstr s, constr fmt, ...);        // write a fmt str     to sstr s, like sprintf
#endif
sstr sstr_wrf(sstr s, constr fmt, ...);        // write a fmt str     to sstr s, using a new definition API, see definition

sstr sstr_cats(sstr s, constr src);
sstr sstr_cate(sstr s, sstr   s2 );
sstr sstr_catb(sstr s, conptr ptr, size   len);
sstr sstr_catv(sstr s, constr fmt, va_list ap );
#ifdef __GNUC__
sstr sstr_catp(sstr s, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
sstr sstr_catp(sstr s, constr fmt, ...);
#endif
sstr sstr_catf(sstr s, constr fmt, ...);

/// -- estr adjusting --
sstr sstr_trim (sstr s, constr cset);
sstr sstr_range(sstr s, int   start, int end);
sstr sstr_lower(sstr s);
sstr sstr_upper(sstr s);
sstr sstr_mapc (sstr s, constr from, constr to);
sstr sstr_mapcl(sstr s, constr from, constr to, size_t len);
sstr sstr_subs (sstr s, constr from, constr to);

/// -- estr cmp ---
int  sstr_cmps(sstr s, constr src );
int  sstr_cmpe(sstr s, sstr   s2);

/// ---------------------- sstr -------------------------
///
///     ebuf - a warpper of estr
///
///     we create a handler to handle estr, so you do not
/// need to inherit estr when using ebuf;
///
///

typedef struct ebuf_s* ebuf;

/// -- ebuf new --
ebuf ebuf_new(constr src);
ebuf ebuf_newLen(conptr ptr, size len);

/// -- ebuf ptr --
cptr ebuf_base(ebuf b);

/// -- ebuf clear or free --
void ebuf_clear(ebuf b);
void ebuf_wipe (ebuf b);
void ebuf_free (ebuf b);

/// -- ebuf len --
size ebuf_len (ebuf b);
size ebuf_cap (ebuf b);

/// -- ebuf show --
void ebuf_shows(ebuf b);
void ebuf_showr(ebuf b);

/// -- ebuf writer --
size ebuf_wrs(ebuf b, constr src);             // write a str         to estr es
size ebuf_wre(ebuf b, ebuf   b2 );             // write a estr        to estr e1
size ebuf_wrb(ebuf b, conptr ptr, size    len);// write a binary date to estr es
size ebuf_wrv(ebuf b, constr fmt, va_list ap );// write a fmt str     to estr es, gets va_list instead of being variadic
#ifdef __GNUC__
size ebuf_wrp(ebuf b, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
size ebuf_wrp(ebuf b, constr fmt, ...);        // write a fmt str     to estr es, like sprintf
#endif
size ebuf_wrf(ebuf b, constr fmt, ...);        // write a fmt str     to estr es, using a new definition API, see definition

size ebuf_cats(ebuf b, constr src);
size ebuf_cate(ebuf b, ebuf   b2 );
size ebuf_catb(ebuf b, conptr ptr, size    len);
size ebuf_catv(ebuf b, constr fmt, va_list ap );
#ifdef __GNUC__
size ebuf_catp(ebuf b, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
size estr_catp(ebuf b, constr fmt, ...);
#endif
size ebuf_catf(ebuf b, constr fmt, ...);

/// -- ebuf adjusting --
ebuf ebuf_trim (ebuf b, constr cset);
ebuf ebuf_range(ebuf b, int start, int end);
ebuf ebuf_lower(ebuf b);
ebuf ebuf_upper(ebuf b);
ebuf ebuf_mapc (ebuf b, constr from, constr to);
ebuf ebuf_mapcl(ebuf b, constr from, constr to, size_t len);
ebuf ebuf_subs (ebuf b, constr from, constr to);

/// -- ebuf cmp ---
int  ebuf_cmps(ebuf b, constr src);
int  ebuf_cmpe(ebuf b, ebuf   b2 );

/// -- Low level functions exposed to the user API --
ebuf ebuf_ensure (ebuf b, size_t addlen);
void ebuf_incrLen(ebuf b, size_t incr);
ebuf ebuf_shrink (ebuf b);

#ifdef __cplusplus
}
#endif

#endif
