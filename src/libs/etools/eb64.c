#include "eb64.h"
#include "libbase64.h"

#include "estr.h"

#define exe_ret(expr, ret ) { expr;      return ret;}
#define is0_ret(cond, ret ) if(!(cond)){ return ret;}
#define is1_ret(cond, ret ) if( (cond)){ return ret;}
#define is0_exe(cond, expr) if(!(cond)){ expr;}
#define is1_exe(cond, expr) if( (cond)){ expr;}

#define is0_exeret(cond, expr, ret) if(!(cond)){ expr;        return ret;}
#define is1_exeret(cond, expr, ret) if( (cond)){ expr;        return ret;}
#define is0_elsret(cond, expr, ret) if(!(cond)){ expr;} else{ return ret;}
#define is1_elsret(cond, expr, ret) if( (cond)){ expr;} else{ return ret;}

/// ---------------------- encoder -------------------------

cstr eb64_encb  (conptr in, size inlen)
{
    estr out; size outlen;

    is0_ret(in, 0); is0_ret(inlen, 0);

    is0_ret(out = estr_newLen(0, (inlen + 2) / 3 * 4), 0);

    base64_encode(in, inlen, out, &outlen, 0);

    estr_incrLen(out, outlen);

    return out;
}

int  eb64_encb2b(conptr in, size inlen, cstr   out  , size* outlen)
{
    is0_ret(in, 0); is0_ret(inlen, 0); is0_ret(out, 0); is0_ret(outlen, 0);

    base64_encode(in, inlen, out, outlen, 0);

    return 1;
}

/// ---------------------- decoder -------------------------

cptr eb64_decb  (constr in, size inlen)
{
    estr out; size outlen;

    is0_ret(in, 0); is0_ret(inlen, 0);

    is0_ret(out = estr_newLen(0, (inlen) / 4 * 3), 0);

    base64_decode(in, inlen, out, &outlen, 0);

    estr_incrLen(out, outlen);

    return out;
}

int  eb64_decb2b(constr in, size inlen, cptr   out  , size* outlen)
{
    is0_ret(in, 0); is0_ret(inlen, 0); is0_ret(out, 0); is0_ret(outlen, 0);

    base64_decode(in, inlen, out, outlen, 0);

    return 1;
}

inline void   eb64_show(conptr d) {        estr_shows((estr)d);}
inline size   eb64_dlen(conptr d) { return estr_len  ((estr)d);}
inline void   eb64_free(conptr d) {        estr_free ((estr)d);}


