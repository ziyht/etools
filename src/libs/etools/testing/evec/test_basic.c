#include "etest.h"

#include "evec.h"
#include "eutils.h"

static int test_new()
{
    typedef struct { etypev t; u16 size; } _IN_;

    evec v; int i;

    _IN_ map[] =
    {
        {E_I8 , 1},
        {E_I16, 2},
        {E_I32, 4},
        {E_I64, 8},

        {E_U8 , 1},
        {E_U16, 2},
        {E_U32, 4},
        {E_U64, 8},

        {E_F32, 4},
        {E_F64, 8},

        {E_PTR, 8},
        {E_STR, 8},
        {E_RAW, 8},

        {E_USER, 16},

        {E_NAV, 0},
    };

    for(i = 0; i < 100; i++)
    {
        if(map[i].t == E_NAV)
            break;

        v = evec_new(map[i].t, map[i].size);
        eexpect_num(evec_type (v), map[i].t);
        eexpect_num(evec_len  (v), 0);
        eexpect_num(evec_esize(v), map[i].size);
        evec_free(v);
    }

    return ETEST_OK;
}

static int test_appd()
{
    typedef struct { etypev t; u16 size; } _IN_;
    char __buf[256] = {0};

    evec v; int i;

    _IN_ map[] =
    {
        {E_I8 , 1},
        {E_I16, 2},
        {E_I32, 4},
        {E_I64, 8},

        {E_U8 , 1},
        {E_U16, 2},
        {E_U32, 4},
        {E_U64, 8},

        {E_F32, 4},
        {E_F64, 8},

        {E_PTR, 8},

        {E_NAV, 0},
    };

    _IN_* itr = map;

    while (itr->t != E_NAV) {

        v = evec_new(itr->t, 0);

        for(i = 0; i < 128; i++)
        {
            evar ev = evar_gen(itr->t, 1, 0);
            ev.v.i8 = i;

            eexpect_num( evec_appdV(v, ev), true);
        }
        eexpect_num( evec_free(v), i );

        itr++;
    }

    //! i64
    {
        v = evec_new(E_I64, 0);

        for(i = 0; i < 128; i++)
        {
            eexpect_num( evec_appdI(v, i), true);
        }
        eexpect_num( evec_free(v), i );
    }

    //! f64
    {
        v = evec_new(E_F64, 0);

        for(i = 0; i < 128; i++)
        {
            eexpect_num( evec_appdF(v, i), true);
        }
        eexpect_num( evec_free(v), i );
    }

    //! str
    {
        v = evec_new(E_STR, 0);

        for(i = 0; i < 128; i++)
        {
            ll2str(i, __buf);
            eexpect_num( evec_appdS(v, __buf), true);
        }
        eexpect_num( evec_free(v), i );
    }

    //! ptr
    {
        v = evec_new(E_PTR, 0);

        for(i = 0; i < 128; i++)
        {
            eexpect_num( evec_appdP(v, &__buf[i]), true);
        }
        eexpect_num( evec_free(v), i );
    }

    //! raw
    {
        v = evec_new(E_RAW, 0);

        for(i = 0; i < 128; i++)
        {
            //ll2str(i, __buf);
            eexpect_num( evec_appdR(v, i), true);
        }
        eexpect_num( evec_free(v), i );
    }

    //! user
    {
        v = evec_new(E_USER, 4);

        for(i = 0; i < 128; i++)
        {
            eexpect_num( evec_appdV(v, EVAR_RAW(__buf, ll2str(i, __buf))), true);
        }
        eexpect_num( evec_free(v), i );
    }

    return ETEST_OK;
}

static int evec_vali_test()
{
    int i;

    evec v = evec_new(E_I64, 0);

    for(i = 0; i < 100; i++)
    {
        evec_appdI(v, i);
    }
    eexpect_num(evec_len(v), 100);

    for(; i < 200; i++)
    {
        evec_appdV(v, EVAR_I64(i) );
    }
    eexpect_num(evec_len(v), 200);

    for(i = 0; i < 200; i++)
    {
        eexpect_num(evec_valI(v, i), i);
    }

    evec_free(v);

    return ETEST_OK;
}

int test_basic(int argc, char* argv[])
{
    (void)argc; (void)argv;

    ETEST_RUN( test_new() );
    ETEST_RUN( test_appd() );

    ETEST_RUN( evec_vali_test() );

    return ETEST_OK;
}
