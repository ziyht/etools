/**
* this file is create by subsystem 'ETest' from EMake framework
*/

#include <etest.h>

#include "eutils.h"

#include "ejson.h"

ejson r;

static int ejson_new_test()
{
    r = ejson_new(EFALSE, 0); eunexpc_ptr(r, 0); eexpect_num(ejson_type(r), EFALSE); eexpect_num(ejson_len(r), 0); ejson_free(r);
    r = ejson_new(ETRUE , 0); eunexpc_ptr(r, 0); eexpect_num(ejson_type(r), ETRUE ); eexpect_num(ejson_len(r), 0); ejson_free(r);
    r = ejson_new(ENULL , 0); eunexpc_ptr(r, 0); eexpect_num(ejson_type(r), ENULL ); eexpect_num(ejson_len(r), 0); ejson_free(r);
    r = ejson_new(ENUM  , 0); eunexpc_ptr(r, 0); eexpect_num(ejson_type(r), ENUM  ); eexpect_num(ejson_len(r), 0); ejson_free(r);
    r = ejson_new(ESTR  , 9); eunexpc_ptr(r, 0); eexpect_num(ejson_type(r), ESTR  ); eexpect_num(ejson_len(r), 9); ejson_free(r);
    r = ejson_new(EPTR  , 0); eunexpc_ptr(r, 0); eexpect_num(ejson_type(r), EPTR  ); eexpect_num(ejson_len(r), 0); ejson_free(r);
    r = ejson_new(ERAW  , 9); eunexpc_ptr(r, 0); eexpect_num(ejson_type(r), ERAW  ); eexpect_num(ejson_len(r), 9); ejson_free(r);
    r = ejson_new(EOBJ  , 0); eunexpc_ptr(r, 0); eexpect_num(ejson_type(r), EOBJ  ); eexpect_num(ejson_len(r), 0); ejson_free(r);
    r = ejson_new(EARR  , 0); eunexpc_ptr(r, 0); eexpect_num(ejson_type(r), EARR  ); eexpect_num(ejson_len(r), 0); ejson_free(r);

    return ETEST_OK;
}

