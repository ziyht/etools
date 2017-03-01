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
    // nTpool_lazy_test();

    // server_max_collect_test();

    // nTrans_verify_test();
    nTPool_verify_test();
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





void ClosedCB      (nTrans t, void* closure __unused)
{
    printf("%s:%sconnection closed\n", nTrans_GetName(t), nTrans_GetUrls(t));
    fflush(stdout);
}

void DisconnectedCB(nTrans t, void* closure __unused)
{
    printf("%s:%sconnection disconnected\n", nTrans_GetName(t), nTrans_GetUrls(t));
    fflush(stdout);
}

void ReconnectedCB (nTrans t, void* closure __unused)
{
    printf("%s:%sconnection reconnected\n", nTrans_GetName(t), nTrans_GetUrls(t));
    fflush(stdout);
}
