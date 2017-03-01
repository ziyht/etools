#include "test_main.h"

static void onMsg3(nTrans t __unused, natsSubscription *sub __unused, natsMsg *msg, void *closure)
{
    if (closure)
        fprintf(stderr, "%s: %s - %.*s\n",
               (char*)closure,
               natsMsg_GetSubject(msg),
               natsMsg_GetDataLength(msg),
               natsMsg_GetData(msg));

    natsMsg_Destroy(msg);
}

void nTpool_nrom_test()
{
    natsStatus s;

    int64_t         last   = 0;
    int64_t         start __unused  = 0;

    extern char* g_url;
    cstr urls = g_url ? g_url :
                            "nats://172.18.4.205:4242"
                            ",nats://172.18.1.181:4242"
                            ",nats://0.0.0.0:4242";

    nTrans_opts_t opts[] = {{urls,
                             "",    // auth
                             "",    // user
                             "",    // pass
                             {          // tls
                                 0,     // enable
                                 "",    // ca
                                 "",    // key
                                 ""     // cert
                             }, 0, 0, 0, 0, 0}};

    start = nats_Now();
    char buf[100];
    int i;

    nTPool p = nTPool_New();
    nTrans t;
    if(!p)
    {
        fprintf(stderr, "trans_t creat err: %s\n", nTPool_LastErr(p));
        return;
    }

    s = nTPool_AddOpts(p, opts);

    if(s != NATS_OK)
        fprintf(stderr, "nTPool_AddOpts add err: %s\n", nTPool_LastErr(p));

    fprintf(stderr, "nTPool connected to %s\n", nTPool_GetConnUrls(p));

    nTPool_Sub(p, "nTrans2", "natsTrans_pool_test", onMsg3, 0/*"recive from [2]"*/);

    nTPool_SetClosedCBByName(p, "nTrans0", ClosedCB, 0);
    nTPool_SetDisconnectedCBByName(p, "nTrans0", DisconnectedCB, 0);
    nTPool_SetReconnectedCBByName(p, "nTrans0", ReconnectedCB, 0);
    nTPool_SetClosedCBByName(p, "nTrans1", ClosedCB, 0);
    nTPool_SetDisconnectedCBByName(p, "nTrans1", DisconnectedCB, 0);
    nTPool_SetReconnectedCBByName(p, "nTrans1", ReconnectedCB, 0);
    nTPool_SetClosedCBByName(p, "nTrans2", ClosedCB, 0);
    nTPool_SetDisconnectedCBByName(p, "nTrans2", DisconnectedCB, 0);
    nTPool_SetReconnectedCBByName(p, "nTrans2", ReconnectedCB, 0);

    for(i = 0; i < 100000; i++)
    {
        sprintf(buf, "%d", i+1);
        //usleep(100000);
        nTPool_PollPub(p, "natsTrans_pool_test", buf, strlen(buf));
        //nTrans_Pub(t, "natsTrans_pool_test", buf, strlen(buf));
        if (nats_Now() - last >= 1000)
        {
            // nTPool_PollTPub(p, "natsTrans_pool_test", i * 100, "1 second", 8);

            //fprintf(stderr, "published:%d\n", i);
            fprintf(stderr, "published:%d\n%s\n", i, nTPool_GetStatsStr(p, STATS_ALL|STATS_SINGEL, "natsTrans_pool_test"));

            last = nats_Now();
        }
    }

    sleep(1);
    fprintf(stderr, "published:%d (over)\n%s\n", i, nTPool_GetStatsStr(p, STATS_ALL|STATS_SINGEL, "test"));

    nTPool_Destroy(&p);

    sleep(2);
    //getchar();
}
