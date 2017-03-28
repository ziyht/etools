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

#define ENATS_VERSION     "enats 1.0.0"

#include "nats.h"
#include "etype.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct enats_statistics_s{
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
}enats_stats_t;

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

typedef struct enats_s* enats;
typedef struct enatp_s* enatp;

/// -- callbacks type
typedef void (*enats_evtHandler) (enats t, void* closure);
typedef void (*enats_errHandler) (enats t, natsSubscription *subscription, natsStatus err, void* closure);
typedef void (*enats_msgHandler) (enats t, natsSubscription *sub, eMsg msg, void *closure);


/// ====================== enats ==================================
///
///     enats - an easier using wrapper for cnats
///


/// --------------------- enats new -------------------------------
enats enats_new (constr urls);                                          // syntax: nats://[[user][:passwd]@]server:port[,nats://...]
enats enats_new2(constr user, constr pass, constr url);                 // syntax: nats://server:port
enats enats_new3(constr user, constr pass, constr server, int port);
enats enats_new4(enats_opts opts);

void  enats_join(enats e);          // blocking until e is been destoried
void  enats_destroy(enats e);

/// -------------------- enats callbacks --------------------------
void enats_setClosedCB      (enats e, enats_evtHandler cb, void* closure);
void enats_setDisconnectedCB(enats e, enats_evtHandler cb, void* closure);
void enats_setReconnectedCB (enats e, enats_evtHandler cb, void* closure);
void enats_setErrHandler    (enats e, enats_errHandler cb, void* closure);

/// -------------------- enats msg transfer --------------------------
natsStatus  enats_pub (enats e, constr subj, conptr data, int dataLen);
natsStatus  enats_pubr(enats e, constr subj, conptr data, int dataLen, constr reply);

natsStatus  enats_sub  (enats e, constr subj, enats_msgHandler onMsg, void* closure);
natsStatus  enats_unsub(enats e, constr subj);

natsStatus  enats_req(enats e, constr subj, conptr data, int dataLen, constr reply, eMsg* replyMsg, int64_t timeout);

/// ---------------------- enats utils ----------------------------
constr enats_allurls(enats e);              // return all urls linked by this enats
constr enats_connurl(enats e);              // return the url  connnected by now
constr enats_lasturl(enats e);              // return the url  conneected at last time

constr enats_name(enats e);                 // return the name of  this enats
enatp  enats_pool(enats e);                 // return the pool who handle this enats

constr enats_statsS(enats e, constr subj);

constr enats_err(enats e);



/// ====================== enatp ==================================
///
///     enatp - an enats pool to handle enats more convenient
///

/// --- macro names ---
#define CONN_TRANS (constr)1
#define LAZY_TRANS (constr)2
#define ALL_TRANS  (constr)3

/// --------------------- enatp new -------------------------------
enatp enatp_new();

void  enatp_join(enatp p);
void  enatp_destroy(enatp p);

int   enatp_addUrls(enatp p, constr name, constr     urls, int lazy, int group);
int   enatp_addOpts(enatp p, constr name, enats_opts opts, int lazy, int group);

/// -------------------- enatp callbacks --------------------------

void  enatp_setConnectedCB   (enatp p, constr name, enats_evtHandler cb, void* closure);
void  enatp_setClosedCB      (enatp p, constr name, enats_evtHandler cb, void* closure);
void  enatp_setDisconnectedCB(enatp p, constr name, enats_evtHandler cb, void* closure);
void  enatp_setReconnectedCB (enatp p, constr name, enats_evtHandler cb, void* closure);
void  enatp_setErrHandler    (enatp p, constr name, enats_errHandler cb, void* closure);

/// -------------------- enats msg transfer -----------------------

/// -- normal mode --
///     the msg will be publishing via the first connected enats
///
natsStatus  enatp_pub (enatp p, constr subj, conptr data, int dataLen);
natsStatus  enatp_pubr(enatp p, constr subj, conptr data, int dataLen, constr reply);

natsStatus  enatp_sub  (enatp p, constr name, constr subj, enats_msgHandler onMsg, void* closure);
natsStatus  enatp_unsub(enatp p, constr name, constr subj);

natsStatus  enatp_req(enatp p, constr subj, conptr data, int dataLen, constr reply, eMsg* replyMsg, int64_t timeout);

/// -- poll mode --
///     the msg will be publishing via polling all the connected enats, a msg only publish once
///
natsStatus  enatp_pubPoll (enatp p, constr subj, conptr data, int dataLen);
natsStatus  enatp_pubrPoll(enatp p, constr subj, conptr data, int dataLen, constr reply);

natsStatus  enatp_reqPoll(enatp p, constr subj, conptr data, int dataLen, constr reply, eMsg* replyMsg, int64_t timeout);


/// ---------------------- enatp utils ----------------------------

int    enatp_cntTrans(enatp p, constr name);

constr enatp_connurls(enatp p);
constr enatp_lazyurls(enatp p);

constr enatp_statsS(enatp p, constr subj, int detail);

constr enatp_err(enatp p);

#ifdef __cplusplus
}
#endif

#endif
