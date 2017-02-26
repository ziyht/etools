#include "test.h"

void ejson_obj_add_test()
{
    printf("\n-- ejson_obj_add_test 1: add_eval() --\n");
    ejson e = ejso_new(_OBJ_);
    cstr k, ks, s; double d; int i;
    printf("e: %s \n", s = ejso_toUStr(e)); ejss_free(s);
    ejso_addE(e, 0, s = "\"null\":null");              printf("add eval to e: %s \n", s);
    ejso_addE(e, 0, s = "\"false\":false");            printf("add eval to e: %s \n", s);
    ejso_addE(e, 0, s = "\"true\":true");              printf("add eval to e: %s \n", s);
    ejso_addE(e, 0, s = "\"int\": 100");               printf("add eval to e: %s \n", s);
    ejso_addE(e, 0, s = "\"double\":100.132");         printf("add eval to e: %s \n", s);
    ejso_addE(e, 0, s = "\"str\":\"this is a str\"");  printf("add eval to e: %s \n", s);
    ejso_addE(e, 0, s = "\"arr\": []");                printf("add eval to e: %s \n", s);
    ejso_addE(e, 0, s = "\"obj\": {}");                printf("add eval to e: %s \n", s);
    printf("e to pretty str:\n%s\n", s = ejso_toFStr(e)); ejss_free(s);ejso_free(e); fflush(stdout);

    printf("\n-- ejson_obj_add_test 2: add_*() --\n");
    e = ejso_new(_OBJ_);
    printf("e: %s \n", s = ejso_toUStr(e)); ejss_free(s);
    ejso_addT(e, k = "key1", _FALSE_);                  printf("add false  to e: k:%s val:none \n", k);
    ejso_addT(e, k = "key2", _TRUE_);                   printf("add true   to e: k:%s val:none \n", k);
    ejso_addT(e, k = "key3", _NULL_);                   printf("add null   to e: k:%s val:none \n", k);
    ejso_addF(e, k = "key4", i = 100);                  printf("add number to e: k:%s val:%d   \n", k, i);
    ejso_addF(e, k = "key5", d = 100.1214);             printf("add number to e: k:%s val:%f   \n", k, d);
    ejso_addS(e, k = "key6", s = "this is a str");      printf("add str    to e: k:%s val:%s   \n", k, s);
    ejso_addT(e, k = "key7", _ARR_);                    printf("add arrar  to e: k:%s val:none   \n", k);
    ejso_addT(e, k = "key8", _OBJ_);                    printf("add obj    to e: k:%s val:none   \n", k);
    printf("to pretty str:\n%s\n", s = ejso_toFStr(e)); ejss_free(s);ejso_free(e); fflush(stdout);


    printf("\n-- ejson_obj_add_test 3: add_evalTo() --\n");
    e = ejss_eval("{}");
    ejso_addO(e, 0, ejss_eval("\"obj\":{}"));
    printf("e: %s \n", s = ejso_toUStr(e)); ejss_free(s);
    ks = "obj";
    ejsk_addE(e, ks, 0, s = "\"null\":null");             printf("add eval to e.%s: %s \n", ks, s);
    ejsk_addE(e, ks, 0, s = "\"false\":false");           printf("add eval to e.%s: %s \n", ks, s);
    ejsk_addE(e, ks, 0, s = "\"true\":true");             printf("add eval to e.%s: %s \n", ks, s);
    ejsk_addE(e, ks, 0, s = "\"int\": 100");              printf("add eval to e.%s: %s \n", ks, s);
    ejsk_addE(e, ks, 0, s = "\"double\":100.132");        printf("add eval to e.%s: %s \n", ks, s);
    ejsk_addE(e, ks, 0, s = "\"str\":\"this is a str\""); printf("add eval to e.%s: %s \n", ks, s);
    ejsk_addE(e, ks, 0, s = "\"arr\": []");               printf("add eval to e.%s: %s \n", ks, s);
    ejsk_addE(e, ks, 0, s = "\"obj\": {}");               printf("add eval to e.%s: %s \n", ks, s);
    printf("to pretty str:\n%s\n", s = ejso_toFStr(e)); ejss_free(s);ejso_free(e); fflush(stdout);

    printf("\n-- ejson_obj_add_test 4: add_*To() --\n");
    e = ejss_eval("{}");
    ejso_addO(e, 0, ejss_eval("\"obj\":{}"));
    ejsk_addO(e, "obj", 0, ejss_eval("\"obj\":{}"));
    printf("e: %s \n", s = ejso_toUStr(e)); ejss_free(s);
    ks = "obj";
    ejsk_addT(e, ks, k = "key1", _FALSE_);               printf("add false  to e.%s: k:%s val:none \n", ks, k);
    ejsk_addT(e, ks, k = "key2", _TRUE_);                printf("add true   to e.%s: k:%s val:none \n", ks, k);
    ejsk_addT(e, ks, k = "key3", _NULL_);                printf("add null   to e.%s: k:%s val:none \n", ks, k);
    ejsk_addF(e, ks, k = "key4", i = 100);               printf("add number to e.%s: k:%s val:%d   \n", ks, k, i);
    ejsk_addF(e, ks, k = "key5", d = 100.1214);          printf("add number to e.%s: k:%s val:%f   \n", ks, k, d);
    ejsk_addS(e, ks, k = "key6", s = "this is a str");   printf("add str    to e.%s: k:%s val:%s   \n", ks, k, s);
    ejsk_addT(e, ks, k = "key7", _ARR_);                 printf("add arrar  to e.%s: k:%s val:none \n", ks, k);
    ejsk_addT(e, ks, k = "key8", _OBJ_);                 printf("add obj    to e.%s: k:%s val:none \n", ks, k);
    printf("to pretty str:\n%s\n", s = ejso_toFStr(e)); ejss_free(s);ejso_free(e); fflush(stdout);



}

