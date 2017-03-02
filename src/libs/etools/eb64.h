/*
   eb64(easy base64) - a easier way to handle base64

   Author: ziyht


*/

#ifndef __EB64_H__
#define __EB64_H__

#define EB64_VERSION "1.0.0"      // new tool

#include "etype.h"

#ifdef __cplusplus
extern "C" {
#endif

cstr eb64_encode(conptr ptr, size len);
cptr eb64_decode(constr src, size len);

int  eb64_encode2(conptr ptr, size len, cstr buf, size* outlen);
int  eb64_decode2(constr src, size len, cstr buf, size* outlen);

void eb64_show(constr s);
size eb64_slen(constr s);
void eb64_free(constr s);


#ifdef __cplusplus
}
#endif

#endif
