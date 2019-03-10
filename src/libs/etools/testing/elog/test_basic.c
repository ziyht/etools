#include "test_main.h"

#define ELOG_LEVEL ELOG_WRN
#include "elog.h"

#define ELOG_LEVEL_RESET
#include "elog.h"

void elog_basic_test()
{
    elog l1 = elog_new("basic", "./basic.log");
    elog l2 = elog_new("temp" , "./basic.log");

    elog_inf(l1, "log create ok");
    elog_inf(l2, "log create ok");


    elog_dbg(l1, "this is a debug log");
    elog_inf(l1, "this is a info  log");
    elog_wrn(l1, "this is a warn  log");
    elog_err(l1, "this is a error log");

    elog_dbg(l1, 0);
    elog_inf(l1, 0);
    elog_wrn(l1, 0);
    elog_err(l1, 0);

    elog_free(l1);
    elog_free(l2);
}


int test_basic(int argc, char* argv[])
{
    elog_basic_test();

    return ETEST_OK;
}
