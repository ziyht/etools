//
// Created by ziyht on 17-2-15.
//


#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

#include "eutils.h"
#include "ejson.h"


static void performance_obj_test(uint scale)
{
    char key[32]; ejson itr; int64_t t;
    ejson e = ejson_new(EOBJ, 0);

    t = eutils_nowms();
    for(uint i = 0; i < scale; i++)
    {
        sprintf(key, "%d", i);

        ejson_addT(e, key, ETRUE);
    }

    if(ejson_len(e) != scale)
    {
        printf("error: add %d elements, but only have %d elements in e\n", scale, ejson_len(e)); fflush(stdout);
        exit(1);
    }

    printf("add   %d \telem: %6"PRIi64"ms\n", scale, eutils_nowms() - t); fflush(stdout);


    t = eutils_nowms();
    uint j = 0;
    ejson_foreach(e, itr)
    {
        if(ejsonk(e, eobj_keyS(itr))) j++;
    }

    if(ejson_len(e) != j)
    {
        printf("error: e have %d elements, but only found %d elements in e\n", scale, j); fflush(stdout);
        exit(1);
    }

    printf("found %d \telem: %6"PRIu64"ms\n", j, eutils_nowms() - t); fflush(stdout);

    t = eutils_nowms();
    ejson_free(e);
    printf("free  %d \telem: %6"PRIu64"ms\n\n", j, eutils_nowms() - t); fflush(stdout);
}

static void performance_arr_test(uint scale)
{
    char key[32]; ejson itr; int64_t t;
    ejson e = ejson_new(EARR, 0);

    t = eutils_nowms();
    for(uint i = 0; i < scale; i++)
    {
        sprintf(key, "%d", i);

        ejson_addT(e, key, ETRUE);
    }

    if(ejson_len(e) != scale)
    {
        printf("error: add %d elements, but only have %d elements in e\n", scale, ejson_len(e)); fflush(stdout);
        exit(1);
    }

    printf("add   %d \telem: %6"PRIi64"ms\n", scale, eutils_nowms() - t); fflush(stdout);

    t = eutils_nowms();
    uint j = 0, i = 0;
    ejson_foreach(e, itr)
    {
        sprintf(key, "[%d]", i++);

        if(itr == ejsonp(e, key)) j++;
    }

    if(ejson_len(e) != j)
    {
        printf("error: e have %d elements, but only found %d elements in e\n", scale, j); fflush(stdout);
        exit(1);
    }

    printf("found %d \telem: %6"PRIu64"ms\n", j, eutils_nowms() - t); fflush(stdout);

    t = eutils_nowms();
    ejson_free(e);
    printf("free  %d \telem: %6"PRIu64"ms\n\n", j, eutils_nowms() - t); fflush(stdout);
}

#define CALLGRIND 0

void ejson_performance_test()
{
    printf("-- performance_obj_test --\n"); fflush(stdout);
    performance_obj_test(10000);
    performance_obj_test(20000);
    performance_obj_test(50000);
#if !CALLGRIND
    performance_obj_test(100000);
    performance_obj_test(200000);
    performance_obj_test(500000);
    performance_obj_test(1000000);
    performance_obj_test(2000000);
#endif

    printf("-- performance_arr_test --\n"); fflush(stdout);
    performance_arr_test(10000);
    performance_arr_test(20000);
    performance_arr_test(50000);
#if !CALLGRIND
    performance_arr_test(100000);
    performance_arr_test(200000);
    performance_arr_test(500000);
    performance_arr_test(1000000);
    performance_arr_test(2000000);
#endif
}

int test_performance(int argc, char* argv[])
{
    ejson_performance_test();

    return ETEST_OK;
}

