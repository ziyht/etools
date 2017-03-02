#include "test_main.h"

void nTrans_verify_test()
{
    nTrans t;


    cstr urls =
                            //"nats://172.18.4.205:4242"      // can connect, have no verify setting
                            //",nats://172.18.1.181:4242"     // can not connect
                            ",nats://0.0.0.0:4242";         // can connect, have verify setting

    nTrans_opts_t opts[] = {{urls,
                             "S3Cr3T0k3n!",    // auth
                             "",    // user
                             "",    // pass
                             {          // tls
                                 1,     // enable
                                 "./ca.pem",    // ca
                                 "./key.pem",    // key
                                 "./cert.pem"     // cert
                             }, 0, 0, 0, 0, 0}};

    t = nTrans_NewTo3(&opts[0]);

    if(!t)
    {
        fprintf(stderr, "trans_t creat err: %s\n", nTrans_LastErr(0));
        return;
    }

    fprintf(stderr, "trans connected to %s\n", nTrans_GetConnUrls(t));

    nTrans_Destroy(&t);

    sleep(1);
}

void nTPool_verify_test()
{
    natsStatus s;

    cstr urls =
                            //"nats://172.18.4.205:4242"      // can connect, have no verify setting
                            //",nats://172.18.1.181:4242"     // can not connect
                            ",nats://0.0.0.0:4242";         // can connect, have verify setting

    nTrans_opts_t opts[] = {{urls,
                             "S3Cr3T0k3n!",    // auth
                             "",    // user
                             "",    // pass
                             {          // tls
                                 1,     // enable
                                 "./ca.pem",    // ca
                                 "./key.pem",    // key
                                 "./cert.pem"     // cert
                             }, 0, 0, 0, 0, 0}};

    nTPool p = nTPool_New();

    if(!p)
    {
        fprintf(stderr, "trans_t creat err: %s\n", nTPool_LastErr(p));
        return;
    }

    s = nTPool_AddOptsLazy(p, &opts[0]);

    if(s != NATS_OK)
    {
        fprintf(stderr, "trans_t add err: %s\n", nTPool_LastErr(p));
    }
    fprintf(stderr, "nTPool connected to %s\n", nTPool_GetConnUrls(p));

    if(nTPool_CntLazyTrans(p))
        fprintf(stderr, "nTPool will auto connected to %s\n", nTPool_GetLazyUrls(p));
    else
        fprintf(stderr, "all connections ok~~!\n");

    nTPool_Destroy(&p);

    sleep(2);
}
