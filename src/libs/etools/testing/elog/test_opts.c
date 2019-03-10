#include "test_main.h"


void elog_opts_test()
{
    elog e;

    e = elog_new("opts", "./opts.log");

    elog_inf(e, "0");
    elog_inf(ELOG_O(e, ELOG_MUTE), "this is a mute log, should not print out in console");
    elog_inf(e, "1, prev should be 0 in console");

    elog_free(e);
}

int test_opts(int argc, char* argv[])
{
    elog_opts_test();

    return ETEST_OK;
}
