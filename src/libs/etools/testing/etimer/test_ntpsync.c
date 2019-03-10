#include "test_main.h"

#include <stdio.h>
#define ECOMPAT_UNISTD
#include "ecompat.h"

void etimer_ntpSync_test()
{
    while(1)
    {

        esync stat = etimer_esyncGet("172.18.4.205", 1000);

        if(stat.status)
            printf("offset: %"PRIu64"s %6"PRIu64"us\n", stat.offusec / 1000000, stat.offusec % 1000000);
        else
            printf("sync faild: %s\n", stat.err);

        fflush(stdout);

        sleep(1);

    }
}

int test_ntpsync(int argc, char* argv[])
{
    etimer_ntpSync_test();

    return ETEST_OK;
}
