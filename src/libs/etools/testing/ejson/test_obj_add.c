#include "test_main.h"

#include "etest.h"

void ejson_obj_add_test()
{
    ejson e; cstr k, ks, s, os = 0; double d; int i;

    printf("\n-- ejson_obj_add_test 4: ejso_add*() --\n");
    e = ejson_new(EOBJ, 0);
    printf("e: %s \n", ejson_toS(e, &os, COMPACT));
    ejson_addT(e, k = "key1", EFALSE);                  printf("add false  to e: k:%s val:none \n", k);
    ejson_addT(e, k = "key2", ETRUE);                   printf("add true   to e: k:%s val:none \n", k);
    ejson_addT(e, k = "key3", ENULL);                   printf("add null   to e: k:%s val:none \n", k);
    ejson_addI(e, k = "key4", i = 100);                 printf("add number to e: k:%s val:%d   \n", k, i);
    ejson_addF(e, k = "key5", d = 100.1214);            printf("add number to e: k:%s val:%f   \n", k, d);
    ejson_addS(e, k = "key6", s = "this is a str");     printf("add str    to e: k:%s val:%s   \n", k, s);
    ejson_addT(e, k = "key7", EARR);                    printf("add arrar  to e: k:%s val:none   \n", k);
    ejson_addT(e, k = "key8", EOBJ);                    printf("add obj    to e: k:%s val:none   \n", k);
    printf("to pretty str:\n%s\n", ejson_toS(e, &os, PRETTY)); ejson_free(e); fflush(stdout);


    printf("\n-- ejson_obj_add_test 5: ejsk_addE() --\n");
    e = ejson_parseS("{}");
    ejson_addO(e, 0, ejson_parseS("\"obj\":{}"));
    printf("e: %s \n", ejson_toS(e, &os, COMPACT));
    ks = "obj";
    ejson_addkJ(e, ks, 0, s = "\"null\":null");             printf("add eval to e.%s: %s \n", ks, s);
    ejson_addkJ(e, ks, 0, s = "\"false\":false");           printf("add eval to e.%s: %s \n", ks, s);
    ejson_addkJ(e, ks, 0, s = "\"true\":true");             printf("add eval to e.%s: %s \n", ks, s);
    ejson_addkJ(e, ks, 0, s = "\"int\": 100");              printf("add eval to e.%s: %s \n", ks, s);
    ejson_addkJ(e, ks, 0, s = "\"double\":100.132");        printf("add eval to e.%s: %s \n", ks, s);
    ejson_addkJ(e, ks, 0, s = "\"str\":\"this is a str\""); printf("add eval to e.%s: %s \n", ks, s);
    ejson_addkJ(e, ks, 0, s = "\"arr\": []");               printf("add eval to e.%s: %s \n", ks, s);
    ejson_addkJ(e, ks, 0, s = "\"obj\": {}");               printf("add eval to e.%s: %s \n", ks, s);
    printf("to pretty str:\n%s\n", ejson_toS(e, &os, PRETTY)); ejson_free(e); fflush(stdout);

    printf("\n-- ejson_obj_add_test 6: ejsk_add*() --\n");
    e = ejson_parseS("{}");
    ejson_addO(e, 0, ejson_parseS("\"obj\":{}"));
    ejson_addkO(e, "obj", 0, ejson_parseS("\"obj\":{}"));
    printf("e: %s \n", ejson_toS(e, &os, COMPACT));
    ks = "obj";
    ejson_addkT(e, ks, k = "key1", EFALSE);               printf("add false  to e.%s: k:%s val:false\n", ks, k);
    ejson_addkT(e, ks, k = "key2", ETRUE);                printf("add true   to e.%s: k:%s val:true \n", ks, k);
    ejson_addkT(e, ks, k = "key3", ENULL);                printf("add null   to e.%s: k:%s val:null \n", ks, k);
    ejson_addkI(e, ks, k = "key4", i = 100);              printf("add number to e.%s: k:%s val:%d   \n", ks, k, i);
    ejson_addkF(e, ks, k = "key5", d = 100.1214);         printf("add number to e.%s: k:%s val:%f   \n", ks, k, d);
    ejson_addkS(e, ks, k = "key6", s = "this is a str");  printf("add str    to e.%s: k:%s val:%s   \n", ks, k, s);
    ejson_addkT(e, ks, k = "key7", EARR);                 printf("add arrar  to e.%s: k:%s val:[] \n", ks, k);
    ejson_addkT(e, ks, k = "key8", EOBJ);                 printf("add obj    to e.%s: k:%s val:{} \n", ks, k);
    printf("to pretty str:\n%s\n", ejson_toS(e, &os, PRETTY)); ejson_free(e); fflush(stdout);

    estr_free(os);

}

int test_obj_add(int argc, char* argv[])
{
    E_UNUSED(argc); E_UNUSED(argv);

    ejson_obj_add_test();

    return ETEST_OK;
}