static int ejson_add_test1()
{
    ejson o; int i; char key[32] = "012345678901234567890", zero[32] = {0};

    r = ejson_new(EOBJ, 0);

    i = -1;
    ++i; ejson_addT(r, &key[i], EFALSE);
    ++i; ejson_addT(r, &key[i], ETRUE );
    ++i; ejson_addT(r, &key[i], ENULL );
    ++i; ejson_addI(r, &key[i], 1);
    ++i; ejson_addF(r, &key[i], 2.0);
    ++i; ejson_addS(r, &key[i], &key[i]);
    ++i; ejson_addP(r, &key[i], &key[i]);
    ++i; ejson_addR(r, &key[i], 9);
    ++i; ejson_addT(r, &key[i], EOBJ);
    ++i; ejson_addT(r, &key[i], EARR);

    eexpect_num(ejson_isEmpty(r), 0);
    eexpect_num(ejson_len(r), ++i);

    i = -1;

    ++i;
    o = ejson_valr(r, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EFALSE); eexpect_num(ejsonr_type(r, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(r, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ETRUE) ; eexpect_num(ejsonr_type(r, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(r, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENULL) ; eexpect_num(ejsonr_type(r, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(r, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejsonr_type(r, &key[i]), ejson_type(o));
    eexpect_num(eobj_valI(o), 1)      ; eexpect_num(ejsonr_valI(   r, &key[i]), eobj_valI( o));
    eexpect_num(eobj_valF(o), 1.0)    ; eexpect_num(ejsonr_valF(   r, &key[i]), eobj_valF( o));

    ++i;
    o = ejson_valr(r, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejsonr_type(r, &key[i]), ejson_type(o));
    eexpect_num(eobj_valI(o), 2)      ; eexpect_num(ejsonr_valI(   r, &key[i]), eobj_valI( o));
    eexpect_num(eobj_valF(o), 2.0)    ; eexpect_num(ejsonr_valF(   r, &key[i]), eobj_valF( o));

    ++i;
    o = ejson_valr(r, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ESTR)  ; eexpect_num(ejsonr_type(r, &key[i]), ejson_type(o));
    eexpect_str(eobj_valS(o), &key[i]); eexpect_str(ejsonr_valS(   r, &key[i]), eobj_valS( o));

    ++i;
    o = ejson_valr(r, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EPTR)  ; eexpect_num(ejsonr_type(r, &key[i]), ejson_type(o));
    eexpect_ptr(eobj_valP(o), &key[i]); eexpect_ptr(ejsonr_valP(   r, &key[i]), eobj_valP( o));

    ++i;
    o = ejson_valr(r, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ERAW)  ; eexpect_num(ejsonr_type(r, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 9   )  ; eexpect_num(ejsonr_len ( r, &key[i]), eobj_len(  o));
    eexpect_raw(eobj_valR(o) , zero, 9); eexpect_raw(ejsonr_valR(  r, &key[i]), eobj_valR( o), 9);

    ++i;
    o = ejson_valr(r, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EOBJ)  ; eexpect_num(ejsonr_type(r, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejsonr_len( r, &key[i]), eobj_len(  o));

    ++i;
    o = ejson_valr(r, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EARR)  ; eexpect_num(ejsonr_type(r, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejsonr_len( r, &key[i]), eobj_len(  o));

    eexpect_num(ejson_free(r), ++i + 1);

    return ETEST_OK;
}

static int ejson_add_test2()
{
    ejson o; int i; char key[32] = "012345678901234567890", zero[32] = {0};

    r = ejson_new(EARR, 0);

    i = -1;
    ++i; ejson_addT(r, 0, EFALSE);
    ++i; ejson_addT(r, 0, ETRUE );
    ++i; ejson_addT(r, 0, ENULL );
    ++i; ejson_addI(r, 0, 1);
    ++i; ejson_addF(r, 0, 2.0);
    ++i; ejson_addS(r, 0, &key[i]);
    ++i; ejson_addP(r, 0, &key[i]);
    ++i; ejson_addR(r, 0, 9);
    ++i; ejson_addT(r, 0, EOBJ);
    ++i; ejson_addT(r, 0, EARR);

    eexpect_num(ejson_isEmpty(r), 0);
    eexpect_num(ejson_len(r), ++i);

    i = -1;

    ++i;
    o = ejson_vali(r, i);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EFALSE); eexpect_num(ejsoni_type(r, i), ejson_type(o));

    ++i;
    o = ejson_vali(r, i);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ETRUE) ; eexpect_num(ejsoni_type(r, i), ejson_type(o));

    ++i;
    o = ejson_vali(r, i);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENULL) ; eexpect_num(ejsoni_type(r, i), ejson_type(o));

    ++i;
    o = ejson_vali(r, i);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejsoni_type(r, i), ejson_type(o));
    eexpect_num(eobj_valI(o), 1)      ; eexpect_num(ejsoni_valI(   r, i), eobj_valI( o));
    eexpect_num(eobj_valF(o), 1.0)    ; eexpect_num(ejsoni_valF(   r, i), eobj_valF( o));

    ++i;
    o = ejson_vali(r, i)        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejsoni_type(r, i), ejson_type(o));
    eexpect_num(eobj_valI(o), 2)      ; eexpect_num(ejsoni_valI(   r, i), eobj_valI( o));
    eexpect_num(eobj_valF(o), 2.0)    ; eexpect_num(ejsoni_valF(   r, i), eobj_valF( o));

    ++i;
    o = ejson_vali(r, i)        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ESTR)  ; eexpect_num(ejsoni_type(r, i), ejson_type(o));
    eexpect_str(eobj_valS(o), &key[i]); eexpect_str(ejsoni_valS(   r, i), eobj_valS( o));

    ++i;
    o = ejson_vali(r, i)        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EPTR)  ; eexpect_num(ejsoni_type(r, i), ejson_type(o));
    eexpect_ptr(eobj_valP(o), &key[i]); eexpect_ptr(ejsoni_valP(   r, i), eobj_valP( o));

    ++i;
    o = ejson_vali(r, i)        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ERAW)  ; eexpect_num(ejsoni_type(r, i), ejson_type(o));
    eexpect_num(eobj_len(o)  , 9   )  ; eexpect_num(ejsoni_len( r, i), eobj_len(  o));
    eexpect_raw(eobj_valR(o) , zero, 9); eexpect_raw(ejsoni_valR(  r, i), eobj_valR( o), 9);

    ++i;
    o = ejson_vali(r, i)        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EOBJ)  ; eexpect_num(ejsoni_type(r, i), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejsoni_len( r, i), eobj_len(  o));

    ++i;
    o = ejson_vali(r, i)        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EARR)  ; eexpect_num(ejsoni_type(r, i), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejsoni_len( r, i), eobj_len(  o));

    eexpect_num(ejson_free(r), ++i + 1);

    return ETEST_OK;
}


