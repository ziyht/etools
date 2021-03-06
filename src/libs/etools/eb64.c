/// =====================================================================================
///
///       Filename:  eb64.c
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

#include "eb64.h"
#include "libbase64.h"

/// ----------------- micros helper -----------------------

#define exe_ret(expr, ret ) { expr;      return ret;}
#define is0_ret(cond, ret ) if(!(cond)){ return ret;}
#define is1_ret(cond, ret ) if( (cond)){ return ret;}
#define is0_exe(cond, expr) if(!(cond)){ expr;}
#define is1_exe(cond, expr) if( (cond)){ expr;}

#define is0_exeret(cond, expr, ret) if(!(cond)){ expr;        return ret;}
#define is1_exeret(cond, expr, ret) if( (cond)){ expr;        return ret;}
#define is0_elsret(cond, expr, ret) if(!(cond)){ expr;} else{ return ret;}
#define is1_elsret(cond, expr, ret) if( (cond)){ expr;} else{ return ret;}

/// ------------------ win32 API setting -------------------
#if (_WIN32)
#define inline
#endif

/// ---------------------- encoder -------------------------

estr eb64_encb  (conptr in, size inlen)
{
    estr out; size outlen;

    is0_ret(in, 0); is0_ret(inlen, 0);

    is0_ret(out = estr_newLen(0, (inlen + 2) / 3 * 4), 0);

    base64_encode(in, inlen, out, &outlen, 0);

    estr_incrLen(out, outlen);

    return out;
}

int  eb64_encb2b(conptr in, size inlen, cptr   out  , size* outlen)
{
    is0_ret(in, 0); is0_ret(inlen, 0); is0_ret(out, 0); is0_ret(outlen, 0);

    base64_encode(in, inlen, out, outlen, 0);

    return 1;
}

/// ---------------------- decoder -------------------------

estr eb64_decb  (conptr in, size inlen)
{
    estr out; size outlen;

    is0_ret(in, 0); is0_ret(inlen, 0);

    is0_ret(out = estr_newLen(0, (inlen) / 4 * 3), 0);

    if(!base64_decode(in, inlen, out, &outlen, 0))
    {
        estr_free(out);
        return 0;
    }

    estr_incrLen(out, outlen);

    return out;
}

int  eb64_decb2b(conptr in, size inlen, cptr   out  , size* outlen)
{
    is0_ret(in, 0); is0_ret(inlen, 0); is0_ret(out, 0); is0_ret(outlen, 0);

    return base64_decode(in, inlen, out, outlen, 0);
}

inline void   eb64_show(estr s) {        estr_show(s);}
inline size   eb64_dlen(estr s) { return estr_len (s);}
inline void   eb64_free(estr s) {        estr_free(s);}

estr eb64_version()
{
    static char buf[16];
    static sstr ver;

    if(!ver)
    {
        ver = sstr_init(buf, sizeof(buf) -1);
        sstr_wrtS(ver, EB64_VERSION);
    }

    return ver;
}
