/// =====================================================================================
///
///       Filename:  enats.h
///
///    Description:  a easier way to handle cnats
///
///        Version:  1.0
///        Created:  02/28/2017 08:51:34 PM
///       Revision:  none
///       Compiler:  gcc
///         Needed:  cnats, ejson, estr
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#ifndef __ENATS__
#define __ENATS__

#include "nats.h"
#include "etype.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ntStatistics_s{
    // -- for conn
    uint64_t    inMsgs;
    uint64_t    outMsgs;
    uint64_t    inBytes;
    uint64_t    outBytes;
    uint64_t    reconnects;

    // -- for sub
    int         pendingMsgs;
    int         pendingBytes;
    int         maxPendingMsgs;
    int         maxPendingBytes;
    int64_t     deliveredMsgs;
    int64_t     droppedMsgs;
}ntStatistics_t, ntStatistics;

typedef struct enats_opts_s {
    char*    conn_string;
    char*    auth;
    char*    username;
    char*    password;
    struct {
        int   enanle;
        char* ca;
        char* key;
        char* cert;
    }tls;
    uint64_t timeout;

    char*    compression;
    char*    encryption;

    int      decode;
    int      polling;
}enats_opts_t, * enats_opts;

typedef struct eMsg_s
{
    void* reserved1;
    void* reserved2;

    const char          *subject;
    const char          *reply;
    const char          *data;
    int                 dataLen;
}eMsg_t, * eMsg;

static inline void eMsg_free(eMsg msg){ if(msg->reserved2) natsMsg_Destroy((natsMsg*)msg); else free(msg); }

typedef struct enats_s* enats, ** enats_p;
typedef struct enatp_s* enatp, ** enatp_p;

/// -- callbacks type
typedef void (*enats_evtHandler) (enats t, void* closure);
typedef void (*enats_errHandler) (enats t, natsSubscription *subscription, natsStatus err, void* closure);
typedef void (*enats_msgHandler) (enats t, natsSubscription *sub, eMsg msg, void *closure);

/// -------------------- enats API -------------------------------
/// -- make a natsTrans handle connect to the urls
///           tag    user     pass         server       port
///  lg urls: nats://paratera:paratera.com@172.18.4.205:4242[,nats://...]
///           nats://paratera@172.18.4.205:4242[,nats://...]
///           nats://172.18.4.205:4242[,nats://...]
///  lg url : 172.18.4.205:4242
///           server       port
///  return NULL    if create faild or can not connect to the urls
///         handle  if create ok, make sure that connected to one of the urls
enats enats_new (constr urls);
enats enats_new2(constr user, constr pass, constr url);
enats enats_new3(constr user, constr pass, constr server, int port);
enats enats_new4(enats_opts opts);

/// -- wait here, blocking, it will unlock when you destroy the enats
void  enats_join(enats e);

/// -- Destroy the handle and release resources
void  enats_destroy(enats e);

/// -- return the urls that have been connected/reserved by natsTrans
constr enats_connurls(enats e);
constr enats_urls(enats e);

/// -- return pool infomation if the enats in a enatp, else return NULL
constr enats_name(enats e);
enatp  enats_pool(enats e);

///
/// \brief -- set callbacks, those callback will be called when event happen
///
/// \param trans   : enats transport handle
/// \param cb      : the CB you want to set
/// \param closure : the pravite data of this cb
///
/// \note:
///     when conn closed, disconnected / reconnected, natsTrans will stop / start publisher auto,
///     and cnats will also try to reconnect the server when disconnected,
///     so do not need to stop / start publisher or do reconnect oprts in the callbacks
///
void enats_setClosedCB      (enats e, enats_evtHandler cb, void* closure);
void enats_setDisconnectedCB(enats e, enats_evtHandler cb, void* closure);
void enats_setReconnectedCB (enats e, enats_evtHandler cb, void* closure);
void enats_setErrHandler    (enats e, enats_errHandler cb, void* closure);

///
/// \brief -- publish msg through enats, thread safe
///
/// \param trans   : enats transport handle
/// \param subj    : the subject you want to publish to
/// \param data    : the data you want to publish
/// \param dataLen : the lengh of the publish data
/// \param reply   : the msg will set reply
/// \return - NAT_OK   if queue ok
///         - !=NAT_OK if queue err, use natsTrans_LastErr() to get err info
///
natsStatus  enats_pub (enats e, constr subj, conptr data, int dataLen);
natsStatus  enats_pubr(enats e, constr subj, conptr data, int dataLen, constr reply);

/// -- create a new subscriber in natsTrans
//
//  Messages will be delivered to the associated natsTrans_MsgHandler
//  And use natsMsg_Destroy(msg) in natsTrans_MsgHandler to release resources (needed)

///
/// \brief sub or unsub a subj
///
/// \param trans   : enats transport handle
/// \param subj    : the subject you want to subscribe
/// \param onMsg   : the cb to be called when a msg coming
/// \param closure : the private data of this subj
/// \return - NAT_OK   if operate faild
///         - !=NAT_OK if operate success
/// \note:
///     1. The subject can have wildcards (partial:*, full:>).
///     2. Messages will be delivered to the associated natsTrans_MsgHandler
///        and using eMsg_free() to free the msg after using it
///
natsStatus  enats_sub  (enats e, constr subj, enats_msgHandler onMsg, void* closure);
natsStatus  enats_unsub(enats e, constr subj);

/// -- request msg, thread safe
//  return
//         !=NAT_OK if queue err, use natsTrans_LastErr() to get err info

