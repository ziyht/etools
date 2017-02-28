#include "transport_cnats.h"

// -- local
#include <string.h>
#include <time.h>

// -- nats
#include "stats.h"
#include "msg.h"
#include "opts.h"

// -- uv
#include "uv.h"
typedef uv_mutex_t  mutex_t;
typedef uv_thread_t thread_t;

#define mutex_init(m)   uv_mutex_init(&m)
#define mutex_free(m)   uv_mutex_destroy(&m)
#define mutex_lock(m)   uv_mutex_lock(&m)
#define mutex_ulck(m)   uv_mutex_unlock(&m)

#define thread_init(t, cb, d)  uv_thread_create(&t, cb, d)
#define thread_join(t)         uv_thread_join(&t)

// -- helpler
#include "uthash.h"
#include "ejson.h"

#undef  VERSION
#define VERSION     1.1.5       // fix bug of errfmt in nTrans

#define __DEBUG_ 0

#if __DEBUG_
static constr _llog_basename(constr path){static constr slash; if (slash) {return slash + 1;}else{slash = strrchr(path, '/');}if (slash) {return slash + 1;}return 0;}
#define log(fmt, ...)   fprintf(stdout, "%s(%d):" fmt "%s", _llog_basename(__FILE__), __LINE__, __VA_ARGS__)
#define llog(...)       log(__VA_ARGS__, "\n");fflush(stdout)
#else
#define llog(...)
#endif

#if defined(_WIN32)
#define __unused
#define inline
#define sleep(sec) Sleep(sec)
#define localtime_r(sec, time) localtime_s(time, sec);
#define __thread_t uv_thread_t
#define __mutex_t  uv_mutex_t
#define __thread_create(t, cb, d) uv_thread_create(&(t), (uv_thread_cb)cb, d)
#define __thread_join(t)   uv_thread_join(&(t))
#define __mutex_init(mu)   uv_mutex_init(&mu)
#define __mutex_lock(mu)   uv_mutex_lock(&mu)
#define __mutex_ulck(mu)   uv_mutex_unlock(&mu)
#define __mutex_free(mu)   uv_mutex_destroy(&mu)
#else
#include <unistd.h>
#define __unused __attribute__((unused))
__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
#define __thread_t pthread_t
#define __mutex_t  pthread_mutex_t
#define __thread_create(t, cb, d) pthread_create(&t, 0, cb, d)
#define __thread_join(t)   pthread_join(t, 0)
#define __mutex_init(mu)   pthread_mutex_init(&mu, 0)
#define __mutex_lock(mu)   pthread_mutex_lock(&mu)
#define __mutex_ulck(mu)   pthread_mutex_unlock(&mu)
#define __mutex_free(mu)   pthread_mutex_destroy(&mu)
#endif

#define exe_ret(expr, ret) {expr;}     return ret
#define is0_ret(cond, ret) if(!(cond)) return ret
#define is1_ret(cond, ret) if( (cond)) return ret

#define is0_exeret(cond, expr, ret) if(!(cond)){ expr; return ret;}
#define is1_exeret(cond, expr, ret) if( (cond)){ expr; return ret;}
#define is0_elsret(cond, expr, ret) if(!(cond)){expr;} else return ret
#define is1_elsret(cond, expr, ret) if( (cond)){expr;} else return ret

/// --------------------- static count record -----------------------------
static int          __cnt_init;
static int          __cnt_conn;
static int          __cnt_pool;
static __mutex_t    __cnt_mut;

static inline void __cnt_connInc()
{
    if(!__cnt_init)
    {
        __mutex_init(__cnt_mut);
        __cnt_init = 1;
    }

    __mutex_lock(__cnt_mut);
    __cnt_conn++;
    __mutex_ulck(__cnt_mut);
}

static inline void __cnt_poolInc()
{
    if(!__cnt_init)
    {
        __mutex_init(__cnt_mut);
        __cnt_init = 1;
    }

    __mutex_lock(__cnt_mut);
    __cnt_pool++;
    __mutex_ulck(__cnt_mut);
}

static inline void __cnt_connDec()
{
    __mutex_lock(__cnt_mut);
    __cnt_conn--;
    if (__cnt_conn <= 0 && __cnt_pool <= 0) {
        llog(" __cnt_connDec: exe nats_Close()");
        nats_Close();
        __cnt_conn = 0;
        __cnt_pool = 0;
    }
    __mutex_ulck(__cnt_mut);
}

static inline void __cnt_poolDec()
{
    __mutex_lock(__cnt_mut);
    __cnt_pool--;
    if (__cnt_conn <= 0 && __cnt_pool <= 0) {
        llog(" __cnt_poolDec: exe nats_Close()");
        nats_Close();
        __cnt_conn = 0;
        __cnt_pool = 0;
    }
    __mutex_ulck(__cnt_mut);
}

/// ---------------------------- natsTrans definition ---------------------
typedef struct nTrans_Connector_s{
    natsConnection* nc;             // connect handler of cnats
    ntStatistics    stats;          // statistics
    natsStatus      s;              // last status
    char            stats_buf[512]; // buf to store statistic info, used by nTrans_GetStatsStr()
    char            conn_urls[512]; // buf to store connected urls, used by nTrans_GetConnUrls()
    char*           urls;

    nTrans_ClosedCB       closed_cb;
    void*                 closed_closure;
    nTrans_DisconnectedCB disconnected_cb;
    void*                 disconnected_closure;
    nTrans_ReconnectedCB  reconnected_cb;
    void*                 reconnected_closure;
    nTrans_ErrHandler     err_handler;
    void*                 err_closure;
}ntConnector_t, * ntConnector;

typedef struct nTrans_Publisher_s{
    bool            stop;           // stop
    bool            quit;           // quit thread if quit == 1
    bool            joined;         //
    uv_thread_t     thread;         // thread handler
    uv_mutex_t      mutex;
}ntPublisher_t, * ntPublisher;


typedef struct nTrans_Subscriber_s{
    char                subj[256];
    natsSubscription*   sub;

    nTrans              t;             // father natsTrans
    nTrans_MsgHandler   msg_handler;
    void*               msg_closure;
}ntSubscriber_t, * ntSubscriber;

typedef struct nTrans_ht_s* nTrans_ht_node;
typedef struct nTrans_s{
    ntConnector_t   conn;       // only one connector in natsTrans
    ntPublisher_t   pub;        // only one publisher in natsTrans
    ejson           sub_dic;    // subscriber in hash table
    mutex_t         sub_mu;

    nTrans_ht_node  self_node;  // point to the nTrans_ht_node which have this nTrans in a nTPool, or NULL if it is not a nTPool nTrans
    bool            quit;
    char*           last_err;
    char            last_err_buf[256];
}nTrans_t;

static constr last_err;
static char   last_err_buf[1024] __unused;

// -- helpler
typedef natsOptions* natsOptions_p;
static nTrans      __nTrans_ConnectTo(natsOptions_p opts);
static constr      __nTrans_MakeUrls(constr user, constr pass, constr url, int *c);
static natsStatus  __processUrlString(natsOptions *opts, constr urls);   // parse cnats url, copy from cnats src

// -- micros
#define _nTrans_setPubSemToMsg(t)
#define _nTrans_setPubSemToZero(t)
#define _nTrans_stopPub(t)
#define _nTrans_startPub(t)
#define _nTrans_pubIsStoped(t)       natsConnection_Status((t)->conn.nc) != CONNECTED
#define _nTrans_quitPubThread(t)     __mutex_ulck((t)->pub.mutex)

// -- callbacks for events
static void __natsTrans_ClosedCB(natsConnection* nc, void* trans);       // handler connection closed
static void __natsTrans_DisconnectedCB(natsConnection* nc, void* trans); // handler connection lost
static void __natsTrans_ReconnectedCB(natsConnection* nc, void* trans);  // handler connection reconnected
static void __natsTrans_ErrHandler(natsConnection *nc, natsSubscription *subscription,
                                   natsStatus err,void *closure);             // errors encountered while processing inbound messages

// -- callbacks for pubulisher
static void* __natsTrans_PubThreadCB(void* trans_);   // a thread to pub msg

// -- callbacks for subscriber
static void __natsTrans_MsgHandler(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *trans);

/// ------------------ natsTrans Pool ---------------------------
typedef struct nTrans_ht_s{
    UT_hash_handle  hh1;        // for transs         link
    UT_hash_handle  hh2;        // for polling_transs link
    UT_hash_handle  hh3;        // for lazy_transs    link

    char            name[64];
    nTrans          t;
    nTPool          p;          // point to the nTPool which have this node

    natsOptions*          opts;                 // used by lazy mode
    nTrans_ConnectedCB    connected_cb;
    void*                 connected_closure;

    nTrans_ClosedCB       closed_cb;
    void*                 closed_closure;
    nTrans_DisconnectedCB disconnected_cb;
    void*                 disconnected_closure;
    nTrans_ReconnectedCB  reconnected_cb;
    void*                 reconnected_closure;
    nTrans_ErrHandler     err_handler;
    void*                 err_closure;
}nTrans_ht_t, * nTrans_ht, nTrans_ht_node_t;

typedef struct urls_ht_s{
    UT_hash_handle  hh;

    char*           url;        // point to the url in nTrans, do not free it
    nTrans_ht_node  t_node;     // point to the nTrans_ht_node which contain this url, do not free it
}urls_ht_t, * urls_ht, urls_ht_node_t, * urls_ht_node;

