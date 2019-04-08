#include "etest.h"

#include "evec.h"

evec v;

int evec_new_test()
{
    v = evec_newEx(1);



    return ETEST_OK;
}

int evec_add_test()
{
    v = evec_newEx(1);

    char data[] = "0123456789";

    for(int i = 0; i < 10; i++)
    {
        evec_addV(v, 10, *(eval*)&data[i]);
    }


    return ETEST_OK;
}



int test_basic(int argc, char* argv[])
{
    (void)argc; (void)argv;

    ETEST_RUN( evec_new_test() );
    ETEST_RUN( evec_add_test() );

    return ETEST_OK;
}