static int ejson_radd_test()
{
    ejson e, o; int i; char key[32] = "012345678901234567890", zero[32] = {0};

#undef  KEY1
#undef  KEY2
#undef  RKEY
#define KEY1 "layer1"
#define KEY2 "layer2"
#define RKEY KEY1"."KEY2

    r = ejson_new(EOBJ, 0);
    e = ejson_addT(ejson_addT(r, KEY1, EOBJ), KEY2, EOBJ);
    e = ejson_addT(r, RKEY, EOBJ);

    /**
      * {
      *     "layer1": {
      *         "layer2": {}
      *     },
      *     "layer1.layer2": {}      <-- e
      * }
      */

    i = -1;
    i++; ejson_raddT(r, RKEY, &key[i], EFALSE);
    i++; ejson_raddT(r, RKEY, &key[i], ETRUE);
    i++; ejson_raddT(r, RKEY, &key[i], ENULL);
    i++; ejson_raddI(r, RKEY, &key[i], 1);
    i++; ejson_raddF(r, RKEY, &key[i], 2.0);
    i++; ejson_raddS(r, RKEY, &key[i], &key[i]);
    i++; ejson_raddP(r, RKEY, &key[i], &key[i]);
    i++; ejson_raddR(r, RKEY, &key[i], 9);
    i++; ejson_raddT(r, RKEY, &key[i], EOBJ);
    i++; ejson_raddT(r, RKEY, &key[i], EARR);

    eexpect_num(ejson_isEmpty(e), 0);
    eexpect_num(ejson_len(e), ++i);

    i = -1;

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EFALSE); eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ETRUE) ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENULL) ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_valI(o), 1)      ; eexpect_num(ejsonr_valI(   e, &key[i]), eobj_valI( o));
    eexpect_num(eobj_valF(o), 1.0)    ; eexpect_num(ejsonr_valF(   e, &key[i]), eobj_valF( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_valI(o), 2)      ; eexpect_num(ejsonr_valI(   e, &key[i]), eobj_valI( o));
    eexpect_num(eobj_valF(o), 2.0)    ; eexpect_num(ejsonr_valF(   e, &key[i]), eobj_valF( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ESTR)  ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));
    eexpect_str(eobj_valS(o), &key[i]); eexpect_str(ejsonr_valS(   e, &key[i]), eobj_valS( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EPTR)  ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));
    eexpect_ptr(eobj_valP(o), &key[i]); eexpect_ptr(ejsonr_valP(   e, &key[i]), eobj_valP( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ERAW)  ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 9   )  ; eexpect_num(ejsonr_len ( e, &key[i]), eobj_len(  o));
    eexpect_raw(eobj_valR(o) , zero, 9); eexpect_raw(ejsonr_valR(  e, &key[i]), eobj_valR( o), 9);

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EOBJ)  ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejsonr_len( e, &key[i]), eobj_len(  o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EARR)  ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejsonr_len( e, &key[i]), eobj_len(  o));

    eexpect_num(ejson_free(r), ++i + 4);

    return ETEST_OK;
}