typedef struct nTPool_s{
    nTrans_ht       conn_transs;    // transs connected to server
    nTrans_ht       poll_transs;    // transs in polling queue, sure connected
    nTrans_ht       lazy_transs;    // transs have not connected to server yet

    nTrans_ht_node  polling_now;

    urls_ht         urls;           // to store all the server urls(with user and pass) that connected or will connected to

    natsStatus      s;              // last status
    ntStatistics    stats;          // statistics for all transs

    __thread_t      wait_thread;    // only for wait when needed
    __mutex_t		wait_mutex;
    __thread_t      lazy_thread;
    __mutex_t		mutex;
    int             conn_num;       // the num of connected trans
    int             quit;

    char*           last_err;
    char            stats_buf[768]; // buf to store statistic info, if can not contained, it will use the next 1024 + 256
    char            conn_urls[1024];
    char            last_err_buf[256];
}nTPool_t;

// -- global
static ntStatistics ZERO_STATS;

// -- helpler
static nTrans_ht_node __nTPool_getNode(nTPool p, constr name);
static void*          __nTPool_waitThread(void*);

// -- micros
#define nTPool_lock(p)           __mutex_lock((p)->mutex)
#define nTPool_unlock(p)         __mutex_ulck((p)->mutex)

#define nTPool_add_conntrans(p,     add) HASH_ADD   (hh1,p->conn_transs, name[0],(unsigned)strlen(add->name),add)
#define nTPool_get_conntrans(p,name,out) HASH_FIND  (hh1,p->conn_transs, name,   (unsigned)strlen(name),     out)
#define nTPool_del_conntrans(p,     del) HASH_DELETE(hh1,p->conn_transs,                                     del)
#define nTPool_cnt_conntrans(p         ) HASH_CNT   (hh1,p->conn_transs                                         )

#define nTPool_add_polltrans(p,     add) HASH_ADD   (hh2,p->poll_transs, name[0],(unsigned)strlen(add->name),add)
#define nTPool_get_polltrans(p,name,out) HASH_FIND  (hh2,p->poll_transs, name   ,(unsigned)strlen(name),     out)
#define nTPool_del_polltrans(p,     del) HASH_DELETE(hh2,p->poll_transs,                                     del)
#define nTPool_cnt_polltrans(p         ) HASH_CNT   (hh2,p->poll_transs                                         )

#define nTPool_add_lazytrans(p,     add) HASH_ADD   (hh3,p->lazy_transs, name[0],(unsigned)strlen(add->name),add)
#define nTPool_get_lazytrans(p,name,out) HASH_FIND  (hh3,p->lazy_transs, name,   (unsigned)strlen(name),     out)
#define nTPool_del_lazytrans(p,     del) HASH_DELETE(hh3,p->lazy_transs,                                     del)
#define nTPool_cnt_lazytrans(p         ) HASH_CNT   (hh3,p->lazy_transs                                         )

#define nTPool_add_url(      p,     add) HASH_ADD   (hh, p->urls,        url[0], (unsigned)strlen(add->url), add)
#define nTPool_get_url(      p,url, out) HASH_FIND  (hh, p->urls,        url,    (unsigned)strlen(url),      out)
#define nTPool_del_url(      p,     del) HASH_DELETE(hh, p->urls,                                            del)

#define nTPool_polling_next(p) p->polling_now = p->polling_now ? (p->polling_now->hh2.next ? p->polling_now->hh2.next : p->poll_transs) : p->poll_transs;

// -- cb
static void* _nTPool_lazy_thread(void* p);

/// ---------------------------- natsTrans definition ---------------------

#define G ((nTrans)0)     // globel err
#define errset(h, err) do{if(h) h->last_err = err;else last_err = err;}while(0)
#define errfmt(h, ...) do{if(h) {h->last_err = h->last_err_buf; sprintf(h->last_err, ##__VA_ARGS__);}else{last_err = last_err_buf; sprintf(last_err_buf, ##__VA_ARGS__);}}while(0)

nTrans nTrans_New(constr urls)
{
    is1_exeret(!urls || !*urls, errset(G, "urls is null or empty"), NULL);

    natsOptions*    opts = NULL;
    natsStatus      s    = NATS_OK;
    nTrans          _out = NULL;

    if (natsOptions_Create(&opts) != NATS_OK)
        s = NATS_NO_MEMORY;

    if(s == NATS_OK)
        s = __processUrlString(opts, urls);

    if(s == NATS_OK)
    {
        _out = __nTrans_ConnectTo(opts);
        natsOptions_Destroy(opts);
    }

    return _out;
}

nTrans nTrans_NewTo(constr user, constr pass, constr url)
{
    constr urls = __nTrans_MakeUrls(user, pass, url, 0);
    nTrans _out = nTrans_New(urls);
    free((char*)urls);

    return _out;
}

nTrans nTrans_NewTo2(constr user, constr pass, constr server, int port)
{
    is1_exeret(!server || !*server, last_err = "server is null or empty", NULL);

    char url[64];
    sprintf(url, "%s:%d", server, port);

    constr urls = __nTrans_MakeUrls(user, pass, url, 0);
    nTrans _out = nTrans_New(urls);
    free((char*)urls);

    return _out;
}

nTrans nTrans_NewTo3(nTrans_opts _opts)
{
    is0_exeret(_opts, last_err = "NULL transport_opts", NULL);

    char*    user        = _opts->username;
    char*    pass        = _opts->password;
    char*    url         = _opts->conn_string;
//  char*    compression = _opts->compression;
//  char*    encryption  = _opts->encryption;
    uint64_t timeout     = _opts->timeout ? _opts->timeout : 2000;  // NATS_OPTS_DEFAULT_TIMEOUT = 2000

    // -- get urls
    constr   urls        = __nTrans_MakeUrls(user, pass, url, 0);
    if(!urls)
    {
        snprintf(last_err_buf, 1024, "can not make urls: opts err(%s): "
                                     "url:%s "
                                     "compression:%s "
                                     "encryption:%s "
                                     "username:%s "
                                     "password:%s "
                                     "timeout:%"PRIu64"",
                 last_err,
                 _opts->conn_string,
                 _opts->compression,
                 _opts->encryption,
                 _opts->username,
                 _opts->password,
                 _opts->timeout);
        last_err = last_err_buf;
        return NULL;
    }

    // --
    natsOptions*    opts = NULL;
    natsStatus      s    = NATS_OK;
    nTrans          _out ;

    if (natsOptions_Create(&opts) != NATS_OK)
        s = NATS_NO_MEMORY;

    if(s == NATS_OK)
        s = __processUrlString(opts, urls);

    if(s == NATS_OK)
    {
        natsOptions_SetTimeout(opts, timeout);

        _out = __nTrans_ConnectTo(opts);
        natsOptions_Destroy(opts);
    }

    free((char*)urls);
    return _out;
}

static inline nTrans __nTrans_ConnectTo(natsOptions_p opts)
{
    natsConnection* nc   = NULL;
    natsStatus      s    = NATS_OK;
    nTrans          _out = calloc(1, sizeof(*_out));

    if(!_out)
        goto err_return;

    natsOptions_SetClosedCB      (opts, __natsTrans_ClosedCB,       _out);
    natsOptions_SetDisconnectedCB(opts, __natsTrans_DisconnectedCB, _out);
    natsOptions_SetReconnectedCB (opts, __natsTrans_ReconnectedCB,  _out);
    natsOptions_SetErrorHandler  (opts, __natsTrans_ErrHandler,     _out);
    natsOptions_SetMaxReconnect  (opts, -1                              );
    natsOptions_SetNoRandomize   (opts, true                            );

    s = natsConnection_Connect(&nc, opts);

    // -- init
    if(s == NATS_OK)
    {
        // -- init connector
        _out->conn.nc    = nc;
        _out->conn.s     = s;

        mutex_init(_out->sub_mu);
        _out->sub_dic = ejso_new(_OBJ_);

        // others
        __cnt_connInc();
        return _out;
    }

err_return:
    free(_out);

    last_err = (s == NATS_OK) ? strerror(errno)
                              : nats_GetLastError(&s);
    return NULL;
}

void nTrans_JoinPubThread(nTrans trans __unused)
{
    if(!trans || trans->pub.joined == true)
        return;

    if(!trans->quit && !trans->pub.thread)
    {
        __mutex_lock(trans->pub.mutex);
        is1_exeret(__thread_create(trans->pub.thread, __natsTrans_PubThreadCB, trans),
                   errfmt(G, "create wait thread for nTPool faild: %s", strerror(errno)),
        );

        trans->pub.joined = true;

        __thread_join(trans->pub.thread);
        trans->pub.thread = 0;
    }
}

void nTrans_Destroy(nTrans_p _trans)
{
    if(!_trans || !(*_trans))  return;
    nTrans t = *_trans;
    t->quit  = 1;

    // -- release publisher
    _nTrans_quitPubThread(t);

    // -- release subscriber
    mutex_lock(t->sub_mu);
    ejson itr; ntSubscriber ntSub;
    ejso_itr(t->sub_dic, itr)
    {
        ntSub = ejso_valR(itr);
        natsSubscription_Unsubscribe(ntSub->sub);
        natsSubscription_Destroy(ntSub->sub);
    }
    ejso_free(t->sub_dic); t->sub_dic = 0;
    mutex_ulck(t->sub_mu);
    mutex_free(t->sub_mu);

    // -- stop and release connector
    natsConnection* nc = t->conn.nc; t->conn.nc = 0;
    natsConnection_Destroy(nc);

    // -- release self
    // free(*_trans);       // free in CloseCB
    *_trans = NULL;

    // -- release cnats if needed
    __cnt_connDec();
}

inline constr nTrans_GetConnUrls(nTrans trans)
{
    is0_ret(trans, NULL);

    trans->conn.s = natsConnection_GetConnectedUrl(trans->conn.nc, trans->conn.conn_urls, 512);

    is1_exeret(trans->conn.s == NATS_OK, trans->conn.urls = 0, trans->conn.conn_urls);
    return NULL;
}

