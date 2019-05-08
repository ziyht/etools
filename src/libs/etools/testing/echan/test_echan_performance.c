#include <stdio.h>

#include "test_main.h"
#include "ethread.h"

static i64 count = 0;

static void* sender(void* chan)
{
    for(int i = 0; i < count; i++)
        echan_sendI(chan, i);

    return NULL;
}

void echan_unbuffered_perf_test(i64 _count)
{
    echan chan = echan_new(ECHAN_LIST, 0);
    ethread_t th; i64 r; i64 t;
    ethread_init(th, sender, chan);

    count = _count;

    t = utils_timer_now_ms();
    for(int i = 0; i < count; i++)
    {
        r = echan_recvI(chan);
        assert_true(r == i, chan, "value not match");
    }
    printf("unbuffered %8"PRIi64": %"PRIi64"\n", count, utils_timer_now_ms() - t); fflush(stdout);

    ethread_join(th);
    echan_free(chan);
    pass();
}

void echan_buffered_perf_test(i64 _count)
{
    echan chan = echan_new(ECHAN_LIST, 100);
    ethread_t th; i64 r; i64 t;
    ethread_init(th, sender, chan);

    count = _count;

    t = utils_timer_now_ms();
    for(int i = 0; i < count; i++)
    {
        r = echan_recvI(chan);
        assert_true(r == i, chan, "value not match");
    }
    printf("  buffered %8"PRIi64": %"PRIi64"\n", count, utils_timer_now_ms() - t); fflush(stdout);

    ethread_join(th);
    echan_free(chan);
    pass();
}


void echan_performance_test()
{
    echan_unbuffered_perf_test(100);
    echan_unbuffered_perf_test(1000);
    echan_unbuffered_perf_test(10000);
    echan_unbuffered_perf_test(100000);
    //echan_unbuffered_perf_test(1000000);

    echan_buffered_perf_test(100);
    echan_buffered_perf_test(1000);
    echan_buffered_perf_test(10000);
    echan_buffered_perf_test(100000);
    echan_buffered_perf_test(1000000);
    echan_buffered_perf_test(2000000);
    echan_buffered_perf_test(10000000);
}

int test_echan_performance(int argc, char* argv[])
{
    echan_performance_test();

    return ETEST_OK;
}