static int ejson_kadd_test()
{
    ejson e, o; int i; char key[32] = "012345678901234567890", zero[32] = {0};

#undef  KEY1
#undef  KEY2
#undef  KEYS
#define KEY1 "layer1"
#define KEY2 "layer2"
#define KEYS KEY1"."KEY2

    r = ejson_new(EOBJ, 0);
    e = ejson_addT(r, KEYS, EOBJ);
    e = ejson_addT(ejson_addT(r, KEY1, EOBJ), KEY2, EOBJ);

    /**
      * {
      *     "layer1": {
      *         "layer2": {}        <-- e
      *     },
      *     "layer1.layer2": {}
      * }
      */

    i = -1;
    i++; ejson_kaddT(r, KEYS, &key[i], EFALSE);
    i++; ejson_kaddT(r, KEYS, &key[i], ETRUE);
    i++; ejson_kaddT(r, KEYS, &key[i], ENULL);
    i++; ejson_kaddI(r, KEYS, &key[i], 1);
    i++; ejson_kaddF(r, KEYS, &key[i], 2.0);
    i++; ejson_kaddS(r, KEYS, &key[i], &key[i]);
    i++; ejson_kaddP(r, KEYS, &key[i], &key[i]);
    i++; ejson_kaddR(r, KEYS, &key[i], 9);
    i++; ejson_kaddT(r, KEYS, &key[i], EOBJ);
    i++; ejson_kaddT(r, KEYS, &key[i], EARR);

    eexpect_num(ejson_isEmpty(e), 0);
    eexpect_num(ejson_len(e), ++i);

    i = -1;

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EFALSE); eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ETRUE) ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENULL) ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_valI(o), 1)      ; eexpect_num(ejsonr_valI(   e, &key[i]), eobj_valI( o));
    eexpect_num(eobj_valF(o), 1.0)    ; eexpect_num(ejsonr_valF(   e, &key[i]), eobj_valF( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_valI(o), 2)      ; eexpect_num(ejsonr_valI(   e, &key[i]), eobj_valI( o));
    eexpect_num(eobj_valF(o), 2.0)    ; eexpect_num(ejsonr_valF(   e, &key[i]), eobj_valF( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ESTR)  ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));
    eexpect_str(eobj_valS(o), &key[i]); eexpect_str(ejsonr_valS(   e, &key[i]), eobj_valS( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EPTR)  ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));
    eexpect_ptr(eobj_valP(o), &key[i]); eexpect_ptr(ejsonr_valP(   e, &key[i]), eobj_valP( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ERAW)  ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 9   )  ; eexpect_num(ejsonr_len( e, &key[i]), eobj_len(  o));
    eexpect_raw(eobj_valR(o) , zero, 9); eexpect_raw(ejsonr_valR(  e, &key[i]), eobj_valR( o), 9);

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EOBJ)  ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejsonr_len( e, &key[i]), eobj_len(  o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EARR)  ; eexpect_num(ejsonr_type(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejsonr_len( e, &key[i]), eobj_len(  o));

    eexpect_num(ejson_free(r), ++i + 4);

    return ETEST_OK;
}

static int ejson_addJ_test()
{
    typedef struct { cstr key; cstr json; etypeo t;} _IN_;

    int i; _IN_* map;

    // key in : param: no  | src: yes
    _IN_ map1[] = {
                {"false" , "\"false\":false"    , EFALSE,    },
                {"true"  , "\"true\":true"      , ETRUE ,    },
                {"null"  , "\"null\":null"      , ENULL ,    },
                {"int"   , "\"int\": 100"       , ENUM  ,    },
                {"double", "\"double\":100.132" , ENUM  ,    },
                {"str"   , "\"str\":\"hello\""  , ESTR  ,    },
                {"obj"   , "\"obj\": {}"        , EOBJ  ,    },
                {"arr"   , "\"arr\": []"        , EARR  ,    },
    };
    map = map1;

    r = ejson_new(EOBJ, 0); i = -1;
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    eexpect_num(ejson_free(r), ++i + 1);

    // key in : param: yes | src: yes  (key in src will be skipped)
    _IN_ map2[] = {
                {"false" , "\"1\":false"        , EFALSE,    },
                {"true"  , "\"2\":true"         , ETRUE ,    },
                {"null"  , "\"3\":null"         , ENULL ,    },
                {"int"   , "\"4\": 100"         , ENUM  ,    },
                {"double", "\"5\":100.132"      , ENUM  ,    },
                {"str"   , "\"6\":\"hello\""    , ESTR  ,    },
                {"obj"   , "\"7\": {}"          , EOBJ  ,    },
                {"arr"   , "\"8\": []"          , EARR  ,    },
    };
    map = map2;

    r = ejson_new(EOBJ, 0); i = -1;
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    eexpect_num(ejson_free(r), ++i + 1);

    // key in : param: yes | src: no
    _IN_ map3[] = {
                {"false" , "\"1\":false"        , EFALSE,    },
                {"true"  , "\"2\":true"         , ETRUE ,    },
                {"null"  , "\"3\":null"         , ENULL ,    },
                {"int"   , "\"4\": 100"         , ENUM  ,    },
                {"double", "\"5\":100.132"      , ENUM  ,    },
                {"str"   , "\"6\":\"hello\""    , ESTR  ,    },
                {"obj"   , "\"7\": {}"          , EOBJ  ,    },
                {"arr"   , "\"8\": []"          , EARR  ,    },
    };
    map = map3;

    r = ejson_new(EOBJ, 0); i = -1;
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejsonr_type(r, map[i].key), map[i].t );
    eexpect_num(ejson_free(r), ++i + 1);

    return ETEST_OK;
}


