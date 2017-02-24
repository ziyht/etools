#include "test.h"


void ejson_obj_get_basic_test()
{
    cstr json_str = "{"
                       "\"key1\":false, "
                       "\"key2\":true ,"
                       "\"key3\":null, "
                       "\"key4\":100, "
                       "\"key5\":100.123, "
                       "\"key6\":\"this is a str obj\","
                       "\"key7\":[false, true, null, 100, 100.123, \"this is a str in arr\", [], {}],"
                       "\"key8\":{}"
                   "}";

    ejson e = ejss_eval(json_str);
    cstr s = ejso_toFStr(e);
    fprintf(stderr, "test json(%d):\n%s\n", ejss_len(s), s);
    ejss_free(s);
    
    cstr us;
    fprintf(stderr, "\n-- ejson_obj_get_basic_test 1 --\n");
    fprintf(stderr, "Keys      Type\tValToUStr\n------------------------------\n");
    fprintf(stderr, ".key1   : %s\t%s\n", ejso_typeS(ejsk(e, "key1" )),                us = ejsk_toUStr(e, "key1")); ejss_free(us);
    fprintf(stderr, ".key2   : %s\t%s\n", ejso_typeS(ejsk(e, "key2" )),                us = ejsk_toUStr(e, "key2")); ejss_free(us);
    fprintf(stderr, ".key3   : %s\t%s\n", ejso_typeS(ejsk(e, "key3" )),                us = ejsk_toUStr(e, "key3")); ejss_free(us);
    fprintf(stderr, ".key4   : %s\t%s\n", ejso_typeS(ejsk(e, "key4" )),                us = ejsk_toUStr(e, "key4")); ejss_free(us);
    fprintf(stderr, ".key5   : %s\t%s\n", ejso_typeS(ejsk(e, "key5" )),                us = ejsk_toUStr(e, "key5")); ejss_free(us);
    fprintf(stderr, ".key6   : %s\t%s\n", ejso_typeS(ejsk(e, "key6" )),                us = ejsk_toUStr(e, "key6")); ejss_free(us);
    fprintf(stderr, ".key7(%d): %s\t%s\n", ejsk_len(e, "key7"), ejsk_typeS(e, "key7"), us = ejsk_toUStr(e, "key7")); ejss_free(us);
    fprintf(stderr, ".key8(%d): %s\t%s\n", ejsk_len(e, "key8"), ejsk_typeS(e, "key8"), us = ejsk_toUStr(e, "key8")); ejss_free(us);
    
    fprintf(stderr, "\n-- ejson_obj_get_basic_test 2 --\n");
    fprintf(stderr, "Keys      Type\tValToUStr\n------------------------------\n");
    fprintf(stderr, ".key7[0]: %s\t%s\n", ejsk_typeS(e, "key7[0]" ), us = ejsk_toUStr(e, "key7[0]" )); ejss_free(us);
    fprintf(stderr, ".key7[1]: %s\t%s\n", ejsk_typeS(e, "key7[1]" ), us = ejsk_toUStr(e, "key7[1]" )); ejss_free(us);
    fprintf(stderr, ".key7[2]: %s\t%s\n", ejsk_typeS(e, "key7[2]" ), us = ejsk_toUStr(e, "key7[2]" )); ejss_free(us);
    fprintf(stderr, ".key7[3]: %s\t%s\n", ejsk_typeS(e, "key7[3]" ), us = ejsk_toUStr(e, "key7[3]" )); ejss_free(us);
    fprintf(stderr, ".key7[4]: %s\t%s\n", ejsk_typeS(e, "key7[4]" ), us = ejsk_toUStr(e, "key7[4]" )); ejss_free(us);
    fprintf(stderr, ".key7[5]: %s\t%s\n", ejsk_typeS(e, "key7[5]" ), us = ejsk_toUStr(e, "key7[5]" )); ejss_free(us);
    fprintf(stderr, ".key7[6]: %s\t%s\n", ejsk_typeS(e, "key7[6]" ), us = ejsk_toUStr(e, "key7[6]" )); ejss_free(us);
    fprintf(stderr, ".key7[7]: %s\t%s\n", ejsk_typeS(e, "key7[7]" ), us = ejsk_toUStr(e, "key7[7]" )); ejss_free(us);
    
    ejson itr;
    fprintf(stderr, "\n-- ejson_obj_get_basic_test 3 --\n");
    fprintf(stderr, "Keys   Type\tValToUStr\n------------------------------\n");
    for(itr = ejso_first(e); itr; itr = ejso_next(itr))
    {
        fprintf(stderr, ".%s: %s\t%s\n", ejso_keyS(itr), ejso_typeS(itr), us = ejso_toUStr(itr)); ejss_free(us);
    }
    for(itr = ejso_last(e); itr; itr = ejso_prev(itr))
    {
        fprintf(stderr, ".%s: %s\t%s\n", ejso_keyS(itr), ejso_typeS(itr), us = ejso_toUStr(itr)); ejss_free(us);
    }
    
    fprintf(stderr, "\n-- ejson_obj_get_basic_test 4 --\n");
    fprintf(stderr, "Keys     Type\ttrue or false\n----------------------------------\n");
    fprintf(stderr, ".key1 is false:\t%s\n", ejsk_is(e, "key1", _FALSE_)? "true" : "false");
    fprintf(stderr, ".key2 is true :\t%s\n", ejsk_is(e, "key2", _TRUE_) ? "true" : "false");
    fprintf(stderr, ".key3 is null :\t%s\n", ejsk_is(e, "key3", _NULL_) ? "true" : "false");
    fprintf(stderr, ".key4 is num  :\t%s\n", ejsk_is(e, "key4", _NUM_)  ? "true" : "false");
    fprintf(stderr, ".key5 is num  :\t%s\n", ejsk_is(e, "key5", _NUM_)  ? "true" : "false");
    fprintf(stderr, ".key6 is str  :\t%s\n", ejsk_is(e, "key6", _STR_)  ? "true" : "false");
    fprintf(stderr, ".key7 is arr  :\t%s\n", ejsk_is(e, "key7", _ARR_)  ? "true" : "false");
    fprintf(stderr, ".key8 is obj  :\t%s\n", ejsk_is(e, "key8", _OBJ_)  ? "true" : "false");
    
    fprintf(stderr, "\n-- ejson_obj_get_basic_test 5 --\n");
    fprintf(stderr, "Keys     Type\ttrue or false\n----------------------------------\n");
    fprintf(stderr, ".key1 is false:\t%s\n", ejsk_is(e, "key1", _FALSE_)? "true" : "false");
    fprintf(stderr, ".key2 is true :\t%s\n", ejsk_is(e, "key2", _TRUE_) ? "true" : "false");
    fprintf(stderr, ".key3 is null :\t%s\n", ejsk_is(e, "key3", _NULL_)  ? "true" : "false");
    fprintf(stderr, ".key4 is num  :\t%s\n", ejsk_is(e, "key4", _NUM_)  ? "true" : "false");
    fprintf(stderr, ".key5 is num  :\t%s\n", ejsk_is(e, "key5", _NUM_)  ? "true" : "false");
    fprintf(stderr, ".key6 is str  :\t%s\n", ejsk_is(e, "key6", _STR_)  ? "true" : "false");
    fprintf(stderr, ".key7 is arr  :\t%s\n", ejsk_is(e, "key7", _ARR_)  ? "true" : "false");
    fprintf(stderr, ".key8 is obj  :\t%s\n", ejsk_is(e, "key8", _OBJ_)  ? "true" : "false");
    
    ejso_free(e);
    fprintf(stderr, "\n");
}

