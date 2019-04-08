#include "test_main.h"


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

    ejson e = ejson_parseS(json_str);
    cstr us = ejson_toS(e, 0, PRETTY);
    printf("test json(%ld):\n%s\n", estr_len(us), us);

    printf("\n-- ejson_obj_get_basic_test 1 --\n");
    printf("Keys      Type\tValToUStr\n------------------------------\n");
    printf(".key1   : %s\t%s\n", eobj_typeoS(ejson_valk(e, "key1" )),                ejson_toSk(e, "key1", &us, COMPACT));
    printf(".key2   : %s\t%s\n", eobj_typeoS(ejson_valk(e, "key2" )),                ejson_toSk(e, "key2", &us, COMPACT));
    printf(".key3   : %s\t%s\n", eobj_typeoS(ejson_valk(e, "key3" )),                ejson_toSk(e, "key3", &us, COMPACT));
    printf(".key4   : %s\t%s\n", eobj_typeoS(ejson_valk(e, "key4" )),                ejson_toSk(e, "key4", &us, COMPACT));
    printf(".key5   : %s\t%s\n", eobj_typeoS(ejson_valk(e, "key5" )),                ejson_toSk(e, "key5", &us, COMPACT));
    printf(".key6   : %s\t%s\n", eobj_typeoS(ejson_valk(e, "key6" )),                ejson_toSk(e, "key6", &us, COMPACT));
    printf(".key7(%d): %s\t%s\n", ejson_valkLen(e, "key7"), ejson_valkTypeS(e, "key7"), ejson_toSk(e, "key7", &us, COMPACT));
    printf(".key8(%d): %s\t%s\n", ejson_valkLen(e, "key8"), ejson_valkTypeS(e, "key8"), ejson_toSk(e, "key8", &us, COMPACT));

    printf("\n-- ejson_obj_get_basic_test 2 --\n");
    printf("Keys      Type\tValToUStr\n------------------------------\n");
    printf(".key7[0]: %s\t%s\n", ejson_valkTypeS(e, "key7[0]" ), ejson_toSk(e, "key7[0]", &us, COMPACT));
    printf(".key7[1]: %s\t%s\n", ejson_valkTypeS(e, "key7[1]" ), ejson_toSk(e, "key7[1]", &us, COMPACT));
    printf(".key7[2]: %s\t%s\n", ejson_valkTypeS(e, "key7[2]" ), ejson_toSk(e, "key7[2]", &us, COMPACT));
    printf(".key7[3]: %s\t%s\n", ejson_valkTypeS(e, "key7[3]" ), ejson_toSk(e, "key7[3]", &us, COMPACT));
    printf(".key7[4]: %s\t%s\n", ejson_valkTypeS(e, "key7[4]" ), ejson_toSk(e, "key7[4]", &us, COMPACT));
    printf(".key7[5]: %s\t%s\n", ejson_valkTypeS(e, "key7[5]" ), ejson_toSk(e, "key7[5]", &us, COMPACT));
    printf(".key7[6]: %s\t%s\n", ejson_valkTypeS(e, "key7[6]" ), ejson_toSk(e, "key7[6]", &us, COMPACT));
    printf(".key7[7]: %s\t%s\n", ejson_valkTypeS(e, "key7[7]" ), ejson_toSk(e, "key7[7]", &us, COMPACT));

    ejson itr;
    printf("\n-- ejson_obj_get_basic_test 3 --\n");
    printf("Keys   Type\tValToUStr\n------------------------------\n");
    for(itr = ejson_first(e); itr; itr = ejson_next(itr))
    {
        printf(".%s: %s\t%s\n", eobj_keyS(itr), eobj_typeoS(itr), ejson_toS(itr, &us, COMPACT));
    }
    for(itr = ejson_last(e); itr; itr = ejson_prev(itr))
    {
        printf(".%s: %s\t%s\n", eobj_keyS(itr), eobj_typeoS(itr), ejson_toS(itr, &us, COMPACT));
    }

    printf("\n-- ejson_obj_get_basic_test 4 --\n");
    printf("Keys     Type\ttrue or false\n----------------------------------\n");
    printf(".key1 is false:\t%s\n", (ejson_valkType(e, "key1") == EFALSE)? "true" : "false");
    printf(".key2 is true :\t%s\n", (ejson_valkType(e, "key2") == ETRUE) ? "true" : "false");
    printf(".key3 is null :\t%s\n", (ejson_valkType(e, "key3") == ENULL) ? "true" : "false");
    printf(".key4 is num  :\t%s\n", (ejson_valkType(e, "key4") == ENUM)  ? "true" : "false");
    printf(".key5 is num  :\t%s\n", (ejson_valkType(e, "key5") == ENUM)  ? "true" : "false");
    printf(".key6 is str  :\t%s\n", (ejson_valkType(e, "key6") == ESTR)  ? "true" : "false");
    printf(".key7 is arr  :\t%s\n", (ejson_valkType(e, "key7") == EARR)  ? "true" : "false");
    printf(".key8 is obj  :\t%s\n", (ejson_valkType(e, "key8") == EOBJ)  ? "true" : "false");

    printf("\n-- ejson_obj_get_basic_test 5 --\n");
    printf("Keys     Type\ttrue or false\n----------------------------------\n");
    printf(".key1 is false:\t%s\n", (ejson_valkType(e, "key1") == EFALSE)? "true" : "false");
    printf(".key2 is true :\t%s\n", (ejson_valkType(e, "key2") == ETRUE) ? "true" : "false");
    printf(".key3 is null :\t%s\n", (ejson_valkType(e, "key3") == ENULL) ? "true" : "false");
    printf(".key4 is num  :\t%s\n", (ejson_valkType(e, "key4") == ENUM)  ? "true" : "false");
    printf(".key5 is num  :\t%s\n", (ejson_valkType(e, "key5") == ENUM)  ? "true" : "false");
    printf(".key6 is str  :\t%s\n", (ejson_valkType(e, "key6") == ESTR)  ? "true" : "false");
    printf(".key7 is arr  :\t%s\n", (ejson_valkType(e, "key7") == EARR)  ? "true" : "false");
    printf(".key8 is obj  :\t%s\n", (ejson_valkType(e, "key8") == EOBJ)  ? "true" : "false");

    ejson_free(e);
    estr_free(us);
    printf("\n"); fflush(stdout);
}

