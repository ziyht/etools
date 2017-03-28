#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "test_main.h"


int test_second = 0;
char* g_url;

void ConnectedCB      (enats t, void* closure __unused)
{
    printf("[%s]%s connection connected\n", enats_name(t), enats_lasturl(t));
    fflush(stdout);
}

void ClosedCB      (enats t, void* closure __unused)
{
    printf("[%s]%s connection closed\n", enats_name(t), enats_lasturl(t));
    fflush(stdout);
}

void DisconnectedCB(enats t, void* closure __unused)
{
    printf("[%s]%s connection disconnected\n", enats_name(t), enats_lasturl(t));
    fflush(stdout);
}

void ReconnectedCB (enats t, void* closure __unused)
{
    printf("[%s]%s connection reconnected\n", enats_name(t), enats_connurl(t));
    fflush(stdout);
}

void enats_test()
{
    // enats_pub_test();
    //server_max_collect_test();

    //enats_multi_conn_test();

    //enatp_nrom_test();
    //enatp_lazy_test();

    //enats_sub_test();
    enatp_sub_test();

    //enatp_verify_test();
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

    enats_test();

    return 0;
}
