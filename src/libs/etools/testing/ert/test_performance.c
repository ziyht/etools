#include "test_main.h"
#include <stdio.h>

#include "eutils.h"

static ert rt;
static u64 count;

static void runner(void* d)
{
    count++;

    static int i;

    if(count % 10000 == 0)
    {
        i++;
        printf("%"PRIi64"\n", count); fflush(stdout);
    }

    if(i > 100)
        ert_destroy(rt, 0);
}

void ert_performance_test()
{
    rt = ert_new(1); int ret;

    i64 t = eutils_nowms();

    while(1)
    {
        if(!(ret = ert_run(rt, 0, runner, 0, 0)))
        {
            printf("err(%d) at: %"PRIi64" cost %"PRIi64"ms\n", ret, count, eutils_nowms() - t);
            break;
        }
    }
}

int test_performance(int argc, char* argv[])
{
    ert_performance_test();

    return ETEST_OK;
}