void ejson_obj_deep_get_test()
{
    cstr json_str = "{"
                        "\"1\": {\"2\": {\"3\": [[[\"val1\"], {\"4\":\"val2\"}]]}}"
                   "}";

    printf("-- ejson_obj_deep_get_test --\n");
    ejson e = ejson_parseS(json_str);

    cstr us = ejson_toS(e, 0, PRETTY);
    printf("test json:\n%s\n", us);

    printf("Keys             Type\tValToUStr\n----------------------------------------\n");
    printf(".1             : %s\t%s\n", ejson_valkTypeS(e, "1"              ), ejson_toSk(e, "1"             , &us, COMPACT));
    printf(".1.2           : %s\t%s\n", ejson_valkTypeS(e, "1.2"            ), ejson_toSk(e, "1.2"           , &us, COMPACT));
    printf(".1.2.3         : %s\t%s\n", ejson_valkTypeS(e, "1.2.3"          ), ejson_toSk(e, "1.2.3"         , &us, COMPACT));
    printf(".1.2.3[0]      : %s\t%s\n", ejson_valkTypeS(e, "1.2.3[0]"       ), ejson_toSk(e, "1.2.3[0]"      , &us, COMPACT));
    printf(".1.2.3[0][0]   : %s\t%s\n", ejson_valkTypeS(e, "1.2.3[0][0]"    ), ejson_toSk(e, "1.2.3[0][0]"   , &us, COMPACT));
    printf(".1.2.3[0][0][0]: %s\t%s\n", ejson_valkTypeS(e, "1.2.3[0][0][0]" ), ejson_toSk(e, "1.2.3[0][0][0]", &us, COMPACT));
    printf(".1.2.3[0][1]   : %s\t%s\n", ejson_valkTypeS(e, "1.2.3[0][1]"    ), ejson_toSk(e, "1.2.3[0][1]"   , &us, COMPACT));
    printf(".1.2.3[0][1].4 : %s\t%s\n", ejson_valkTypeS(e, "1.2.3[0][1].4"  ), ejson_toSk(e, "1.2.3[0][1].4" , &us, COMPACT));

    ejson_free(e);
    estr_free(us);
    printf("\n"); fflush(stdout);
}

void ejson_arr_get_test()
{
    cstr json_str = "["
                        "{\"2\": {\"3\": [[[\"val1\"], {\"4\":\"val2\"}]]}}"
                   "]";

    printf("-- ejson_arr_get_test --\n");
    ejson e = ejson_parseS(json_str);

    cstr us = ejson_toS(e, 0, PRETTY);
    printf("test json:\n%s\n", us);

    printf("Keys             Type\tValToUStr\n----------------------------------------\n");
    printf("[0]             : %s\t%s\n", ejson_valkTypeS(e, "[0]"             ), ejson_toSk(e, "[0]"             , &us, COMPACT));

    ejson_free(e);
    estr_free(us);
    printf("\n"); fflush(stdout);
}

void ejson_obj_get_test()
{
    ejson_obj_get_basic_test();
    ejson_obj_deep_get_test();
    ejson_arr_get_test();
}

int test_obj_get(int argc, char* argv[])
{
    E_UNUSED(argc); E_UNUSED(argv);

    ejson_obj_get_test();

    return ETEST_OK;
}
