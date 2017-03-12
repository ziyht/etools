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
/// but it will be more convenient and High-Performance
/// with our APIs.
///
///     for convenient using, the write of estr will create
/// a estr autolly when you passed NULL:
///     estr s = estr_wrtS(0, "1234");
///     now the s is a estr of "1234", so you can also using
/// the writer to create a estr, note that this operation is
/// not applicable for sstr and ebuf;
///
/// @note:
///     1. as the API will auto expand(realloc) the estr ,
///        so you must inherit it when you using most of
///        the APIs who returned estr, lg:
///           estr s = estr_new("12");
///           s = estr_wrtS(s, "1234");
///     2. you must free it by using estr_free()
///

typedef char* estr;

/// -- estr creator and destroyer -----------------------
estr estr_new(constr src);
estr estr_newLen(conptr ptr, size len);

estr estr_fromS64(s64 val);
estr estr_fromU64(u64 val);

estr estr_dup(estr s);

void estr_free(estr s);

/// -- estr writer --------------------------------------
estr estr_wrt (estr s, estr   s2 );                 // wrt: write from the beginning, if s is 0, will create a new estr autolly
estr estr_wrtS(estr s, constr src);
estr estr_wrtB(estr s, conptr ptr, size    len);
estr estr_wrtV(estr s, constr fmt, va_list ap );
#ifdef __GNUC__
estr estr_wrtP(estr s, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
estr estr_wrtP(estr s, constr fmt, ...);
#endif
estr estr_wrtF(estr s, constr fmt, ...);

estr estr_cat (estr s, estr   s2 );                 // cat: write continued, if s is 0, will create a new estr autolly
estr estr_catS(estr s, constr src);
estr estr_catB(estr s, conptr ptr, size    len);
estr estr_catV(estr s, constr fmt, va_list ap );
#ifdef __GNUC__
estr estr_catP(estr s, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
estr estr_catP(estr s, constr fmt, ...);
#endif
estr estr_catF(estr s, constr fmt, ...);

//estr estr_set (estr s, int  start, conptr ptr, size len);   // todo
estr estr_setT(estr s, char    c);                  // set the last char of s, if c == '\0', the len of s will decrease 1 autolly

void estr_clear(estr s);
void estr_wipe (estr s);

/// -- estr utils ---------------------------------------
void estr_show(estr s);

size estr_len (estr s);
size estr_cap (estr s);

int  estr_cmp (estr s, estr   s2 );
int  estr_cmpS(estr s, constr src);

estr estr_lower(estr s);
estr estr_upper(estr s);
estr estr_range(estr s, int   start, int end);
estr estr_trim (estr s, constr cset);
estr estr_mapc (estr s, constr from, constr to);
estr estr_mapcl(estr s, constr from, constr to, size_t len);
estr estr_subs (estr s, constr from, constr to);

estr estr_join (char** argv, int argc, constr sep);

estr*estr_split    (estr   s  ,          constr sep,             int* cnt);
estr*estr_splitS   (constr src,          constr sep,             int* cnt);
estr*estr_splitLen (conptr ptr, int len, conptr sep, int seplen, int* cnt);

estr*estr_splitArgv(char** argv, int argc, int* cnt);
estr*estr_splitArgs(constr line, int*argc);

void estr_showSplit(estr* _s, int max);
void estr_freeSplit(estr* _s, int cnt);

/// -- Low level functions exposed to the user API ------
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

/// -- sstr initializer ---------------------------------
sstr sstr_init(cptr buf, uint len);

/// -- sstr writer --------------------------------------
sstr sstr_wrt (sstr s, sstr   s2 );                  // wrt: write from the beginning
sstr sstr_wrtS(sstr s, constr src);
sstr sstr_wrtB(sstr s, conptr ptr, size    len);
sstr sstr_wrtV(sstr s, constr fmt, va_list ap );
#ifdef __GNUC__
sstr sstr_wrtP(sstr s, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
sstr sstr_wrtP(sstr s, constr fmt, ...);
#endif
sstr sstr_wrtF(sstr s, constr fmt, ...);

sstr sstr_cat (sstr s, sstr   s2 );
sstr sstr_catS(sstr s, constr src);                 // cat: write continued
sstr sstr_catB(sstr s, conptr ptr, size   len);
sstr sstr_catV(sstr s, constr fmt, va_list ap );
#ifdef __GNUC__
sstr sstr_catP(sstr s, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
sstr sstr_catP(sstr s, constr fmt, ...);
#endif
sstr sstr_catF(sstr s, constr fmt, ...);

void sstr_clear(sstr s);
void sstr_wipe (sstr s);

/// -- estr utils ------------------------------------
void sstr_show(sstr s);

size sstr_len (sstr s);
size sstr_cap (sstr s);

int  sstr_cmp (sstr s, sstr   s2);
int  sstr_cmpS(sstr s, constr src );

sstr sstr_lower(sstr s);
sstr sstr_upper(sstr s);
sstr sstr_range(sstr s, int   start, int end);
sstr sstr_trim (sstr s, constr cset);
sstr sstr_mapc (sstr s, constr from, constr to);
sstr sstr_mapcl(sstr s, constr from, constr to, size_t len);
sstr sstr_subs (sstr s, constr from, constr to);


/// ---------------------- ebuf -------------------------
///
///     ebuf - a warpper of estr
///
///     we create a handler to handle estr, so you do not
/// need to inherit estr when using ebuf;
///
///

typedef struct ebuf_s* ebuf;

/// -- ebuf creator and destroyer -----------------------
ebuf ebuf_new(constr src);
ebuf ebuf_newLen(conptr ptr, size len);

void ebuf_free (ebuf b);

/// -- ebuf base ----------------------------------------
cptr ebuf_base(ebuf b);

/// -- ebuf writer --------------------------------------
size ebuf_wrt (ebuf b, ebuf   b2 );                 // wrt: write form the beginning
size ebuf_wrtE(ebuf b, estr   s  );
size ebuf_wrtS(ebuf b, constr src);
size ebuf_wrtB(ebuf b, conptr ptr, size    len);
size ebuf_wrtV(ebuf b, constr fmt, va_list ap );
#ifdef __GNUC__
size ebuf_wrtP(ebuf b, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
size ebuf_wrtP(ebuf b, constr fmt, ...);
#endif
size ebuf_wrtF(ebuf b, constr fmt, ...);

size ebuf_cat (ebuf b, ebuf   b2 );                 // cat: write continued
size ebuf_catE(ebuf b, estr   s  );
size ebuf_catS(ebuf b, constr src);
size ebuf_catB(ebuf b, conptr ptr, size    len);
size ebuf_catV(ebuf b, constr fmt, va_list ap );
#ifdef __GNUC__
size ebuf_catP(ebuf b, constr fmt, ...) __attribute__((format(printf, 2, 3)));
#else
size estr_catP(ebuf b, constr fmt, ...);
#endif
size ebuf_catF(ebuf b, constr fmt, ...);

void ebuf_clear(ebuf b);
void ebuf_wipe (ebuf b);

/// -- ebuf utils ---------------------------------------
void ebuf_show(ebuf b);

size ebuf_len (ebuf b);
size ebuf_cap (ebuf b);

int  ebuf_cmp (ebuf b, ebuf   b2 );
int  ebuf_cmpE(ebuf b, estr   s  );
int  ebuf_cmpS(ebuf b, constr src);

ebuf ebuf_lower(ebuf b);
ebuf ebuf_upper(ebuf b);
ebuf ebuf_range(ebuf b, int   start, int end);
ebuf ebuf_trim (ebuf b, constr cset);
ebuf ebuf_mapc (ebuf b, constr from, constr to);
ebuf ebuf_mapcl(ebuf b, constr from, constr to, size_t len);
ebuf ebuf_subs (ebuf b, constr from, constr to);

/// -- Low level functions exposed to the user API ------
ebuf ebuf_ensure (ebuf b, size_t addlen);
void ebuf_incrLen(ebuf b, size_t incr);
ebuf ebuf_shrink (ebuf b);

#ifdef __cplusplus
}
#endif

#endif
