#include "test_main.h"

#if 1

static void onMsg3(enats t __unused, natsSubscription *sub __unused, eMsg msg, void *closure)
{
    if (closure)
        fprintf(stderr, "%s: %s - %.*s\n",
               (char*)closure,
                msg->subject,
                msg->dataLen,
                msg->data);


    eMsg_free(msg);
}

void enatp_nrom_test()
{
    natsStatus s;

    int64_t         last   = 0;
    int64_t         start __unused  = 0;

    extern char* g_url;
    cstr urls = g_url ? g_url :
                            ",nats://0.0.0.0:4242"
                            ",nats://localhost:4243";

    enats_opts_t opts[] = {{urls,
                             "",    // auth
                             "",    // user
                             "",    // pass
                             {          // tls
                                 0,     // enable
                                 "",    // ca
                                 "",    // key
                                 ""     // cert
                             }}};

    start = nats_Now();
    char buf[100];
    int i;

    enatp p = enatp_new();
    enats t;
    if(!p)
    {
        fprintf(stderr, "trans_t creat err: %s\n", enatp_err(p));
        return;
    }

    s = enatp_addOpts(p, "norm", opts, 0, 0);

    if(s == 0)
        fprintf(stderr, "enatp_addOpts add err: %s\n", enatp_err(p));

    fprintf(stderr, "enatp connected to %s\n", enatp_connurls(p));

    enatp_sub(p, "nTrans2", "natsTrans_pool_test", onMsg3, 0/*"recive from [2]"*/);

    enatp_setClosedCB      (p, "norm", ClosedCB      , 0);
    enatp_setDisconnectedCB(p, "norm", DisconnectedCB, 0);
    enatp_setReconnectedCB (p, "norm", ReconnectedCB , 0);

    for(i = 0; i < 100000; i++)
    {
        sprintf(buf, "%d", i+1);

        enatp_pubPoll(p, "natsTrans_pool_test", buf, strlen(buf));
        //nTrans_Pub(t, "natsTrans_pool_test", buf, strlen(buf));
        if (nats_Now() - last >= 1000)
        {
            // nTPool_PollTPub(p, "natsTrans_pool_test", i * 100, "1 second", 8);

            //fprintf(stderr, "published:%d\n", i);
            fprintf(stderr, "published:%d\n%s\n", i, enatp_statsS(p, "natsTrans_pool_test", 1));

            last = nats_Now();
        }
    }

    sleep(1);
    fprintf(stderr, "published:%d (over)\n%s\n", i, enatp_statsS(p, "test", 1));

    enatp_destroy(p);

    usleep(200000);
}

#endif
