#include "unit_test.h"
#include "transport_cnats.h"

#if test_transport_cnats
static int64_t     total   = 1000000;

static volatile int64_t count   = 0;
static volatile int64_t dropped = 0;
static int64_t          start   = 0;
static volatile int64_t elapsed = 0;
static bool             print   = false;

#define __unused __attribute__((unused))

static void
onMsg(nTrans t __unused, natsSubscription *sub __unused, natsMsg *msg, void *closure __unused)
{
    if (print)
        printf("Received msg: %s - %.*s\n",
               natsMsg_GetSubject(msg),
               natsMsg_GetDataLength(msg),
               natsMsg_GetData(msg));

    if (start == 0)
        start = nats_Now();

    // We should be using a mutex to protect those variables since
    // they are used from the subscription's delivery and the main
    // threads. For demo purposes, this is fine.
    if (++count == total)
        elapsed = nats_Now() - start;

    natsMsg_Destroy(msg);
}

static void
onMsg2(nTrans t __unused, natsSubscription *sub __unused, natsMsg *msg, void *closure)
{
//    if (print)
        fprintf(stderr, "Received msg %s: %s - %.*s\n",
               (char*)closure,
               natsMsg_GetSubject(msg),
               natsMsg_GetDataLength(msg),
               natsMsg_GetData(msg));

    if (start == 0)
        start = nats_Now();

    // We should be using a mutex to protect those variables since
    // they are used from the subscription's delivery and the main
    // threads. For demo purposes, this is fine.
    if (++count == total)
        elapsed = nats_Now() - start;

    natsMsg_Destroy(msg);
}

void ClosedCB      (nTrans t, void* closure __unused)
{
    constr name = nTrans_GetName(t);
    fprintf(stderr, "%s%sconnection closed\n", name ? name : "", name ? ":" : "");
}

void DisconnectedCB(nTrans t, void* closure __unused)
{
    constr name = nTrans_GetName(t);
    fprintf(stderr, "%s%sconnection disconnected\n", name ? name : "", name ? ":" : "");
}

void ReconnectedCB (nTrans t, void* closure __unused)
{
    constr name = nTrans_GetName(t);
    fprintf(stderr, "%s%sconnection reconnected\n", name ? name : "", name ? ":" : "");
}

#define _natsTrans_postPubSem(t)        uv_sem_post(&(t)->pub.sem)
#define _natsTrans_setPubSemToMsg(t)    {for(uint i = 0; i < (t)->pub.msg_pool.count;i++) _natsTrans_postPubSem(t);}
#define _natsTrans_setPubSemToZero(t)   {while((t)->pub.sem.__align > 0) {fprintf(stderr,"%d\n", (t)->pub.sem.__align);uv_sem_trywait(&(t)->pub.sem);}}


void natsTrans_pub_test()
{
    nTrans_opts_t opts[] __unused = {{"172.18.4.205:4242", 0, 0, "paratera", "paratera.com", 0}};
    nTPool p = nTPool_New();

    //natsTrans_p trans = natsTrans_NewTo2("paratera", "paratera.com","172.18.4.205", 4242);
    //nTrans trans2 = nTrans_NewTo3(opts);
    // nTrans trans2 = nTrans_New("nats://172.18.4.205:4242");
    nTrans trans2 = nTPool_Add(p, "trans1", "nats://172.18.4.205:4242");
    
    
    int64_t         last   = 0;
    int64_t         start  __unused = 0;
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
       fprintf(stderr, "trans_t creat ok: connected to %s\n", nTrans_GetConnUrls(trans2));
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
}

