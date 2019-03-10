#include "test_main.h"


void enats_verify_test()
{
    enats t;


    cstr urls =
                            //"nats://172.18.4.205:4242"      // can connect, have no verify setting
                            //",nats://172.18.1.181:4242"     // can not connect
                            ",nats://@0.0.0.0:4242";         // can connect, have verify setting

    enats_opts_t opts[] = {{urls,
                             "S3Cr3T0k3n!",    // auth
                             "ziyht",    // user
                             "abc",    // pass
                             {          // tls
                                 1,     // enable
                                 "../src/test/enats/tls/ca.pem",    // ca
                                 "../src/test/enats/tls/key.pem",    // key
                                 "../src/test/enats/tls/cert.pem"     // cert
                             }, 0}};

    t = enats_newOpts(&opts[0]);

    if(!t)
    {
        fprintf(stderr, "trans_t creat err: %s\n", enats_err(0));
        return;
    }

    fprintf(stderr, "trans connected to %s\n", enats_connurl(t, 0));

    enats_destroy(t);

    sleep(1);
}

void enatp_verify_test()
{
    natsStatus s;

    cstr urls =
                            //"nats://172.18.4.205:4242"      // can connect, have no verify setting
                            //",nats://172.18.1.181:4242"     // can not connect
                            ",nats://0.0.0.0:4242";         // can connect, have verify setting

    enats_opts_t opts[] = {{urls,
                             "S3Cr3T0k3n!",     // auth
                             "ziyht",           // user
                             "",                // pass
                             {                  // tls
                                 1,                                     // enable
                                 "../src/test/enats/tls/ca.pem",        // ca
                                 "../src/test/enats/tls/key.pem",       // key
                                 "../src/test/enats/tls/cert.pem"       // cert
                             }, 0}};

    enatp p = enatp_new();

    if(!p)
    {
        fprintf(stderr, "trans_t creat err: %s\n", enatp_err(p));
        return;
    }

    s = enatp_addOpts(p, "verify", &opts[0], ENATP_LAZY | ENATP_GROUP);

    if(s != NATS_OK)
    {
        fprintf(stderr, "trans_t add err: %s\n", enatp_err(p));
    }
    fprintf(stderr, "nTPool connected to %s\n"     , enatp_connurls(p, 0));
    fprintf(stderr, "nTPool connected to %s auto\n", enatp_lazyurls(p, 0));

    if(!enatp_cntTrans(p, ENATP_LAZY_TRANS))
        fprintf(stderr, "all connections ok~~!\n");

    enatp_destroy(p);

    sleep(2);
}

int test_verify_cert(int argc, char* argv[])
{
    enats_verify_test();
    enatp_verify_test();

    return ETEST_OK;
}

