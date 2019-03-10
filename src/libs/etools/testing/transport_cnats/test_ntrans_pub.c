#include "test_main.h"

void nTrans_pub_test()
{
    //nTrans_opts_t opts[] __unused = {{"172.18.4.205:4242", 0, 0, "paratera", "paratera.com", 0}};
    nTPool p = nTPool_New();

    //natsTrans_p trans = natsTrans_NewTo2("paratera", "paratera.com","172.18.4.205", 4242);
    //nTrans trans2 = nTrans_NewTo3(opts);
    // nTrans trans2 = nTrans_New("nats://172.18.4.205:4242");
    nTrans trans2 = nTPool_Add(p, "trans1", "nats://localhost:4242");


    int64_t         last   = 0;
//    int64_t         start  = 0;
//    if(!trans)
//    {
//        fprintf(stderr, "trans_t creat err: %s\n", natsTrans_LastErr(trans));
//        return;
//    }
    if(!trans2)
    {
        fprintf(stderr, "trans_t creat err: %s\n", nTrans_LastErr(trans2));
        return;
    }else
    {
       fprintf(stderr, "trans_t creat ok: connected to %s\n", nTrans_GetConnUrls(trans2, 1));
    }

    nTrans_SetClosedCB(trans2, ClosedCB, 0);
    nTrans_SetDisconnectedCB(trans2, DisconnectedCB, 0);

    //nTrans_Sub(trans2, "test", onMsg, "test onMsg");
    //nTrans_Sub(trans2, "report", onMsg2, "report onMsg2");

    start = nats_Now();

    for (int count = 0;  (count < 10000000); count++)
    {
        if(nTrans_Pub(trans2, "test", "testdata", 8))
        {    /*fprintf(stderr, "%d: %s\n", count, natsTrans_LastErr(trans2))*/;}
        //usleep(100000);
        if (nats_Now() - last >= 1000)
        {
            nTrans_Pub(trans2, "report", "1 second", 8);
            fprintf(stderr, "published:%d\n%s\n", count, nTrans_GetStatsStr(trans2, STATS_ALL, "test"));

            last = nats_Now();
        }
//        if(nats_Now() - start > 8000)
//        {
//            if((trans2)->pub.msg_pool.count < 1024)
//                _natsTrans_setPubSemToMsg(trans2);
//            uv_sem_post(&(trans2)->pub.sem);
//        }
//        else if(nats_Now() - start > 3000)
//        {
//            int i = (int)(trans2)->pub.sem.__align;
//            fprintf(stderr,"%d %d\n", (int)(trans2)->pub.sem.__align, i);
//            while((int)(trans2)->pub.sem.__align > 0)
//            {
//                fprintf(stderr,"%d %d\n", ((int*)&(trans2)->pub.sem)[0], ((int*)&(trans2)->pub.sem)[1]);
//                uv_sem_trywait(&(trans2)->pub.sem);
//            }
//        }
//        else
//        {
//               _natsTrans_setPubSemToMsg(trans2);
//        }
    }

    sleep(1);
    fprintf(stderr, "----\n");
    fprintf(stderr, "%s\n", nTrans_GetStatsStr(trans2, STATS_ALL, "test"));

    //natsTrans_Destroy(&trans);
    nTrans_Destroy(&trans2);

    sleep(2);
}
