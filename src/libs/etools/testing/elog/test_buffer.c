#include "test_main.h"
#include <stdio.h>
#include <string.h>

void elog_buffer_size_test()
{
    elog e; elog_opts_t opts; int ret;

    memset(&opts, 0, sizeof(opts));
    opts.id   = 20;
    opts.path = "./buffer.log";
    opts.buf.max_logs = 0;
    opts.buf.max_size = 200;

    e = elog_newOpts("buffer", &opts);

    ret = elog_inf(e, "-- elog buffer max size test --"); if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_dbg(e, "0");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_inf(e, "1");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_wrn(e, "2");if(ret == 1) {printf("io here\n"); fflush(stdout);}   // reach max_buf_sz, write
    ret = elog_dbg(e, "3");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_dbg(e, "4");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_inf(e, "5");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_dbg(e, "6");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_err(e, "7");if(ret == 1) {printf("io here\n"); fflush(stdout);}   // err, write
    ret = elog_dbg(e, "8");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_dbg(e, "9");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_inf(e, "10");if(ret == 1) {printf("io here\n"); fflush(stdout);}

    ret = elog_free(e); if(ret == 1) {printf("io here in free\n"); fflush(stdout);}      // close, write if have msgs
}

void elog_buffer_cnt_test()
{
    elog e; elog_opts_t opts; int ret;

    memset(&opts, 0, sizeof(opts));
    opts.path = "./buffer.log";
    opts.buf.max_logs = 5;
    opts.buf.max_size = 0;

    e = elog_newOpts("buffer", &opts);

    ret = elog_inf(e, "-- elog buffer max cnt test --"); if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_dbg(e, "0");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_inf(e, "1");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_wrn(e, "2");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_dbg(e, "3");if(ret == 1) {printf("io here\n"); fflush(stdout);}   // reach max_logs, write
    ret = elog_dbg(e, "4");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_inf(e, "5");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_dbg(e, "6");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_err(e, "7");if(ret == 1) {printf("io here\n"); fflush(stdout);}   // err, write
    ret = elog_dbg(e, "8");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_dbg(e, "9");if(ret == 1) {printf("io here\n"); fflush(stdout);}
    ret = elog_inf(e, "10");if(ret == 1) {printf("io here\n"); fflush(stdout);}

    ret = elog_free(e); if(ret == 1) {printf("io here in free\n"); fflush(stdout);}      // close, write if have msgs
}

void elog_buffer_test()
{
    elog_buffer_size_test();
    elog_buffer_cnt_test();
}

int test_buffer(int argc, char* argv[])
{
    elog_buffer_test();

    return ETEST_OK;
}
