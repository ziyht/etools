#include "test_main.h"

void test_echan_new_buffered()
{
    size_t size = 5;
    echan chan = echan_new(ECHAN_LIST, size);

    echan_free(chan);

    pass();
}

void test_echan_init_unbuffered()
{
    echan chan = echan_new(ECHAN_LIST, 0);
    echan_free(chan);
    pass();
}

void echan_new_test()
{
    test_echan_new_buffered();
    test_echan_init_unbuffered();
}

int test_echan_init(int argc, char* argv[])
{
    echan_new_test();

    return ETEST_OK;
}
