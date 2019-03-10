
#include "test_main.h"

static void onMsg(enats e __unused, natsSubscription *sub __unused, eMsg msg, void *closure __unused)
{
   printf("[%s] Received msg: %s - %.*s\n", enats_name(e),
               msg->subject,
               msg->dataLen,
               msg->data);


    eMsg_free(msg);
}

void enats_sub_test()
{
    char buf[100];

    enats e = enats_newUrl1("nats://localhost:4242");


    if(!e)
    {
        printf(enats_err(0));
        exit(1);
    }

    enats_sub(e, "sub1", onMsg, 0);

    usleep(1000);

    for(int i = 0; i < 20; i++)
    {
        sprintf(buf, "data to sub1 %d", i + 1);

        enats_pub(e, "sub1", buf, strlen(buf));

        usleep(100);
    }

    usleep(1000);

    enats_destroy(e);

    usleep(200000);
}

int test_enats_sub(int argc, char* argv[])
{
    enats_sub_test();

    return ETEST_OK;
}
