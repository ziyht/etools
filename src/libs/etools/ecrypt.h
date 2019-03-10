/// =====================================================================================
///
///       Filename:  ecrypt.h
///
///    Description:  a wrapper of crypt_blowfish, using as a easier way
///
///        Version:  1.0
///        Created:  03/04/2017 04:51:34 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#ifndef __ECRYPT_H__
#define __ECRYPT_H__

#define ECRYPT_VERSION "ecrypt 1.0.2"      // compat win32 of lib crypt_blowfish

#include "etype.h"
#include "estr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ECRYPT_SIZE     64

/// ----------------------- hash 1 -------------------------
///
///     original way to hash a passwd, in this case, you must
/// generate a salt first, and using this salt to encode your
/// passwd to a crypt hashcode, we using "$2a$" as the prefix
/// like bcrypt.
///
/// @return [int ]  1 : operate ok
///         [int ]  0 : operate faild
///         [estr] !0 : operate ok
///         [estr]  0 : operate faild
///
/// @note:
///     1. the returned estr must be freed by ecrypt_free() after
/// using it
///

int  ecrypt_gensalt(uint   factor, char   salt[ECRYPT_SIZE]);
estr ecrypt_hashs  (constr passwd, coni8 salt[ECRYPT_SIZE]);
int  ecrypt_hashs2s(constr passwd, coni8 salt[ECRYPT_SIZE], char hash[ECRYPT_SIZE]);

/// ----------------------- hash 2 -------------------------
///
///     a more easier way to hash a passwd, in this way, we
/// will generate a salt autolly before hashing it, so you
/// only need to pass your passwd in, and then you can get
/// the hashcode of this passwd.
///
/// @return [int ]  1: operate ok
///         [int ]  0: operate faild
///         [estr] !0: operate ok
///         [estr]  0: operate faild
///

estr ecrypt_encs  (constr passwd);
int  ecrypt_encs2s(constr passwd, char   hash[ECRYPT_SIZE]);

/// ----------------------- check --------------------------
///
///     to check a hashcode is whether generated based on the
/// passwd.
///
/// @return [int ] 1: check ok
///                0: check faild
///

int  ecrypt_check (constr passwd, coni8 hash[ECRYPT_SIZE]);

/// ----------------------- utils --------------------------
///
/// @note:
///     1. @param s should be a estr returned by ecrypt API
///

void ecrypt_show(estr s);           // show the data to stdout
size ecrypt_dlen(estr s);           // return the length of data
void ecrypt_free(estr s);           // release data

estr ecrypt_version();              // return the version of ecrypt

#ifdef __cplusplus
}
#endif

#endif
