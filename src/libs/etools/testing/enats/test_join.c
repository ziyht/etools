#include "test_main.h"


#define ECOMPAT_THREAD_MAIN
#include "ecompat.h"
#include "elog.h"
#include "ethread.h"

static elog e;

static enats t;
static enatp p;

void* wait1_cb(void* d)
{
    enats_join(t);
    enatp_join(p);

    elog_inf(e, "wait 1 quit");

    return 0;
}

void* wait2_cb(void* d)
{
    enats_join(t);
    enatp_join(p);

    elog_inf(e, "wait 2 quit");

    return 0;
}

void* quit_cb(void* d)
{
    sleep(1);

    elog_inf(e, "exe destroy");

    enats_destroy(t); t = 0;
    enatp_destroy(p); p = 0;

    return 0;
}

void enats_join_test()
{
    t = enats_newUrl1("nats://localhost:4242");

    e = elog_new("enats", "./ab.log");

    if(!t)
    {
        fprintf(stderr, "trans_t creat err: %s\n", enats_err(t));   fflush(stderr);
        return;
    }else
    {
       fprintf(stdout, "trans_t creat ok: connected to %s\n", enats_connurl(t, 0)); fflush(stdout);
    }

    thread_t wait1, wait2, quit;
    thread_init(wait1, wait1_cb, 0);
    thread_init(wait2, wait2_cb, 0);
    thread_init(quit , quit_cb , 0);
    thread_detach(wait1);
    thread_detach(wait2);
    thread_detach(quit);

    enats_join(t);
    elog_inf(e, "wait main quit");

    usleep(1000000);

    elog_free(e);
}

void enatp_join_test()
{
    p = enatp_new();

    e = elog_new("enatp", "./ab.log");

    enatp_addUrls(p, "join_test", "nats://localhost:4242", ENATP_LAZY);

    thread_t wait1, wait2, quit;
    thread_init(wait1, wait1_cb, 0);
    thread_init(wait2, wait2_cb, 0);
    thread_init(quit , quit_cb , 0);
    thread_detach(wait1);
    thread_detach(wait2);
    thread_detach(quit);

    enatp_join(p);
    elog_wrn(e, "wait main quit");

    usleep(1000000);

    elog_free(e);
}

int test_join(int argc, char* argv[])
{
    enats_join_test();
    enatp_join_test();

    return ETEST_OK;
}