constr      nTrans_GetUrls(nTrans trans)
{
    is0_ret(trans, NULL);

    if(trans->conn.nc)
    {
        strncpy(trans->conn.conn_urls, trans->conn.nc->opts->url, 512);
        trans->conn.urls = trans->conn.conn_urls;
    }

    return trans->conn.urls;
}

inline constr nTrans_GetName(nTrans trans)
{
    is0_ret(trans, NULL);

    return trans->self_node ? trans->self_node->name : NULL;
}

inline nTPool nTrans_GetPool(nTrans trans)
{
    is0_ret(trans, NULL);

    return trans->self_node ? trans->self_node->p : NULL;
}

inline void nTrans_SetClosedCB(nTrans trans, nTrans_ClosedCB cb, void* closure)
{
    is0_ret(trans, );

    trans->conn.closed_cb       = cb;
    trans->conn.closed_closure  = closure;
}
inline void nTrans_SetDisconnectedCB(nTrans trans, nTrans_DisconnectedCB cb, void* closure)
{
    is0_ret(trans, );

    trans->conn.disconnected_cb       = cb;
    trans->conn.disconnected_closure  = closure;
}
inline void nTrans_SetReconnectedCB(nTrans trans, nTrans_ReconnectedCB cb, void* closure)
{
    is0_ret(trans, );

    trans->conn.reconnected_cb       = cb;
    trans->conn.reconnected_closure  = closure;
}

inline void nTrans_SetErrHandler(nTrans trans, nTrans_ErrHandler cb, void* closure)
{
    is0_ret(trans, );

    trans->conn.err_handler = cb;
    trans->conn.err_closure = closure;
}

inline natsStatus nTrans_Pub(nTrans trans, constr subj, conptr data, int data_len)
{
    is0_exeret(trans, last_err = "invalid nTrans (nullptr)", NATS_ERR);

    trans->conn.s = natsConnection_Publish(trans->conn.nc, subj, data, data_len);
    return trans->conn.s;
}

natsStatus  nTrans_PubReq(nTrans trans, constr subj, conptr data, int dataLen, constr reply)
{
    is0_exeret(trans, last_err = "invalid nTrans (nullptr)", NATS_ERR);

    trans->conn.s = natsConnection_PublishRequest(trans->conn.nc, subj, reply, data, dataLen);
    return trans->conn.s;
}

#include "conn.h"
natsStatus
nTrans_Request(natsMsg **replyMsg, natsConnection *nc, const char *subj,
               const void *data, int dataLen, int64_t timeout, constr reply)
{
    natsStatus          s       = NATS_OK;
    natsSubscription    *sub    = NULL;

    if (replyMsg == NULL)
        return nats_setDefaultError(NATS_INVALID_ARG);
    s = natsConn_subscribe(&sub, nc, reply, NULL, NULL, NULL);
    if (s == NATS_OK)
        s = natsSubscription_AutoUnsubscribe(sub, 1);
    if (s == NATS_OK)
        s = natsConnection_PublishRequest(nc, subj, reply, data, dataLen);
    if (s == NATS_OK)
        s = natsSubscription_NextMsg(replyMsg, sub, timeout);

    natsSubscription_Destroy(sub);

    return NATS_UPDATE_ERR_STACK(s);
}

natsStatus  nTrans_Req(nTrans trans, constr subj, conptr data, int dataLen, constr reply, natsMsg**replyMsg, int64_t timeout)
{
    is0_exeret(trans, errset(G, "invalid nTrans (nullptr)"), NATS_ERR);

    //trans->conn.s = natsConnection_Request(replyMsg, trans->conn.nc, subj, data, dataLen, timeout);
    trans->conn.s = nTrans_Request(replyMsg, trans->conn.nc, subj, data, dataLen, timeout, reply);

    return trans->conn.s;
}

natsStatus  nTrans_Sub(nTrans trans, constr subj, nTrans_MsgHandler onMsg, void* closure)
{
    is0_exeret(trans, errset(G,     "invalid nTrans (nullptr)"), NATS_ERR);
    is0_exeret(onMsg, errfmt(trans, "null callbacks for subj %s", subj), NATS_ERR);
    is0_exeret(subj , errset(trans, "null subj"), NATS_ERR);
    is1_exeret(strstr(subj, "..") , errfmt(trans, "subj %s has \"..\" in it", subj), NATS_ERR);

    mutex_lock(trans->sub_mu);
    if(!trans->sub_dic) trans->sub_dic = ejso_new(_OBJ_);
    ntSubscriber ntSub = ejso_addR(trans->sub_dic, subj, sizeof(*ntSub));
    mutex_ulck(trans->sub_mu);
    is0_exeret(ntSub, errfmt(trans, "sub of %s already exist", subj), NATS_ERR);

    strcpy(ntSub->subj, subj);
    ntSub->t           = trans;
    ntSub->msg_handler = onMsg;
    ntSub->msg_closure = closure;
    trans->conn.s = natsConnection_Subscribe(&ntSub->sub, trans->conn.nc, subj, __natsTrans_MsgHandler, ntSub);

    if(trans->conn.s == NATS_OK)
        trans->conn.s = natsSubscription_SetPendingLimits(ntSub->sub, -1, -1);
    else
    {
        mutex_lock(trans->sub_mu);
        ejso_freeR(trans->sub_dic, subj);
        mutex_ulck(trans->sub_mu);
        trans->last_err = (char*)nats_GetLastError(&trans->conn.s);
        return trans->conn.s;
    }

    if(trans->conn.s != NATS_OK)
    {
        natsSubscription_Unsubscribe(ntSub->sub);
        mutex_lock(trans->sub_mu);
        ejso_freeR(trans->sub_dic, subj);
        mutex_ulck(trans->sub_mu);
        trans->last_err = (char*)nats_GetLastError(&trans->conn.s);
        return trans->conn.s;
    }

    return trans->conn.s;
}

natsStatus  nTrans_unSub(nTrans trans, constr subj)
{
    ntSubscriber ntSub;

    is0_exeret(trans, errset(G, "invalid nTrans (nullptr)"), NATS_ERR);

    mutex_lock(trans->sub_mu);

    if((ntSub = ejsr_valR(trans->sub_dic, subj)))
    {
        natsSubscription_Unsubscribe(ntSub->sub);
    }
    ejsr_free(trans->sub_dic, subj);

    mutex_ulck(trans->sub_mu);

    return NATS_OK;
}

inline ntStatistics nTrans_GetStats(nTrans trans, constr subj)
{
    is0_exeret(trans, errset(G, "invalid nTrans (nullptr)");, ZERO_STATS);

    ntStatistics*   stats =&trans->conn.stats;
    natsStatus      s     = NATS_OK;
    ntSubscriber    ntSub = NULL;

    s = natsConnection_GetStats(trans->conn.nc, (natsStatistics*)stats);

    mutex_lock(trans->sub_mu);
    ntSub = ejsr_valR(trans->sub_dic, subj);
    mutex_ulck(trans->sub_mu);
    if ((s == NATS_OK) && (ntSub != NULL))
    {
        s = natsSubscription_GetStats(ntSub->sub,
                                      &stats->pendingMsgs,
                                      &stats->pendingBytes,
                                      &stats->maxPendingMsgs,
                                      &stats->maxPendingBytes,
                                      &stats->deliveredMsgs,
                                      &stats->droppedMsgs);

        // if use AutoUnsubscribe(), when the max has been reached,
        // the subscription is automatically closed, so this call would
        // return "Invalid Subscription". Ignore this error.
        if (s == NATS_INVALID_SUBSCRIPTION)
        {
            s = NATS_OK;
            stats->pendingMsgs = 0;
        }
    }

    trans->conn.s = s;
    return trans->conn.stats;
}

inline constr nTrans_GetStatsStr(nTrans trans, int mode, constr subj)
{
    is0_exeret(trans, last_err = "invalid nTrans (nullptr)", "");

    nTrans_GetStats(trans, subj);

    if (trans->conn.s == NATS_OK)
    {
        char*           buf   = trans->conn.stats_buf;
        ntStatistics*   stats =&trans->conn.stats;
        int len = 0;
        if (mode & STATS_IN)
        {
            len += snprintf(buf + len, 1024 - len, "In Msgs: %9" PRIu64 " - "\
                   "In Bytes: %9" PRIu64 " - ", stats->inMsgs, stats->inBytes);
        }
        if (mode & STATS_OUT)
        {
            len += snprintf(buf + len, 1024 - len, "Out Msgs: %9" PRIu64 " - "\
                   "Out Bytes: %9" PRIu64 " - ", stats->outMsgs, stats->outBytes);
        }
        if (mode & STATS_COUNT)
        {
            len += snprintf(buf + len, 1024 - len, "Delivered: %9" PRId64 " - ", stats->deliveredMsgs);
            len += snprintf(buf + len, 1024 - len, "Pending: %5d - ", stats->pendingMsgs);
            len += snprintf(buf + len, 1024 - len, "Dropped: %5" PRId64 " - ", stats->droppedMsgs);
        }
        len += snprintf(buf + len, 1024 - len, "Reconnected: %3" PRIu64 "", stats->reconnects);
    }

    return trans->conn.stats_buf;
}

inline constr nTrans_LastErr(nTrans trans)
{
    return trans ? trans->last_err : last_err;
}

// --
static void _nTPool_ExeLazyThread(nTPool p);

static void __natsTrans_ClosedCB_DestroySubs(nTrans t){
    ejson itr; ntSubscriber s;

    mutex_lock(t->sub_mu);
    ejso_itr(t->sub_dic, itr)
    {
        s = ejso_valR(itr);

        natsSubscription_Destroy(s->sub);
        s->sub = 0;
    }
    mutex_ulck(t->sub_mu);
}

