/*
   eb64(easy base64) - a easier way to handle base64

   Author: ziyht


*/

#ifndef __EB64_H__
#define __EB64_H__

#include "etype.h"
#include "estr.h"

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------

    API statement:

            |---- b: binary data
    eb64_encb2f(...)
              |-- f: file

---------------------------------*/

/// ---------------------- encoder -------------------------
///
///     to encode a binary data to a base64 data;
///
/// @note:
///     the returned cstr must be freed by eb64_free() after using it
///

estr eb64_encb  (conptr in, size inlen);
int  eb64_encb2b(conptr in, size inlen, cptr   out  , size* outlen);
int  eb64_encb2f();

estr eb64_encf  ();
int  eb64_encf2b();
int  eb64_encf2f();

/// ---------------------- decoder -------------------------
///
///     to decode a base64 data to origin data
///
/// @note:
///     the returned cptr must be freed by eb64_free() after using it
///

estr eb64_decb  (conptr in, size inlen);
int  eb64_decb2b(conptr in, size inlen, cptr   out  , size* outlen);
int  eb64_decb2f();

estr eb64_decf  ();
int  eb64_decf2b();
int  eb64_decf2f();

/// ----------------------- utils --------------------------
///
/// @note:
///     1. @param s should be a estr returned by eb64 API
///

void eb64_show(estr d);       // show the data to stdout
size eb64_dlen(estr d);       // return the length of data
void eb64_free(estr d);       // release data

estr eb64_version();

#ifdef __cplusplus
}
#endif

#endif