///
/// \brief enats_req -- request a msg through the trans, it will autolly subscribe the reply before publish the data
///                     and autolly unsubscribe it when recieve a msg
///
/// \param e        : enats transport handle
/// \param subj     : the subject you want to publish to
/// \param data     : the data you want to publish
/// \param dataLen  : the lengh of the publish data
/// \param reply    : the reply subject to auto subscribe and unsubscribe
/// \param replyMsg : the msg recieved
/// \param timeout  : timeout setting
/// \return - NAT_OK   if requst ok
///         - !=NAT_OK if requst faild
///
/// \note:
///     if timeout = 0, this API will wait forever unless recieve a msg
///
natsStatus  enats_req(enats e, constr subj, conptr data, int dataLen, constr reply, eMsg* replyMsg, int64_t timeout);

/// \brief -- get statistics of enats
///
/// \param trans: enats transport handle
/// \param subj : the specific subject you want to query
/// \return
///
ntStatistics enats_stats (enats e, constr subj);
constr       enats_statsS(enats e, int mode, constr subj);

/// -- get the lats err info of natsTrans, trans can be NULL
constr       enats_err(enats e);


/// ---------------------------------------------------------
/// ---------------- natsTrans Pool API ---------------------

enatp enatp_new();
void  enatp_destroy(enatp_p _p);

void  enatp_join(enatp p);   // blocking until p is been destoried

/// -- add a connection in enatp
//           tag    user     pass         server       port
//  lg urls: nats://paratera:paratera.com@172.18.4.205:4242[,nats://...]
//
//  note:
//    in normal mode, return NATS_OK only all urls have been connected
//    in lazy   mode, the unconnected urls are added to a lazy_loop which try to connect to server autolly every second, always return NATS_OK except for the last condition
//    in both   mode, if one url in urls is in enatp already, return NATS_ERR
//
enats       enatp_Add(enatp p, constr name, constr urls);
natsStatus  enatp_AddLazy(enatp p, constr name, constr urls);
natsStatus  enatp_AddOpts(enatp p, enats_opts opts);
natsStatus  enatp_AddOptsLazy(enatp p, enats_opts opts);

int         enatp_IsInLazyQueue(enatp p, constr name);
enats      enatp_Get(enatp p, constr name);
enats      enatp_Del(enatp p, constr name);
void        enatp_Release(enatp p, constr name);

int         enatp_CntTrans(enatp p);
int         enatp_CntPollTrans(enatp p);
int         enatp_CntLazyTrans(enatp p);

constr      enatp_GetConnUrls(enatp p);
constr      enatp_GetLazyUrls(enatp p);
constr*     enatp_GetNConnUrls(enatp p, int* cnt);
constr*     enatp_GetNLazyUrls(enatp p, int* cnt);

#define CONN_TRANS 0
#define LAZY_TRANS 1
#define ALL_TRANS  2
void        enatp_SetConnectedCB(enatp p, int type, enats_ConnectedCB cb, void* closure);            // only effect on unconnected lazy trans
void        enatp_SetClosedCB(enatp p, int type, enats_ClosedCB cb, void* closure);
void        enatp_SetDisconnectedCB(enatp p, int type, enats_DisconnectedCB cb, void* closure);
void        enatp_SetReconnectedCB(enatp p, int type, enats_ReconnectedCB cb, void* closure);
void        enatp_SetErrHandler(enatp p, int type, enats_ErrHandler cb, void* closure);

void        enatp_SetConnectedCBByName(enatp p, constr name, enats_ConnectedCB cb, void* closure);   // only effect on unconnected lazy trans
void        enatp_SetClosedCBByName(enatp p, constr name, enats_ClosedCB cb, void* closure);
void        enatp_SetDisconnectedCBByName(enatp p, constr name, enats_DisconnectedCB cb, void* closure);
void        enatp_SetReconnectedCBByName(enatp p, constr name, enats_ReconnectedCB cb, void* closure);
void        enatp_SetErrHandlerByName(enatp p, constr name, enats_ErrHandler cb, void* closure);

/// -- publish msgs
// note:
//   in normal mode, the msg will be publishing via the first connected url
//   in poll   mode, the msg will be publishing via polling all the connected urls, a msg only publish once
//
natsStatus  enatp_Pub    (enatp p, constr subj, conptr data, int dataLen);
natsStatus  enatp_PollPub(enatp p, constr subj, conptr data, int dataLen);

natsStatus  enatp_PubReq    (enatp p, constr subj, conptr data, int dataLen, constr reply);
natsStatus  enatp_PollPubReq(enatp p, constr subj, conptr data, int dataLen, constr reply);

/// -- request msg
natsStatus  enatp_Req    (enatp p, constr subj, conptr data, int dataLen, constr reply, natsMsg**replyMsg, int64_t timeout);
natsStatus  enatp_PollReq(enatp p, constr subj, conptr data, int dataLen, constr reply, natsMsg**replyMsg, int64_t timeout);

/// -- subscrib msg in specific enats
///
#define _ALL_enats_ (constr)-1         // use it as a name
natsStatus  enatp_Sub(enatp p, constr name, constr subj, enats_MsgHandler onMsg, void* closure);
natsStatus  enatp_Unsub(enatp p, constr name, constr subj);

ntStatistics enatp_GetStats(enatp p, constr subj);
constr       enatp_GetStatsStr(enatp p, int mode, constr subj);

int          enatp_IsErr(enatp p);
constr       enatp_LastErr(enatp p);

#ifdef __cplusplus
}
#endif

#endif
