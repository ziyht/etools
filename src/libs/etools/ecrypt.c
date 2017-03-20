/// =====================================================================================
///
///       Filename:  ecrypt.c
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

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ecrypt.h"
#include "crypt_blowfish/ow-crypt.h"

#define exe_ret(expr, ret ) { expr;      return ret;}
#define is0_ret(cond, ret ) if(!(cond)){ return ret;}
#define is1_ret(cond, ret ) if( (cond)){ return ret;}
#define is0_exe(cond, expr) if(!(cond)){ expr;}
#define is1_exe(cond, expr) if( (cond)){ expr;}

#define is0_exeret(cond, expr, ret) if(!(cond)){ expr;        return ret;}
#define is1_exeret(cond, expr, ret) if( (cond)){ expr;        return ret;}
#define is0_elsret(cond, expr, ret) if(!(cond)){ expr;} else{ return ret;}
#define is1_elsret(cond, expr, ret) if( (cond)){ expr;} else{ return ret;}

#define RANDBYTES (16)

/*
 * This is a best effort implementation. Nothing prevents a compiler from
 * optimizing this function and making it vulnerable to timing attacks, but
 * this method is commonly used in crypto libraries like NaCl.
 *
 * Return value is zero if both strings are equal and nonzero otherwise.
*/
static int timing_safe_strcmp(const char *str1, const char *str2)
{
    const unsigned char *u1;
    const unsigned char *u2;
    int ret;
    int i;

    int len1 = strlen(str1);
    int len2 = strlen(str2);

    /* In our context both strings should always have the same length
     * because they will be hashed passwords. */
    if (len1 != len2)
        return 1;

    /* Force unsigned for bitwise operations. */
    u1 = (const unsigned char *)str1;
    u2 = (const unsigned char *)str2;

    ret = 0;
    for (i = 0; i < len1; ++i)
        ret |= (u1[i] ^ u2[i]);

    return ret;
}

/*
 * This function expects a work factor between 4 and 31 and a char array to
 * store the resulting generated salt. The char array should typically have
 * BCRYPT_HASHSIZE bytes at least. If the provided work factor is not in the
 * previous range, it will default to 12.
 *
 * retutn 1 generate ok
 *        0 faild
 *
 */
int  ecrypt_gensalt(uint   factor, char   salt[ECRYPT_SIZE])
{
    static uint seed; char input[RANDBYTES]; int workf, i; char *aux;

    seed += time(0);
    srand(seed);

    for(i = 0; i < RANDBYTES; i++) input[i] = rand()%255;

    /* Generate salt. */
    workf = (factor < 4 || factor > 31) ? 12 : factor;
    aux   = crypt_gensalt_rn("$2a$", workf, input, RANDBYTES, salt, ECRYPT_SIZE);

    return aux ? 1 : 0;
}

/*
 * This function expects a password to be hashed, a salt to hash the password
 * with and a char array to leave the result. Both the salt and the hash
 * parameters should have room for BCRYPT_HASHSIZE characters at least.
 *
 * It can also be used to verify a hashed password. In that case, provide the
 * expected hash in the salt parameter and verify the output hash is the same
 * as the input hash. However, to avoid timing attacks, it's better to use
 * bcrypt_checkpw when verifying a password.
 *
 */
estr ecrypt_hashs  (constr passwd, conchr salt[ECRYPT_SIZE])
{
    estr hash;

    is0_ret(hash = estr_newLen(0, ECRYPT_SIZE), 0);

    is0_exeret(crypt_rn(passwd, salt, hash, ECRYPT_SIZE), estr_free(hash), 0);
    estr_incrLen(hash, strlen(hash));

    return hash;
}

int  ecrypt_hashs2s(constr passwd, conchr salt[ECRYPT_SIZE], char hash[ECRYPT_SIZE])
{
    return crypt_rn(passwd, salt, hash, ECRYPT_SIZE) ? 1 : 0;
}

estr ecrypt_encs  (constr passwd)
{
    estr hash; char salt[ECRYPT_SIZE];

    is0_ret(ecrypt_gensalt(12, salt), 0);

    is0_ret(hash = estr_newLen(0, ECRYPT_SIZE), 0);

    is0_exeret(crypt_rn(passwd, salt, hash, ECRYPT_SIZE), estr_free(hash), 0);
    estr_incrLen(hash, strlen(hash));

    return hash;
}

int  ecrypt_encs2s(constr passwd, char hash[ECRYPT_SIZE])
{
    char salt[ECRYPT_SIZE];

    is0_ret(ecrypt_gensalt(12, salt), 0);

    return crypt_rn(passwd, salt, hash, ECRYPT_SIZE) ? 1 : 0;
}

int  ecrypt_check (constr passwd, conchr hash[ECRYPT_SIZE])
{
    char outhash[ECRYPT_SIZE];

    is0_ret(ecrypt_hashs2s(passwd, hash, outhash), 0);

    return !timing_safe_strcmp(hash, outhash);
}

inline void   ecrypt_show(estr s) {        estr_show(s);}
inline size   ecrypt_dlen(estr s) { return estr_len (s);}
inline void   ecrypt_free(estr s) {        estr_free(s);}

estr ecrypt_version()
{
    static char buf[16];
    static sstr ver;

    if(!ver)
    {
        ver = sstr_init(buf, sizeof(buf) -1);
        sstr_wrtS(ver, ECRYPT_VERSION);
    }

    return ver;
}
