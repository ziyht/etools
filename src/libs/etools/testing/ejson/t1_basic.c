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

static int ejson_add_test()
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
    eexpect_num(ejson_type(o), EFALSE); eexpect_num(ejson_valrType(r, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(r, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ETRUE) ; eexpect_num(ejson_valrType(r, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(r, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENULL) ; eexpect_num(ejson_valrType(r, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(r, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejson_valrType(r, &key[i]), ejson_type(o));
    eexpect_num(eobj_valI(o), 1)      ; eexpect_num(ejson_valrI(   r, &key[i]), eobj_valI( o));
    eexpect_num(eobj_valF(o), 1.0)    ; eexpect_num(ejson_valrF(   r, &key[i]), eobj_valF( o));

    ++i;
    o = ejson_valr(r, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejson_valrType(r, &key[i]), ejson_type(o));
    eexpect_num(eobj_valI(o), 2)      ; eexpect_num(ejson_valrI(   r, &key[i]), eobj_valI( o));
    eexpect_num(eobj_valF(o), 2.0)    ; eexpect_num(ejson_valrF(   r, &key[i]), eobj_valF( o));

    ++i;
    o = ejson_valr(r, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ESTR)  ; eexpect_num(ejson_valrType(r, &key[i]), ejson_type(o));
    eexpect_str(eobj_valS(o), &key[i]); eexpect_str(ejson_valrS(   r, &key[i]), eobj_valS( o));

    ++i;
    o = ejson_valr(r, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EPTR)  ; eexpect_num(ejson_valrType(r, &key[i]), ejson_type(o));
    eexpect_ptr(eobj_valP(o), &key[i]); eexpect_ptr(ejson_valrP(   r, &key[i]), eobj_valP( o));

    ++i;
    o = ejson_valr(r, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ERAW)  ; eexpect_num(ejson_valrType(r, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 9   )  ; eexpect_num(ejson_valrLen( r, &key[i]), eobj_len(  o));
    eexpect_raw(eobj_valR(o) , zero, 9); eexpect_raw(ejson_valrR(  r, &key[i]), eobj_valR( o), 9);

    ++i;
    o = ejson_valr(r, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EOBJ)  ; eexpect_num(ejson_valrType(r, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejson_valrLen( r, &key[i]), eobj_len(  o));

    ++i;
    o = ejson_valr(r, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EARR)  ; eexpect_num(ejson_valrType(r, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejson_valrLen( r, &key[i]), eobj_len(  o));

    eexpect_num(ejson_free(r), ++i + 1);

    return ETEST_OK;
}

static int ejson_addr_test()
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
    i++; ejson_addrT(r, RKEY, &key[i], EFALSE);
    i++; ejson_addrT(r, RKEY, &key[i], ETRUE);
    i++; ejson_addrT(r, RKEY, &key[i], ENULL);
    i++; ejson_addrI(r, RKEY, &key[i], 1);
    i++; ejson_addrF(r, RKEY, &key[i], 2.0);
    i++; ejson_addrS(r, RKEY, &key[i], &key[i]);
    i++; ejson_addrP(r, RKEY, &key[i], &key[i]);
    i++; ejson_addrR(r, RKEY, &key[i], 9);
    i++; ejson_addrT(r, RKEY, &key[i], EOBJ);
    i++; ejson_addrT(r, RKEY, &key[i], EARR);

    eexpect_num(ejson_isEmpty(e), 0);
    eexpect_num(ejson_len(e), ++i);

    i = -1;

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EFALSE); eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ETRUE) ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENULL) ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_valI(o), 1)      ; eexpect_num(ejson_valrI(   e, &key[i]), eobj_valI( o));
    eexpect_num(eobj_valF(o), 1.0)    ; eexpect_num(ejson_valrF(   e, &key[i]), eobj_valF( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_valI(o), 2)      ; eexpect_num(ejson_valrI(   e, &key[i]), eobj_valI( o));
    eexpect_num(eobj_valF(o), 2.0)    ; eexpect_num(ejson_valrF(   e, &key[i]), eobj_valF( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ESTR)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_str(eobj_valS(o), &key[i]); eexpect_str(ejson_valrS(   e, &key[i]), eobj_valS( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EPTR)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_ptr(eobj_valP(o), &key[i]); eexpect_ptr(ejson_valrP(   e, &key[i]), eobj_valP( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ERAW)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 9   )  ; eexpect_num(ejson_valrLen( e, &key[i]), eobj_len(  o));
    eexpect_raw(eobj_valR(o) , zero, 9); eexpect_raw(ejson_valrR(  e, &key[i]), eobj_valR( o), 9);

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EOBJ)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejson_valrLen( e, &key[i]), eobj_len(  o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EARR)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejson_valrLen( e, &key[i]), eobj_len(  o));

    eexpect_num(ejson_free(r), ++i + 4);

    return ETEST_OK;
}

static int ejson_addk_test()
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
    i++; ejson_addkT(r, KEYS, &key[i], EFALSE);
    i++; ejson_addkT(r, KEYS, &key[i], ETRUE);
    i++; ejson_addkT(r, KEYS, &key[i], ENULL);
    i++; ejson_addkI(r, KEYS, &key[i], 1);
    i++; ejson_addkF(r, KEYS, &key[i], 2.0);
    i++; ejson_addkS(r, KEYS, &key[i], &key[i]);
    i++; ejson_addkP(r, KEYS, &key[i], &key[i]);
    i++; ejson_addkR(r, KEYS, &key[i], 9);
    i++; ejson_addkT(r, KEYS, &key[i], EOBJ);
    i++; ejson_addkT(r, KEYS, &key[i], EARR);

    eexpect_num(ejson_isEmpty(e), 0);
    eexpect_num(ejson_len(e), ++i);

    i = -1;

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EFALSE); eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ETRUE) ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENULL) ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));

    ++i;
    o = ejson_valr(e, &key[i]);
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_valI(o), 1)      ; eexpect_num(ejson_valrI(   e, &key[i]), eobj_valI( o));
    eexpect_num(eobj_valF(o), 1.0)    ; eexpect_num(ejson_valrF(   e, &key[i]), eobj_valF( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ENUM)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_valI(o), 2)      ; eexpect_num(ejson_valrI(   e, &key[i]), eobj_valI( o));
    eexpect_num(eobj_valF(o), 2.0)    ; eexpect_num(ejson_valrF(   e, &key[i]), eobj_valF( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ESTR)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_str(eobj_valS(o), &key[i]); eexpect_str(ejson_valrS(   e, &key[i]), eobj_valS( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EPTR)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_ptr(eobj_valP(o), &key[i]); eexpect_ptr(ejson_valrP(   e, &key[i]), eobj_valP( o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), ERAW)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 9   )  ; eexpect_num(ejson_valrLen( e, &key[i]), eobj_len(  o));
    eexpect_raw(eobj_valR(o) , zero, 9); eexpect_raw(ejson_valrR(  e, &key[i]), eobj_valR( o), 9);

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EOBJ)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejson_valrLen( e, &key[i]), eobj_len(  o));

    ++i;
    o = ejson_valr(e, &key[i])        ;
    eunexpc_ptr(o, 0);
    eexpect_num(ejson_type(o), EARR)  ; eexpect_num(ejson_valrType(e, &key[i]), ejson_type(o));
    eexpect_num(eobj_len(o)  , 0   )  ; eexpect_num(ejson_valrLen( e, &key[i]), eobj_len(  o));

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
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, 0, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
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
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
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
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    i++; ejson_addJ(r, map[i].key, map[i].json); eexpect_num(ejson_len(r), i + 1); eexpect_num(ejson_valrType(r, map[i].key), map[i].t );
    eexpect_num(ejson_free(r), ++i + 1);

    return ETEST_OK;
}


