#include <stdio.h>
#include "test_main.h"

#include "eatom.h"

int main()
{
#if 1

    cb_test();
    //ert_runtask_test();
    //popen_kill_test();
#else
    ert_performance_test();
#endif
#ifdef _WIN32
    puts("press any key to quit");
    getchar();
#endif

    return 0;
}
