#include "test_main.h"
#include "ecompat.h"

#include <string.h>

void elog_rtt_test()
{

    elog e; elog_opts_t opts;

    memset(&opts, 0, sizeof(elog_opts_t));

    opts.path = "./rtt.log";
    opts.buf.max_logs = 10;
    opts.buf.max_size = 200;
    opts.rtt.enable = 1;
    opts.rtt.period = 3;
    opts.rtt.files  = 7;
    opts.rtt.terms  = 7;

    e = elog_newOpts("rtt", &opts);

    for(int i = 0; i < 50; i++)
    {
        elog_dbg(e, "%d", i);
        elog_inf(e, "%d", i);
        elog_wrn(e, "%d", i);
        elog_err(e, "%d", i);
        sleep(1);
    }

    elog_free(e);

}

int test_rtt(int argc, char* argv[])
{
    elog_rtt_test();

    return ETEST_OK;
}
