#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "test_main.h"


int test_second = 0;
char* g_url;


void nTrans_test()
{
    // nTrans_pub_test();
    // nTrans_multi_conn_test();
    // nTpool_nrom_test();
    natsTrans_pool_lazytest();



    // server_max_collect_test();
}

int main(int argc, char* argv[])
{
    if(argc > 1 )
    {
        if(argv[1][0] >= '0' && argv[1][0] <= '9')
            sscanf(argv[1], "%d", &test_second);
        if(0 == strncmp(argv[1], "-url:", 5))
            g_url = argv[1] + 5;
    }

    signal(SIGPIPE, SIG_IGN);

    nTrans_test();

    return 0;
}
