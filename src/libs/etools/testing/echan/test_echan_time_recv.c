#include <string.h>

#include "test_main.h"
#include "ethread.h"


static cstr msg = "foo";

static void* list_sender(void* chan)
{
    usleep(500000);
    echan_sendP(chan, msg);
    return NULL;
}

void echan_time_recv_buffered_list_test1()
{
    echan chan = echan_new(ECHAN_LIST, 1);

    ethread_t th;

    ethread_init(th, list_sender, chan);

    eobj received = echan_timeRecvObj(chan, 1000);

    assert_true(received !=0, chan, "Recv failed");
    assert_true(eobj_valP(received) == msg, chan, "Messages are not equal");
    assert_true(echan_size(chan) == 0, chan, "Queue is not empty");

    echan_freeO(received);

    ethread_join(th);
    echan_free(chan);
    pass();
}

void echan_time_recv_buffered_list_test2()
{
    echan chan = echan_new(ECHAN_LIST, 1);

    ethread_t th;

    ethread_init(th, list_sender, chan);

    // sender will send msg in 500ms, here wait 300ms, timeout
    eobj received = echan_timeRecvObj(chan, 300);

    assert_true(received == 0, chan, "Recv ok");
    assert_true(echan_size(chan) == 0, chan, "Queue is not empty");

    ethread_join(th);
    echan_free(chan);
    pass();
}

static void* sigs_sender(void* chan)
{
    usleep(500000);
    echan_sendSig(chan, 10);
    return NULL;
}

void echan_time_recv_buffered_sigs_test1()
{
    echan chan = echan_new(ECHAN_SIGS, 100);

    ethread_t th;

    ethread_init(th, sigs_sender, chan);

    int received = echan_timeRecvSig(chan, 10, 1000);

    assert_true(received !=0, chan, "Recv failed");
    assert_true(received == 10, chan, "Messages are not equal");
    assert_true(echan_size(chan) == 0, chan, "Queue is not empty");

    ethread_join(th);
    echan_free(chan);
    pass();
}

void echan_time_recv_buffered_sigs_test2()
{
    echan chan = echan_new(ECHAN_SIGS, 100);

    ethread_t th;

    ethread_init(th, sigs_sender, chan);

    int received = echan_timeRecvSig(chan, 20, 1000);

    assert_true(received ==0, chan, "Recv OK");
    assert_true(echan_size(chan) == 0, chan, "Queue is empty");

    ethread_join(th);
    echan_free(chan);
    pass();
}


void echan_time_recv_test()
{
    echan_time_recv_buffered_list_test1();
    echan_time_recv_buffered_list_test2();

    echan_time_recv_buffered_sigs_test1();
    echan_time_recv_buffered_sigs_test2();
}

int test_echan_time_recv(int argc, char* argv[])
{
    echan_time_recv_test();

    return ETEST_OK;
}
