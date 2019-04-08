/**
* this file is create by subsystem 'ETest' from EMake framework
*/

#include <etest.h>



static int new_test_case1()
{
    eexpect_num(1, 1);      // passed

    return ETEST_OK;
}

static int new_test_case2()
{
    eexpect_num(1, 0);      // will failed

    return ETEST_OK;
}

int new_test(int argc, char* argv[])
{
    ETEST_RUN( new_test_case1() );
    ETEST_RUN( new_test_case2() );

    return ETEST_OK;
}

