#include "test_main.h"

void assert_true(int expression, echan chan, char* msg)
{
    if (!expression)
    {
        echan_free(chan);
        fprintf(stderr, "Assertion failed: %s\n", msg);
        exit(1);
    }
}

static int passed;

void pass()
{
    passed++;
}

void show_pass()
{
    printf("passed: %d\n", passed);
    fflush(stdout);
}

#if _WIN32
#include <Windows.h>
#include <sys/timeb.h>
static double hrtime_interval_ = -1;
static CRITICAL_SECTION process_title_lock;
#endif

i64 utils_timer_now_ms() {
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
