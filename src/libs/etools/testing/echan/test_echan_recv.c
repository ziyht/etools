#include <string.h>

#include "test_main.h"
#include "ethread.h"

void echan_recv_buffered_test()
{
    echan chan = echan_new(ECHAN_LIST, 1);
    void* msg = "foo";

    assert_true(echan_size(chan) == 0, chan, "Queue is not empty");
    echan_sendP(chan, msg);
    assert_true(echan_size(chan) == 1, chan, "Queue is empty");

    void* received;
    assert_true((received = echan_recvP(chan))!=0, chan, "Recv failed");
    assert_true(msg == received, chan, "Messages are not equal");
    assert_true(echan_size(chan) == 0, chan, "Queue is not empty");

    echan_free(chan);
    pass();
}

static void* sender(void* chan)
{
    echan_sendS(chan, "foo");
    return NULL;
}

void echan_recv_unbuffered_test()
{
    echan chan = echan_new(ECHAN_LIST, 0);
    thread_t th;
    thread_init(th, sender, chan);

    assert_true(echan_size(chan) == 0, chan, "Chan size is not 0");

    void *msg;
    assert_true((msg = echan_recvS(chan)) != NULL, chan, "Recv failed");
    assert_true(strcmp(msg, "foo") == 0, chan, "Messages are not equal");
    assert_true(echan_size(chan) == 0, chan, "Chan size is not 0");

    thread_join(th);
    echan_freeO(msg);
    echan_free(chan);
    pass();
}

void echan_recv_test()
{
    echan_recv_buffered_test();
    echan_recv_unbuffered_test();
}

int test_echan_recv(int argc, char* argv[])
{
    echan_recv_test();

    return ETEST_OK;
}
