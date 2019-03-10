#include "test_main.h"
#include "elog.h"
#define   ECOMPAT_ALL
#include "ecompat.h"

void on_timer1(etimer e)
{
    elog_inf(0, "11111");
}

void on_timer2(etimer e)
{
    elog_inf(0, "22222");

    etimer_destroy(e);
}

void etimer_timeline_test()
{
    etloop loop = etloop_new(1);

    etimer timer1 = etimer_new(loop);

    etimer_start(timer1, on_timer1, 0, 1000);

    while(1)
    {
        for(int i = 0; i < 10 ; i++)
        {
            etimer timer2 = etimer_new(loop);
            etimer_start (timer2, on_timer2, 0, 0);
        }

        sleep(1);
    }
}

int test_timeline(int argc, char* argv[])
{
    etimer_timeline_test();

    return ETEST_OK;
}
