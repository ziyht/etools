#include "test_main.h"

#if 1

void enatp_lazy_test()
{
    int64_t         last   = 0;
    int64_t         start __unused = 0;
    int64_t         last2   = 0;
    int64_t         start2  = 0;

    extern char* g_url;
    cstr urls = g_url ? g_url :
                            ",nats://0.0.0.0:4242"
                            ",nats://0.0.0.0:4243";

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
    int i, cnt;

    enatp p = enatp_new();

    if(!p)
    {
        fprintf(stderr, "trans_t creat err: %s\n", enatp_err(p));
        return;
    }

    fprintf(stderr, "trans_t add no lazy: %s\n", opts[0].conn_string);
    cnt = enatp_addOpts(p, "lazy", opts, 1, 1);

    if(cnt == 0)
    {
        fprintf(stderr, "trans_t add err: %s\n", enatp_err(p));
    }

    fprintf(stderr, "enatp connected to %s\n", enatp_connurls(p));

    if(enatp_cntTrans(p, LAZY_TRANS))
        fprintf(stderr, "enatp will auto connected to %s\n", enatp_lazyurls(p));
    else
        fprintf(stderr, "all connections ok~~!\n");

    enatp_setClosedCB      (p, "lazy", ClosedCB      , 0);
    enatp_setDisconnectedCB(p, "lazy", DisconnectedCB, 0);
    enatp_setReconnectedCB (p, "lazy", ReconnectedCB , 0);

    for(i = 0; i < 10000000; i++)
    {
        //sprintf(buf, "%d", i+1);
        //enatp_pubPoll(p, "natsTrans_pool_test", buf, strlen(buf));

        enatp_pubPoll(p, "test", "testdata", 8);

        if (nats_Now() - last >= 1000)
        {
            // nTPool_PollTPub(p, "natsTrans_pool_test", i * 100, "1 second", 8);

            //fprintf(stderr, "published:%d\n", i);

            start2 = nats_Now();
            constr str = enatp_statsS(p, 0/*"natsTrans_pool_test"*/, 1);
            last2 = last = nats_Now();
            fprintf(stderr, "published:%d\n%s\nlast:%ld\n\n", i, str, last2 - start2);

        }
    }

    //sleep(1);
    fprintf(stderr, "published:%d (over)\n%s\n", i, enatp_statsS(p, "test", 1));

    enatp_destroy(p);

    usleep(200000);
}

#endif
