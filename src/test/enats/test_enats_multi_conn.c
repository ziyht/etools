#include "test_main.h"

void enats_multi_conn_test()
{
    enats trans = enats_new(",nats://0.0.0.0:4242"
                            ",nats://localhost:4243"
                              );
    if(!trans)
    {
        fprintf(stderr, "trans_t creat err: %s\n", enats_err(trans));
        return;
    }else
    {
       fprintf(stderr, "trans_t creat ok: connected to %s\n", enats_connurl(trans));
    }

    enats_setClosedCB(trans, ClosedCB, 0);
    enats_setDisconnectedCB(trans, DisconnectedCB, 0);

    enats_destroy(trans);

    sleep(1);
}

