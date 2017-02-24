#include "test.h" 

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
    
    ejson e; cstr s, us;
    
    fprintf(stderr, "\n-- ejson_eval_test 1 --\n");
    e = ejss_eval(json_str0); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str0, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str1); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str1, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str2); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str2, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str3); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str3, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str4); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str4, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str5); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str5, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str6); fprintf(stderr, "%s \t-> k: %s, to ufmt str: %s\n",   json_str6, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str7); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str7, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str8); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str8, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    
    json_str0 = "\"key0\":";
    json_str1 = "\"key1\":false";
    json_str2 = "\"key2\":true";
    json_str3 = "\"key3\":null";
    json_str4 = "\"key4\":100";
    json_str5 = "\"key5\":100.123";
    json_str6 = "\"key6\":\"this is a str\"";
    json_str7 = "\"key7\":[]";
    json_str8 = "\"key8\":{}";
    e = ejss_eval(json_str0); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str0, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str1); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str1, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str2); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str2, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str3); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str3, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str4); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str4, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str5); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str5, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str6); fprintf(stderr, "%s \t-> k: %s, to ufmt str: %s\n",   json_str6, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str7); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str7, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    e = ejss_eval(json_str8); fprintf(stderr, "%s \t\t-> k: %s, to ufmt str: %s\n", json_str8, ejso_keyS(e), us = ejso_toUStr(e)); ejso_free(e);ejss_free(us);
    
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


    
    fprintf(stderr, "\n-- ejson_eval_test 2 --\n");
    fprintf(stderr, "src: %s\n", json_str);
    
    e = ejss_evalOpts(json_str, 0, CMMT_ON);
    
    if(!e)
    {
        fprintf(stderr, "err: %s, pos: %s\n",  ejse_str(), ejse_pos());
        exit(0);
    }
    
    s   = ejso_toFStr(e);
    us  = ejso_toUStr(e);
    
    fprintf(stderr, "to pretty str:\n%s\nto ufmt str:\n%s\n", s, us);
    
    ejss_free(s);ejss_free(us);ejso_free(e);
    fprintf(stderr, "\n");

}

void ejson_eval_str_test()
{
    cstr json = "\"1\n2\n3\n\t4\"";
    ejson e; cstr us;

    fprintf(stderr, "\n-- ejson_eval_str_test --\n");
    e = ejss_eval(json);
    fprintf(stderr, "str len: %d\n", ejss_len(ejsk_valS(e, 0)));
    us = ejso_toUStr(e);
    fprintf(stderr, "\t\t-> k: %s, to ufmt str: (%d)%s\n", ejso_keyS(e), ejss_len(us), us);
    ejso_free(e);ejss_free(us);
    
}

void ejson_eval_file_test()
{
    ejson e;

#define FILE1 "test_comment.json"
#define FILE2 "test_nocomment.json"

    e = ejsf_eval("../tests/json/"FILE1); fprintf(stderr, "eval comment ON %s:\t %s %s\n", FILE1, e ? "ok " : "err", e ? "" : ejse_pos()); ejso_free(e);
    e = ejsf_eval("../tests/json/"FILE2); fprintf(stderr, "eval comment ON %s:\t %s %s\n", FILE2, e ? "ok " : "err", e ? "" : ejse_pos()); ejso_free(e);

    ejsf_set(CMMT_OFF);
    e = ejsf_eval("../tests/json/"FILE1); fprintf(stderr, "eval comment OFF %s:\t %s %s\n", FILE1, e ? "ok " : "err", e ? "" : ejse_pos()); ejso_free(e);
    e = ejsf_eval("../tests/json/"FILE2); fprintf(stderr, "eval comment OFF %s:\t %s %s\n", FILE2, e ? "ok " : "err", e ? "" : ejse_pos()); ejso_free(e);

#define FILE3 "test_comment2.json"
    //ejsf_set(CMMT_ON);
    //e = ejsf_eval("../tests/json/"FILE3); fprintf(stderr, "eval comment OFF %s:\t %s %s\n", FILE1, e ? "ok " : "err", e ? "" : ejse_pos()); ejso_free(e);

}

void ejson_eval_test()
{
    ejson_eval_basic_test();
    ejson_eval_str_test();
    ejson_eval_file_test();
}
