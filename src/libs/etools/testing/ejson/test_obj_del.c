#include "test_main.h"


void ejson_del_basic()
{
    cstr json_str = "{"
                       "\"false\":false, "
                       "\"true\":true ,"
                       "\"null\":null, "
                       "\"int\":100, "
                       "\"double\":100.123, "
                       "\"str\":\"this is a str obj\","
                       "\"arr\":[false, true, null, 100, 100.123, \"this is a str in arr\", {}]"
                   "}";

    ejson e = ejson_parseS(json_str);
    cstr s = ejson_toS(e, 0, PRETTY);
    printf("\n-- ejson_obj_del_test 1: ejson_delK() --\n");
    printf("test json(%ld):(%d element)\n%s\n", estr_len(s), ejson_len(e), s);

    ejson del;
    del = ejson_takeK(e, "false"); printf("del \"false\" :(%d element)\n%s\n", ejson_len(e), ejson_toS(e, &s, PRETTY)); ejson_free(del);
    del = ejson_takeK(e, "true");  printf("del \"true\"  :(%d element)\n%s\n", ejson_len(e), ejson_toS(e, &s, PRETTY)); ejson_free(del);
    del = ejson_takeK(e, "null");  printf("del \"null\"  :(%d element)\n%s\n", ejson_len(e), ejson_toS(e, &s, PRETTY)); ejson_free(del);
    del = ejson_takeK(e, "int");   printf("del \"int\"   :(%d element)\n%s\n", ejson_len(e), ejson_toS(e, &s, PRETTY)); ejson_free(del);
    del = ejson_takeK(e, "double");printf("del \"double\":(%d element)\n%s\n", ejson_len(e), ejson_toS(e, &s, PRETTY)); ejson_free(del);
    del = ejson_takeK(e, "str");   printf("del \"str\"   :(%d element)\n%s\n", ejson_len(e), ejson_toS(e, &s, PRETTY)); ejson_free(del);
    del = ejson_takeK(e, "arr");   printf("del \"arr\"   :(%d element)\n%s\n", ejson_len(e), ejson_toS(e, &s, PRETTY)); ejson_free(del);
    ejson_free(e);

    printf("\n-- ejson_obj_del_test 2: ejson_del() --\n");
    e = ejson_new(EOBJ, 0);
    ejson add[8];
    add[0] = ejson_addJ(e, "false", "false");
    add[1] = ejson_addJ(e, "true", "true");
    add[2] = ejson_addJ(e, "null", "null");
    add[3] = ejson_addJ(e, "int", "100");
    add[4] = ejson_addJ(e, "double", "100.123");
    add[5] = ejson_addJ(e, "str", "\"this is a str obj\"");
    add[6] = ejson_addJ(e, "arr", "[false, true, null, 100, 100.123, \"this is a str in arr\", {}]");
    add[7] = ejson_addJ(e, "obj", "{}");
    ejson_toS(e, &s, PRETTY);
    printf("test json(%ld):(%d element)\n%s\n", estr_len(s), ejson_len(e), s);
    del = ejson_takeO(e, add[0]);printf("del \"false\" :(%d element)\n%s\n", ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    del = ejson_takeO(e, add[1]);printf("del \"true\"  :(%d element)\n%s\n", ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    del = ejson_takeO(e, add[2]);printf("del \"null\"  :(%d element)\n%s\n", ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    del = ejson_takeO(e, add[3]);printf("del \"int\"   :(%d element)\n%s\n", ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    del = ejson_takeO(e, add[4]);printf("del \"double\":(%d element)\n%s\n", ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    del = ejson_takeO(e, add[5]);printf("del \"str\"   :(%d element)\n%s\n", ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    del = ejson_takeO(e, add[6]);printf("del \"arr\"   :(%d element)\n%s\n", ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    del = ejson_takeO(e, add[7]);printf("del \"obj\"   :(%d element)\n%s\n", ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);

    ejson_free(e);
    estr_free(s);
    printf("\n"); fflush(stdout);
}

void ejson_deep_del_test()
{
    cstr json_str = "{"
                        "\"1\": {\"2\": {\"3\": \"val\"}}"
                   "}";
    ejson e, del; cstr s, k;

    printf("\n-- ejson_obj_del_test 3: ejson_delK() deep test --\n");
    e = ejson_parseS(json_str);
    s = ejson_toS(e, 0, PRETTY);
    printf("test json(%ld):(%d element)\n%s\n", estr_len(s), ejson_len(e), s);
    k ="1.2.3"; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="1.2";   del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="1";     del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);

    ejson_free(e);
    estr_free(s);
    printf("\n"); fflush(stdout);
}

void ejson_arr_del_test()
{
    cstr json_str = "{"
                        "\"arr\": [false, true, null, 100, 100.123, \"this is a str obj\", [false, true, null, 100, 100.123, \"this is a str obj\", [], {}],{}]"
                   "}";
    ejson e, del; cstr s, k;
    printf("\n-- ejson_obj_del_test 4: ejson_delk() arr test --\n");
    e = ejson_parseS(json_str);
    s = ejson_toS(e, 0, PRETTY);
    printf("test json(%ld):(%d element)\n%s\n", estr_len(s), ejson_len(e), s);
    k ="arr[6][7]"; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[6][6]"; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[6][1]"; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[6][0]"; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[6][0]"; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[6][0]"; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[6][0]"; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[6][0]"; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[5]"   ; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[2]"   ; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[3]"   ; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[4]"   ; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[3]"   ; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[1]"   ; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[0]"   ; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[0]"   ; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr[3]"   ; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);
    k ="arr"      ; del = ejson_takeK(e, k);printf("del \"%s\" :(%d element)\n%s\n", k, ejson_len(e), ejson_toS(e, &s, PRETTY));ejson_free(del);

    ejson_free(e);
    estr_free(s);
    printf("\n"); fflush(stdout);
}

void ejson_del_test()
{
    ejson_del_basic();
    ejson_deep_del_test();
    ejson_arr_del_test();
}

int test_obj_del(int argc, char* argv[])
{
    E_UNUSED(argc), E_UNUSED(argv);

    ejson_del_test();

    return ETEST_OK;
}
