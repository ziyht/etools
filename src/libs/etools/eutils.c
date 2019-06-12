/// =====================================================================================
///
///       Filename:  eutils.c
///
///    Description:  tools for etools
///
///        Version:  1.0
///        Created:  09/27/2017 05:58:18 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#define EUTILS_VERSION     "eutils 1.0.5"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "eutils.h"

#ifndef _WIN32
#include <sys/sysinfo.h>
#else
#include <windows.h>
#endif

typedef enum {
  _CLOCK_PRECISE = 0,  /* Use the highest resolution clock available. */
  _CLOCK_FAST    = 1   /* Use the fastest clock with <= 1ms granularity. */
} clocktype_t;

/* Available from 2.6.32 onwards. */
#ifndef CLOCK_MONOTONIC_COARSE
#define CLOCK_MONOTONIC_COARSE 6
#endif

#ifndef _WIN32
#include <sys/time.h>
static i64     __hrtime_nsec_offset;
static clock_t __fast_clock_id;
static inline int __hrtime_init()
{

    struct timespec t1 = {0, 0},
                    t2 = {0, 0};
    struct timeval  t3 = {0, 0};

    // -- check function
    if(clock_gettime(CLOCK_MONOTONIC, &t1))
    {
        if(clock_gettime(CLOCK_REALTIME, &t1))
        {
            perror("clock_gettime not working correctly");
            abort();        /* Not really possible. */
        }

        return 1;           // offset is 0;
    }

    // -- get offset
    if(-1 == gettimeofday(&t3, 0))
    {
        clock_gettime(CLOCK_REALTIME, &t2);
        __hrtime_nsec_offset = (i64)t2.tv_sec * 1000000000 + (i64)t2.tv_nsec - (i64)t1.tv_sec * 1000000000 + (i64)t1.tv_nsec;
    }
    else
        __hrtime_nsec_offset = (i64)t3.tv_sec * 1000000000 + (i64)t3.tv_usec*1000 - (i64)t1.tv_sec * 1000000000 + (i64)t1.tv_nsec;

    // -- check fast get
    if (clock_getres(CLOCK_MONOTONIC_COARSE, &t1) == 0 && t1.tv_nsec <= 1 * 1000 * 1000)
        __fast_clock_id = CLOCK_MONOTONIC_COARSE;
    else
        __fast_clock_id = CLOCK_MONOTONIC;

    return 1;
}

static inline u64 __hrtime_ns(clocktype_t type)
{
    static int __hrtime_init_needed = 1;
    struct timespec t;
    clock_t clock_id;

    if( __hrtime_init_needed ) {
        __hrtime_init();
        __hrtime_init_needed = 0;
    }

    clock_id = (type == _CLOCK_FAST) ? __fast_clock_id : CLOCK_MONOTONIC;

    if (clock_gettime(clock_id, &t))
        clock_gettime(CLOCK_REALTIME, &t);

    return t.tv_sec * 1000000000 + t.tv_nsec + __hrtime_nsec_offset;
}

#else

#include <sys/timeb.h>
static i64    __hrtime_nsec_offset;
static double __hrtime_interval;
#define __hrtime_precise 1000000000LL
static inline int __hrtime_init()
{
    if(__hrtime_interval == 0)
    {
        LARGE_INTEGER perf_frequency; struct timeb tm; LARGE_INTEGER counter;

        if (QueryPerformanceFrequency(&perf_frequency)) {	__hrtime_interval = 1.0 / perf_frequency.QuadPart;}
        else{
            perror("clock_gettime not working correctly");
            abort();        /* Not really possible. */
        }

        ftime(&tm);										// note: the PRECISE of window of this func is 15ms
        QueryPerformanceCounter(&counter);
        __hrtime_nsec_offset = __hrtime_precise * tm.time + __hrtime_precise / 1000 * tm.millitm - (i64) ((double) counter.QuadPart * __hrtime_interval * __hrtime_precise);
    }

    return 1;
}

static inline u64 __hrtime_ns(clocktype_t type)
{
    LARGE_INTEGER counter;

    if(__hrtime_interval == 0) __hrtime_init();

    QueryPerformanceCounter(&counter);

    return (u64) ((double) counter.QuadPart * __hrtime_interval * __hrtime_precise) + __hrtime_nsec_offset;
}
#endif

i64  eutils_nowns() { return __hrtime_ns(_CLOCK_PRECISE)          ; }
i64  eutils_nowms() { return __hrtime_ns(_CLOCK_FAST   ) / 1000000; }

i64  nowns() { return __hrtime_ns(_CLOCK_PRECISE)          ; }
i64  nowms() { return __hrtime_ns(_CLOCK_FAST   ) / 1000000; }

int ll2str(i64 value, cstr s)
{
    char *p, aux;
    unsigned long long v;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    v = (value < 0) ? -value : value;
    p = s;
    do {
        *p++ = '0'+(v%10);
        v /= 10;
    } while(v);
    if (value < 0) *p++ = '-';

    /* Compute length and add null term. */
    l = p-s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while(s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

int ull2str(u64 v, cstr s)
{
    char *p, aux;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    p = s;
    do {
        *p++ = '0'+(v%10);
        v /= 10;
    } while(v);

    /* Compute length and add null term. */
    l = p-s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while(s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

int eutils_rand()
{
    static uint _seed;

    if(!_seed)
    {
        _seed = (unsigned)time(0);
        srand(_seed);
    }

    return rand();
}

int e_strcasecmp(const char *s1, const char *s2)
{
    const unsigned char
    *us1 = (const unsigned char *)s1,
    *us2 = (const unsigned char *)s2;

    while (tolower(*us1) == tolower(*us2++))
        if (*us1++ == '\0')
            return (0);

    return (tolower(*us1) - tolower(*--us2));
}

int e_strncasecmp(const char *s1, const char *s2, size_t n)
{
    if (n != 0)
    {
        const unsigned char
        *us1 = (const unsigned char *)s1,
        *us2 = (const unsigned char *)s2;

        do {
            if (tolower(*us1) != tolower(*us2++))
                    return (tolower(*us1) - tolower(*--us2));
            if (*us1++ == '\0')
                    break;
        } while (--n != 0);
    }
    return (0);
}

int  eutils_nprocs()
{
#ifndef _WIN32
    return get_nprocs();
#else
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
#endif
}

cstr eutils_version()
{
    return EUTILS_VERSION;
}
