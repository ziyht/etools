#include <stdio.h>
#include <stdlib.h>
#include "ecompat.h"
#include "test_main.h"

int main()
{
#if 1
    //epopen_test();
    epopen_test_eio_tgets();

#else

#endif
#ifdef _WIN32
    puts("press any key to quit");
    getchar();
#endif

    return 0;
}


void epopen_test_eio_tgets()
{
    char buf[64];

    eio io = epopen("echo \"132\"");

    sleep(1);

    while(1)
    {
        int ret = eio_tgets(io, buf, 64, 1000);

        switch (ret) {
            // ok
            case  1:
                      continue;

            // timeout
            case  0:
                      break;

            // eof
            case -1:  ret = -88;
                      break;
        }

        if(ret == -88)
            break;
    }
    epkillclose(io);

}
