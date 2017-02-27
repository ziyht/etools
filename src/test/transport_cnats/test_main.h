#ifndef __TRANSPORT_CNATS_TEST
#define __TRANSPORT_CNATS_TEST

#include <string.h>
#include <unistd.h>

#include "transport_cnats.h"

#define __unused __attribute__((unused))

static int64_t     total   = 1000000;

static volatile int64_t count   = 0;
static volatile int64_t dropped = 0;
static int64_t          start   = 0;
static volatile int64_t elapsed = 0;
static bool             print   = false;


void nTrans_pub_test();
void nTrans_multi_conn_test();

void nTpool_nrom_test();
void natsTrans_pool_lazytest();

void server_max_collect_test();











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

static void ClosedCB      (nTrans t, void* closure __unused)
{
    constr name = nTrans_GetName(t);
    fprintf(stderr, "%s%s%sconnection closed\n", name ? name : "", name ? ":" : "", nTrans_GetUrls(t));
}

static void DisconnectedCB(nTrans t, void* closure __unused)
{
    constr name = nTrans_GetName(t);
    fprintf(stderr, "%s%sconnection disconnected\n", name ? name : "", name ? ":" : "");
}

static void ReconnectedCB (nTrans t, void* closure __unused)
{
    constr name = nTrans_GetName(t);
    fprintf(stderr, "%s%sconnection reconnected\n", name ? name : "", name ? ":" : "");
}


#endif
