//
// Created by ziyht on 17-2-15.
//

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

#include "ejson.h"
#include "test.h"

int64_t utils_timer_now_ms() {
#if _WIN32
    #define PRECISE		1000								// 这个函数的精度为1ms，如果想要得到微秒，则把 1000 改为 1000000
	static int64_t offset; LARGE_INTEGER counter;
	if(hrtime_interval_ == 0)
		return 0;
	else if(hrtime_interval_ == -1)
	{
		LARGE_INTEGER perf_frequency; struct timeb tm; LARGE_INTEGER counter;
		InitializeCriticalSection(&process_title_lock);
		if (QueryPerformanceFrequency(&perf_frequency)) {	hrtime_interval_ = 1.0 / perf_frequency.QuadPart;}
		else											{	hrtime_interval_ = 0; return 0;	     			 }

		ftime(&tm);										// note: window 获取时间的精度一般为 15ms 左右， 即系统时间片的长度
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
    char key[32]; ejson itr; int64_t t;
    ejson e = ejso_new(_OBJ_);

    t = utils_timer_now_ms();
    for(uint i = 0; i < scale; i++)
    {
        sprintf(key, "%d", i);

        ejso_addT(e, key, _TRUE_);
    }

    if(ejso_len(e) != scale)
    {
        printf("error: add %d elements, but only have %d elements in e\n", scale, ejso_len(e)); fflush(stdout);
        exit(1);
    }

    printf("add   %d \telem: %6"PRIi64"ms\n", scale, utils_timer_now_ms() - t); fflush(stdout);


    t = utils_timer_now_ms();
    uint j = 0;
    ejso_itr(e, itr)
    {
        if(ejsr(e, ejso_keyS(itr))) j++;
    }

    if(ejso_len(e) != j)
    {
        printf("error: e have %d elements, but only found %d elements in e\n", scale, j); fflush(stdout);
        exit(1);
    }

    printf("found %d \telem: %6"PRIu64"ms\n", j, utils_timer_now_ms() - t); fflush(stdout);

    t = utils_timer_now_ms();
    ejso_free(e);
    printf("free  %d \telem: %6"PRIu64"ms\n\n", j, utils_timer_now_ms() - t); fflush(stdout);
}

static void performance_arr_test(uint scale)
{
    char key[32]; ejson itr; int64_t t;
    ejson e = ejso_new(_ARR_);

    t = utils_timer_now_ms();
    for(uint i = 0; i < scale; i++)
    {
        sprintf(key, "%d", i);

        ejso_addT(e, key, _TRUE_);
    }

    if(ejso_len(e) != scale)
    {
        printf("error: add %d elements, but only have %d elements in e\n", scale, ejso_len(e)); fflush(stdout);
        exit(1);
    }

    printf("add   %d \telem: %6"PRIi64"ms\n", scale, utils_timer_now_ms() - t); fflush(stdout);

    t = utils_timer_now_ms();
    uint j = 0, i = 0;
    ejso_itr(e, itr)
    {
        sprintf(key, "[%d]", i++);
        
        if(itr == ejsk(e, key)) j++;
    }
    
    if(ejso_len(e) != j)
    {
        printf("error: e have %d elements, but only found %d elements in e\n", scale, j); fflush(stdout);
        exit(1);
    }

    printf("found %d \telem: %6"PRIu64"ms\n", j, utils_timer_now_ms() - t); fflush(stdout);

    t = utils_timer_now_ms();
    ejso_free(e);
    printf("free  %d \telem: %6"PRIu64"ms\n\n", j, utils_timer_now_ms() - t); fflush(stdout);
}



void ejson_performance_test()
{
    printf("-- performance_obj_test --\n"); fflush(stdout);
    performance_obj_test(10000);
    performance_obj_test(20000);
    performance_obj_test(50000);
    performance_obj_test(100000);
    performance_obj_test(200000);
    performance_obj_test(500000);
    performance_obj_test(1000000);
    performance_obj_test(2000000);
    
    printf("-- performance_arr_test --\n"); fflush(stdout);
    performance_arr_test(10000);
    performance_arr_test(20000);
    performance_arr_test(50000);
    performance_arr_test(100000);
    performance_arr_test(200000);
    performance_arr_test(500000);
    performance_arr_test(1000000);
    performance_arr_test(2000000);
}

