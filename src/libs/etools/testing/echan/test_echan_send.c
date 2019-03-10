#include "test_main.h"
#include "ethread.h"

void echan_send_buffered_test()
{
    echan chan = echan_new(ECHAN_LIST, 1);
    void* msg = "foo";

    assert_true(echan_size(chan) == 0, chan, "Queue is not empty");
    assert_true(echan_sendS(chan, msg) == 1, chan, "Send failed");
    assert_true(echan_size(chan) == 1, chan, "Queue is empty");

    echan_free(chan);
    pass();
}

void* receiver(void* chan)
{
    char* msg;
    msg = (cstr)echan_recvS(chan);
    echan_freeO((eobj)msg);
    return NULL;
}

void echan_send_unbuffered_test()
{
    echan chan = echan_new(ECHAN_LIST, 0);
    void* msg = "foo";

    thread_t th;
    thread_init(th, receiver, chan);

    assert_true(echan_size(chan) == 0, chan, "Chan size is not 0");
    assert_true(echan_sendS(chan, msg) == 1, chan, "Send failed");
    assert_true(echan_size(chan) == 0, chan, "Chan size is not 0");

    thread_join(th);
    echan_free(chan);
    pass();
}

void echan_send_test()
{
    echan_send_buffered_test();
    echan_send_unbuffered_test();
}

int test_echan_send(int argc, char* argv[])
{
    echan_send_test();

    return ETEST_OK;
}