static void __natsTrans_ClosedCB(natsConnection* nc __unused, void* trans)
{
    nTPool p = NULL; nTrans t; int quit_wait_thread = 0;
    t = (nTrans)trans;

    if(t->self_node)
    {
        p = t->self_node->p;
        nTPool_lock(p);

        if(!p->quit)
        {
            if(p->polling_now)
            {
                if(p->polling_now->t == t)
                {
                    nTPool_polling_next(p);
                    if(p->polling_now->t == t)
                        p->polling_now = NULL;
                }
            }

            nTrans_ht_node fd;
            nTPool_get_polltrans(p, t->self_node->name, fd);
            if(fd) nTPool_del_polltrans(p, t->self_node);

            nTPool_get_conntrans(p, t->self_node->name, fd);
            if(fd) nTPool_del_conntrans(p, t->self_node);

            nTPool_get_lazytrans(p, t->self_node->name, fd);
            if(!fd) nTPool_add_lazytrans(p, t->self_node);

            __natsTrans_ClosedCB_DestroySubs(t);

            natsConnection* nc = t->conn.nc; t->conn.nc = 0;
            natsConnection_Destroy(nc);
            __cnt_connDec();

            _nTPool_ExeLazyThread(p);
        }

        nTPool_unlock(p);
    }

    if(t->conn.closed_cb)
        t->conn.closed_cb(t, t->conn.closed_closure);

    if(t->quit)
    {   if(t->self_node) free(t->self_node); free(trans); }

    if(p && p->quit)
    {
        nTPool_lock(p);
        if(p->conn_num > 0)     // p->conn_num will be set only when call nTPool_destroy()
            p->conn_num --;

        if(p->conn_num == 0)
        {
            __cnt_poolDec();

            if(p->wait_thread)  quit_wait_thread = 1;
            else                free(p);
        }
        else
            nTPool_unlock(p);
    }

    if(quit_wait_thread)
    {
        nTPool_unlock(p);
        __mutex_ulck(p->wait_mutex);
    }
}

static void __natsTrans_DisconnectedCB(natsConnection* nc __unused, void* trans)
{
    nTrans t = (nTrans)trans;

    // -- make a copy here, so the nTrans_GetUrls() can have a result in disconnected_cb and closed_cb
    if(!t->conn.urls)
    {
        strncpy(t->conn.conn_urls, nc->url->fullUrl, 512);
        t->conn.urls = t->conn.conn_urls;
    }

    if(t->self_node)
    {
        nTPool p = t->self_node->p;
        //nTPool_unlock(p);
        nTPool_lock(p);
        if(p->polling_now)
        {
            if(p->polling_now->t == t)
            {
                nTPool_polling_next(p);
                if(p->polling_now->t == t)
                    p->polling_now = NULL;
            }
            nTrans_ht_node fd;
            nTPool_get_polltrans(p, t->self_node->name, fd);
            if(fd) nTPool_del_polltrans(p, t->self_node);
        }
        nTPool_unlock(p);
    }

    if(t->conn.disconnected_cb)
        t->conn.disconnected_cb(t, t->conn.disconnected_closure);
}

static void __natsTrans_ReconnectedCB(natsConnection* nc __unused, void* trans)  // handler connect reconnected
{
    nTrans t = (nTrans)trans;
    _nTrans_setPubSemToMsg(t);
    _nTrans_startPub(t);

#if 1
    if(t->self_node)
    {
        nTPool p = t->self_node->p;
        nTPool_lock(p);

        nTrans_ht_node fd;
        nTPool_get_polltrans(p, t->self_node->name, fd);
        if(!fd) nTPool_add_polltrans(p, t->self_node);

        if(p->polling_now == NULL)
            nTPool_polling_next(p);
        nTPool_unlock(p);
    }
#endif
    if(t->conn.reconnected_cb)
        t->conn.reconnected_cb(t, t->conn.reconnected_closure);
}

static void __natsTrans_ErrHandler(natsConnection *nc __unused, natsSubscription *subscription, natsStatus err, void *trans __unused)
{
    nTrans t = (nTrans)trans;

    if(t->conn.err_handler)
        t->conn.err_handler(t, subscription, err, t->conn.err_closure);
}


static void* __natsTrans_PubThreadCB(void* trans_)
{
    nTrans t = (nTrans)trans_;
    __mutex_lock(t->pub.mutex);
    return 0;
}


static inline void __natsTrans_MsgHandler(natsConnection *nc __unused, natsSubscription *sub, natsMsg *msg, void *subscriber)
{
    ntSubscriber s = (ntSubscriber)subscriber;

    s->msg_handler(s->t, sub, msg, s->msg_closure);
}

#define USERPASS_ERR    0
#define USERPASS_OK     1
#define USER_ERR        2
#define PASS_ERR        3
static inline int __nTrans_CheckUserPass(constr user, constr pass)
{
    int _out = USERPASS_ERR;
    if(user && *user)
    {
        _out = PASS_ERR;
        last_err = "password is empty or NULL";
    }
    if(pass && *pass)
    {
        if(_out == PASS_ERR)
            _out = USERPASS_OK;
        else
        {
            _out = USER_ERR;
            last_err = "username is empty or NULL";
        }
    }

    return _out;
}

static inline constr __nTrans_MakeUrls(constr user, constr pass, constr url, int* c)
{
    int    s;
    char    out_buf[1024];
    char  * url_next, * url_dump;
    int     len = 0, count = 0;

    is1_exeret(!url || !*url, errset(G, "url is null or empty"), NULL);
    is1_ret((s = __nTrans_CheckUserPass(user, pass)) == USER_ERR, NULL);

    url_dump = strdup(url);
    url = url_dump;
    url_next = strchr(url, ',');

    do{
        url_next ? (*url_next = '\0') : (0);

        if(0 == strncmp(url, "nats://", 7))
            url += 7;

        if     (s == USERPASS_OK )  len += snprintf(out_buf + len, 1024 - len, "nats://%s:%s@%s,", user, pass, url);
        else if(s == PASS_ERR    )  len += snprintf(out_buf + len, 1024 - len, "nats://%s@%s,", user, url);
        else if(s == USERPASS_ERR)  len += snprintf(out_buf + len, 1024 - len, "nats://%s,", url);
        count ++;

        url = url_next ? url_next + 1 : 0;
        url_next = url_next ? strchr(url, ',') : 0;
    }while(url);

    out_buf[len ? len - 1 : 0] = '\0';
    free(url_dump);

    c ? *c = count : 0;
    return count ? strdup(out_buf) : 0;
}

static inline natsStatus __processUrlString(natsOptions *opts, constr urls)
{
    int         count        = 0;
    natsStatus  s            = NATS_OK;
    char        **serverUrls = NULL;
    char        *urlsCopy    = NULL;
    char        *commaPos    = NULL;
    char        *ptr         = NULL;
    int         len;

    ptr = (char*) urls;
    while ((ptr = strchr(ptr, ',')) != NULL)
    {
        ptr++;
        count++;
    }
    if (count == 0)
        return natsOptions_SetURL(opts, urls);

    serverUrls = (char**) calloc(count + 1, sizeof(char*));
    if (serverUrls == NULL)
        s = NATS_NO_MEMORY;
    if (s == NATS_OK)
    {
        urlsCopy = strdup(urls);
        if (urlsCopy == NULL)
        {
            free(serverUrls);
            return NATS_NO_MEMORY;
        }
    }

    count = 0;
    ptr = urlsCopy;

    do
    {
        while (*ptr == ' ')
            ptr++;
        serverUrls[count++] = ptr;

        commaPos = strchr(ptr, ',');
        if (commaPos != NULL)
        {
            ptr = (char*)(commaPos + 1);
            *(commaPos) = '\0';
        }

        len = (int) strlen(ptr);
        while ((len > 0) && (ptr[len-1] == ' '))
            ptr[--len] = '\0';

    } while (commaPos != NULL);

    if (s == NATS_OK)
        s = natsOptions_SetServers(opts, (const char**) serverUrls, count);

    free(urlsCopy);
    free(serverUrls);

    return s;
}

/// ------------------ natsTrans Pool definitions ---------------------------

#undef errset
#undef errfmt
#undef G
#define G ((nTPool)0)
#define errset(h, err) do{if(h) {h->last_err = (char*)err; h->s = NATS_ERR;}else last_err = err;}while(0)
#define errfmt(h, ...) do{if(h) {h->last_err = h->last_err_buf; sprintf(h->last_err, ##__VA_ARGS__);h->s = NATS_ERR;}else{last_err = last_err_buf; sprintf(last_err_buf, ##__VA_ARGS__);}}while(0)

inline nTPool nTPool_New()
{
    nTPool out = calloc(1, sizeof(nTPool_t));

    is0_exeret(out, errset(G, "mem faild in calloc new nTPool"), 0);
    __mutex_init(out->mutex);

    __cnt_poolInc();

    return out;
}

void nTPool_Destroy(nTPool_p _p)
{
    is1_ret(!_p || !*_p, );

    nTPool p = *_p; *_p = 0;
    nTrans_ht_node itr,  tmp;
    urls_ht_node   itr2, tmp2;
    int free_self_here = 0;

    if(p->quit) return;

    nTPool_lock(p);

    p->quit = 1;

    // -- clear lazy_transs
    itr = p->lazy_transs;
    p->lazy_transs = 0;
    if(p->lazy_thread)
        __thread_join(p->lazy_thread);

    p->lazy_transs = itr;
    HASH_ITER(hh3, p->lazy_transs, itr, tmp)
    {
        HASH_DELETE(hh3, p->lazy_transs, itr);
        natsOptions_Destroy(itr->opts);
        free(itr);
    }

    // -- clear polling_transs
    p->polling_now    = 0;
    HASH_ITER(hh2, p->poll_transs, itr, tmp)
    {
        nTPool_del_polltrans(p, itr);
    }

    // -- destroy transs
    p->conn_num = HASH_CNT(hh1, p->conn_transs);
    free_self_here = !p->conn_num;
    HASH_ITER(hh1, p->conn_transs, itr, tmp)
    {
        nTPool_del_conntrans(p, itr);
        nTrans_Destroy(&itr->t);             // release trans in CloseCB() of nTrans
        natsOptions_Destroy(itr->opts);
    }

    // -- release urls
    HASH_ITER(hh, p->urls, itr2, tmp2)
    {
        nTPool_del_url(p, itr2);
        free(itr2);
    }

    nTPool_unlock(p);

    if(free_self_here)
    {
        if(p->wait_thread)  __mutex_ulck(p->wait_mutex);    // p will be free in nTPool_Join()
        else                free(p);

        __cnt_poolDec();
    }
}

