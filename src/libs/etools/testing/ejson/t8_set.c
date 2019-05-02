/**
* this file is create by subsystem 'ETest' from EMake framework
*/

#include <etest.h>

#include "eutils.h"

#include "ejson.h"

char key[32] = "01234567890";
char vals[32] = {0};

static constr itostr(int i)
{
    static char __[32];

    sprintf(__, "%d", i);

    return __;
}

#define CHILD_CNT 10

static ejson gen_test_ejson(etypeo type, int deep)
{
    ejson r; int i;

    if(deep <= 0)
    {
        return ejson_new(type, 0);
    }
    else if(deep == 1)
    {
        r = ejson_new(type, 0);

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

        return r;
    }
    else if(deep > 1)
    {
        deep--;

        r = ejson_new(type, 0);

        i = -1;
        ++i; ejson_addT(r, &key[i], EFALSE);
        ++i; ejson_addT(r, &key[i], ETRUE );
        ++i; ejson_addT(r, &key[i], ENULL );
        ++i; ejson_addI(r, &key[i], 1);
        ++i; ejson_addF(r, &key[i], 2.0);
        ++i; ejson_addS(r, &key[i], &key[i]);
        ++i; ejson_addP(r, &key[i], &key[i]);
        ++i; ejson_addR(r, &key[i], 9);
        ++i; ejson_addO(r, &key[i], gen_test_ejson(EOBJ, deep));
        ++i; ejson_addO(r, &key[i], gen_test_ejson(EARR, deep));

        return r;
    }

    return 0;
}

static int test_resetTr(ejson r, etypeo type)
{
    uint i;

    switch (ejson_type(r))
    {
        case EOBJ:  for(i = 0; i < CHILD_CNT; i++)
                    {
                        ejsonk_setT(r, &key[i], type);

                        eexpect_num(ejsonk_type(r, &key[i]),  type);
                    }
                    break;

        case EARR:  for(i = 0; i < CHILD_CNT; i++)
                    {
                        ejsonk_setT(r, itostr(i), type);

                        if(!ejson_len(r))
                            continue;

                        eexpect_num(ejsonk_type(r, itostr(i)),  type);
                    }
                    break;

        default:    return ETEST_ERR;
    }

    return ETEST_OK;
}

static int test_resetIr(ejson r)
{
    uint i;

    switch (ejson_type(r))
    {
        case EOBJ:  for(i = 0; i < CHILD_CNT; i++)
                    {
                        ejsonk_setI(r, &key[i], 123);

                        eexpect_num(ejsonk_type(r, &key[i]),  ENUM);
                        eexpect_num(ejsonk_valI   (r, &key[i]),  123);
                    }
                    break;

        case EARR:  for(i = 0; i < CHILD_CNT; i++)
                    {
                        ejsonk_setI(r, itostr(i), 123);

                        if(!ejson_len(r))
                            continue;

                        eexpect_num(ejsonk_type(r, itostr(i)),  ENUM);
                        eexpect_num(ejsonk_valI   (r, itostr(i)),  123);
                    }
                    break;

        default :   return ETEST_ERR;
    }

    return ETEST_OK;
}

static int test_resetFr(ejson r)
{
    uint i;

    switch (ejson_type(r))
    {
        case EOBJ:  for(i = 0; i < CHILD_CNT; i++)
                    {
                        ejsonk_setF(r, &key[i], 123.12);

                        eexpect_num(ejsonk_type(r, &key[i]),  ENUM);
                        eexpect_num(ejsonk_valF   (r, &key[i]),  123.12);
                    }
                    break;

        case EARR:  for(i = 0; i < CHILD_CNT; i++)
                    {
                        ejsonk_setF(r, itostr(i), 123.12);

                        if(!ejson_len(r))
                            continue;

                        eexpect_num(ejsonk_type(r, itostr(i)),  ENUM);
                        eexpect_num(ejsonk_valF   (r, itostr(i)),  123.12);
                    }
                    break;

        default :   return ETEST_ERR;
    }

    return ETEST_OK;
}

