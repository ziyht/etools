#include "test.h"

void ejson_sort_obj_test(){
  
    ejson e = ejso_new(_OBJ_); cstr s;

    //ejso_addE(e, 0, "\"1\":1");
    ejso_addF(e, "1", 1);
    ejso_addF(e, "10", 10);
    ejso_addF(e, "0", 0);
    ejso_addF(e, "01", 1);
    ejso_addF(e, "101", 101);
    ejso_addF(e, "100", 100);
    ejso_addF(e, "001", 1);
    ejso_addF(e, "111", 111);
    ejso_addF(e, "010", 10);

    printf("before sort:\n"); fflush(stdout);
    s = ejso_toFStr(e); printf("%s\n", s); fflush(stdout); ejss_free(s);
    
    printf("sort by keys in Ascending:\n"); fflush(stdout);
    ejso_sort(e, __KEYS_ACS);
    s = ejso_toFStr(e); printf("%s\n", s); fflush(stdout); ejss_free(s);
    
    printf("sort by keys in Descending:\n"); fflush(stdout);
    ejso_sort(e, __KEYS_DES);
    s = ejso_toFStr(e); printf("%s\n", s); fflush(stdout); ejss_free(s);
    
    printf("sort by vali in Ascending:\n"); fflush(stdout);
    ejso_sort(e, __VALI_ACS);
    s = ejso_toFStr(e); printf("%s\n", s); fflush(stdout); ejss_free(s);
    
    printf("sort by vali in Descending:\n"); fflush(stdout);
    ejso_sort(e, __VALI_DES);
    s = ejso_toFStr(e); printf("%s\n", s); fflush(stdout); ejss_free(s);
    
    ejso_free(e);
}

void ejson_sort_arr_test(){
  
    ejson e = ejso_new(_ARR_); cstr s;

    ejso_addE(e, 0, "\"1\":1");
    ejso_addE(e, 0, "\"10\":10");
    ejso_addE(e, 0, "100000");      //  no key, this obj should put in tail when sort by keys
    ejso_addE(e, 0, "100001");
    ejso_addE(e, 0, "\"101\":101");
    ejso_addE(e, 0, "\"100\":100");
    ejso_addE(e, 0, "\"001\":1");
    ejso_addE(e, 0, "\"111\":111");
    ejso_addE(e, 0, "\"010\":10");

    printf("before sort:\n"); fflush(stdout);
    s = ejso_toFStr(e); printf("%s\n", s); fflush(stdout); ejss_free(s);
    
    printf("sort by keys in Ascending:\n"); fflush(stdout);
    ejso_sort(e, __KEYS_ACS);
    s = ejso_toFStr(e); printf("%s\n", s); fflush(stdout); ejss_free(s);
    
    printf("sort by keys in Descending:\n"); fflush(stdout);
    ejso_sort(e, __KEYS_DES);
    s = ejso_toFStr(e); printf("%s\n", s); fflush(stdout); ejss_free(s);
    
    printf("sort by vali in Ascending:\n"); fflush(stdout);
    ejso_sort(e, __VALI_ACS);
    s = ejso_toFStr(e); printf("%s\n", s); fflush(stdout); ejss_free(s);
    
    printf("sort by vali in Descending:\n"); fflush(stdout);
    ejso_sort(e, __VALI_DES);
    s = ejso_toFStr(e); printf("%s\n", s); fflush(stdout); ejss_free(s);
    
    ejso_free(e);
}

void ejson_sort_test()
{
    ejson_sort_obj_test();
    ejson_sort_arr_test();
}