void natsTrans_multi_conn_test()
{
    nTrans trans = nTrans_New("nats://172.18.4.205:4242"
                              //",nats://172.18.1.181:4242"
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

static void onMsg3(nTrans t __unused, natsSubscription *sub __unused, natsMsg *msg, void *closure)
{
    if (closure)
        fprintf(stderr, "%s: %s - %.*s\n",
               (char*)closure,
               natsMsg_GetSubject(msg),
               natsMsg_GetDataLength(msg),
               natsMsg_GetData(msg));

    natsMsg_Destroy(msg);
}
void natsTrans_pool_test()
{
    int64_t         last   = 0;
    int64_t         start __unused  = 0;
    
    nTrans_opts_t opts[] = {{""
                             "nats://172.18.4.205:4242,"
                             "nats://172.18.1.181:4242", 0, 0, "", "", 0}};
    
    start = nats_Now();
    char buf[100];
    int i;
    
    nTPool p = nTPool_New();
    nTrans t;
    if(!p)
    {
        fprintf(stderr, "trans_t creat err: %s\n", nTPool_LastErr(p));
        return;
    }
    
    
    t = nTPool_Add(p, "trans1", "nats://172.18.1.181:4242");
    nTPool_AddOpts(p, opts);
    
    if(!t)
        fprintf(stderr, "nTrans add err: %s\n", nTPool_LastErr(p));

    fprintf(stderr, "nTPool connected to %s\n", nTPool_GetConnUrls(p));


    
    nTPool_Sub(p, "trans2", "natsTrans_pool_test", onMsg3, 0/*"recive from [2]"*/);
    nTPool_SetClosedCBByName(p, "trans2", ClosedCB, 0);
    nTPool_SetDisconnectedCBByName(p, "trans2", DisconnectedCB, 0);
    nTPool_SetReconnectedCBByName(p, "trans2", ReconnectedCB, 0);
    
    t =  nTPool_Get(p, "trans1");
    
    //nTPool_NewSubscriber(p, "trans1", "natsTrans_pool_test", onMsg3, 0/*"recive from [1]"*/);
    nTPool_SetClosedCBByName(p, "trans1", ClosedCB, 0);
    nTPool_SetDisconnectedCBByName(p, "trans1", DisconnectedCB, 0);
    nTPool_SetReconnectedCBByName(p, "trans1", ReconnectedCB, 0);
    
     
    nTPool_SetClosedCBByName(p, "nTrans0", ClosedCB, 0);
    nTPool_SetDisconnectedCBByName(p, "nTrans0", DisconnectedCB, 0);
    nTPool_SetReconnectedCBByName(p, "nTrans0", ReconnectedCB, 0);
    nTPool_SetClosedCBByName(p, "nTrans1", ClosedCB, 0);
    nTPool_SetDisconnectedCBByName(p, "nTrans1", DisconnectedCB, 0);
    nTPool_SetReconnectedCBByName(p, "nTrans1", ReconnectedCB, 0);
    
    
    
    
    for(i = 0; i < 1000000; i++)
    {
        sprintf(buf, "%d", i+1);
        //usleep(100000);
        nTPool_PollPub(p, "natsTrans_pool_test", buf, strlen(buf));
        //nTrans_Pub(t, "natsTrans_pool_test", buf, strlen(buf));
        if (nats_Now() - last >= 1000)
        {
            // nTPool_PollTPub(p, "natsTrans_pool_test", i * 100, "1 second", 8);
            
            //fprintf(stderr, "published:%d\n", i);
            fprintf(stderr, "published:%d\n%s\n", i, nTPool_GetStatsStr(p, STATS_ALL|STATS_SINGEL, "natsTrans_pool_test"));

            last = nats_Now();
        }
    }
    
    sleep(1);
    fprintf(stderr, "published:%d (over)\n%s\n", i, nTPool_GetStatsStr(p, STATS_ALL|STATS_SINGEL, "test"));
    nTPool_Destroy(&p);
}

void natsTrans_pool_lazytest()
{
    int64_t         last   = 0;
    int64_t         start __unused = 0;
    int64_t         last2   = 0;
    int64_t         start2  = 0;
    
    extern char* g_url;
    cstr url = g_url ? g_url :
                            "nats://172.18.4.205:4242,"
                            "nats://172.18.1.181:4242";
    nTrans_opts_t opts[] = {{url, 0, 0, "", "", 0}};
    
    start = nats_Now();
    char buf[100];
    int i;
    
    nTPool p = nTPool_New();
    nTrans t __unused;
    
    if(!p)
    {
        fprintf(stderr, "trans_t creat err: %s\n", nTPool_LastErr(p));
        return;
    }
    
    fprintf(stderr, "trans_t add no lazy: %s\n", opts[0].conn_string);
    nTPool_AddOpts(p, opts);
    if(nTPool_IsErr(p))
    {
        fprintf(stderr, "trans_t add err: %s\n", nTPool_LastErr(p));
    }
    
    fprintf(stderr, "trans_t add lazy: %s\n", opts[0].conn_string);
    nTPool_AddOptsLazy(p, opts);
    if(nTPool_IsErr(p))
    {
        fprintf(stderr, "trans_t add err: %s\n", nTPool_LastErr(p));
    }
    fprintf(stderr, "nTPool connected to %s\n", nTPool_GetConnUrls(p));
    if(nTPool_CntLazyTrans(p))
        fprintf(stderr, "nTPool will auto connected to %s\n", nTPool_GetLazyUrls(p));
    else
        fprintf(stderr, "all connections ok~~!\n");
    
#if 0
    nTPool_SetClosedCBByName(p, "nTrans0", ClosedCB, 0);
    nTPool_SetDisconnectedCBByName(p, "nTrans0", DisconnectedCB, 0);
    nTPool_SetReconnectedCBByName(p, "nTrans0", ReconnectedCB, 0);
    nTPool_SetClosedCBByName(p, "nTrans1", ClosedCB, 0);
    nTPool_SetDisconnectedCBByName(p, "nTrans1", DisconnectedCB, 0);
    nTPool_SetReconnectedCBByName(p, "nTrans1", ReconnectedCB, 0);
#else
    nTPool_SetClosedCB(p, ALL_TRANS, ClosedCB, 0);
    nTPool_SetDisconnectedCB(p, ALL_TRANS, DisconnectedCB, 0);
    nTPool_SetReconnectedCB(p, ALL_TRANS, ReconnectedCB, 0);
#endif
    
    for(i = 0; i < 100000; i++)
    {
        sprintf(buf, "%d", i+1);
        //usleep(100000);
        nTPool_PollPub(p, "natsTrans_pool_test", buf, strlen(buf));
        //nTrans_Pub(t, "natsTrans_pool_test", buf, strlen(buf));
        if (nats_Now() - last >= 1000)
        {
            // nTPool_PollTPub(p, "natsTrans_pool_test", i * 100, "1 second", 8);
            
            //fprintf(stderr, "published:%d\n", i);
            
            start2 = nats_Now();
            constr str = nTPool_GetStatsStr(p, STATS_ALL|STATS_SINGEL, 0/*"natsTrans_pool_test"*/);
            last2 = last = nats_Now();
            fprintf(stderr, "published:%d\n%s\nlast:%ld\n\n", i, str, last2 - start2);
        }
    }
    
    sleep(1);
    fprintf(stderr, "published:%d (over)\n%s\n", i, nTPool_GetStatsStr(p, STATS_ALL|STATS_SINGEL, "test"));
    nTPool_Destroy(&p);
}

void server_max_collect_test()
{
    
    cstr url = "nats://172.18.4.255:4222";
    
    int i ;nTrans n;
    
    for (i = 0; (n = nTrans_New(url)) && i < 2000;)
    {
        i++;
        
        printf("%d\n", i);
        fflush(stdout);
    }

    sleep(1000);
}

void natsTrans_test()
{
    natsTrans_pub_test();
    // natsTrans_multi_conn_test();
    //natsTrans_pool_test();
    // natsTrans_pool_lazytest();
    
    
    
    //server_max_collect_test();
}
#else
void natsTrans_test(){}
#endif
