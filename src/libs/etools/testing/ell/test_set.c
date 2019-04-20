#include "etest.h"

#include "ell.h"


int set_all_type_test(ell l, uint idx)
{
    char _buf_zore[128] = {0,};

    eexpect_num(ell_len(l) > idx , 1);

    ell_setI(l, idx, 10);
    eexpect_num(ell_valLen(l, idx), 0);
    eexpect_num(ell_valI(l, idx), 10);
    eexpect_num(ell_valType(l, idx), ENUM);

    ell_setF(l, idx, 23.4);
    eexpect_num(ell_valLen(l, idx), 0);
    eexpect_num(ell_valF(l, idx), 23.4);
    eexpect_num(ell_valType(l, idx), ENUM);

    ell_setP(l, idx, (cptr)30);
    eexpect_num(ell_valLen(l, idx), 0);
    eexpect_ptr(ell_valP(l, idx), (cptr)30);
    eexpect_num(ell_valType(l, idx), EPTR);

    ell_setS(l, idx, "40");
    eexpect_num(ell_valLen(l, idx), 2);
    eexpect_str(ell_valS(l, idx), "40");
    eexpect_num(ell_valType(l, idx), ESTR);

    ell_setR(l, idx, 50);
    eexpect_num(ell_valLen(l, idx), 50);
    eexpect_raw(ell_valR(l, idx), _buf_zore, ell_valLen(l, idx));
    eexpect_num(ell_valType(l, idx), ERAW);

    return ETEST_OK;
}

int test_convert_i()
{
    ell l = ell_new();

    ell_appdI(l, 1);
    ell_appdF(l, 2.0);
    ell_appdP(l, (cptr)3);
    ell_appdS(l, "4");
    ell_appdR(l, 5);

    ETEST_RUN(set_all_type_test(l, 0));
    ETEST_RUN(set_all_type_test(l, 1));
    ETEST_RUN(set_all_type_test(l, 2));
    ETEST_RUN(set_all_type_test(l, 3));
    ETEST_RUN(set_all_type_test(l, 4));

    ell_free(l);

    return ETEST_OK;
}

int test_set(int argc, char* argv[])
{
    ETEST_RUN( test_convert_i() );

    return ETEST_OK;
}
