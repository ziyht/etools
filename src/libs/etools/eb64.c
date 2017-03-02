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

cstr eb64_encode(conptr ptr, size len)
{
    estr e; size outlen;

    is0_ret(ptr, 0); is0_ret(len, 0);

    is0_ret(e = estr_newLen(0, (len + 2) / 3 * 4), 0);

    base64_encode(ptr, len, e, &outlen, 0);

    estr_incrLen(e, outlen);

    return e;
}

cptr eb64_decode(constr src, size len)
{
    estr e; size outlen;

    is0_ret(src, 0); is0_ret(len, 0);

    is0_ret(e = estr_newLen(0, (len) / 4 * 3), 0);

    base64_decode(src, len, e, &outlen, 0);

    estr_incrLen(e, outlen);

    return e;
}

int  eb64_encode2(conptr ptr, size len, cstr buf, size* outlen)
{
    is0_ret(ptr, 0); is0_ret(len, 0); is0_ret(buf, 0); is0_ret(outlen, 0);

    base64_encode(ptr, len, buf, outlen, 0);

    return 1;
}

int  eb64_decode2(constr src, size len, cstr buf, size* outlen)
{
    is0_ret(src, 0); is0_ret(len, 0); is0_ret(buf, 0); is0_ret(outlen, 0);

    base64_decode(src, len, buf, outlen, 0);

    return 1;
}

inline void   eb64_show(constr s)
{
    estr_shows((estr)s);
}

inline size   eb64_slen(constr s)
{
    return estr_len((estr)s);
}

inline void   eb64_free(constr s)
{
    estr_free((estr)s);
}


