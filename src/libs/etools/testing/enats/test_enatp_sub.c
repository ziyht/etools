#include "test_main.h"


static void onMsg(enats e __unused, natsSubscription *sub __unused, eMsg msg, void *closure __unused)
{
   printf("[%s] Received msg: %s - %.*s\n", enats_name(e),
               msg->subject,
               msg->dataLen,
               msg->data);

   fflush(stdout);

    eMsg_free(msg);
}

void enatp_sub_test()
{
    char buf[100];

    enatp p = enatp_new();

    cstr name = NULL;

    int cnt =  enatp_addUrls(p, name, "nats://localhost:4242,nats://localhost:4243", ENATP_LAZY | ENATP_GROUP);

    enatp_setConnectedCB   (p, name, ConnectedCB   , 0);
    enatp_setClosedCB      (p, name, ClosedCB      , 0);
    enatp_setDisconnectedCB(p, name, DisconnectedCB, 0);
    enatp_setReconnectedCB (p, name, ReconnectedCB , 0);

    enatp_sub(p, name, "sub1", onMsg, 0);

    usleep(1000);

    for(int i = 0; i < 10; i++)
    {
        sprintf(buf, "data to sub1 %d", i + 1);

        enatp_pubPoll(p, "sub1", buf, strlen(buf));

        usleep(1000000);
    }

    usleep(1000);

    enatp_destroy(p);

    usleep(200000);
}

int test_enatp_sub(int argc, char* argv[])
{
    enatp_sub_test();

    return ETEST_OK;
}
