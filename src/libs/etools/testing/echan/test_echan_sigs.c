#include "test_main.h"
#include "ethread.h"

static void* receiver1(void* chan)
{
    echan_recvSig(chan, 20);

    return NULL;
}

void echan_unbuffered_sigs_test()
{
    echan chan = echan_new(ECHAN_SIGS, 0);

    thread_t th;
    thread_init(th, receiver1, chan);

    assert_true(echan_size(chan) == 0, chan, "Chan size is not 0");
    assert_true(echan_sendSig(chan, 20) == 1, chan, "Send failed");
    assert_true(echan_size(chan) == 0, chan, "Chan size is not 0");

    thread_join(th);
    echan_free(chan);
    pass();
}

static void* receiver2(void* chan)
{
    for(int i = 0; i < 20; i++)
        echan_recvSig(chan, 5);

    return NULL;
}

void echan_buffered_sigs_test()
{
    echan chan = echan_new(ECHAN_SIGS, 100);

    thread_t th;
    thread_init(th, receiver2, chan);

    assert_true(echan_size(chan) == 0, chan, "Chan size is not 0");
    assert_true(echan_sendSig(chan, 100) == 1, chan, "Send failed");
    assert_true(echan_size(chan) == 0, chan, "Chan size is not 0");

    thread_join(th);
    echan_free(chan);
    pass();
}

void echan_sigs_test()
{
    echan_unbuffered_sigs_test();
    echan_buffered_sigs_test();

}

int test_echan_sigs(int argc, char* argv[])
{
    echan_sigs_test();

    return ETEST_OK;
}
