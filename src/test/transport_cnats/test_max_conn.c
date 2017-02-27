#include "test_main.h"

void server_max_collect_test()
{

    cstr url = "nats://172.18.4.205:4242";

    int i ;nTrans n;

    for (i = 0; (n = nTrans_New(url)) && i < 2000;)
    {
        i++;

        printf("%d\n", i);
        fflush(stdout);
    }

}
