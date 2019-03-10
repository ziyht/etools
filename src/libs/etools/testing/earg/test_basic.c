#include "test_main.h"
#include "estr.h"

void earg_basic_test(int argc, char* argv[])
{
    earg eArg = earg_new("earg test    version: 1.0.0");

    earg_addArgS(eArg, "c:", "conf"    , "../ab.conf", "conf file path");
    earg_addArgI(eArg, "i:", "interval", 1000        , "set interval");

    earg_addArgI(eArg, "d" , "d_name"  , 0           , "d_desc");


    constr s = earg_info(eArg);

    puts(s); fflush(stdout);

    earg_parse(eArg, argc, argv);

    arg arg_c = earg_find(eArg, 'c');
    arg arg_i = earg_find(eArg, 'i');
    arg arg_d = earg_find(eArg, 'd');


    earg_free(eArg);
}


int test_basic(int argc, char* argv[])
{
    esplt args = 0;

    argc = esplt_splitCmdl(args, "-c ../conf/test.json -i 1200 -d abcd");

    earg_basic_test(argc, args);

    esplt_free(args);

    return ETEST_OK;
}