static int ejson_raddJ_test()
{
    typedef struct { cstr key; cstr json; etypeo t;} _IN_;

    int i; _IN_* map; ejson e;

#undef  KEY1
#undef  KEY2
#undef  RKEY
#define KEY1 "layer1"
#define KEY2 "layer2"
#define RKEY KEY1"."KEY2

    r = ejson_new(EOBJ, 0);
    e = ejson_addT(ejson_addT(r, KEY1, EOBJ), KEY2, EOBJ);
    e = ejson_addT(r, RKEY, EOBJ);

    /**
      * {
      *     "layer1": {
      *         "layer2": {}
      *     },
      *     "layer1.layer2": {}      <-- e
      * }
      */

    // key in : param: no  | src: yes
    _IN_ map1[] = {
                {"false" , "\"false\":false"    , EFALSE,    },
                {"true"  , "\"true\":true"      , ETRUE ,    },
                {"null"  , "\"null\":null"      , ENULL ,    },
                {"int"   , "\"int\": 100"       , ENUM  ,    },
                {"double", "\"double\":100.132" , ENUM  ,    },
                {"str"   , "\"str\":\"hello\""  , ESTR  ,    },
                {"obj"   , "\"obj\": {}"        , EOBJ  ,    },
                {"arr"   , "\"arr\": []"        , EARR  ,    },
    };
    map = map1;

    i = -1;
    i++; ejson_raddJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    eexpect_num(ejson_clear(e), ++i);
    eexpect_num(ejson_isEmpty(e), 1);

    // key in : param: yes | src: yes  (key in src will be skipped)
    _IN_ map2[] = {
                {"false" , "\"1\":false"        , EFALSE,    },
                {"true"  , "\"2\":true"         , ETRUE ,    },
                {"null"  , "\"3\":null"         , ENULL ,    },
                {"int"   , "\"4\": 100"         , ENUM  ,    },
                {"double", "\"5\":100.132"      , ENUM  ,    },
                {"str"   , "\"6\":\"hello\""    , ESTR  ,    },
                {"obj"   , "\"7\": {}"          , EOBJ  ,    },
                {"arr"   , "\"8\": []"          , EARR  ,    },
    };
    map = map2;

    i = -1;
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    eexpect_num(ejson_clear(e), ++i);
    eexpect_num(ejson_isEmpty(e), 1);

    // key in : param: yes | src: no
    _IN_ map3[] = {
                {"false" , "\"1\":false"        , EFALSE,    },
                {"true"  , "\"2\":true"         , ETRUE ,    },
                {"null"  , "\"3\":null"         , ENULL ,    },
                {"int"   , "\"4\": 100"         , ENUM  ,    },
                {"double", "\"5\":100.132"      , ENUM  ,    },
                {"str"   , "\"6\":\"hello\""    , ESTR  ,    },
                {"obj"   , "\"7\": {}"          , EOBJ  ,    },
                {"arr"   , "\"8\": []"          , EARR  ,    },
    };
    map = map3;

    i = -1;
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_raddJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    eexpect_num(ejson_clear(e), ++i);
    eexpect_num(ejson_isEmpty(e), 1);

    eexpect_num(ejson_free(r), 4);

    return ETEST_OK;
}

