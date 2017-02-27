/*
   transport_cnats - a easier way to handle cnats

   Author: ziyht

*/

#ifndef _TRANSPORT_CNATS_
#define _TRANSPORT_CNATS_

#include "nats.h"

// -- seting
#define USE_MEGPOOL 0

// -- pagentd
#define MSGPOOL_MAX_COUNT 1024

#include "etype.h"

typedef struct ntStatistics_s{
    // for conn
    uint64_t    inMsgs;
    uint64_t    outMsgs;
    uint64_t    inBytes;
    uint64_t    outBytes;
    uint64_t    reconnects;

    // for sub
    int         pendingMsgs;
    int         pendingBytes;
    int         maxPendingMsgs;
    int         maxPendingBytes;
    int64_t     deliveredMsgs;
    int64_t     droppedMsgs;
}ntStatistics_t, ntStatistics;

typedef struct nTrans_s* nTrans, ** nTrans_p;
typedef struct nTPool_s* nTPool, ** nTPool_p;

typedef struct nTrans_opts_s {
    char*    conn_string;
    char*    compression;
    char*    encryption;
    char*    username;
    char*    password;
    uint64_t timeout;
    int      polling;                       // polling the transport or not when pub msg
}nTrans_opts_t, * nTrans_opts;

/// -- callbacks type
typedef void (*nTrans_ConnectedCB)   (nTrans t, void* closure);
typedef void (*nTrans_ClosedCB)      (nTrans t, void* closure);
typedef void (*nTrans_DisconnectedCB)(nTrans t, void* closure);
typedef void (*nTrans_ReconnectedCB) (nTrans t, void* closure);
typedef void (*nTrans_ErrHandler)    (nTrans t, natsSubscription *subscription, natsStatus err, void* closure);

typedef void (*nTrans_MsgHandler)    (nTrans t, natsSubscription *sub, natsMsg *msg, void *closure);

/// -------------------- natsTrans API ---------------------
/// -- make a natsTrans handle connect to the urls
//           tag    user     pass         server       port
//  lg urls: nats://paratera:paratera.com@172.18.4.205:4242[,nats://...]
//           nats://paratera@172.18.4.205:4242[,nats://...]
//           nats://172.18.4.205:4242[,nats://...]
//  lg url : 172.18.4.205:4242
//           server       port
//  return NULL    if create faild or can not connect to the urls
//         handle  if create ok, make sure that connected to one of the urls
nTrans   nTrans_New(constr urls);
nTrans   nTrans_NewTo(constr user, constr pass, constr url);
nTrans   nTrans_NewTo2(constr user, constr pass, constr server, int port);
nTrans   nTrans_NewTo3(nTrans_opts opts);    // opts from pagentd.h

/// -- wait the pub thread, blocking
void     nTrans_JoinPubThread(nTrans trans);

/// -- Destroy the handle and release resources
void     nTrans_Destroy(nTrans_p _trans);

/// -- return the urls that have been connected/reserved by natsTrans
constr   nTrans_GetConnUrls(nTrans trans);
constr   nTrans_GetUrls(nTrans trans);

/// -- return pool infomation if the trans in a nTPool, or return NULL
constr   nTrans_GetName(nTrans trans); 
nTPool   nTrans_GetPool(nTrans trans);

/// -- set callbacks, those callback will be called when event happen
//  note: when conn closed, disconnected / reconnected, natsTrans will stop / start publisher auto,
//        and cnats will also try to reconnect the server when disconnected,
//        so do not need to stop / start publisher or do reconnect oprts in the callbacks
void     nTrans_SetClosedCB(nTrans trans, nTrans_ClosedCB cb, void* closure);
void     nTrans_SetDisconnectedCB(nTrans trans, nTrans_DisconnectedCB cb, void* closure);
void     nTrans_SetReconnectedCB(nTrans trans, nTrans_ReconnectedCB cb, void* closure);
void     nTrans_SetErrHandler(nTrans trans, nTrans_ErrHandler cb, void* closure);

/// -- publish msg, thread safe
//  return NAT_OK   if queue ok
//         !=NAT_OK if queue err, use natsTrans_LastErr() to get err info
natsStatus  nTrans_Pub(nTrans trans, constr subj, conptr data, int dataLen);
natsStatus  nTrans_PubReq(nTrans trans, constr subj, conptr data, int dataLen, constr reply);

/// -- request msg, thread safe
//  return NAT_OK   if queue ok
//         !=NAT_OK if queue err, use natsTrans_LastErr() to get err info
natsStatus  nTrans_Req(nTrans trans, constr subj, conptr data, int dataLen, constr reply, natsMsg**replyMsg, int64_t timeout);

/// -- create a new subscriber in natsTrans
//  The subject can have wildcards (partial:*, full:>).
//  Messages will be delivered to the associated natsTrans_MsgHandler
//  And use natsMsg_Destroy(msg) in natsTrans_MsgHandler to release resources (needed)
natsStatus  nTrans_Sub(nTrans trans, constr subj, nTrans_MsgHandler onMsg, void* closure);
natsStatus  nTrans_unSub(nTrans trans, constr subj);

