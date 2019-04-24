/**
* this file is create by subsystem 'ETest' from EMake framework
*/

#include <etest.h>

#include "eutils.h"

#include "ejson.h"

static int ejson_sort_obj_test(){

    ejson e = ejson_new(EOBJ, 0);

    ejson_addI(e, "9", 1);
    ejson_addI(e, "8", 1);
    ejson_addI(e, "7", 10);
    ejson_addI(e, "6", 0);
    ejson_addI(e, "5", 1);
    ejson_addI(e, "4", 101);
    ejson_addI(e, "3", 100);
    ejson_addI(e, "2", 1);
    ejson_addI(e, "1", 111);
    ejson_addI(e, "0", 10);

    printf("before sort:\n"); fflush(stdout);
    ejson_show(e);

    printf("sort by keys in Ascending:\n"); fflush(stdout);
    ejson_sort(e, __KEYS_ACS);
    ejson_show(e);

    printf("sort by keys in Descending:\n"); fflush(stdout);
    ejson_sort(e, __KEYS_DES);
    ejson_show(e);

    printf("sort by vali in Ascending:\n"); fflush(stdout);
    ejson_sort(e, __VALI_ACS);
    ejson_show(e);

    printf("sort by vali in Descending:\n"); fflush(stdout);
    ejson_sort(e, __VALI_DES);
    ejson_show(e);

    ejson_free(e);

    return ETEST_OK;
}

static int  ejson_sort_arr_test()
{

    ejson e = ejson_new(EARR, 0);

    ejson_addJ(e, 0, "\"1\":1");
    ejson_addJ(e, 0, "\"10\":10");
    ejson_addJ(e, 0, "100000");      //  no key, this obj should put in tail when sort by keys
    ejson_addJ(e, 0, "100001");
    ejson_addJ(e, 0, "\"101\":101");
    ejson_addJ(e, 0, "\"100\":100");
    ejson_addJ(e, 0, "\"001\":1");
    ejson_addJ(e, 0, "\"111\":111");
    ejson_addJ(e, 0, "\"010\":10");

    printf("before sort:\n"); fflush(stdout);
    ejson_show(e);

    printf("sort by keys in Ascending:\n"); fflush(stdout);
    ejson_sort(e, __KEYS_ACS);
    ejson_show(e);

    printf("sort by keys in Descending:\n"); fflush(stdout);
    ejson_sort(e, __KEYS_DES);
    ejson_show(e);

    printf("sort by vali in Ascending:\n"); fflush(stdout);
    ejson_sort(e, __VALI_ACS);
    ejson_show(e);

    printf("sort by vali in Descending:\n"); fflush(stdout);
    ejson_sort(e, __VALI_DES);
    ejson_show(e);

    ejson_free(e);

    return ETEST_OK;
}

static int perftest(int cnt)
{
    int i; ejson e; char key[16];

    e = ejson_new(EOBJ, 0);

    for(i = 0; i < cnt; i++)
    {
        ll2str(i, key);

        ejson_addI(e, key, i);
    }

    ejson_sort(e, __KEYS_DES);

    ejson_free(e);

    return ETEST_OK;
}

int t6_sort(int argc, char* argv[])
{
    E_UNUSED(argc); E_UNUSED(argv);

    ETEST_RUN( ejson_sort_obj_test() );
    ETEST_RUN( ejson_sort_arr_test() );

    ETEST_RUN( perftest(10000) );

    return ETEST_OK;
}