static int ejson_kaddJ_test()
{
    typedef struct { cstr key; cstr json; etypeo t;} _IN_;

    int i; _IN_* map; ejson e;

#undef  KEY1
#undef  KEY2
#undef  KEYS
#define KEY1 "layer1"
#define KEY2 "layer2"
#define KEYS KEY1"."KEY2

    r = ejson_new(EOBJ, 0);
    e = ejson_addT(r, RKEY, EOBJ);
    e = ejson_addT(ejson_addT(r, KEY1, EOBJ), KEY2, EOBJ);

    /**
      * {
      *     "layer1": {
      *         "layer2": {}      <-- e
      *     },
      *     "layer1.layer2": {}
      * }
      */

    // key in : param: no  | src: yes
    _IN_ map1[] = {
                {"false" , "\"false\":false"    , EFALSE,    },
                {"true"  , "\"true\":true"      , ETRUE ,    },
                {"null"  , "\"null\":null"      , ENULL ,    },
                {"int"   , "\"int\": 100"       , ENUM  ,    },
                {"double", "\"double\":100.132" , ENUM  ,    },
                {"str"   , "\"str\":\"hello\""  , ESTR  ,    },
                {"obj"   , "\"obj\": {}"        , EOBJ  ,    },
                {"arr"   , "\"arr\": []"        , EARR  ,    },
    };
    map = map1;

    i = -1;
    i++; ejson_kaddJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    eexpect_num(ejson_clear(e), ++i);
    eexpect_num(ejson_isEmpty(e), 1);

    // key in : param: yes | src: yes  (key in src will be skipped)
    _IN_ map2[] = {
                {"false" , "\"1\":false"        , EFALSE,    },
                {"true"  , "\"2\":true"         , ETRUE ,    },
                {"null"  , "\"3\":null"         , ENULL ,    },
                {"int"   , "\"4\": 100"         , ENUM  ,    },
                {"double", "\"5\":100.132"      , ENUM  ,    },
                {"str"   , "\"6\":\"hello\""    , ESTR  ,    },
                {"obj"   , "\"7\": {}"          , EOBJ  ,    },
                {"arr"   , "\"8\": []"          , EARR  ,    },
    };
    map = map2;

    i = -1;
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    eexpect_num(ejson_clear(e), ++i);
    eexpect_num(ejson_isEmpty(e), 1);

    // key in : param: yes | src: no
    _IN_ map3[] = {
                {"false" , "\"1\":false"        , EFALSE,    },
                {"true"  , "\"2\":true"         , ETRUE ,    },
                {"null"  , "\"3\":null"         , ENULL ,    },
                {"int"   , "\"4\": 100"         , ENUM  ,    },
                {"double", "\"5\":100.132"      , ENUM  ,    },
                {"str"   , "\"6\":\"hello\""    , ESTR  ,    },
                {"obj"   , "\"7\": {}"          , EOBJ  ,    },
                {"arr"   , "\"8\": []"          , EARR  ,    },
    };
    map = map3;

    i = -1;
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    i++; ejson_kaddJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejsonr_type(e, map[i].key), map[i].t );
    eexpect_num(ejson_clear(e), ++i);
    eexpect_num(ejson_isEmpty(e), 1);

    eexpect_num(ejson_free(r), 4);

    return ETEST_OK;
}

int t1_basic(int argc, char* argv[])
{
    E_UNUSED(argc); E_UNUSED(argv);

    ETEST_RUN( ejson_new_test () );

    ETEST_RUN( ejson_add_test1() ); // obj
    ETEST_RUN( ejson_add_test2() ); // arr
    ETEST_RUN( ejson_radd_test() );
    ETEST_RUN( ejson_kadd_test() );

    ETEST_RUN( ejson_addJ_test () );
    ETEST_RUN( ejson_raddJ_test() );
    ETEST_RUN( ejson_kaddJ_test() );

    return ETEST_OK;
}

