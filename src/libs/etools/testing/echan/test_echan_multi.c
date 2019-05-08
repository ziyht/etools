#include <string.h>

#include "test_main.h"
#include "ecompat.h"
#include "ethread.h"

static void* sender(void* chan)
{
    echan_sendP(chan, "foo");
    return NULL;
}
static void* receiver(void* chan)
{
    echan_recvP(chan);
    return NULL;
}

void test_chan_multi()
{
    echan chan = echan_new(ECHAN_LIST, 5);
    ethread_t th[100];
    for (int i = 0; i < 50; ++i)
    {
       ethread_init(th[i], sender, chan);
    }

    for (;;)
    {

       int all_waiting = echan_wwait(chan) == 45;
       if (all_waiting) break;
       //sched_yield();
       sleep(0);
    }

    for (int i = 50; i < 100; ++i)
    {
       ethread_init(th[i], receiver, chan);
    }

    for (int i = 0; i < 100; ++i)
    {
       ethread_join(th[i]);
    }

    echan_free(chan);
    pass();
}

void test_chan_multi2()
{
    echan chan = echan_new(ECHAN_LIST, 5);
    ethread_t th[100];
    for (int i = 0; i < 100; ++i)
    {
        ethread_init(th[i], receiver, chan);
    }

    for (;;)
    {

       int all_waiting = echan_rwait(chan) == 100;

       if (all_waiting) break;
       //sched_yield();
       sleep(0);
    }

    // Wake up other waiting reader.
    for (int i = 0; i < 100; ++i)
    {
        echan_sendP(chan, "foo");
    }

    for (int i = 0; i < 100; ++i)
    {
        ethread_join(th[i]);
    }

    echan_free(chan);
    pass();
}

int test_echan_multi(int argc, char* argv[])
{
    test_chan_multi();
    test_chan_multi2();

    return ETEST_OK;
}
