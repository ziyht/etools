#include "test_main.h"


void nTpool_lazy_test()
{
    int64_t         last   = 0;
    int64_t         start __unused = 0;
    int64_t         last2   = 0;
    int64_t         start2  = 0;

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
    nTrans t __unused;

    if(!p)
    {
        fprintf(stderr, "trans_t creat err: %s\n", nTPool_LastErr(p));
        return;
    }

    fprintf(stderr, "trans_t add no lazy: %s\n", opts[0].conn_string);
    nTPool_AddOpts(p, opts);
    if(nTPool_IsErr(p))
    {
        fprintf(stderr, "trans_t add err: %s\n", nTPool_LastErr(p));
    }

    fprintf(stderr, "trans_t add lazy: %s\n", opts[0].conn_string);
    nTPool_AddOptsLazy(p, opts);
    if(nTPool_IsErr(p))
    {
        fprintf(stderr, "trans_t add err: %s\n", nTPool_LastErr(p));
    }
    fprintf(stderr, "nTPool connected to %s\n", nTPool_GetConnUrls(p));
    if(nTPool_CntLazyTrans(p))
        fprintf(stderr, "nTPool will auto connected to %s\n", nTPool_GetLazyUrls(p));
    else
        fprintf(stderr, "all connections ok~~!\n");

#if 0
    nTPool_SetClosedCBByName(p, "nTrans0", ClosedCB, 0);
    nTPool_SetDisconnectedCBByName(p, "nTrans0", DisconnectedCB, 0);
    nTPool_SetReconnectedCBByName(p, "nTrans0", ReconnectedCB, 0);
    nTPool_SetClosedCBByName(p, "nTrans1", ClosedCB, 0);
    nTPool_SetDisconnectedCBByName(p, "nTrans1", DisconnectedCB, 0);
    nTPool_SetReconnectedCBByName(p, "nTrans1", ReconnectedCB, 0);
#else
    nTPool_SetClosedCB(p, ALL_TRANS, ClosedCB, 0);
    nTPool_SetDisconnectedCB(p, ALL_TRANS, DisconnectedCB, 0);
    nTPool_SetReconnectedCB(p, ALL_TRANS, ReconnectedCB, 0);
#endif

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

            start2 = nats_Now();
            constr str = nTPool_GetStatsStr(p, STATS_ALL|STATS_SINGEL, 0/*"natsTrans_pool_test"*/);
            last2 = last = nats_Now();
            fprintf(stderr, "published:%d\n%s\nlast:%ld\n\n", i, str, last2 - start2);
        }
    }

    sleep(1);
    fprintf(stderr, "published:%d (over)\n%s\n", i, nTPool_GetStatsStr(p, STATS_ALL|STATS_SINGEL, "test"));

    nTPool_Destroy(&p);

    sleep(2);
}
