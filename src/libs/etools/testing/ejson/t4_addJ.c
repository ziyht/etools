/**
* this file is create by subsystem 'ETest' from EMake framework
*/

#include <etest.h>

static int t4_addJ_case1()
{
    eexpect_num(1, 1);      // passed

    return ETEST_OK;
}

static int t4_addJ_case2()
{
    eexpect_num(1, 0);      // will failed

    return ETEST_OK;
}

int t4_addJ(int argc, char* argv[])
{
    ETEST_RUN( t4_addJ_case1() );
    ETEST_RUN( t4_addJ_case2() );

    return ETEST_OK;
}

