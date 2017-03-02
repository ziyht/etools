/*
   eb64(easy base64) - a easier way to handle base64

   Author: ziyht


*/

#ifndef __EB64_H__
#define __EB64_H__

#define EB64_VERSION "1.1.0"      // new API

#include "etype.h"

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------

  API statement:

  e64_encb2f(...)
         * *

    b -- binary data
    f -- file

-----------------------*/

/// -- encoder
///
/// @note:
///     the returned cstr must be freed by eb64_free() after using it
///

cstr eb64_encb  (conptr in, size inlen);
int  eb64_encb2b(conptr in, size inlen, cstr   out  , size* outlen);
int  eb64_encb2f();

cstr eb64_encf  ();
int  eb64_encf2b();
int  eb64_encf2f();

/// -- decoder
///
/// @note:
///     the returned cptr must be freed by eb64_free() after using it
///

cptr eb64_decb  (constr in, size inlen);
int  eb64_decb2b(constr in, size inlen, cptr   out  , size* outlen);
int  eb64_decb2f();

cptr eb64_decf  ();
int  eb64_decf2b();
int  eb64_decf2f();

/// -- utils
///
/// @note:
///     the @param d must be a cstr or cptr returned by eb64 API
///

void eb64_show(conptr d);       // show the data to stdout
size eb64_dlen(conptr d);       // return the length of data
void eb64_free(conptr d);       // release data


#ifdef __cplusplus
}
#endif

#endif
