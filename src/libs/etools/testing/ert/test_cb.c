#include "test_main.h"
#include "ecompat.h"

static void _cb(void* p)
{
    while(1)
    {
        usleep(10000);
    }
}


int cb_test()
{
    ert rt = ert_new(100);

    for(int i = 0; i < 20; i++)
        ert_run(rt, 0, _cb, 0, 0);

    sleep(10);

    return 0;
}

int test_cb(int argc, char* argv[])
{
    cb_test();

    return ETEST_OK;
}