static void* __nTPool_waitThread(void* d)
{
    nTPool p = d;
    __mutex_lock(p->wait_mutex);
    return 0;
}

void   nTPool_Join(nTPool p)
{
    int free_nTPool_here = 0;

    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), );

    if(!p->quit && !p->wait_thread)
    {
        __mutex_init(p->wait_mutex);
        __mutex_lock(p->wait_mutex);
        is1_exeret(__thread_create(p->wait_thread, __nTPool_waitThread, p),
                   errfmt(G, "create wait thread for nTPool faild: %s", strerror(errno)),
        );

        __thread_join(p->wait_thread);
        free_nTPool_here = 1;
    }

    while(p->conn_num) usleep(100000);

    if(free_nTPool_here)
        free(p);
}

nTrans nTPool_Add(nTPool p, constr name, constr urls)
{
    natsOptions*   opts = NULL;
    natsStatus     s    = NATS_OK;
    nTrans_ht_node add  = NULL;
    nTrans         out  = NULL;
    char*          url;
    urls_ht_node   url_fd = NULL;
    urls_ht_node*  url_add = NULL;
    int            c, i = 0, j;

    // -- check args
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), 0);
    is1_exeret(!name || !*name, errset(p, "name is null or empty"), 0);

    // -- check name in pool
    nTPool_get_conntrans(p, name, add);
    is1_exeret(add, errfmt(p, "name \"%s\" already in nTPool", name), 0);

    // -- check urls in pool, if pool have one of the urls, return
    is1_exeret(s = natsOptions_Create(&opts)      != NATS_OK, errset(p, nats_GetLastError(&s)), 0);
    is1_exeret(s = __processUrlString(opts, urls) != NATS_OK, errset(p, nats_GetLastError(&s)); goto err_return;, 0);

    c = opts->url ? 1 : opts->serversCount;
    for(i = 0; i < c; i++)
    {
        url = opts->url ? opts->url : opts->servers[i];
        nTPool_get_url(p, url, url_fd);
        is1_exeret(url_fd, errfmt(p, "url \"%s\" is already in nTPool linked to [%s]", url_fd->url, url_fd->t_node->name); goto err_return;, 0);
    }
    is0_exeret(url_add = calloc(c, sizeof(void*)), errset(p, "mem faild for calloc url_add"); goto err_return;, 0);
    for(i = 0; i < c; i++)
    {
        url_add[i] = calloc(1, sizeof(urls_ht_node_t));
        is0_exeret(url_add[i], errfmt(p, "mem faild for calloc url_add[%d]", i); goto err_return;, 0);
    }

    // -- new nTrans_ht_node
    is0_exeret(out = __nTrans_ConnectTo(opts), errfmt(p, "%s when connect to %s", last_err, urls); goto err_return;, 0);
    is0_exeret(add = calloc(1, sizeof(*add)), errset(p, "mem faild for calloc nTrans_ht_node"); goto err_return;, 0);

    // -- init new nTrans_ht_node
    strcpy(add->name, name);
    out->self_node = add;
    add->p         = p;
    add->t         = out;
    add->opts      = opts;

    // -- add to pool
    nTPool_lock(p);

    nTPool_add_conntrans(p, add);
    nTPool_add_polltrans(p, add);
    for(i = 0; i < c; i++)
    {
        url_add[i]->url    = opts->url ? out->conn.nc->opts->url : out->conn.nc->opts->servers[i];
        url_add[i]->t_node = add;
        nTPool_add_url(p, url_add[i]);
    }
    if(!p->polling_now) nTPool_polling_next(p);

    nTPool_unlock(p);

    p->s = NATS_OK;
    goto ok_return;

err_return:
    natsOptions_Destroy(opts);
    nTrans_Destroy(&out);           // out will set to NULL
    for(j = 0; j < i; j++)
        free(url_add[j]);

ok_return:

    free(url_add);

    return out;
}

static void _nTPool_lazy_thread_reSub(nTrans trans){
    ejson itr; ntSubscriber ntSub;

    mutex_lock(trans->sub_mu);

    ejso_itr(trans->sub_dic, itr)
    {
        ntSub = ejso_valR(itr);

        trans->conn.s = natsConnection_Subscribe(&ntSub->sub, trans->conn.nc, ntSub->subj, __natsTrans_MsgHandler, ntSub);

        if(trans->conn.s == NATS_OK)
            trans->conn.s = natsSubscription_SetPendingLimits(ntSub->sub, -1, -1);
        else
        {
            ejso_freeO(trans->sub_dic, itr);
            trans->last_err = (char*)nats_GetLastError(&trans->conn.s);
            continue;
        }

        if(trans->conn.s != NATS_OK)
        {
            natsSubscription_Unsubscribe(ntSub->sub);
            ejso_freeO(trans->sub_dic, itr);
            trans->last_err = (char*)nats_GetLastError(&trans->conn.s);
            continue;
        }
    }

    mutex_ulck(trans->sub_mu);
}

static void* _nTPool_lazy_thread(void* _p)
{
    nTPool            p = (nTPool)_p;
    int          i,   c = 0;
    urls_ht_node url_fd = 0;

    nTrans_ht_node itr, tmp;
    while(p->lazy_transs)
    {
        sleep(1);

        HASH_ITER(hh3, p->lazy_transs, itr, tmp)
        {
            if(itr->t)                      // this is a trans that have connected ever
            {
                if(itr->t->conn.nc)         // this maybe wrong
                {
                    nTPool_lock(p);

                    // -- add to pool
                    nTrans_ht_node fd;
                    nTPool_del_lazytrans(p, itr);
                    nTPool_get_polltrans(p, itr->name, fd);
                    if(!fd) nTPool_add_polltrans(p, itr);
                    nTPool_get_conntrans(p, itr->name, fd);
                    if(!fd) nTPool_add_conntrans(p, itr);

                    if(!p->polling_now) nTPool_polling_next(p);
                    nTPool_unlock(p);

                    continue;
                }

                itr->t->conn.s = natsConnection_Connect(&itr->t->conn.nc, itr->opts);
                if(itr->t->conn.s == NATS_OK)
                {
                    nTPool_lock(p);

                    // -- add to pool
                    nTPool_del_lazytrans(p, itr);
                    nTPool_add_polltrans(p, itr);
                    nTPool_add_conntrans(p, itr);
                    if(!p->polling_now) nTPool_polling_next(p);

                    nTPool_unlock(p);
                    _nTPool_lazy_thread_reSub(itr->t);
                    if(itr->connected_cb)
                        itr->connected_cb(itr->t, itr->connected_closure);
                }

            }
            else if((itr->t = __nTrans_ConnectTo(itr->opts)))       // this is a trans that have never been connected
            {
                nTPool_lock(p);

                // -- update trans
                memcpy(&itr->t->conn.closed_cb, &itr->closed_cb, sizeof(void*)*8);
                itr->t->self_node = itr;

                // -- add to pool
                nTPool_del_lazytrans(p, itr);
                nTPool_add_polltrans(p, itr);
                nTPool_add_conntrans(p, itr);
                if(!p->polling_now) nTPool_polling_next(p);

                // -- update urls link in pool
                c = itr->opts->url ? 1 : itr->opts->serversCount;
                for(i = 0; i < c; i++)
                {
                    nTPool_get_url(p, c == 1 ? itr->opts->url : itr->opts->servers[i], url_fd);
                    url_fd->url = c == 1 ? itr->t->conn.nc->opts->url : itr->t->conn.nc->opts->servers[i];
                }

                nTPool_unlock(p);
                if(itr->connected_cb)
                    itr->connected_cb(itr->t, itr->connected_closure);
            }
        }
    }

    return 0;
}

static void _nTPool_ExeLazyThread(nTPool p)
{
    if(p->quit) return;

    if(p->lazy_thread == 0)
    {
        __thread_create(p->lazy_thread, _nTPool_lazy_thread, p);
        return ;
    }
#if (_WIN32)
    DWORD exitCode;
    GetExitCodeThread(p->lazy_thread, &exitCode);
    if(exitCode != STILL_ACTIVE)
#else
    else if(pthread_kill(p->lazy_thread, 0) == ESRCH)
#endif
    {
        __thread_join(p->lazy_thread);        // wo sure the thread is over, so this will not blocking, only release the resource of the quited thread
        __thread_create(p->lazy_thread, _nTPool_lazy_thread, p);
    }
}

