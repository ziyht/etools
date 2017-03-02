/*
   edes(easy des) - a easier des encoder/decoder

   Author: ziyht


*/

#ifndef __EDES_H__
#define __EDES_H__

#define EB64_VERSION "1.0.0"      // new tool

#include "etype.h"


/*---------------------

  b -- binary data
  f -- file

-----------------------*/

cstr edes_newkey(cstr key);         // 8 bit len

cptr edes_encb  (constr key, conptr in, size inlen);
cptr edes_encb2b(constr key, conptr in, size inlen, conptr out , size* outlen);
cptr edes_encb2f(constr key, conptr in, size inlen, constr outf);

cptr edes_encf();
cptr edes_encf2b();
cptr edes_encf2f();

cptr edes_decb  (constr key, conptr in, size inlen);
cptr edes_decb2b();
cptr edes_decb2f();

cptr edes_decf();
cptr edes_decf2b();
cptr edes_decf2f();

void edes_show(constr s);
size edes_slen(constr s);
void edes_free(constr s);

#ifdef __cplusplus
extern "C" {
#endif

#endif
