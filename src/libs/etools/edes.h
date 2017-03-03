/*
   edes(easy des) - a easier des encoder/decoder

   Author: ziyht


*/

#ifndef __EDES_H__
#define __EDES_H__

#define EDES_VERSION "1.0.1"      // add EDES_SAFE_CODEC option to encode data

#include "etype.h"


/*---------------------

    API statement:

            |---- b: binary data
    edes_encb2f(...)
              |-- f: file

-----------------------*/

/// ---------------------- setting ---------------------
///
/// EDES_SAFE_CODEC:
///         In this case, the API will generate a random
///     key to encode the data, and this random key will
///     be encode to the first 8 byte of output data with
///     your input key, so it will take more 8 byte space
///     to handle the whole data;
///         Otherwise, the first 8 byte space of out data
///     will be filled with our inner magic key.
///

#define EDES_SAFE_CODEC 1           // encode in safe mode

/// ---------------------- key -------------------------
///
/// @note:
///     1. @param key requires at least 8 bytes
///     2. set @param human can generate a human-readable key
///

cstr edes_newkey(cstr key, int human);

/// ---------------------- encoder -------------------------
///
/// @note:
///     1. @param key requires at least 8 bytes, and only
///        the first 8 byte is usable
///     2. @param out must have 16 byte more than @param in
///     3. the returned cptr must be freed by edes_free()
///        after using it
///

cptr edes_encb  (constr key, conptr in, size inlen);
int  edes_encb2b(constr key, conptr in, size inlen, cptr out, size* outlen);
int  edes_encb2f();

cptr edes_encf  ();
int  edes_encf2b();
int  edes_encf2f();

/// ---------------------- decoder -------------------------
///
/// @note:
///     1. @param key requires at least 8 bytes, and only
///        the first 8 byte is usable
///     2. the returned cptr must be freed by edes_free()
///        after using it
///

cptr edes_decb  (constr key, conptr in, size inlen);
int  edes_decb2b(constr key, conptr in, size inlen, cptr out, size* outlen);
int  edes_decb2f();

cptr edes_decf  ();
int  edes_decf2b();
int  edes_decf2f();

/// ----------------------- utils --------------------------
///
/// @note:
///     1. @param d must be a cptr returned by edes API
///

void edes_show(conptr d);       // show the data to stdout
size edes_dlen(conptr d);       // return the length of data
void edes_free(conptr d);       // release data

#ifdef __cplusplus
extern "C" {
#endif

#endif
