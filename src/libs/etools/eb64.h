/// =====================================================================================
///
///       Filename:  eb64.h
///
///    Description:  an easier b64 encoder/decoder
///
///        Version:  1.0
///        Created:  03/02/2017 04:38:34 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#ifndef __EB64_H__
#define __EB64_H__

#define EB64_VERSION "eb64 1.1.5"      // compat prev version of gcc

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

void eb64_show(estr s);         // show the data to stdout
size eb64_dlen(estr s);         // return the length of data
void eb64_free(estr s);         // release data

estr eb64_version();            // return the version of eb64

#ifdef __cplusplus
}
#endif

#endif
