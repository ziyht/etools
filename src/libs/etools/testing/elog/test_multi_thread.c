#include "test_main.h"
#include "ethread.h"

#include <string.h>

ethread_t th1, th2, th3;

elog elogh;

void* _cb(void* d)
{
    while(1)
    {
        for(int i = 0; i< 100; i++)
        {
            if(!elog_dbg(elogh, "%ld: dbg", (i64)d)) goto quit;
            if(!elog_inf(elogh, "%ld: inf", (i64)d)) goto quit;
            if(!elog_wrn(elogh, "%ld: wrn", (i64)d)) goto quit;
            if(!elog_err(elogh, "%ld: err", (i64)d)) goto quit;
        }
        sleep(1);
    }

quit:
    return 0;
}

void elog_multi_thread_test()
{
    elog_opts_t opt;
    memset(&opt, 0, sizeof(opt));
    opt.buf.max_logs = 20;
    opt.buf.max_size = 4096;

    elogh = elog_newOpts("multi", &opt);

    ethread_init(th1, _cb, (cptr)1);
    ethread_init(th2, _cb, (cptr)2);
    ethread_init(th3, _cb, (cptr)3);

    sleep(10);

    elog_free(elogh);
    ethread_join(th1);
    ethread_join(th2);
    ethread_join(th3);

}

int test_multi_thread(int argc, char* argv[])
{
    elog_multi_thread_test();

    return ETEST_OK;
}