natsStatus nTPool_AddLazy(nTPool p, constr name, constr urls)
{
    natsOptions*   opts = NULL, * opts_;
    natsStatus     s    = NATS_OK;
    nTrans_ht_node add  = NULL;
    nTrans         out  = NULL;
    char*          url;
    urls_ht_node   url_fd = NULL;
    urls_ht_node*  url_add = NULL;
    int            c, i;

    // -- check args
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), 0);
    is1_exeret(!name || !*name, errset(p, "name is null or empty"), 0);

    // -- check name in pool
    nTPool_get_conntrans(p, name, add);
    is1_exeret(add, errfmt(p, "name \"%s\" already in nTPool", name), 0);

    // -- check urls in pool, if pool have one of the urls, return
    is1_exeret((s = natsOptions_Create(&opts)    )  != NATS_OK, errset(p, nats_GetLastError(&s)), NATS_ERR);
    is1_exeret((s = __processUrlString(opts, urls)) != NATS_OK, errset(p, nats_GetLastError(&s)); goto err_return;, NATS_ERR);

    c = opts->url ? 1 : opts->serversCount;
    for(i = 0; i < c; i++)
    {
        url = opts->url ? opts->url : opts->servers[i];
        nTPool_get_url(p, url, url_fd);
        is1_exeret(url_fd, errfmt(p, "url \"%s\" is already in nTPool linked to [%s]", url_fd->url, url_fd->t_node->name); goto err_return;, NATS_ERR);
    }
    is0_exeret(url_add = calloc(c, sizeof(void*)), errset(p, "mem faild for calloc url_add"); goto err_return;, NATS_ERR);
    for(i = 0; i < c; i++)
    {
        url_add[i] = calloc(1, sizeof(urls_ht_node_t));
        is0_exeret(url_add[i], errfmt(p, "mem faild for calloc url_add[%d]", i); goto err_return;, NATS_ERR);
    }

    // -- new nTrans_ht_node
    is0_exeret(add = calloc(1, sizeof(*add)), errset(p, "mem faild for calloc nTrans_ht_node"); goto err_return;, NATS_ERR);
    strcpy(add->name, name);
    add->p    = p;
    add->opts = opts;
    add->t    = __nTrans_ConnectTo(opts);

    if(add->t)
        add->t->self_node = add;

    // -- add to pool
    nTPool_lock(p);

    if(!add->t)
    {
        nTPool_add_lazytrans(p, add);
        _nTPool_ExeLazyThread(p);
    }
    else
    {
        nTPool_add_conntrans(p, add);
        nTPool_add_polltrans(p, add);
        if(!p->polling_now) nTPool_polling_next(p);
    }

    opts_ = add->opts ? add->opts : out->conn.nc->opts;
    for(i = 0; i < c; i++)
    {
        url_add[i]->url    = opts_->url ? opts_->url : opts_->servers[i];
        url_add[i]->t_node = add;
        nTPool_add_url(p, url_add[i]);
    }

    nTPool_unlock(p);

    free(url_add);
    return p->s = NATS_OK;

err_return:
    free(url_add);
    natsOptions_Destroy(opts);
    return p->s = NATS_ERR;
}

natsStatus nTPool_AddOpts(nTPool p, nTrans_opts opts)
{
    cstr  urls, url, next_url;
    cstr* transs_names, * transs_urls;
    char transs_names_buf[1024];
    char name[10];
    int count, i, j;

    // -- check args
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), NATS_ERR);
    is0_exeret(opts, errset(p, "invalid transport opts (nullptr)"), NATS_ERR)

    urls = (cstr)__nTrans_MakeUrls(opts->username, opts->password, opts->conn_string, &count);
    is0_exeret(count, errset(p, last_err), NATS_ERR);
    is1_exeret(count > 100, free(urls); errset(p, "to many connect urls");, NATS_ERR);

#if 1
    transs_names = calloc(count, sizeof(cstr));
    for(i = 0, j = 0; j < count && i < 100; i++)
    {
        snprintf(name, 10, "nTrans%d", i);

        if(nTPool_Get(p, name))
            continue;

        transs_names[j] = transs_names_buf + i * 10;
        snprintf(transs_names[j++], 10, "%s", name);
    }

    transs_urls = calloc(count, sizeof(cstr));
    i        = 0;
    url      = urls;
    next_url = strchr(url, ',');
    transs_urls[i++] = url;
    while(next_url)
    {
        *next_url = '\0';
        url = next_url + 1;
        transs_urls[i++] = url;

        next_url = strchr(url, ',');
    }

    for(i = 0; i < count; i++)
    {
        if(nTPool_Add(p, transs_names[i], transs_urls[i]))
            continue;
        else
        {
            for(j = 0; j < i; j++)
                nTPool_Release(p, transs_names[j]);
            break;
        }
    }
#endif


    free(transs_names);
    free(transs_urls);
    free(urls);

    return p->s;
}

natsStatus  nTPool_AddOptsLazy(nTPool p, nTrans_opts opts)
{
    cstr  urls, url; ejson urls_dic; char name[10];
    int count, i;

    // -- check args
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), NATS_ERR);
    is0_exeret(opts, errset(p, "invalid transport opts (nullptr)"), NATS_ERR);

    urls = (cstr)__nTrans_MakeUrls(opts->username, opts->password, opts->conn_string, &count);
    is0_exeret(count, errset(p, last_err), NATS_ERR);
    is1_exeret(count > 100, free(urls); errset(p, "to many connect urls");, NATS_ERR);

    urls_dic = ejso_new(_OBJ_);
    i        = 0;
    url      = urls;
    char* tmp ;
    do{
        snprintf(name, 10, "nTrans%d", i++);

        while(nTPool_Get(p, name))
            snprintf(name, 10, "nTrans%d", i++);

        tmp = strchr(url, ',');
        if(tmp)
            *tmp = 0;

        ejso_addS(urls_dic, name, url);

        url = tmp ? tmp + 1 : 0;
    }while(url);

    ejson itr;
    ejso_itr(urls_dic, itr)
    {
        nTPool_AddLazy(p, ejso_keyS(itr), ejso_valS(itr));
    }

    ejso_free(urls_dic);

    free(urls);

    return p->s;
}

static inline nTrans_ht_node __nTPool_GetNode(nTPool p, constr name)
{
    nTrans_ht_node fd;

    // -- check args
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), 0);
    is1_exeret(!name || !*name, errset(p, "name is null or empty"), 0);

    // -- search name
    nTPool_lock(p);
    nTPool_get_conntrans(p, name, fd);
    if(0 == fd)
        nTPool_get_lazytrans(p, name, fd);
    is0_exeret(fd, errfmt(p, "nTPool have no nTrans named \"%s\"", name); nTPool_unlock(p);, 0);
    nTPool_unlock(p);

    return fd;
}

inline int nTPool_IsInLazyQueue(nTPool p, constr name)
{
    nTrans_ht_node n = __nTPool_GetNode(p, name);
    return n ? (n->opts ? 1 : 0) : 0;
}

inline nTrans nTPool_Get(nTPool p, constr name)
{
    nTrans_ht_node n = __nTPool_GetNode(p, name);
    return n ? n->t : 0;
}

nTrans nTPool_Del(nTPool p, constr name)
{
    natsOptions*   opts = NULL;
    nTrans_ht_node fd;
    nTrans         out;
    char*          url;
    urls_ht_node   url_fd;
    int            c, i;

    // -- get node
    is0_ret(fd = __nTPool_GetNode(p, name), 0);

    // -- delete it from pool
    nTPool_lock(p);

    if(fd->t)
    {
        if(p->polling_now == fd)
            nTPool_polling_next(p);
        if(p->polling_now == fd)
            p->polling_now = NULL;
        nTPool_del_polltrans(p, fd);
        nTPool_del_conntrans(p, fd);
        opts = fd->t->conn.nc->opts;
    }
    else
    {
        nTPool_del_lazytrans(p, fd);
        opts = fd->opts;
    }

    // -- delete urls from pool
    c    = opts->url ? 1 : opts->serversCount;
    for(i = 0; i < c; i++)
    {
        url = opts->url ? opts->url : opts->servers[i];
        nTPool_get_url(p, url, url_fd);
        nTPool_del_url(p, url_fd);
        free(url_fd);
    }
    if(fd->t)   fd->t->self_node = NULL;    // fd->t == NULL, when add a lazy url but can not connected

    natsOptions_Destroy(fd->opts);

    nTPool_unlock(p);

    out = fd->t;
    free(fd);
    return out;
}

inline void nTPool_Release(nTPool p, constr name)
{
    nTrans t = nTPool_Del(p, name);
    nTrans_Destroy(&t);
}

inline int nTPool_CntTrans(nTPool p)    {int c; is0_ret(p, 0); nTPool_lock(p); c = nTPool_cnt_conntrans(p); nTPool_unlock(p); return c;}
inline int nTPool_CntPollTrans(nTPool p){int c; is0_ret(p, 0); nTPool_lock(p); c = nTPool_cnt_polltrans(p); nTPool_unlock(p); return c;}
inline int nTPool_CntLazyTrans(nTPool p){int c; is0_ret(p, 0); nTPool_lock(p); c = nTPool_cnt_lazytrans(p); nTPool_unlock(p); return c;}

constr nTPool_GetConnUrls(nTPool p)
{
    nTrans_ht_node itr, tmp;
    int            len = 0;

    // -- check args
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), 0);
    is0_ret(p->conn_transs, 0);

    // -- travles
    nTPool_lock(p);
    p->s = NATS_OK;

    HASH_ITER(hh1, p->conn_transs, itr, tmp)
    {
        nTrans_GetConnUrls(itr->t);
        if(itr->t->conn.s == NATS_OK)
        {
            len += snprintf(p->conn_urls + len, 1024 - len, "[%s]%s", itr->name, itr->t->conn.conn_urls);
        }
        else
        {
            p->s = NATS_ERR;
            errfmt(p, "faild when get connected urls for trans [%s]", itr->name);
            break;
        }
    }
    nTPool_unlock(p);

    return len ? p->conn_urls : "";
}

