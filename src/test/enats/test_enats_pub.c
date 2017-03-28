#include "test_main.h"
#include "unistd.h"

void enats_pub_test()
{
    enats trans = enats_new("nats://localhost:4242");

    int64_t         last   = 0;
    if(!trans)
    {
        fprintf(stderr, "trans_t creat err: %s\n", enats_err(trans));   fflush(stderr);
        return;
    }else
    {
       fprintf(stdout, "trans_t creat ok: connected to %s\n", enats_connurl(trans)); fflush(stdout);
    }

    enats_setClosedCB      (trans, ClosedCB      , 0);
    enats_setDisconnectedCB(trans, DisconnectedCB, 0);

    start = nats_Now();

    for (int count = 0;  (count < 10000000); count++)
    {
        if(enats_pub(trans, "test", "testdata", 8))
        {    fprintf(stderr, "%d: %s\n", count, enats_err(trans));}

        if (nats_Now() - last >= 1000)
        {
            enats_pub(trans, "report", "1 second", 8);
            fprintf(stderr, "published:%d\n%s\n", count, enats_statsS(trans, "test"));

            last = nats_Now();
        }
    }

    sleep(1);
    fprintf(stderr, "----\n");
    fprintf(stderr, "%s\n", enats_statsS(trans, "test"));

    enats_destroy(trans);

    usleep(200000);
}
