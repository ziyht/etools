#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

#include "test_main.h"

#if _WIN32
#include <Windows.h>
#include <sys/timeb.h>
static double hrtime_interval_ = -1;
static CRITICAL_SECTION process_title_lock;
#endif

int64_t utils_timer_now_ms() {
#if _WIN32
    #define PRECISE		1000
    static int64_t offset; LARGE_INTEGER counter;
    if(hrtime_interval_ == 0)
        return 0;
    else if(hrtime_interval_ == -1)
    {
        LARGE_INTEGER perf_frequency; struct timeb tm; LARGE_INTEGER counter;
        InitializeCriticalSection(&process_title_lock);
        if (QueryPerformanceFrequency(&perf_frequency)) {	hrtime_interval_ = 1.0 / perf_frequency.QuadPart;}
        else											{	hrtime_interval_ = 0; return 0;	     			 }

        ftime(&tm);
        QueryPerformanceCounter(&counter);
        offset = tm.time * PRECISE + tm.millitm * PRECISE / 1000 - (int64_t) ((double) counter.QuadPart * hrtime_interval_ * PRECISE);
        return tm.time * PRECISE + tm.millitm * PRECISE / 1000;
    }

    QueryPerformanceCounter(&counter);
    return (int64_t) ((double) counter.QuadPart * hrtime_interval_ * PRECISE) + offset;
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (int64_t)ts.tv_sec * 1000 + (int64_t)ts.tv_nsec / 1000000;
#endif
}

static void performance_obj_test(uint scale)
{
    erb root = erb_new(0, 0);  u64 t;

    t = utils_timer_now_ms();
    for(i64 i = 0; i < scale; i++)
    {
        erb_addR(root, (ekey){i}, 1);
    }

    if(erb_len(root) != scale)
    {
        printf("error: add %d elements, but only have %d elements in e\n", scale, erb_len(root)); fflush(stdout);
        exit(1);
    }

    printf("add   %d \telem: %6"PRIi64"ms\n", scale, utils_timer_now_ms() - t); fflush(stdout);


    t = utils_timer_now_ms();
    uint j = 0;
    for(i64 i = 0; i < scale; i++)
    {
        if(erb_find(root, (ekey){i})) j++;
    }

    if(erb_len(root) != j)
    {
        printf("error: e have %d elements, but only found %d elements in e\n", scale, j); fflush(stdout);
        exit(1);
    }

    printf("found %d \telem: %6"PRIu64"ms\n", j, utils_timer_now_ms() - t); fflush(stdout);

    t = utils_timer_now_ms();
    erb_free(root);
    printf("free  %d \telem: %6"PRIu64"ms\n\n", j, utils_timer_now_ms() - t); fflush(stdout);
}


void erb_performance_test()
{
    printf("-- performance_obj_test --\n"); fflush(stdout);
    performance_obj_test(10000);
    performance_obj_test(20000);

#if 1
    performance_obj_test(50000);
    performance_obj_test(100000);
    performance_obj_test(200000);
    performance_obj_test(500000);
    performance_obj_test(1000000);
    performance_obj_test(2000000);
#endif
}

int test_performance(int argc, char* argv[])
{
    erb_performance_test();

    return ETEST_OK;
}
