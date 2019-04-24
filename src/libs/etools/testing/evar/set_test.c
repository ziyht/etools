/**
* this file is create by subsystem 'ETest' from EMake framework
*/

#include <etest.h>

#include "eutils.h"

#include "evar.h"

static int set_test_case1()
{
    int i; evar v; char str[32];

    v = evar_gen(E_I64, 100, 0);
    for(i = 0; i < 100; i++)
    {
        evar_setI(&v, i, i);
    }
    for(i = 0; i < 100; i++)
    {
        eexpect_num(evar_valI(&v, i), i );
    }
    evar_free(&v);

    v = evar_gen(E_STR, 100, 0);
    for(i = 0; i < 100; i++)
    {
        ll2str(i, str);
        evar_setS(&v, i, str);
    }
    for(i = 0; i < 100; i++)
    {
        ll2str(i, str);
        eexpect_str(evar_valS(&v, i), str);
    }
    evar_free(&v);

    return ETEST_OK;
}

static int set_test_case2()
{
    return ETEST_OK;
}

int set_test(int argc, char* argv[])
{
    ETEST_RUN( set_test_case1() );
    ETEST_RUN( set_test_case2() );

    return ETEST_OK;
}