void ejson_obj_deep_get_test()
{
    cstr json_str = "{"
                        "\"1\": {\"2\": {\"3\": [[[\"val1\"], {\"4\":\"val2\"}]]}}"
                   "}";
    
    fprintf(stderr, "-- ejson_obj_deep_get_test --\n");
    ejson e = ejss_eval(json_str);
    
    cstr s = ejso_toFStr(e);
    fprintf(stderr, "test json:\n%s\n", s);
    ejss_free(s);
    
    cstr us;
    fprintf(stderr, "Keys             Type\tValToUStr\n----------------------------------------\n");
    fprintf(stderr, ".1             : %s\t%s\n", ejsk_typeS(e, "1"              ), us = ejsk_toUStr(e, "1"             )); ejss_free(us);
    fprintf(stderr, ".1.2           : %s\t%s\n", ejsk_typeS(e, "1.2"            ), us = ejsk_toUStr(e, "1.2"           )); ejss_free(us);
    fprintf(stderr, ".1.2.3         : %s\t%s\n", ejsk_typeS(e, "1.2.3"          ), us = ejsk_toUStr(e, "1.2.3"         )); ejss_free(us);
    fprintf(stderr, ".1.2.3[0]      : %s\t%s\n", ejsk_typeS(e, "1.2.3[0]"       ), us = ejsk_toUStr(e, "1.2.3[0]"      )); ejss_free(us);
    fprintf(stderr, ".1.2.3[0][0]   : %s\t%s\n", ejsk_typeS(e, "1.2.3[0][0]"    ), us = ejsk_toUStr(e, "1.2.3[0][0]"   )); ejss_free(us);
    fprintf(stderr, ".1.2.3[0][0][0]: %s\t%s\n", ejsk_typeS(e, "1.2.3[0][0][0]" ), us = ejsk_toUStr(e, "1.2.3[0][0][0]")); ejss_free(us);
    fprintf(stderr, ".1.2.3[0][1]   : %s\t%s\n", ejsk_typeS(e, "1.2.3[0][1]"    ), us = ejsk_toUStr(e, "1.2.3[0][1]"   )); ejss_free(us);
    fprintf(stderr, ".1.2.3[0][1].4 : %s\t%s\n", ejsk_typeS(e, "1.2.3[0][1].4"  ), us = ejsk_toUStr(e, "1.2.3[0][1].4" )); ejss_free(us);
    
    ejso_free(e);
    fprintf(stderr, "\n");
}

void ejson_obj_get_test()
{
    ejson_obj_get_basic_test();
    ejson_obj_deep_get_test();
}