static int ejson_addrJ_test()
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
    i++; ejson_addrJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
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
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
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
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addrJ(r, RKEY, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    eexpect_num(ejson_clear(e), ++i);
    eexpect_num(ejson_isEmpty(e), 1);

    eexpect_num(ejson_free(r), 4);

    return ETEST_OK;
}

static int ejson_addkJ_test()
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
    i++; ejson_addkJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, 0, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
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
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
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
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    i++; ejson_addkJ(r, KEYS, map[i].key, map[i].json); eexpect_num(ejson_len(e), i + 1); eexpect_num(ejson_valrType(e, map[i].key), map[i].t );
    eexpect_num(ejson_clear(e), ++i);
    eexpect_num(ejson_isEmpty(e), 1);

    eexpect_num(ejson_free(r), 4);

    return ETEST_OK;
}

int t1_basic(int argc, char* argv[])
{
    E_UNUSED(argc); E_UNUSED(argv);

    ETEST_RUN( ejson_new_test () );

    ETEST_RUN( ejson_add_test () );
    ETEST_RUN( ejson_addr_test() );
    ETEST_RUN( ejson_addk_test() );

    ETEST_RUN( ejson_addJ_test () );
    ETEST_RUN( ejson_addrJ_test() );
    ETEST_RUN( ejson_addkJ_test() );

    return ETEST_OK;
}

