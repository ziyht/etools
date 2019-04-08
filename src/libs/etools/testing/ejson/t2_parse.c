/**
* this file is create by subsystem 'ETest' from EMake framework
*/

#include <etest.h>

static int t2_parse_case1()
{
    eexpect_num(1, 1);      // passed

    return ETEST_OK;
}

static int t2_parse_case2()
{
    eexpect_num(1, 0);      // will failed

    return ETEST_OK;
}

int t2_parse(int argc, char* argv[])
{
    ETEST_RUN( t2_parse_case1() );
    ETEST_RUN( t2_parse_case2() );

    return ETEST_OK;
}