constr nTPool_GetLazyUrls(nTPool p)
{
    nTrans_ht_node itr, tmp;
    cstr           url;
    int            j, c, len = 0;

    // -- check args
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)");, 0);
    is0_ret(p->lazy_transs, 0);

    // -- travles
    nTPool_lock(p);
    p->s = NATS_OK;
    HASH_ITER(hh3, p->lazy_transs, itr, tmp)
    {
        len += snprintf(p->conn_urls + len, 1024 - len, "[%s]", itr->name);
        c = itr->opts->url ? 1 : itr->opts->serversCount;
        for(j = 0; j < c; j++)
        {
            url  = c == 1 ? itr->opts->url : itr->opts->servers[j];
            len += snprintf(p->conn_urls + len, 1024 - len, "%s,", url);
        }
        p->conn_urls[--len] = '\0';
    }
    nTPool_unlock(p);

    return len ? p->conn_urls : "";
}

constr* nTPool_GetNConnUrls(nTPool p, int* cnt)
{
    nTrans_ht_node itr, tmp;
    int            i=0, len = 0;
    cstr*          out;

    // -- check args
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)");*cnt = 0;, 0);
    is0_exeret(p->conn_transs, *cnt = 0;, 0);
    is0_ret(*cnt =HASH_CNT(hh1, p->conn_transs), 0);

    if(!(out = calloc(*cnt, sizeof(cstr))))
    {
        *cnt = 0;
        return 0;
    }

    // -- travles
    nTPool_lock(p);
    p->s = NATS_OK;

    HASH_ITER(hh1, p->conn_transs, itr, tmp)
    {
        nTrans_GetConnUrls(itr->t);
        if(itr->t->conn.s == NATS_OK)
        {
            out[i++] = p->conn_urls + len;
            len += snprintf(p->conn_urls + len, 1024 - len, "[%s]%s", itr->name, itr->t->conn.conn_urls)+1;
        }
        else
        {
            p->s = NATS_ERR;
            errfmt(p, "faild when get connected urls for trans [%s]", itr->name);
            break;
        }
    }
    nTPool_unlock(p);

    *cnt = i;
    if(*cnt == 0)
    {
        free(out);
        return 0;
    }

    return (constr*)out;
}

constr* nTPool_GetNLazyUrls(nTPool p, int* cnt)
{
    nTrans_ht_node itr, tmp;
    cstr           url;
    int            i=0, j, c, len = 0;
    cstr*        out;

    // -- check args
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)");*cnt = 0;, 0);
    is0_exeret(p->lazy_transs, *cnt = 0;, 0);
    is0_ret(*cnt = HASH_CNT(hh3, p->lazy_transs), 0);

    if(!(out = calloc(*cnt, sizeof(cstr))))
    {
        *cnt = 0;
        return 0;
    }

    // -- travles
    nTPool_lock(p);
    p->s = NATS_OK;
    HASH_ITER(hh3, p->lazy_transs, itr, tmp)
    {
        out[i++] = p->conn_urls + len;
        len += snprintf(p->conn_urls + len, 1024 - len, "[%s]", itr->name);
        c = itr->opts->url ? 1 : itr->opts->serversCount;
        for(j = 0; j < c; j++)
        {
            url  = c == 1 ? itr->opts->url : itr->opts->servers[j];
            len += snprintf(p->conn_urls + len, 1024 - len, "%s,", url);
        }
        p->conn_urls[--len] = '\0';
        len++;
    }

    nTPool_unlock(p);

    *cnt = i;
    if(*cnt == 0)
    {
        free(out);
        return 0;
    }

    return (constr*)out;
}

void        nTPool_SetConnectedCB(nTPool p, int type, nTrans_ConnectedCB cb, void* closure)
{
    is0_ret(p, );
    nTrans_ht_node itr, tmp;

    nTPool_lock(p);
    if(type == CONN_TRANS || type == ALL_TRANS)
    {
        HASH_ITER(hh1, p->conn_transs, itr, tmp)
        {
            itr->connected_cb              = cb;
            itr->connected_closure         = closure;
        }
    }
    if(type == LAZY_TRANS || type == ALL_TRANS)
    {
        HASH_ITER(hh3, p->lazy_transs, itr, tmp)
        {
            itr->connected_cb      = cb;
            itr->connected_closure = closure;
        }
    }
    nTPool_unlock(p);
}

void        nTPool_SetClosedCB(nTPool p, int type, nTrans_ClosedCB cb, void* closure)
{
    is0_ret(p, );
    nTrans_ht_node itr, tmp;

    nTPool_lock(p);
    if(type == CONN_TRANS || type == ALL_TRANS)
    {
        HASH_ITER(hh1, p->conn_transs, itr, tmp)
        {
            itr->closed_cb              = cb;
            itr->closed_closure         = closure;
            itr->t->conn.closed_cb      = cb;
            itr->t->conn.closed_closure = closure;
        }
    }
    if(type == LAZY_TRANS || type == ALL_TRANS)
    {
        HASH_ITER(hh3, p->lazy_transs, itr, tmp)
        {
            itr->closed_cb      = cb;
            itr->closed_closure = closure;
        }
    }
    nTPool_unlock(p);
}
void        nTPool_SetDisconnectedCB(nTPool p, int type, nTrans_DisconnectedCB cb, void* closure)
{
    is0_ret(p, );
    nTrans_ht_node itr, tmp;

    nTPool_lock(p);
    if(type == CONN_TRANS || type == ALL_TRANS)
    {
        HASH_ITER(hh1, p->conn_transs, itr, tmp)
        {
            itr->disconnected_cb              = cb;
            itr->disconnected_closure         = closure;
            itr->t->conn.disconnected_cb      = cb;
            itr->t->conn.disconnected_closure = closure;
        }
    }
    if(type == LAZY_TRANS || type == ALL_TRANS)
    {
        HASH_ITER(hh3, p->lazy_transs, itr, tmp)
        {
            itr->disconnected_cb      = cb;
            itr->disconnected_closure = closure;
        }
    }
    nTPool_unlock(p);
}
void        nTPool_SetReconnectedCB(nTPool p, int type, nTrans_ReconnectedCB cb, void* closure)
{
    is0_ret(p, );
    nTrans_ht_node itr, tmp;

    nTPool_lock(p);
    if(type == CONN_TRANS || type == ALL_TRANS)
    {
        HASH_ITER(hh1, p->conn_transs, itr, tmp)
        {
            itr->reconnected_cb              = cb;
            itr->reconnected_closure         = closure;
            itr->t->conn.reconnected_cb      = cb;
            itr->t->conn.reconnected_closure = closure;
        }
    }
    if(type == LAZY_TRANS || type == ALL_TRANS)
    {
        HASH_ITER(hh3, p->lazy_transs, itr, tmp)
        {
            itr->reconnected_cb      = cb;
            itr->reconnected_closure = closure;
        }
    }
    nTPool_unlock(p);
}
void        nTPool_SetErrHandler(nTPool p, int type, nTrans_ErrHandler cb, void* closure)
{
    is0_ret(p, );
    nTrans_ht_node itr, tmp;

    nTPool_lock(p);
    if(type == CONN_TRANS || type == ALL_TRANS)
    {
        HASH_ITER(hh1, p->conn_transs, itr, tmp)
        {
            itr->err_handler         = cb;
            itr->err_closure         = closure;
            itr->t->conn.err_handler = cb;
            itr->t->conn.err_closure = closure;
        }
    }
    if(type == LAZY_TRANS || type == ALL_TRANS)
    {
        HASH_ITER(hh3, p->lazy_transs, itr, tmp)
        {
            itr->err_handler = cb;
            itr->err_closure = closure;
        }
    }
    nTPool_unlock(p);
}

void        nTPool_SetConnectedCBByName(nTPool p, constr name, nTrans_ConnectedCB cb, void* closure)
{
    nTrans_ht_node n = __nTPool_GetNode(p, name);
    is0_ret(n, );

    n->connected_cb      = cb;
    n->connected_closure = closure;
}

inline void nTPool_SetClosedCBByName(nTPool p, constr name, nTrans_ClosedCB cb, void* closure)
{
    nTrans_ht_node n = __nTPool_GetNode(p, name);
    is0_ret(n, );

    nTrans_SetClosedCB(n->t, cb, closure);

    n->closed_cb      = cb;
    n->closed_closure = closure;
}

inline void nTPool_SetDisconnectedCBByName(nTPool p, constr name, nTrans_DisconnectedCB cb, void* closure)
{
    nTrans_ht_node n = __nTPool_GetNode(p, name);
    is0_ret(n, );

    nTrans_SetDisconnectedCB(n->t, cb, closure);

    n->disconnected_cb      = cb;
    n->disconnected_closure = closure;
}

inline void nTPool_SetReconnectedCBByName(nTPool p, constr name, nTrans_ReconnectedCB cb, void* closure)
{
    nTrans_ht_node n = __nTPool_GetNode(p, name);
    is0_ret(n, );

    nTrans_SetReconnectedCB(n->t, cb, closure);

    n->reconnected_cb      = cb;
    n->reconnected_closure = closure;
}

inline void nTPool_SetErrHandlerByName(nTPool p, constr name, nTrans_ErrHandler cb, void* closure)
{
    nTrans_ht_node n = __nTPool_GetNode(p, name);
    is0_ret(n, );

    nTrans_SetErrHandler(n->t, cb, closure);

    n->err_handler = cb;
    n->err_closure = closure;
}

inline natsStatus  nTPool_Pub(nTPool p, constr subj, conptr data, int dataLen)
{
    natsStatus     s = NATS_ERR;
    nTrans_ht_node itr, tmp;

    // -- check args
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), NATS_ERR);

    // -- travles
    nTPool_lock(p);

    HASH_ITER(hh1, p->conn_transs, itr, tmp)
    {
        if(natsConnection_Status(itr->t->conn.nc) == CONNECTED)
        {
            s = nTrans_Pub(itr->t, subj, data, dataLen);
            if(s == NATS_OK)
                break;
        }
    }

    nTPool_unlock(p);
    return p->s = s;
}

