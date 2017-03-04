/*
   edes(easy des) - a easier des encoder/decoder

   Author: ziyht


*/

#ifndef __EDES_H__
#define __EDES_H__

#include "etype.h"
#include "estr.h"

/*--------------------------------

    API statement:

            |---- b: binary data
    edes_encb2f(...)
              |-- f: file

---------------------------------*/

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
///     to generate a random key so that you can use it
/// to encode data.
///
/// @note:
///     1. @param key requires at least 8 bytes
///     2. set @param human can generate a human-readable key
///

cstr edes_genkey(char key[8], int human);

/// ---------------------- encoder -------------------------
///
///     using a key to encode a binary data to a encrypted
/// data by our DES algorithm.
///
/// @note:
///     1. @param key requires at least 8 bytes, and only
///        the first 8 byte is usable
///     2. @param out must have 16 byte more than @param in
///     3. the returned cptr must be freed by edes_free()
///        after using it
///

estr edes_encb  (constr key, conptr in, size inlen);
int  edes_encb2b(constr key, conptr in, size inlen, cptr out, size* outlen);
int  edes_encb2f();

estr edes_encf  ();
int  edes_encf2b();
int  edes_encf2f();

/// ---------------------- decoder -------------------------
///
///     using our DES algorithm to decode a encrypted data
/// to the origin data by the key you passed in.
///
/// @note:
///     1. @param key requires at least 8 bytes, and only
///        the first 8 byte is usable
///     2. the returned cptr must be freed by edes_free()
///        after using it
///

estr edes_decb  (constr key, conptr in, size inlen);
int  edes_decb2b(constr key, conptr in, size inlen, cptr out, size* outlen);
int  edes_decb2f();

estr edes_decf  ();
int  edes_decf2b();
int  edes_decf2f();

/// ----------------------- utils --------------------------
///
/// @note:
///     1. @param s should be a estr returned by edes API
///

void edes_show(estr s);         // show the data to stdout
size edes_dlen(estr s);         // return the length of data
void edes_free(estr s);         // release data

sstr edes_version();            // return the version of edes

#ifdef __cplusplus
extern "C" {
#endif

#endif