static int test_resetSr(ejson r)
{

#define STR      "123456"
#define STR_LEN  strlen(STR)

    uint i;

    switch (ejson_type(r))
    {
        case EOBJ:  for(i = 0; i < CHILD_CNT; i++)
                    {
                        ejsonk_setS(r, &key[i], STR);

                        eexpect_num(ejsonk_type(r, &key[i]),  ESTR);
                        eexpect_num(ejsonk_len (r, &key[i]),  STR_LEN);
                        eexpect_str(ejsonk_valS   (r, &key[i]),  STR );
                    }
                    break;

        case EARR:  for(i = 0; i < CHILD_CNT; i++)
                    {
                        ejsonk_setS(r, itostr(i), STR);

                        if(!ejson_len(r))
                            continue;

                        eexpect_num(ejsonk_type(r, itostr(i)),  ESTR);
                        eexpect_num(ejsonk_len (r, itostr(i)),  STR_LEN);
                        eexpect_str(ejsonk_valS   (r, itostr(i)),  STR );
                    }
                    break;

        default :   return ETEST_ERR;
    }

    return ETEST_OK;
}

static int test_resetPr(ejson r)
{
    uint i;

    switch (ejson_type(r))
    {
        case EOBJ:  for(i = 0; i < CHILD_CNT; i++)
                    {
                        ejsonk_setP(r, &key[i], &key[i]);

                        eexpect_num(ejsonk_type(r, &key[i]),  EPTR);
                        eexpect_ptr(ejsonk_valP   (r, &key[i]),  &key[i]);
                    }
                    break;

        case EARR:  for(i = 0; i < CHILD_CNT; i++)
                    {
                        ejsonk_setP(r, itostr(i), &key[i]);

                        if(!ejson_len(r))
                            continue;

                        eexpect_num(ejsonk_type(r, itostr(i)),  EPTR);
                        eexpect_ptr(ejsonk_valP   (r, itostr(i)),  &key[i]);
                    }
                    break;

        default :   return ETEST_ERR;
    }

    return ETEST_OK;
}

static int test_resetRr(ejson r)
{
    uint i;

    switch (ejson_type(r))
    {
        case EOBJ:  for(i = 0; i < CHILD_CNT; i++)
                    {
                        ejsonk_setR(r, &key[i], i);

                        eexpect_num(ejsonk_type(r, &key[i]),  ERAW);
                        eexpect_num(ejsonk_len (r, &key[i]),  i);
                        eexpect_raw(ejsonk_valR   (r, &key[i]),  vals, i);
                    }
                    break;

        case EARR:  for(i = 0; i < CHILD_CNT; i++)
                    {
                        ejsonk_setR(r, itostr(i), i);

                        if(!ejson_len(r))
                            continue;

                        eexpect_num(ejsonk_type(r, itostr(i)),  ERAW);
                        eexpect_num(ejsonk_len (r, itostr(i)),  i);
                        eexpect_raw(ejsonk_valR   (r, itostr(i)),  vals, i);
                    }
                    break;

        default :   return ETEST_ERR;
    }

    return ETEST_OK;
}

static int test_create_in_obj_r()
{
    ejson r; int deep; etypeo t = EOBJ;

    deep = 0;
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, EFALSE) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, ETRUE ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, ENULL ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, EOBJ  ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, EARR  ) ); ejson_free(r);

    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetIr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetFr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetSr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetPr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetRr(r        ) ); ejson_free(r);

    return ETEST_OK;
}

static int test_create_in_arr_r()
{
    ejson r; int deep; etypeo t = EARR;

    deep = 0;
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, EFALSE) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, ETRUE ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, ENULL ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, EOBJ  ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, EARR  ) ); ejson_free(r);

    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetIr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetFr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetSr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetPr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetRr(r        ) ); ejson_free(r);

    return ETEST_OK;
}

static int test_reset_in_obj_r()
{
    ejson r; int deep; etypeo t = EOBJ;

    deep = 1;
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, EFALSE) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, ETRUE ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, ENULL ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, EOBJ  ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, EARR  ) ); ejson_free(r);

    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetIr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetFr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetSr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetPr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetRr(r        ) ); ejson_free(r);

    return ETEST_OK;
}

static int test_reset_in_arr_r()
{
    ejson r; int deep; etypeo t = EARR;

    deep = 1;
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, EFALSE) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, ETRUE ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, ENULL ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, EOBJ  ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetTr(r, EARR  ) ); ejson_free(r);

    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetIr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetFr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetSr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetPr(r        ) ); ejson_free(r);
    r = gen_test_ejson(t, deep); ETEST_RUN( test_resetRr(r        ) ); ejson_free(r);

    return ETEST_OK;
}

