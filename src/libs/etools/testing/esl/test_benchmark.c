#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

#include "etest.h"
#include "eutils.h"

#include "esl.h"

static void performance_keyI_test(uint scale)
{
    esl h = esl_new(EKEY_I);  u64 t, tn; uint j, len; i64 i;

    t = eutils_nowms();
    tn = eutils_nowns(); i = 0;
    esl_addR(h, (ekey){i}, 0);
    printf("add   1   \telem: %6"PRIu64"ns\n", eutils_nowns() - tn); fflush(stdout);
    for(i = 1; i < scale; i++)
    {
        esl_addR(h, (ekey){i}, 0);
    }

    if(esl_len(h) != scale)
    {
        printf("error: add %d elements, but only have %d elements in e\n", scale, esl_len(h)); fflush(stdout);
        exit(1);
    }

    printf("add   %d \telem: %6"PRIi64"ms\n", scale, eutils_nowms() - t); fflush(stdout);

    t = eutils_nowms();
    j = 0; scale ++;
    tn = eutils_nowns(); i = 0;
    if(esl_val(h, (ekey){i})) j++;
    printf("find  1   \telem: %6"PRIu64"ns\n", eutils_nowns() - tn); fflush(stdout);
    for(i = 1; i < scale; i++)
    {
        if(esl_val(h, (ekey){i})) j++;
    }

    if(esl_len(h) != j)
    {
        printf("error: e have %d elements, but only found %d elements in e\n", esl_len(h), j); fflush(stdout);
        exit(1);
    }

    printf("found %d \telem: %6"PRIu64"ms\n", j, eutils_nowms() - t); fflush(stdout);

    t = eutils_nowms();
    len = esl_len(h); j = 0; scale ++;

    tn = eutils_nowns(); i = 0;
    if(esl_freeOne(h, (ekey){i})) j++;
    printf("del   1   \telem: %6"PRIu64"ns\n", eutils_nowns() - tn); fflush(stdout);

    for(i = 1; i < scale; i++)
    {
        if(esl_freeOne(h, (ekey){i})) j++;
    }

    if(len != j)
    {
        printf("error: e have %d elements, but only found %d elements in e\n", len, j); fflush(stdout);
        exit(1);
    }

    printf("del   %d \telem: %6"PRIu64"ms\n\n", j, eutils_nowms() - t); fflush(stdout);

    esl_free(h);
}

static void performance_keyS_test(uint scale)
{
    esl h = esl_new(EKEY_S);  char keyS[16]; u64 t; uint j, len;

    t = eutils_nowms();
    for(i64 i = 0; i < scale; i++)
    {
        sprintf(keyS, "%"PRIi64"", i);
        esl_addR(h, ekey_s(keyS), 0);
    }

    if(esl_len(h) != scale)
    {
        printf("error: add %d elements, but only have %d elements in e\n", scale, esl_len(h)); fflush(stdout);
        exit(1);
    }

    printf("add   %d \telem: %6"PRIi64"ms\n", scale, eutils_nowms() - t); fflush(stdout);

#if 1
    t = eutils_nowms();
    j = 0; scale ++;
    for(i64 i = 0; i < scale; i++)
    {
        sprintf(keyS, "%"PRIi64"", i);
        if(esl_val(h, ekey_s(keyS))) j++;
    }

    if(esl_len(h) != j)
    {
        printf("error: e have %d elements, but only found %d elements in e\n", esl_len(h), j); fflush(stdout);
        exit(1);
    }

    printf("found %d \telem: %6"PRIu64"ms\n", j, eutils_nowms() - t); fflush(stdout);
#endif

    t = eutils_nowms();
    len = esl_len(h); j = 0; scale ++;
    for(i64 i = 0; i < scale; i++)
    {
        sprintf(keyS, "%"PRIi64"", i);
        if(esl_freeOne(h, ekey_s(keyS))) j++;
    }

    if(len != j)
    {
        printf("error: e have %d elements, but only found %d elements in e\n", len, j); fflush(stdout);
        exit(1);
    }

    printf("del   %d \telem: %6"PRIu64"ms\n\n", j, eutils_nowms() - t); fflush(stdout);

    esl_free(h);
}

void performance_keyI_rand_test(int scale)
{
    esl h = esl_new(EKEY_I);  int j; i64 i;
    u64 tn1, tn, tn_total, tn_min, tn_max; int key, key_min, key_max;

    for(i = 0; i < scale; i++)
    {
        esl_addR(h, (ekey){i}, 0);
    }

    scale = scale / 10;
    j = 0; tn_total = 0; tn_min = ULLONG_MAX; tn_max = 0;
    for(int i = 0; i < scale; i++)
    {
        key = (uint)eutils_rand() % scale;
        tn1 = eutils_nowns();
        if(esl_val(h, (ekey){key}))j++;
        tn  = eutils_nowns() - tn1;

        tn_total += tn;

        if(tn >= tn_max)
        {
            tn_max = tn;
            key_max = key;
        }
        if(tn < tn_min)
        {
            tn_min = tn;
            key_min = key;
        }
    }

    if(scale != j)
    {
        printf("error: e have %d times, but only found %d elements in e\n", 10, j); fflush(stdout);
        exit(1);
    }
    printf("-- %7d scale\n", scale * 10);
    printf("found %d  \telem: %6"PRIu64"ms\n", j, tn_total/1000); fflush(stdout);
    printf("found %d \t    : %6"PRIu64"ns (min)\n", key_min, tn_min); fflush(stdout);
    printf("found %d \t    : %6"PRIu64"ns (max)\n", key_max, tn_max); fflush(stdout);
    puts("");

    esl_free(h);
}

#define calgrind 1

int test_benchmark(int argc, char* argv[])
{
#if 1
    printf("-- performance_keyS_test --\n"); fflush(stdout);
    performance_keyS_test(  10000);
    performance_keyS_test(  20000);

#if !calgrind
    performance_keyS_test(  50000);
    performance_keyS_test( 100000);
    performance_keyS_test( 200000);
    performance_keyS_test( 500000);
    performance_keyS_test(1000000);
    performance_keyS_test(2000000);

#endif
#endif

#if 1
    printf("-- performance_keyI_test --\n"); fflush(stdout);
    performance_keyI_test(  10000);
    performance_keyI_test(  20000);

#if !calgrind
    performance_keyI_test(  50000);
    performance_keyI_test( 100000);
    performance_keyI_test( 200000);
    performance_keyI_test( 500000);
    performance_keyI_test(1000000);
    performance_keyI_test(2000000);
#endif
#endif


#if 1
    printf("-- performance_keyI_rand_test --\n"); fflush(stdout);
    performance_keyI_rand_test(  10000);
    performance_keyI_rand_test(  20000);

#if !calgrind
    performance_keyI_rand_test(  50000);
    performance_keyI_rand_test( 100000);
    performance_keyI_rand_test( 200000);
    performance_keyI_rand_test( 500000);
    performance_keyI_rand_test(1000000);
    performance_keyI_rand_test(2000000);
#endif

#endif

    return ETEST_OK;
}
