/**
* this file is create by subsystem 'ETest' from EMake framework
*/

#include <etest.h>

#include "eutils.h"
#include "ejson.h"

static int t7_cmp_case1()
{
    ejson ei, ef, es;

    ejson r = ejson_parseS("{"
                           "    \"i\": 1,"
                           "    \"f\": 1.0,"
                           "    \"s\": \"str\""
                           "}");

    eunexpc_ptr(r, 0);

    ei = ejson_valr(r, "i"); eunexpc_ptr(ei, 0);
    ef = ejson_valr(r, "f"); eunexpc_ptr(ef, 0);
    es = ejson_valr(r, "s"); eunexpc_ptr(es, 0);

    eexpect_num(ejson_cmpI(ei, 0), 1);
    eexpect_num(ejson_cmpI(ei, 1), 0);
    eexpect_num(ejson_cmpI(ei, 2), -1);

    eexpect_num(ejsonr_cmpI(r, "i", 0), 1);
    eexpect_num(ejsonr_cmpI(r, "i", 1), 0);
    eexpect_num(ejsonr_cmpI(r, "i", 2), -1);

    eexpect_num(ejsonk_cmpI(r, "i", 0), 1);
    eexpect_num(ejsonk_cmpI(r, "i", 1), 0);
    eexpect_num(ejsonk_cmpI(r, "i", 2), -1);

    eexpect_num(ejson_cmpF(ef, 0), 1);
    eexpect_num(ejson_cmpF(ef, 1), 0);
    eexpect_num(ejson_cmpF(ef, 2), -1);

    eexpect_num(ejsonr_cmpF(r, "f", 0), 1);
    eexpect_num(ejsonr_cmpF(r, "f", 1), 0);
    eexpect_num(ejsonr_cmpF(r, "f", 2), -1);

    eexpect_num(ejsonk_cmpF(r, "f", 0), 1);
    eexpect_num(ejsonk_cmpF(r, "f", 1), 0);
    eexpect_num(ejsonk_cmpF(r, "f", 2), -1);

    eexpect_num(ejson_cmpS(es, "rtr"), 1);
    eexpect_num(ejson_cmpS(es, "str"), 0);
    eexpect_num(ejson_cmpS(es, "ttr"), -1);
    eexpect_num(ejson_cmpS(es,  0   ), -4);

    eexpect_num(ejsonr_cmpS(r, "s", "rtr"), 1);
    eexpect_num(ejsonr_cmpS(r, "s", "str"), 0);
    eexpect_num(ejsonr_cmpS(r, "s", "ttr"), -1);
    eexpect_num(ejsonr_cmpS(r, "s", 0   ), -4);

    eexpect_num(ejson_cmpI(0 , 2), -2);
    eexpect_num(ejson_cmpI(r , 2), -3);

    eexpect_num(ejson_cmpF(0 , 2), -2);
    eexpect_num(ejson_cmpF(r , 2), -3);

    eexpect_num(ejson_cmpS(0 , ""), -2);
    eexpect_num(ejson_cmpS(r , ""), -3);

    ejson_free(r);

    return ETEST_OK;
}


int t7_cmp(int argc, char* argv[])
{
    E_UNUSED(argc); E_UNUSED(argv);

    ETEST_RUN( t7_cmp_case1() );

    return ETEST_OK;
}

