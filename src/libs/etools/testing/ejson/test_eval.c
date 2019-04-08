#include "test_main.h"

void ejson_eval_basic_test()
{

    cstr json_str0 = "";
    cstr json_str1 = "false";
    cstr json_str2 = "true";
    cstr json_str3 = "null";
    cstr json_str4 = "100";
    cstr json_str5 = "100.123";
    cstr json_str6 = "\"this is a str\"";
    cstr json_str7 = "[]";
    cstr json_str8 = "{}";

    ejson e; cstr s = 0, us = 0;

    printf("\n-- ejson_eval_test 1 --\n");
    e = ejson_parseS(json_str0); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str0, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str1); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str1, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str2); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str2, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str3); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str3, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str4); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str4, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str5); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str5, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str6); printf("%s \t-> k: %s, to ufmt str: %s\n",   json_str6, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str7); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str7, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str8); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str8, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    fflush(stdout);

    json_str0 = "\"key0\":";
    json_str1 = "\"key1\":false";
    json_str2 = "\"key2\":true";
    json_str3 = "\"key3\":null";
    json_str4 = "\"key4\":100";
    json_str5 = "\"key5\":100.123";
    json_str6 = "\"key6\":\"this is a str\"";
    json_str7 = "\"key7\":[]";
    json_str8 = "\"key8\":{}";
    e = ejson_parseS(json_str0); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str0, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str1); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str1, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str2); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str2, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str3); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str3, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str4); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str4, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str5); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str5, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str6); printf("%s \t-> k: %s, to ufmt str: %s\n",   json_str6, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str7); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str7, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    e = ejson_parseS(json_str8); printf("%s \t\t-> k: %s, to ufmt str: %s\n", json_str8, eobj_keyS(e), ejson_toS(e, &us, COMPACT)); ejson_free(e);
    fflush(stdout);

    cstr json_str = "{"
                       "\"false\":false, #asdasdfdsasdf\n"
                       "\"true\":true ,//asdfasdfasdf\n"
                       "\"null\":null,/*sdfasdgdfsgsdfgsdfgsdfgsdfg*/"
                       "\"int\":100, "
                       "\"double\":100.123, "
                       "\"str\":\"this is a str obj\","
                       "\"arr\":[false, true, null, 100, 100.123, \"this is a str in arr\", {}],"
                       "\"obj\":{"
                           "\"false\":false, "
                           "\"true\":true ,"
                           "\"null\":null, "
                           "\"int\":100, "
                           "\"double\":100.123, "
                           "\"str\":\"this is a str obj\","
                           "\"arr\":[false, true, null, 100, 100.123, \"this is a str in arr\", {}],"
                           "\"obj\":{}"
                       "}"
                   "}";



    printf("\n-- ejson_eval_test 2 --\n");
    printf("src: %s\n", json_str);

    e = ejson_parseSEx(json_str, 0, COMMENT);

    if(!e)
    {
        fprintf(stderr, "err: %s, pos: %s\n",  ejson_err(), ejson_errp());
        exit(0);
    }

    ejson_toS(e, &s , PRETTY);
    ejson_toS(e, &us, COMPACT);

    printf("to pretty str:\n%s\nto ufmt str:\n%s\n", s, us);

    estr_free(s);
    estr_free(us);
    ejson_free(e);
    printf("\n"); fflush(stdout);
}

void ejson_eval_str_test()
{
    cstr json = "\"1\n2\n3\n\t4\"";
    ejson e; cstr us = 0;

    printf("\n-- ejson_eval_str_test --\n");
    e = ejson_parseS(json);
    printf("str len: %d\n", eobj_len(e));
    ejson_toS(e, &us, COMPACT);
    printf("\t\t-> k: %s, to ufmt str: (%ld)%s\n", eobj_keyS(e), estr_len(us), us);
    ejson_free(e); estr_free(us);
    fflush(stdout);
}

void ejson_eval_file_test()
{
    ejson e;

#define DIR MAIN_PROJECT_ROOT_DIR "src/libs/etools/testing/ejson/json/"

#define FILE1 "test_comment.json"
#define FILE2 "test_nocomment.json"

    e = ejson_parseF(DIR FILE1); printf("eval comment ON %s:\t %s %s\n", FILE1, e ? "ok " : "err", e ? "" : ejson_errp()); ejson_free(e);
    e = ejson_parseF(DIR FILE2); printf("eval comment ON %s:\t %s %s\n", FILE2, e ? "ok " : "err", e ? "" : ejson_errp()); ejson_free(e);

    e = ejson_parseFEx(DIR FILE1, 0, ENDCHECK); printf("eval comment OFF %s:\t %s %s\n", FILE1, e ? "ok " : "err", e ? "" : ejson_errp()); ejson_free(e);
    e = ejson_parseFEx(DIR FILE2, 0, ENDCHECK); printf("eval comment OFF %s:\t %s %s\n", FILE2, e ? "ok " : "err", e ? "" : ejson_errp()); ejson_free(e);

#define FILE3 "test_comment2.json"
    e = ejson_parseFEx(DIR FILE3, 0, COMMENT); fprintf(stderr, "eval comment OFF %s:\t %s %s\n", FILE3, e ? "ok " : "err", e ? "" : ejson_errp()); ejson_free(e);

}

void ejson_eval_bigfile_test()
{
    ejson e; i64 t;

    t = eutils_nowms();
    e = ejson_parseF(DIR "big.json"); printf("eval comment ON %s:\t %s %s\n", FILE1, e ? "ok " : "err", e ? "" : ejson_errp());
    printf("eval   \t cost: %6"PRId64"ms\n", eutils_nowms() - t); fflush(stdout);

    t = eutils_nowms();
    ejson_free(e);
    printf("release\t cost: %6"PRId64"ms\n", eutils_nowms() - t); fflush(stdout);
}

void ejson_eval_bug_test()
{
    ejson e; i64 t;

    t = eutils_nowms();
    e = ejson_parseS("{\"req\":\"query\",\"jobid\":[1553551,1543058,1545532]}");

    cstr s = ejson_toS(e, 0, PRETTY);

    printf("%s\n\n", s); estr_free(s);

    t = eutils_nowms();
    ejson_free(e);
    printf("release\t cost: %6"PRIi64"ms\n", eutils_nowms() - t); fflush(stdout);
}

void ejson_eval_test()
{
    ejson_eval_basic_test();
    ejson_eval_str_test();
    ejson_eval_file_test();

    ejson_eval_bigfile_test();

    ejson_eval_bug_test();
}

int test_eval(int argc, char* argv[])
{
    E_UNUSED(argc); E_UNUSED(argv);

    ejson_eval_test();

    return ETEST_OK;
}