/// -- get statistics of natsTrans
#define STATS_IN        0x1
#define STATS_OUT       0x2
#define STATS_COUNT     0x4
#define STATS_ALL       0x7
ntStatistics nTrans_GetStats(nTrans trans, constr subj);
constr       nTrans_GetStatsStr(nTrans trans, int mode, constr subj);

/// -- get the lats err info of natsTrans, trans can be NULL
constr       nTrans_LastErr(nTrans trans);

/// ---------------------------------------------------------
/// ---------------- natsTrans Pool API ---------------------


nTPool nTPool_New();
void   nTPool_Destroy(nTPool_p _p);

void   nTPool_Join(nTPool p);   // blocking until p is been destoried

/// -- add a connection in nTPool
//           tag    user     pass         server       port
//  lg urls: nats://paratera:paratera.com@172.18.4.205:4242[,nats://...]
//
//  note:
//    in normal mode, return NATS_OK only all urls have been connected
//    in lazy   mode, the unconnected urls are added to a lazy_loop which try to connect to server autolly every second, always return NATS_OK except for the last condition
//    in both   mode, if one url in urls is in nTPool already, return NATS_ERR
//
nTrans      nTPool_Add(nTPool p, constr name, constr urls);
natsStatus  nTPool_AddLazy(nTPool p, constr name, constr urls);
natsStatus  nTPool_AddOpts(nTPool p, nTrans_opts opts);
natsStatus  nTPool_AddOptsLazy(nTPool p, nTrans_opts opts);

int         nTPool_IsInLazyQueue(nTPool p, constr name);
nTrans      nTPool_Get(nTPool p, constr name);
nTrans      nTPool_Del(nTPool p, constr name);
void        nTPool_Release(nTPool p, constr name);

int         nTPool_CntTrans(nTPool p);
int         nTPool_CntPollTrans(nTPool p);
int         nTPool_CntLazyTrans(nTPool p);

constr      nTPool_GetConnUrls(nTPool p);
constr      nTPool_GetLazyUrls(nTPool p);
constr*     nTPool_GetNConnUrls(nTPool p, int* cnt);
constr*     nTPool_GetNLazyUrls(nTPool p, int* cnt);

#define CONN_TRANS 0
#define LAZY_TRANS 1
#define ALL_TRANS  2
void        nTPool_SetConnectedCB(nTPool p, int type, nTrans_ConnectedCB cb, void* closure);            // only effect on unconnected lazy trans
void        nTPool_SetClosedCB(nTPool p, int type, nTrans_ClosedCB cb, void* closure);
void        nTPool_SetDisconnectedCB(nTPool p, int type, nTrans_DisconnectedCB cb, void* closure);
void        nTPool_SetReconnectedCB(nTPool p, int type, nTrans_ReconnectedCB cb, void* closure);
void        nTPool_SetErrHandler(nTPool p, int type, nTrans_ErrHandler cb, void* closure);

void        nTPool_SetConnectedCBByName(nTPool p, constr name, nTrans_ConnectedCB cb, void* closure);   // only effect on unconnected lazy trans
void        nTPool_SetClosedCBByName(nTPool p, constr name, nTrans_ClosedCB cb, void* closure);
void        nTPool_SetDisconnectedCBByName(nTPool p, constr name, nTrans_DisconnectedCB cb, void* closure);
void        nTPool_SetReconnectedCBByName(nTPool p, constr name, nTrans_ReconnectedCB cb, void* closure);
void        nTPool_SetErrHandlerByName(nTPool p, constr name, nTrans_ErrHandler cb, void* closure);

/// -- publish msgs
// note:
//   in normal mode, the msg will be publishing via the first connected url
//   in poll   mode, the msg will be publishing via polling all the connected urls, a msg only publish once
//
natsStatus  nTPool_Pub    (nTPool p, constr subj, conptr data, int dataLen);
natsStatus  nTPool_PollPub(nTPool p, constr subj, conptr data, int dataLen);

natsStatus  nTPool_PubReq    (nTPool p, constr subj, conptr data, int dataLen, constr reply);
natsStatus  nTPool_PollPubReq(nTPool p, constr subj, conptr data, int dataLen, constr reply);

/// -- request msg
natsStatus  nTPool_Req    (nTPool p, constr subj, conptr data, int dataLen, constr reply, natsMsg**replyMsg, int64_t timeout);
natsStatus  nTPool_PollReq(nTPool p, constr subj, conptr data, int dataLen, constr reply, natsMsg**replyMsg, int64_t timeout);

/// -- subscrib msg in specific nTrans
/// 
#define _ALL_NTRANS_ (constr)-1         // use it as a name
natsStatus  nTPool_Sub(nTPool p, constr name, constr subj, nTrans_MsgHandler onMsg, void* closure);
natsStatus  nTPool_Unsub(nTPool p, constr name, constr subj);

/*
#define STATS_IN        1<<0
#define STATS_OUT       1<<1
#define STATS_COUNT     1<<2
#define STATS_ALL       1<<3 - 1
*/
#define STATS_SINGEL    1<<4    // default is off
#define STATS_SUM_OFF   1<<5    // default is on
ntStatistics nTPool_GetStats(nTPool p, constr subj);
constr       nTPool_GetStatsStr(nTPool p, int mode, constr subj);

int          nTPool_IsErr(nTPool p);
constr       nTPool_LastErr(nTPool p);

#endif
