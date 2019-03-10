#include "test_main.h"


void echan_close_test()
{
    echan chan = echan_new(ECHAN_LIST, 0);

    assert_true(!echan_closed(chan), chan, "Chan is closed");
    assert_true(echan_close(chan) == 1, chan, "Close failed");
    assert_true(echan_close(chan) == 0, chan, "Close succeeded");
    assert_true(echan_closed(chan), chan, "Chan is not closed");

    echan_free(chan);
    pass();
}

int test_echan_close(int argc, char* argv[])
{
    echan_close_test();

    return ETEST_OK;
}