inline natsStatus  nTPool_PollPub(nTPool p, constr subj, conptr data, int dataLen)
{
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), NATS_ERR);

    nTPool_lock(p);

    is0_exeret(p->polling_now, errset(p, "have no polling nTrans in nTPool");nTPool_unlock(p);, p->s);

    p->s = nTrans_Pub(p->polling_now->t, subj, data, dataLen);
    nTPool_polling_next(p);

    nTPool_unlock(p);

    return p->s;
}

natsStatus  nTPool_PubReq    (nTPool p, constr subj, conptr data, int dataLen, constr reply)
{
    natsStatus     s = NATS_ERR;
    nTrans_ht_node itr, tmp;

    // -- check args
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), NATS_ERR);

    // -- travles
    nTPool_lock(p);

    HASH_ITER(hh1, p->conn_transs, itr, tmp)
    {
        if(natsConnection_Status(itr->t->conn.nc) == CONNECTED)
        {
            s = nTrans_PubReq(itr->t, subj, data, dataLen, reply);
            if(s == NATS_OK)
                break;
        }
    }

    nTPool_unlock(p);
    return p->s = s;
}
natsStatus  nTPool_PollPubReq(nTPool p, constr subj, conptr data, int dataLen, constr reply)
{
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), NATS_ERR);

    nTPool_lock(p);

    is0_exeret(p->polling_now, errset(p, "have no polling nTrans in nTPool");nTPool_unlock(p);, p->s);

    p->s = nTrans_PubReq(p->polling_now->t, subj, data, dataLen, reply);
    nTPool_polling_next(p);

    nTPool_unlock(p);

    return p->s;
}

natsStatus  nTPool_Req    (nTPool p, constr subj, conptr data, int dataLen, constr reply, natsMsg**replyMsg, int64_t timeout)
{
    nTrans         nt;
    nTrans_ht_node itr, tmp;

    // -- check args
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), NATS_ERR);

    // -- travles
    nTPool_lock(p);

    HASH_ITER(hh1, p->conn_transs, itr, tmp)
    {
        if(natsConnection_Status(itr->t->conn.nc) == CONNECTED)
        {
            nt = itr->t;
            break;
        }
    }
    nTPool_unlock(p);

    p->s = nTrans_Req(nt, subj, data, dataLen, reply, replyMsg, timeout);

    return p->s;
}

natsStatus  nTPool_PollReq(nTPool p, constr subj, conptr data, int dataLen, constr reply, natsMsg**replyMsg, int64_t timeout)
{
    nTrans         nt;
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), NATS_ERR);

    nTPool_lock(p);

    is0_exeret(p->polling_now, errset(p, "have no polling nTrans in nTPool");nTPool_unlock(p);, p->s);
    nt = p->polling_now->t;

    nTPool_polling_next(p);

    nTPool_unlock(p);

    p->s = nTrans_Req(nt, subj, data, dataLen, reply, replyMsg, timeout);

    return p->s;
}

natsStatus  nTPool_Sub(nTPool p, constr name, constr subj, nTrans_MsgHandler onMsg, void* closure)
{
    nTrans_ht_node itr, tmp; nTrans nt;
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), NATS_ERR);
    is1_ret(!name || !subj || !*subj, NATS_ERR);

    nTPool_lock(p);

    if(name == _ALL_NTRANS_)
    {
        HASH_ITER(hh1, p->conn_transs, itr, tmp)
        {
            nt = itr->t;
            p->s = nTrans_Sub(nt, subj, onMsg, closure);
        }
    }
    else
    {
        nTPool_get_conntrans(p, name, itr);
        p->s = itr ? nTrans_Sub(itr->t, subj, onMsg, closure) : NATS_ERR;
    }

    nTPool_unlock(p);

    return p->s;
}

natsStatus  nTPool_Unsub(nTPool p, constr name, constr subj)
{
    nTrans_ht_node itr, tmp; nTrans nt;
    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), NATS_ERR);

    nTPool_lock(p);

    if(name == _ALL_NTRANS_)
    {
        HASH_ITER(hh1, p->conn_transs, itr, tmp)
        {
            nt = itr->t;
            nTrans_unSub(nt, subj);
        }
    }
    else
    {
        nt = nTPool_Get(p, name);
        nTrans_unSub(nt, subj);
    }

    nTPool_unlock(p);

    return NATS_OK;
}

ntStatistics nTPool_GetStats(nTPool p, constr subj)
{
    ntStatistics   sum = ZERO_STATS;
    nTrans_ht_node itr, tmp;
    ntStatistics*  stats;

    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), sum);
    is0_exeret(p->conn_transs, errset(p, "have no trans in nTPool"), sum);

    nTPool_lock(p);

    p->s = NATS_OK;
    HASH_ITER(hh1, p->conn_transs, itr, tmp)
    {
        nTrans_GetStats(itr->t, subj);
        if (itr->t->conn.s == NATS_OK)
        {
            stats                = &itr->t->conn.stats;
            sum.inMsgs          += stats->inMsgs;
            sum.outMsgs         += stats->outMsgs;
            sum.inBytes         += stats->inBytes;
            sum.outBytes        += stats->outBytes;
            sum.reconnects      += stats->reconnects;
            sum.pendingMsgs     += stats->pendingMsgs;
            sum.pendingBytes    += stats->pendingBytes;
            sum.maxPendingMsgs  += stats->maxPendingMsgs;
            sum.maxPendingBytes += stats->maxPendingBytes;
            sum.deliveredMsgs   += stats->deliveredMsgs;
            sum.droppedMsgs     += stats->droppedMsgs;
        }
        else
        {
            p->s = NATS_ERR;
            errfmt(p, "faild when get statistics for trans [%s]", itr->name);
            break;
        }

    }

    nTPool_unlock(p);

    return sum;
}

constr nTPool_GetStatsStr(nTPool p, int mode, constr subj)
{
    nTrans_ht_node itr, tmp;
    ntStatistics*  stats  , * sum     = &p->stats;
    char*                     buf     =  p->stats_buf;
    int            len = 0,   buf_len =  2048;

    is0_exeret(p, errset(p, "invalid nTPool (nullptr)"), "");
    is0_exeret(p->conn_transs, errset(p, "have no trans in nTPool"), "");

    nTPool_lock(p);
    p->s = NATS_OK;
    memset(&p->stats, 0, sizeof(ntStatistics));
    HASH_ITER(hh1, p->conn_transs, itr, tmp)
    {
        nTrans_GetStats(itr->t, subj);
        if (itr->t->conn.s == NATS_OK)
        {
            stats                 = &itr->t->conn.stats;
            sum->inMsgs          += stats->inMsgs;
            sum->outMsgs         += stats->outMsgs;
            sum->inBytes         += stats->inBytes;
            sum->outBytes        += stats->outBytes;
            sum->reconnects      += stats->reconnects;
            sum->pendingMsgs     += stats->pendingMsgs;
            sum->pendingBytes    += stats->pendingBytes;
            sum->maxPendingMsgs  += stats->maxPendingMsgs;
            sum->maxPendingBytes += stats->maxPendingBytes;
            sum->deliveredMsgs   += stats->deliveredMsgs;
            sum->droppedMsgs     += stats->droppedMsgs;

            if(mode & STATS_SINGEL)
            {
                if (mode & STATS_IN)
                {
                    len += snprintf(buf + len, buf_len - len, "[%s]\tIn Msgs: %9" PRIu64 " - In Bytes: %9" PRIu64 " - ", itr->name, stats->inMsgs, stats->inBytes);
                }
                if (mode & STATS_OUT)
                {
                    len += snprintf(buf + len, buf_len - len, "Out Msgs: %9" PRIu64 " - "\
                           "Out Bytes: %9" PRIu64 " - ", stats->outMsgs, stats->outBytes);
                }
                if (mode & STATS_COUNT)
                {
                    len += snprintf(buf + len, buf_len - len, "Delivered: %9" PRId64 " - ", stats->deliveredMsgs);
                    len += snprintf(buf + len, buf_len - len, "Pending: %5d - ", stats->pendingMsgs);
                    len += snprintf(buf + len, buf_len - len, "Dropped: %5" PRId64 " - ", stats->droppedMsgs);
                }
                len += snprintf(buf + len, buf_len - len, "Reconnected: %3" PRIu64 "\n", stats->reconnects);
            }
        }
        else
        {
            p->s = NATS_ERR;
            errfmt(p, "faild when get statistics for trans [%s]", itr->name);
            break;
        }
    }
    if(!(mode & STATS_SUM_OFF))
    {
        stats = sum;
        if (mode & STATS_IN)
        {
            len += snprintf(buf + len, buf_len - len, "[SUM]\tIn Msgs: %9" PRIu64 " - In Bytes: %9" PRIu64 " - ", stats->inMsgs, stats->inBytes);
        }
        if (mode & STATS_OUT)
        {
            len += snprintf(buf + len, buf_len - len, "Out Msgs: %9" PRIu64 " - "\
                   "Out Bytes: %9" PRIu64 " - ", stats->outMsgs, stats->outBytes);
        }
        if (mode & STATS_COUNT)
        {
            len += snprintf(buf + len, buf_len - len, "Delivered: %9" PRId64 " - ", stats->deliveredMsgs);
            len += snprintf(buf + len, buf_len - len, "Pending: %5d - ", stats->pendingMsgs);
            len += snprintf(buf + len, buf_len - len, "Dropped: %5" PRId64 " - ", stats->droppedMsgs);
        }
        len += snprintf(buf + len, buf_len - len, "Reconnected: %3" PRIu64 "", stats->reconnects);
    }
    nTPool_unlock(p);

    return p->stats_buf;
}

inline int nTPool_IsErr(nTPool p)
{
    return p->s == NATS_OK ? 0 : 1;
}

inline constr nTPool_LastErr(nTPool p)
{
    return p ? p->last_err : last_err;
}
