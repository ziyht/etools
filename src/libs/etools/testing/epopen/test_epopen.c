#include "test_main.h"

#include "ert.h"

#include "ecompat.h"

eio io;

void epopen_worker(void* d)
{
    char buf[1024];

    io = epopen("echo \"ab\";"
                   "sleep 100;"
                   //"sleep 10;"
                   //"sleep 10;"
                   //"sleep 100;"
                   );


    while(fgets(buf, 1024, io->ofp) != NULL)
    {
        printf(buf); fflush(stdout);
    }
    epclose(io); io = 0;
}

void epopen_test()
{

    ert rt = ert_new(10);

    ert_run(rt, "popen", epopen_worker, 0, 0);

    sleep(1);

    epkill(io);

    ert_destroy(rt, ERT_WAITING_RUNNING_TASKS);

    sleep(1);

}

int test_epopen(int argc, char* argv[])
{
    epopen_test();

    return ETEST_OK;
}
