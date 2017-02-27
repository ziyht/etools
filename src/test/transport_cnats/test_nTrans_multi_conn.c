#include "test_main.h"

void nTrans_multi_conn_test()
{
    nTrans trans = nTrans_New("nats://172.18.4.205:4242"
                              ",nats://172.18.1.181:4242"
                              "");
    if(!trans)
    {
        fprintf(stderr, "trans_t creat err: %s\n", nTrans_LastErr(trans));
        return;
    }else
    {
       fprintf(stderr, "trans_t creat ok: connected to %s\n", nTrans_GetConnUrls(trans));
    }

    nTrans_Destroy(&trans);
}