static int test_create_in_obj_k()
{
    typedef struct {constr keys; etypeo t; } _IN_;

    ejson r; int i;

    _IN_ map[] = {
        {"obj.false",   EFALSE},
        {"obj.true" ,   ETRUE },
        {"obj.null" ,   ENULL },
        {"obj.i64"  ,   ENUM  },
        {"obj.f64"  ,   ENUM  },
        {"obj.str"  ,   ESTR  },
        {"obj.ptr"  ,   EPTR  },
        {"obj.raw"  ,   ERAW  },
        {"obj.obj"  ,   EOBJ  },
        {"obj.arr"  ,   EARR  },

    };

    r = ejson_new(EOBJ, 0);

    ejson_addO(r, "obj", gen_test_ejson(EOBJ, 0));

    /**
      * {
      *     "obj": {}
      * }
      *
      */

    i = -1;

    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);

    ejson_free(r);

    return ETEST_OK;
}

static int test_create_in_arr_k()
{
    typedef struct {constr keys; etypeo t; } _IN_;

    ejson r; int i;

    _IN_ map[] = {
        {"obj.false",   EOBJ_UNKNOWN},
        {"obj.true" ,   EOBJ_UNKNOWN },
        {"obj.null" ,   EOBJ_UNKNOWN },
        {"obj.i64"  ,   EOBJ_UNKNOWN  },
        {"obj.f64"  ,   EOBJ_UNKNOWN  },
        {"obj.str"  ,   EOBJ_UNKNOWN  },
        {"obj.ptr"  ,   EOBJ_UNKNOWN  },
        {"obj.raw"  ,   EOBJ_UNKNOWN  },
        {"obj.obj"  ,   EOBJ_UNKNOWN  },
        {"obj.arr"  ,   EOBJ_UNKNOWN  },

    };

    r = ejson_new(EOBJ, 0);

    ejson_addO(r, "obj", gen_test_ejson(EARR, 0));

    /**
      * {
      *     "obj": []
      * }
      *
      */

    i = -1;

    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);
    i++;  ejson_setTp(r, map[i].keys, map[i].t); eexpect_num( ejsonp_type(r, map[i].keys),  map[i].t);

    ejson_free(r);

    return ETEST_OK;
}

static int test_reset_particular_k()
{
    ejson r;

    r = ejson_new(EOBJ, 0);

    // 1. create automatically when path not exsit, we only create EOBJ obj
    ejson_setTp(r, "obj.obj", EOBJ);
    ejson_setTp(r, "obj.arr", EARR);
    eexpect_num(ejsonp_type(r, "obj"    ), EOBJ);
    eexpect_num(ejsonp_type(r, "obj.obj"), EOBJ);
    eexpect_num(ejsonp_type(r, "obj.arr"), EARR);

    // 2. if EOBJ obj not contains the specific obj, create automatically, but not for EARR obj
    ejson_setTp(r, "obj.obj.false", EFALSE);        // create
    ejson_setTp(r, "obj.arr[0]"   , EFALSE);        // failed, not create EFALSE in 'obj.arr'
    eexpect_num(ejsonp_type(r, "obj.obj.false"), EFALSE);
    eexpect_num(ejsonp_type(r, "obj.arr[0]"   ), EOBJ_UNKNOWN);

    // 3. can not create or reset when expect obj can not be found because of a val obj
    ejson_setTp(r, "obj.obj.false.true", ETRUE);    // failed, 'obj.obj.false' is invalid to create path
    //ejson_show(r);
    eexpect_ptr(ejsonp(r, "obj.obj.false.true"), 0);

    // 4. you can reset any obj which can be found no matter what it is, so be careful
    ejson_setTp(r, "obj.obj", EARR);    // ok, now 'obj.obj' is a EARR obj, objs in prev one will be delete automatically
    //ejson_show(r);
    eexpect_num(ejsonp_type(r, "obj.obj"), EARR);

    // 5. the created obj in path is EOBJ type
    ejson_setTp(r, "[1][2][3][4]", ETRUE);
    //ejson_show(r);
    eexpect_num(ejsonp_type(r, "1"), EOBJ);
    eexpect_num(ejsonp_type(r, "1.2"), EOBJ);
    eexpect_num(ejsonp_type(r, "1.2.3"), EOBJ);
    eexpect_num(ejsonp_type(r, "1.2.3.4"), ETRUE);

    ejson_free(r);

    return ETEST_OK;
}

int t8_set(int argc, char* argv[])
{
    E_UNUSED(argc); E_UNUSED(argv);

    ETEST_RUN( test_create_in_obj_r() );
    ETEST_RUN( test_create_in_arr_r() );

    ETEST_RUN( test_reset_in_obj_r() );
    ETEST_RUN( test_reset_in_arr_r() );

    ETEST_RUN( test_create_in_obj_k() );
    ETEST_RUN( test_create_in_arr_k() );

    ETEST_RUN( test_reset_particular_k() );

    return ETEST_OK;
}

