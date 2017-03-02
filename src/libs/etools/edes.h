/*
   edes(easy des) - a easier des encoder/decoder

   Author: ziyht


*/

#ifndef __EDES_H__
#define __EDES_H__

#define EB64_VERSION "1.0.0"      // new tool

#include "etype.h"


/*---------------------

  API statement:

  edes_encb2f(...)
         * *

    b -- binary data
    f -- file

-----------------------*/

cstr edes_newkey(cstr key);         // 8 bit len

/// -- encoder
///
/// @note:
///     the returned cptr must be freed by edes_free() after using it
///

cptr edes_encb  (constr key, conptr in, size inlen);
int  edes_encb2b();
int  edes_encb2f();

cptr edes_encf  ();
int  edes_encf2b();
int  edes_encf2f();

/// -- decoder
///
/// @note:
///     the returned cptr must be freed by edes_free() after using it
///

cptr edes_decb  (constr key, conptr in, size inlen);
int  edes_decb2b();
int  edes_decb2f();

cptr edes_decf  ();
int  edes_decf2b();
int  edes_decf2f();

/// -- utils
///
/// @note:
///     the @param d must be a cptr returned by edes API
///

void edes_show(conptr d);       // show the data to stdout
size edes_dlen(conptr d);       // return the length of data
void edes_free(conptr d);       // release data

#ifdef __cplusplus
extern "C" {
#endif

#endif
