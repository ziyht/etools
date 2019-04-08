#include "test_main.h"

void ejson_sort_obj_test(){

    ejson e = ejson_new(EOBJ, 0); cstr s = 0;

    //ejson_addJ(e, 0, "\"1\":1");
    ejson_addI(e, "1", 1);
    ejson_addI(e, "10", 10);
    ejson_addI(e, "0", 0);
    ejson_addI(e, "01", 1);
    ejson_addI(e, "101", 101);
    ejson_addI(e, "100", 100);
    ejson_addI(e, "001", 1);
    ejson_addI(e, "111", 111);
    ejson_addI(e, "010", 10);

    printf("before sort:\n"); fflush(stdout);
    ejson_toS(e, &s, PRETTY); printf("%s\n", s); fflush(stdout);

//    printf("sort by keys in Ascending:\n"); fflush(stdout);
//    ejso_sort(e, __KEYS_ACS);
//    ejson_toS_p(e, &s); printf("%s\n", s); fflush(stdout);

//    printf("sort by keys in Descending:\n"); fflush(stdout);
//    ejso_sort(e, __KEYS_DES);
//    ejson_toS_p(e, &s); printf("%s\n", s); fflush(stdout);

//    printf("sort by vali in Ascending:\n"); fflush(stdout);
//    ejso_sort(e, __VALI_ACS);
//    ejson_toS_p(e, &s); printf("%s\n", s); fflush(stdout);

//    printf("sort by vali in Descending:\n"); fflush(stdout);
//    ejso_sort(e, __VALI_DES);
//    ejson_toS_p(e, &s); printf("%s\n", s); fflush(stdout);

    ejson_free(e);
    estr_free(s);
}

void ejson_sort_arr_test(){

    ejson e = ejson_new(EARR, 0); cstr s = 0;

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
    ejson_toS(e, &s, PRETTY); printf("%s\n", s); fflush(stdout);

//    printf("sort by keys in Ascending:\n"); fflush(stdout);
//    ejso_sort(e, __KEYS_ACS);
//    ejson_toS_p(e, &s); printf("%s\n", s); fflush(stdout);

//    printf("sort by keys in Descending:\n"); fflush(stdout);
//    ejso_sort(e, __KEYS_DES);
//    ejson_toS_p(e, &s); printf("%s\n", s); fflush(stdout);

//    printf("sort by vali in Ascending:\n"); fflush(stdout);
//    ejso_sort(e, __VALI_ACS);
//    ejson_toS_p(e, &s); printf("%s\n", s); fflush(stdout);

//    printf("sort by vali in Descending:\n"); fflush(stdout);
//    ejso_sort(e, __VALI_DES);
//    ejson_toS_p(e, &s); printf("%s\n", s); fflush(stdout);

    ejson_free(e);
    estr_free(s);
}

void ejson_sort_test()
{
    ejson_sort_obj_test();
    ejson_sort_arr_test();
}

int test_obj_sort(int argc, char* argv[])
{
    E_UNUSED(argc); E_UNUSED(argv);

    ejson_sort_test();

    return ETEST_OK;
}
