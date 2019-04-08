/**
* this file is create by subsystem 'ETest' from EMake framework
*/

#include <etest.h>

#include "ejson.h"

ejson e;

static int ejson_new_test()
{
    e = ejson_new(EFALSE, 0); eunexpc_ptr(e, 0); eexpect_num(ejson_type(e), EFALSE); eexpect_num(ejson_len(e), 0); ejson_free(e);
    e = ejson_new(ETRUE , 0); eunexpc_ptr(e, 0); eexpect_num(ejson_type(e), ETRUE ); eexpect_num(ejson_len(e), 0); ejson_free(e);
    e = ejson_new(ENULL , 0); eunexpc_ptr(e, 0); eexpect_num(ejson_type(e), ENULL ); eexpect_num(ejson_len(e), 0); ejson_free(e);
    e = ejson_new(ENUM  , 0); eunexpc_ptr(e, 0); eexpect_num(ejson_type(e), ENUM  ); eexpect_num(ejson_len(e), 0); ejson_free(e);
    e = ejson_new(ESTR  , 9); eunexpc_ptr(e, 0); eexpect_num(ejson_type(e), ENUM  ); eexpect_num(ejson_len(e), 9); ejson_free(e);
    e = ejson_new(EPTR  , 0); eunexpc_ptr(e, 0); eexpect_num(ejson_type(e), EPTR  ); eexpect_num(ejson_len(e), 0); ejson_free(e);
    e = ejson_new(ERAW  , 9); eunexpc_ptr(e, 0); eexpect_num(ejson_type(e), ERAW  ); eexpect_num(ejson_len(e), 9); ejson_free(e);
    e = ejson_new(EOBJ  , 0); eunexpc_ptr(e, 0); eexpect_num(ejson_type(e), EOBJ  ); eexpect_num(ejson_len(e), 0); ejson_free(e);
    e = ejson_new(EARR  , 0); eunexpc_ptr(e, 0); eexpect_num(ejson_type(e), EARR  ); eexpect_num(ejson_len(e), 0); ejson_free(e);

    return ETEST_OK;
}

static int ejson_add_test()
{
    ejson o; int i; char key[32] = "012345678901234567890", zero[32] = {0};

    e = ejson_new(EOBJ, 0);

    i = -1;
    ++i; ejson_addT(e, &key[i], EFALSE);
    ++i; ejson_addT(e, &key[i], ETRUE );
    ++i; ejson_addT(e, &key[i], ENULL );
    ++i; ejson_addI(e, &key[i], 1);
    ++i; ejson_addF(e, &key[i], 2.0);
    ++i; ejson_addS(e, &key[i], &key[i]);
    ++i; ejson_addP(e, &key[i], &key[i]);
    ++i; ejson_addR(e, &key[i], 9);
    ++i; ejson_addT(e, &key[i], EOBJ);
    ++i; ejson_addT(e, &key[i], EARR);

    eexpect_num(ejson_len(e), i);

    i = -1;

    ++i;
    o = ejson_valr(e, &key[i]);
    eexpect_num(ejson_type(o), EFALSE); eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(e, &key[i]);
    eexpect_num(ejson_type(o), ETRUE) ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(e, &key[i]);
    eexpect_num(ejson_type(o), ENULL) ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(e, &key[i]);
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_valI(o), 1)      ; eexpect_num(ejson_valrI(   e, &key[i]), eobj_valI( o));
    eexpect_num(eobj_valF(o), 1.0)    ; eexpect_num(ejson_valrF(   e, &key[i]), eobj_valF( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_valI(o), 2)      ; eexpect_num(ejson_valrI(   e, &key[i]), eobj_valI( o));
    eexpect_num(eobj_valF(o), 2.0)    ; eexpect_num(ejson_valrF(   e, &key[i]), eobj_valF( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eexpect_num(ejson_type(o), ESTR)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_str(eobj_valS(o), &key[i]); eexpect_str(ejson_valrS(   e, &key[i]), eobj_valS( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eexpect_num(ejson_type(o), EPTR)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_ptr(eobj_valP(o), &key[i]); eexpect_ptr(ejson_valrP(   e, &key[i]), eobj_valP( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eexpect_num(ejson_type(o), ERAW)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 9   )  ; eexpect_num(ejson_valrLen( e, &key[i]), eobj_len(  o));
    eexpect_raw(eobj_valR(o) , zero, 9); eexpect_raw(ejson_valrR(  e, &key[i]), eobj_valR( o), 9);

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eexpect_num(ejson_type(o), EOBJ)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejson_valrLen( e, &key[i]), eobj_len(  o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eexpect_num(ejson_type(o), EARR)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejson_valrLen( e, &key[i]), eobj_len(  o));

    eexpect_num(ejson_free(e), i + 1);

    return ETEST_OK;
}

int 1_basic_test(int argc, char* argv[])
{
    ETEST_RUN( ejson_new_test() );
    ETEST_RUN( ejson_add_test() );

    return ETEST_OK;
}

