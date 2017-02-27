#include "enats.h"

// -- local
#include <string.h>
#include <time.h>

// -- nats
#include "stats.h"
#include "msg.h"
#include "opts.h"

// -- etools
#include "estr.h"
#include "ejson.h"

// -- uv
#if 0
#include "uv.h"
typedef uv_mutex_t  mutex_t;
typedef uv_thread_t thread_t;

#define mutex_init(m)   uv_mutex_init(&m)
#define mutex_free(m)   uv_mutex_destroy(&m)
#define mutex_lock(m)   uv_mutex_lock(&m)
#define mutex_ulck(m)   uv_mutex_unlock(&m)

#define thread_init(t, cb, d)  uv_thread_create(&t, cb, d)
#define thread_join(t)         uv_thread_join(&t)
#else
#define COMPAT_THREAD
#include "compat.h"
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

/// ---------------------------- natsTrans definition ---------------------
static int    conn_cnt;
static int    pool_cnt;


/// ---------------------------- natsTrans definition ---------------------
typedef struct enats_Connector_s{
    natsConnection* nc;             // connect handler of cnats
    ntStatistics    stats;          // statistics
    natsStatus      s;              // last status
    char            stats_buf[512]; // buf to store statistic info, used by enats_GetStatsStr()
    char            conn_urls[512]; // buf to store connected urls, used by enats_GetConnUrls()
    char*           urls;

    enats_closedCB       closed_cb;
    void*                closed_closure;
    enats_disconnectedCB disconnected_cb;
    void*                disconnected_closure;
    enats_reconnectedCB  reconnected_cb;
    void*                reconnected_closure;
    enats_errHandler     err_handler;
    void*                err_closure;
}ntConnector_t, * ntConnector;

typedef struct enats_Publisher_s{
    bool            stop;           // stop
    bool            quit;           // quit thread if quit == 1
    bool            joined;         //
    thread_t        thread;         // thread handler
    mutex_t         mutex;
}ntPublisher_t, * ntPublisher;

typedef struct enats_Subscriber_s{
    char                subj[256];
    natsSubscription*   sub;

    enats              t;             // father natsTrans
    enats_msgHandler   msg_cb;
    void*              msg_closure;
}ntSubscriber_t, * ntSubscriber;

typedef struct enats_ht_s* enats_ht_node;
typedef struct enats_s{
    ntConnector_t   conn;       // only one connector in natsTrans
    ntPublisher_t   pub;        // only one publisher in natsTrans
    ejson           sub_dic;    // subscriber in hash table
    mutex_t         sub_mu;

    enats_ht_node   self_node;  // point to the enats_ht_node which have this enats in a enatp, or NULL if it is not a enatp enats
    bool            quit;
    char*           last_err;
    char            last_err_buf[256];
}enats_t;

static constr last_err;
static char   last_err_buf[1024] __unused;

// -- helpler
typedef natsOptions* natsOptions_p;
static enats       __enats_ConnectTo(natsOptions_p opts);
static constr      __enats_MakeUrls(constr user, constr pass, constr url, int *c);
static natsStatus  __processUrlString(natsOptions *opts, constr urls);   // parse cnats url, copy from cnats src

// -- micros
#define _enats_quitPubThread(t)     __mutex_ulck((t)->pub.mutex)

// -- callbacks for events
static void __on_closed      (natsConnection* nc, void* trans);         // handler connection closed
static void __on_disconnected(natsConnection* nc, void* trans);         // handler connection lost
static void __on_reconnected (natsConnection* nc, void* trans);         // handler connection reconnected
static void __on_erroccurred (natsConnection* nc, natsSubscription* subscription, natsStatus err, void* closure);      // errors encountered while processing inbound messages

// -- callbacks for pubulisher
static void* __join(void* trans_);   // a thread

// -- callbacks for subscriber
static void __on_msg(natsConnection* nc, natsSubscription* sub, natsMsg* msg, void *trans);


#define G ((enats)0)     // globel err
#define errset(h, err) do{if(h) h->last_err = err;else last_err = err;}while(0)
#define errfmt(h, ...) do{if(h) {h->last_err = h->last_err_buf; sprintf(h->last_err, ##__VA_ARGS__);}else{last_err = last_err_buf; sprintf(h->last_err_buf, ##__VA_ARGS__);}}while(0)

enats enats_new(constr urls)
{
    is1_exeret(!urls || !*urls, errset(G, "urls is null or empty"), NULL);

    natsOptions*    opts = NULL;
    natsStatus      s    = NATS_OK;
    enats          _out = NULL;


    if (natsOptions_Create(&opts) != NATS_OK)
        s = NATS_NO_MEMORY;

    if(s == NATS_OK)
        s = __processUrlString(opts, urls);

    if(s == NATS_OK)
    {
        _out = __enats_ConnectTo(opts);
        natsOptions_Destroy(opts);
    }

    return _out;
}

enats enats_new2(constr user, constr pass, constr url)
{
    constr urls = __enats_MakeUrls(user, pass, url, 0);
    enats _out = enats_New(urls);
    free((char*)urls);

    return _out;
}

enats enats_new3(constr user, constr pass, constr server, int port)
{
    is1_exeret(!server || !*server, last_err = "server is null or empty", NULL);

    char url[64];
    sprintf(url, "%s:%d", server, port);

    constr urls = __enats_MakeUrls(user, pass, url, 0);
    enats _out = enats_New(urls);
    free((char*)urls);

    return _out;
}

enats enats_new4(enats_opts _opts)
{
    is0_exeret(_opts, last_err = "NULL transport_opts", NULL);

    char*    user        = _opts->username;
    char*    pass        = _opts->password;
    char*    url         = _opts->conn_string;
//  char*    compression = _opts->compression;
//  char*    encryption  = _opts->encryption;
    uint64_t timeout     = _opts->timeout ? _opts->timeout : 2000;  // NATS_OPTS_DEFAULT_TIMEOUT = 2000

    // -- get urls
    constr   urls        = __enats_MakeUrls(user, pass, url, 0);
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
    enats          _out = NULL;

    if (natsOptions_Create(&opts) != NATS_OK)
        s = NATS_NO_MEMORY;

    if(s == NATS_OK)
        s = __processUrlString(opts, urls);

    if(s == NATS_OK)
    {
        natsOptions_SetTimeout(opts, timeout);

        _out = __enats_ConnectTo(opts);
        natsOptions_Destroy(opts);
    }

    free((char*)urls);
    return _out;
}

static inline enats __enats_ConnectTo(natsOptions_p opts)
{
    natsConnection* nc   = NULL;
    natsStatus      s    = NATS_OK;
    enats          _out = calloc(1, sizeof(*_out));

    if(!_out)
        goto err_return;

    natsOptions_SetClosedCB      (opts, __on_closed,       _out);
    natsOptions_SetDisconnectedCB(opts, __on_disconnected, _out);
    natsOptions_SetReconnectedCB (opts, __on_reconnected,  _out);
    natsOptions_SetErrorHandler  (opts, __on_reconnected,  _out);
    natsOptions_SetMaxReconnect  (opts, -1                     );
    natsOptions_SetNoRandomize   (opts, true                   );

    s = natsConnection_Connect(&nc, opts);

    // -- init
    if(s == NATS_OK)
    {
        // -- init connector
        _out->conn.nc    = nc;
        _out->conn.s     = s;

        // -- init subscribe
        mutex_init(_out->sub_mu);
        _out->sub_dic = ejso_new(_OBJ_);

        // others
        conn_count++;
        return _out;
    }

err_return:
    natsConnection_Destroy(nc);
    free(_out);

    last_err = (s == NATS_OK) ? strerror(errno)
                              : nats_GetLastError(&s);
    return NULL;
}

void enats_join(enats trans)
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

void enats_Destroy(enats_p _trans)
{
    if(!_trans || !(*_trans))  return;
    enats t = *_trans;
    t->quit  = 1;

    // -- release publisher
    _enats_quitPubThread(t);
    __mutex_free(t->pub.mutex);

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
    natsConnection_Destroy(t->conn.nc);

    // -- release self
    // free(*_trans);       // free in CloseCB
    *_trans = NULL;

    // -- release cnats if needed
    conn_count--;
    if(conn_count <= 0 && pool_count <= 0)
    {
//      fprintf(stderr, "exe nats_Close()");
        nats_Close();
    }
}

inline constr enats_GetConnUrls(enats trans)
{
    is0_ret(trans, NULL);

    trans->conn.s = natsConnection_GetConnectedUrl(trans->conn.nc, trans->conn.conn_urls, 512);

    is1_exeret(trans->conn.s == NATS_OK, trans->conn.urls = 0, trans->conn.conn_urls);
    return NULL;
}

constr      enats_GetUrls(enats trans)
{
    is0_ret(trans, NULL);

    if(trans->conn.nc)
    {
        strncpy(trans->conn.conn_urls, trans->conn.nc->opts->url, 512);
        trans->conn.urls = trans->conn.conn_urls;
    }

    return trans->conn.urls;
}

inline constr enats_GetName(enats trans)
{
    is0_ret(trans, NULL);

    return trans->self_node ? trans->self_node->name : NULL;
}

inline enatp enats_GetPool(enats trans)
{
    is0_ret(trans, NULL);

    return trans->self_node ? trans->self_node->p : NULL;
}

inline void enats_SetClosedCB(enats trans, enats_ClosedCB cb, void* closure)
{
    is0_ret(trans, );

    trans->conn.closed_cb       = cb;
    trans->conn.closed_closure  = closure;
}
inline void enats_SetDisconnectedCB(enats trans, enats_DisconnectedCB cb, void* closure)
{
    is0_ret(trans, );

    trans->conn.disconnected_cb       = cb;
    trans->conn.disconnected_closure  = closure;
}
inline void enats_SetReconnectedCB(enats trans, enats_ReconnectedCB cb, void* closure)
{
    is0_ret(trans, );

    trans->conn.reconnected_cb       = cb;
    trans->conn.reconnected_closure  = closure;
}

inline void enats_SetErrHandler(enats trans, enats_ErrHandler cb, void* closure)
{
    is0_ret(trans, );

    trans->conn.err_handler = cb;
    trans->conn.err_closure = closure;
}

inline natsStatus enats_Pub(enats trans, constr subj, conptr data, int data_len)
{
    is0_exeret(trans, last_err = "invalid enats (nullptr)", NATS_ERR);
    trans->conn.s = natsConnection_Publish(trans->conn.nc, subj, data, data_len);
    return trans->conn.s;
}

natsStatus  enats_PubReq(enats trans, constr subj, conptr data, int dataLen, constr reply)
{
    is0_exeret(trans, last_err = "invalid enats (nullptr)", NATS_ERR);

    trans->conn.s = natsConnection_PublishRequest(trans->conn.nc, subj, reply, data, dataLen);
    return trans->conn.s;
}

#include "conn.h"
natsStatus
enats_Request(natsMsg **replyMsg, natsConnection *nc, const char *subj,
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

natsStatus  enats_Req(enats trans, constr subj, conptr data, int dataLen, constr reply, natsMsg**replyMsg, int64_t timeout)
{
    is0_exeret(trans, errset(G, "invalid enats (nullptr)"), NATS_ERR);

    //trans->conn.s = natsConnection_Request(replyMsg, trans->conn.nc, subj, data, dataLen, timeout);
    trans->conn.s = enats_Request(replyMsg, trans->conn.nc, subj, data, dataLen, timeout, reply);

    return trans->conn.s;
}

natsStatus  enats_Sub(enats trans, constr subj, enats_MsgHandler onMsg, void* closure)
{
    is0_exeret(trans, errset(G, "invalid enats (nullptr)"), NATS_ERR);
    is0_exeret(onMsg, errfmt(trans, "null callbacks for subj %s", subj), NATS_ERR);

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

natsStatus  enats_unSub(enats trans, constr subj)
{
    ntSubscriber ntSub;

    is0_exeret(trans, errset(G, "invalid enats (nullptr)"), NATS_ERR);

    mutex_lock(trans->sub_mu);

    if((ntSub = ejsr_valR(trans->sub_dic, subj)))
    {
        trans->conn.s = natsSubscription_Unsubscribe(ntSub->sub);
    }
    ejsr_free(trans->sub_dic, subj);

    mutex_ulck(trans->sub_mu);

    return trans->conn.s;
}

inline ntStatistics enats_GetStats(enats trans, constr subj)
{
    is0_exeret(trans, errset(G, "invalid enats (nullptr)");, ZERO_STATS);

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

inline constr enats_GetStatsStr(enats trans, int mode, constr subj)
{
    is0_exeret(trans, last_err = "invalid enats (nullptr)", "");

    enats_GetStats(trans, subj);

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

inline constr enats_LastErr(enats trans)
{
    return trans ? trans->last_err : last_err;
}

// --
static void _enatp_ExeLazyThread(enatp p);




static void* __natsTrans_PubThreadCB(void* trans_)
{
    enats t = (enats)trans_;

    ntPublisher  pub = &t->pub;
    __mutex_lock(pub->mutex);
    return 0;
}


static inline void __on_msg(natsConnection *nc __unused, natsSubscription *sub, natsMsg *msg, void *subscriber)
{
    ntSubscriber s = (ntSubscriber)subscriber;

    s->msg_handler(s->t, sub, msg, s->msg_closure);
}

#define USERPASS_ERR    0
#define USERPASS_OK     1
#define USER_ERR        2
#define PASS_ERR        3
static inline int __enats_CheckUserPass(constr user, constr pass)
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

static inline constr __enats_MakeUrls(constr user, constr pass, constr url, int* c)
{
    int    s;
    char    out_buf[1024];
    char  * url_next, * url_dump;
    int     len = 0, count = 0;

    is1_exeret(!url || !*url, errset(G, "url is null or empty"), NULL);
    is1_ret((s = __enats_CheckUserPass(user, pass)) == USER_ERR, NULL);

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


/// ------------------ natsTrans Pool ---------------------------
typedef struct enats_ht_s{
    char            name[64];
    enats          t;
    enatp          p;          // point to the enatp which have this node

    natsOptions*         opts;                 // used by lazy mode
    enats_connectedCB    connected_cb;
    void*                connected_closure;

    enats_closedCB       closed_cb;
    void*                closed_closure;
    enats_disconnectedCB disconnected_cb;
    void*                disconnected_closure;
    enats_reconnectedCB  reconnected_cb;
    void*                reconnected_closure;
    enats_errHandler     err_handler;
    void*                err_closure;
}enats_ht_t, * enats_ht, enats_ht_node_t;

typedef struct enatp_s{
    ejson       conn_transs;    // transs connected to server
    ejson       poll_transs;    // transs in polling queue, sure connected
    ejson       lazy_transs;    // transs have not connected to server yet

    ejson       polling_itr;
    enats       polling_now;

    ejson       urls;           // to store all the server urls(with user and pass) that connected or will connected to

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
}enatp_t;

// -- global
static ntStatistics ZERO_STATS;

// -- helpler
static enats_ht_node __enatp_getNode(enatp p, constr name);
static void*         __enatp_waitThread(void*);

// -- micros
#define enatp_init_mutex(p)     __mutex_init((p)->mutex);__mutex_init((p)->wait_mutex)
#define enatp_destrot_mutex(p)  __mutex_free((p)->mutex);__mutex_free((p)->wait_mutex)
#define enatp_lock(p)           __mutex_lock((p)->mutex)
#define enatp_unlock(p)         __mutex_ulck((p)->mutex)

#define enatp_add_conenats(p, add) ejso_addP(p->conn_transs, add->name, add)
#define enatp_get_conenats(p,name) ejsr_valP(p->conn_transs, name)
#define enatp_del_conenats(p, del) ejsr_free(p->conn_transs, del->name)
#define enatp_cnt_conenats(p     ) ejso_len (p->conn_transs)

#define enatp_add_polltrans(p, add) ejso_addP(p->poll_transs, add->name, add)
#define enatp_get_polltrans(p,name) ejsr_valP(p->poll_transs, name)
#define enatp_del_polltrans(p, del) ejsr_free(p->poll_transs, del->name)
#define enatp_cnt_polltrans(p     ) ejso_len (p->poll_transs)

#define enatp_add_lazytrans(p, add) ejso_addP(p->lazy_transs, add->name, add)
#define enatp_get_lazytrans(p,name) ejsr_valP(p->lazy_transs, name)
#define enatp_del_lazytrans(p, del) ejsr_free(p->lazy_transs, del->name)
#define enatp_cnt_lazytrans(p     ) ejso_len (p->lazy_transs)

#define enatp_add_url(p,url,ntname) ejso_addS(p->urls, url, ntname)
#define enatp_get_url(p,url       ) ejsr     (p->urls, url        )
#define enatp_del_url(p,url       ) ejsr_free(p->urls, url        )

// -- cb
static void* _enatp_lazy_thread(void* p);
static void  _enatp_polling_next(enatp p, enats reject);


#undef errset
#undef errfmt
#undef G
#define G ((enatp)0)
#define errset(h, err) do{if(h) {h->last_err = (char*)err; h->s = NATS_ERR;}else last_err = err;}while(0)
#define errfmt(h, ...) do{if(h) {h->last_err = h->last_err_buf; sprintf(h->last_err, ##__VA_ARGS__);h->s = NATS_ERR;}else{last_err = last_err_buf; sprintf(last_err_buf, ##__VA_ARGS__);}}while(0)

inline enatp enatp_New()
{
    enatp out = calloc(1, sizeof(enatp_t));

    is0_exeret(out, errset(G, "mem faild in calloc new enatp"), 0);
    enatp_init_mutex(out);

    out->conn_transs = ejso_new(_OBJ_);
    out->poll_transs = ejso_new(_OBJ_);
    out->lazy_transs = ejso_new(_OBJ_);
    out->urls        = ejso_new(_OBJ_);

    pool_count++;
    return out;
}

void enatp_Destroy(enatp_p _p)
{
    is1_ret(!_p || !*_p, );

    enatp p = *_p; *_p = 0;
    enats_ht_node nt_node;
    ejson ebackup;
    ejson itr;
    int free_self_here = 0;

    if(p->quit) return;

    enatp_lock(p);

    p->quit = 1;

    // -- clear lazy_transs
    ebackup = p->lazy_transs; p->lazy_transs = 0;
    if(p->lazy_thread)
        __thread_join(p->lazy_thread);

    ejso_itr(ebackup, itr)
    {
        nt_node = ejso_valP(itr);
        natsOptions_Destroy(nt_node->opts);
        free(nt_node);
    }

    ejso_free(ebackup);

    // -- clear polling_transs
    p->polling_now = 0;
    ebackup = p->poll_transs; p->poll_transs = 0;
    ejso_free(ebackup);

    // -- destroy transs
    p->conn_num = enatp_cnt_conenats(p);
    free_self_here = !p->conn_num;
    ebackup = p->conn_transs; p->conn_transs = 0;

    ejso_itr(ebackup, itr)
    {
        nt_node = ejso_valP(itr);
        enats_Destroy(&nt_node->t);    // release trans in CloseCB() of enats
        natsOptions_Destroy(nt_node->opts);
    }

    ejso_free(ebackup);

    // -- release urls
    ejso_free(p->urls); p->urls = 0;

    enatp_unlock(p);

    if(free_self_here)
    {
        if(p->wait_thread)  __mutex_ulck(p->wait_mutex);    // p will be free in wait_thread
        else                free(p);

        pool_count--;
        if (conn_count <= 0 && pool_count <= 0) {
            // fprintf(stderr, "exe nats_Close()\n");
            nats_Close();
            conn_count = 0;
            pool_count = 0;
        }
    }
}

static void* __enatp_waitThread(void* d)
{
    enatp p = d;
    __mutex_lock(p->wait_mutex);
    return 0;
}

void   enatp_Join(enatp p)
{
    int free_enatp_here = 0;

    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), );

    if(!p->quit && !p->wait_thread)
    {
        __mutex_lock(p->wait_mutex);
        is1_exeret(__thread_create(p->wait_thread, __enatp_waitThread, p),
                   errfmt(G, "create wait thread for enatp faild: %s", strerror(errno)),
        );

        __thread_join(p->wait_thread);
        free_enatp_here = 1;
    }

    while(p->conn_num) sleep(1);

    if(free_enatp_here)
        free(p);
}

enats enatp_Add(enatp p, constr name, constr urls)
{
    natsOptions*   opts = NULL;
    natsStatus     s    = NATS_OK;
    enats_ht_node add  = NULL;
    enats         out  = NULL;
    ejson          eurl;
    char*          url;
    int            c, i, j;

    // -- check args
    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), 0);
    is1_exeret(!name || !*name, errset(p, "name is null or empty"), 0);

    // -- check name in pool
    is1_exeret(enatp_get_conenats(p, name), errfmt(p, "name \"%s\" already in enatp", name), 0);

    // -- check urls in pool, if pool have one of the urls, return
    is1_exeret(s = natsOptions_Create(&opts)      != NATS_OK, errset(p, nats_GetLastError(&s)), 0);
    is1_exeret(s = __processUrlString(opts, urls) != NATS_OK, errset(p, nats_GetLastError(&s)); goto err_return;, 0);

    c = opts->url ? 1 : opts->serversCount;
    for(i = 0; i < c; i++)
    {
        url  = opts->url ? opts->url : opts->servers[i];
        eurl = enatp_get_url(p, url);
        is1_exeret(eurl, errfmt(p, "url \"%s\" is already in enatp linked to [%s]", url, ejso_valS(eurl)); goto err_return;, 0);
    }

    // -- new enats_ht_node
    is0_exeret(out = __enats_ConnectTo(opts), errfmt(p, "%s when connect to %s", last_err, urls); goto err_return;, 0);
    is0_exeret(add = calloc(1, sizeof(*add)), errset(p, "mem faild for calloc enats_ht_node"); goto err_return;, 0);

    // -- init new enats_ht_node
    strcpy(add->name, name);
    out->self_node = add;
    add->p         = p;
    add->t         = out;

    // -- add to pool
    enatp_lock(p);

    enatp_add_conenats(p, add);
    enatp_add_polltrans(p, add);
    for(i = 0; i < c; i++)
    {
        url  = opts->url ? opts->url : opts->servers[i];
        enatp_add_url(p, url, add->name);
    }
    _enatp_polling_next(p, 0);

    enatp_unlock(p);

    p->s = NATS_OK;
    goto ok_return;

err_return:
    enats_Destroy(&out);           // out will set to NULL
    natsOptions_Destroy(opts);

ok_return:

    return out;
}

static void _enatp_lazy_thread_reSub(enats trans){
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

static void* _enatp_lazy_thread(void* _p)
{
    enatp            p = (enatp)_p;
    int            i, c = 0;
    ejson          itr;
    char*          url;
    enats_ht_node nt_node;

    while(enatp_cnt_lazytrans(p))
    {
        sleep(1);

        ejso_itr(p->lazy_transs, itr)
        {
            nt_node = ejso_valP(itr);

            nt_node->t = __enats_ConnectTo(nt_node->opts);
            if(nt_node->t)                                   // connect ok
            {
                enatp_lock(p);

                // -- update trans
                memcpy(&nt_node->t->conn.closed_cb, &nt_node->closed_cb, sizeof(void*)*8);
                nt_node->t->self_node = nt_node;

                // -- add to pool
                enatp_del_lazytrans(p, nt_node);
                enatp_add_polltrans(p, nt_node);
                enatp_add_conenats(p, nt_node);
                _enatp_polling_next(p, 0);

                // -- update urls link in pool
                c = nt_node->opts->url ? 1 : nt_node->opts->serversCount;
                for(i = 0; i < c; i++)
                {
                    url  = nt_node->opts->url ? nt_node->opts->url : nt_node->opts->servers[i];
                    ejsr_setS(p->urls, url, nt_node->name);
                }

                enatp_unlock(p);

                _enatp_lazy_thread_reSub(nt_node->t);

                if(nt_node->connected_cb)
                    nt_node->connected_cb(nt_node->t, nt_node->connected_closure);
            }
            else
                continue;
        }
    }

    return 0;
}

static void _enatp_ExeLazyThread(enatp p)
{
    if(p->quit) return;

    if(p->lazy_thread == 0)
    {
        __thread_create(p->lazy_thread, _enatp_lazy_thread, p);
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
        __thread_create(p->lazy_thread, _enatp_lazy_thread, p);
    }
}

natsStatus enatp_AddLazy(enatp p, constr name, constr urls)
{
    natsOptions*   opts = NULL;
    natsStatus     s    = NATS_OK;
    enats_ht_node add  = NULL;
    ejson          eurl;
    char*          url;
    int            c, i;

    // -- check args
    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), 0);
    is1_exeret(!name || !*name, errset(p, "name is null or empty"), 0);

    // -- check name in pool
    is1_exeret(enatp_get_conenats(p, name), errfmt(p, "name \"%s\" already in enatp", name), 0);

    // -- check urls in pool, if pool have one of the urls, return
    is1_exeret(s = natsOptions_Create(&opts)      != NATS_OK, errset(p, nats_GetLastError(&s)), NATS_ERR);
    is1_exeret(s = __processUrlString(opts, urls) != NATS_OK, errset(p, nats_GetLastError(&s)); goto err_return;, NATS_ERR);

    c = opts->url ? 1 : opts->serversCount;
    for(i = 0; i < c; i++)
    {
        url  = opts->url ? opts->url : opts->servers[i];
        eurl = enatp_get_url(p, url);
        is1_exeret(eurl, errfmt(p, "url \"%s\" is already in enatp linked to [%s]", url, ejso_valS(eurl)); goto err_return;, NATS_ERR);
    }

    // -- new enats_ht_node
    is0_exeret(add = calloc(1, sizeof(*add)), errset(p, "mem faild for calloc enats_ht_node"); goto err_return;, NATS_ERR);
    strcpy(add->name, name);
    add->p    = p;
    add->opts = opts;
    add->t    = __enats_ConnectTo(opts);

    if(add->t)
        add->t->self_node = add;

    // -- add to pool
    enatp_lock(p);

    if(!add->t)
    {
        enatp_add_lazytrans(p, add);
        _enatp_ExeLazyThread(p);
    }
    else
    {
        enatp_add_conenats(p, add);
        enatp_add_polltrans(p, add);
        _enatp_polling_next(p, 0);
    }

    for(i = 0; i < c; i++)
    {
        url    = opts->url ? opts->url : opts->servers[i];
        enatp_add_url(p, url, add->name);
    }

    enatp_unlock(p);

    return p->s = NATS_OK;

err_return:
    natsOptions_Destroy(opts);
    return p->s = NATS_ERR;

}

natsStatus enatp_AddOpts(enatp p, enats_opts opts)
{
    cstr  urls, url, next_url;
    cstr* transs_names, * transs_urls;
    char transs_names_buf[1024];
    char name[10];
    int count = 0, i, j;

    // -- check args
    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), NATS_ERR);
    is0_exeret(opts, errset(p, "invalid transport opts (nullptr)"), NATS_ERR)

    urls = (cstr)__enats_MakeUrls(opts->username, opts->password, opts->conn_string, &count);
    is0_exeret(count, errset(p, last_err), NATS_ERR);
    is1_exeret(count > 100, free(urls); errset(p, "to many connect urls");, NATS_ERR);

#if 1
    transs_names = calloc(count, sizeof(cstr));
    for(i = 0, j = 0; j < count && i < 100; i++)
    {
        snprintf(name, 10, "enats%d", i);

        if(enatp_Get(p, name))
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
        if(enatp_Add(p, transs_names[i], transs_urls[i]))
            continue;
        else
        {
            for(j = 0; j < i; j++)
                enatp_Release(p, transs_names[j]);
            break;
        }
    }
#endif


    free(transs_names);
    free(transs_urls);
    free(urls);

    return p->s;
}

natsStatus  enatp_AddOptsLazy(enatp p, enats_opts opts)
{
    cstr  urls, url; ejson urls_dic; char name[10];
    int count, i;

    // -- check args
    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), NATS_ERR);
    is0_exeret(opts, errset(p, "invalid transport opts (nullptr)"), NATS_ERR);

    urls = (cstr)__enats_MakeUrls(opts->username, opts->password, opts->conn_string, &count);
    is0_exeret(count, errset(p, last_err), NATS_ERR);
    is1_exeret(count > 100, free(urls); errset(p, "to many connect urls");, NATS_ERR);

    urls_dic = ejso_new(_OBJ_);
    i        = 0;
    url      = urls;
    char* tmp ;
    do{
        snprintf(name, 10, "enats%d", i++);

        while(enatp_Get(p, name))
            snprintf(name, 10, "enats%d", i++);

        tmp = strchr(url, ',');
        if(tmp)
            *tmp = 0;

        ejso_addS(urls_dic, name, url);

        url = tmp ? tmp + 1 : 0;
    }while(url);

    ejson itr;
    ejso_itr(urls_dic, itr)
    {
        enatp_AddLazy(p, ejso_keyS(itr), ejso_valS(itr));
    }

    ejso_free(urls_dic);

    free(urls);

    return p->s;
}

static inline enats_ht_node __enatp_GetNode(enatp p, constr name)
{
    enats_ht_node fd;

    // -- check args
    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), 0);
    is1_exeret(!name || !*name, errset(p, "name is null or empty"), 0);

    // -- search name
    enatp_lock(p);
    fd = enatp_get_conenats(p, name);
    if(0 == fd)
        fd = enatp_get_lazytrans(p, name);
    is0_exeret(fd, errfmt(p, "enatp have no enats named \"%s\"", name); enatp_unlock(p);, 0);
    enatp_unlock(p);

    return fd;
}

inline int enatp_IsInLazyQueue(enatp p, constr name)
{
    enats_ht_node n = __enatp_GetNode(p, name);
    return n ? (n->t ? 0 : 1) : 0;
}

inline enats enatp_Get(enatp p, constr name)
{
    enats_ht_node n = __enatp_GetNode(p, name);
    return n ? n->t : 0;
}

enats enatp_Del(enatp p, constr name)
{
    natsOptions*   opts = NULL;
    enats_ht_node fd;
    enats         out;
    char*          url;
    int            c, i;

    // -- get node
    is0_ret(fd = __enatp_GetNode(p, name), 0);

    // -- delete it from pool
    enatp_lock(p);

    if(fd->t)
    {
        _enatp_polling_next(p, fd->t);
        enatp_del_polltrans(p, fd);
        enatp_del_conenats(p, fd);
        opts = fd->t->conn.nc->opts;
    }
    else
    {
        enatp_del_lazytrans(p, fd);
        opts = fd->opts;
    }

    // -- delete urls from pool
    c     = opts->url ? 1 : opts->serversCount;
    for(i = 0; i < c; i++)
    {
        url = opts->url ? opts->url : opts->servers[i];
        enatp_del_url(p, url);
    }
    if(fd->t)   fd->t->self_node = NULL;    // fd->t == NULL, when add a lazy url but can not connected

    natsOptions_Destroy(fd->opts);

    enatp_unlock(p);

    out = fd->t;
    free(fd);

    return out;
}

inline void enatp_Release(enatp p, constr name)
{
    enats t = enatp_Del(p, name);
    enats_Destroy(&t);
}

inline int enatp_CntTrans(enatp p)    {int c; is0_ret(p, 0); enatp_lock(p); c = enatp_cnt_conenats(p); enatp_unlock(p); return c;}
inline int enatp_CntPollTrans(enatp p){int c; is0_ret(p, 0); enatp_lock(p); c = enatp_cnt_polltrans(p); enatp_unlock(p); return c;}
inline int enatp_CntLazyTrans(enatp p){int c; is0_ret(p, 0); enatp_lock(p); c = enatp_cnt_lazytrans(p); enatp_unlock(p); return c;}

constr enatp_GetConnUrls(enatp p)
{
    enats_ht_node nt_node; ejson itr;
    int            len = 0;

    // -- check args
    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), 0);
    is0_ret(p->conn_transs, 0);

    // -- travles
    enatp_lock(p);
    p->s = NATS_OK;

    ejso_itr(p->conn_transs, itr)
    {
        nt_node = ejso_valP(itr);

        enats_GetConnUrls(nt_node->t);
        if(nt_node->t->conn.s == NATS_OK)
        {
            len += snprintf(p->conn_urls + len, 1024 - len, "[%s]%s", nt_node->name, nt_node->t->conn.conn_urls);
        }
        else
        {
            p->s = NATS_ERR;
            errfmt(p, "faild when get connected urls for trans [%s]", nt_node->name);
            break;
        }
    }
    enatp_unlock(p);

    return len ? p->conn_urls : "";
}

constr enatp_GetLazyUrls(enatp p)
{
    enats_ht_node nt_node; ejson itr;
    cstr           url;
    int            j, c, len = 0;

    // -- check args
    is0_exeret(p, errset(p, "invalid enatp (nullptr)");, 0);
    is0_ret(p->lazy_transs, 0);

    // -- travles
    enatp_lock(p);
    p->s = NATS_OK;
    ejso_itr(p->lazy_transs, itr)
    {
        nt_node = ejso_valP(itr);

        len += snprintf(p->conn_urls + len, 1024 - len, "[%s]", nt_node->name);
        c = nt_node->opts->url ? 1 : nt_node->opts->serversCount;
        for(j = 0; j < c; j++)
        {
            url  = c == 1 ? nt_node->opts->url : nt_node->opts->servers[j];
            len += snprintf(p->conn_urls + len, 1024 - len, "%s,", url);
        }
        p->conn_urls[--len] = '\0';
    }
    enatp_unlock(p);

    return len ? p->conn_urls : "";
}

constr* enatp_GetNConnUrls(enatp p, int* cnt)
{
    enats_ht_node nt_node; ejson itr;
    int            i=0, len = 0;
    cstr*          out;

    // -- check args
    is0_exeret(p, errset(p, "invalid enatp (nullptr)");*cnt = 0;, 0);
    is0_ret(*cnt =enatp_cnt_conenats(p), 0);

    if(!(out = calloc(*cnt, sizeof(cstr))))
    {
        *cnt = 0;
        return 0;
    }

    // -- travles
    enatp_lock(p);
    p->s = NATS_OK;

    ejso_itr(p->conn_transs, itr)
    {
        nt_node = ejso_valP(itr);

        enats_GetConnUrls(nt_node->t);
        if(nt_node->t->conn.s == NATS_OK)
        {
            out[i++] = p->conn_urls + len;
            len += snprintf(p->conn_urls + len, 1024 - len, "[%s]%s", nt_node->name, nt_node->t->conn.conn_urls)+1;
        }
        else
        {
            p->s = NATS_ERR;
            errfmt(p, "faild when get connected urls for trans [%s]", nt_node->name);
            break;
        }
    }
    enatp_unlock(p);

    *cnt = i;
    if(*cnt == 0)
    {
        free(out);
        return 0;
    }

    return (constr*)out;
}

constr* enatp_GetNLazyUrls(enatp p, int* cnt)
{
    enats_ht_node nt_node; ejson itr;
    cstr           url;
    int            i=0, j, c, len = 0;
    cstr*        out;

    // -- check args
    is0_exeret(p, errset(p, "invalid enatp (nullptr)");*cnt = 0;, 0);
    is0_ret(*cnt = enatp_cnt_lazytrans(p), 0);

    if(!(out = calloc(*cnt, sizeof(cstr))))
    {
        *cnt = 0;
        return 0;
    }

    // -- travles
    enatp_lock(p);
    p->s = NATS_OK;

    ejso_itr(p->lazy_transs, itr)
    {
        nt_node = ejso_valP(itr);

        out[i++] = p->conn_urls + len;
        len += snprintf(p->conn_urls + len, 1024 - len, "[%s]", nt_node->name);
        c = nt_node->opts->url ? 1 : nt_node->opts->serversCount;
        for(j = 0; j < c; j++)
        {
            url  = c == 1 ? nt_node->opts->url : nt_node->opts->servers[j];
            len += snprintf(p->conn_urls + len, 1024 - len, "%s,", url);
        }
        p->conn_urls[--len] = '\0';
        len++;
    }

    enatp_unlock(p);

    *cnt = i;
    if(*cnt == 0)
    {
        free(out);
        return 0;
    }

    return (constr*)out;
}

void        enatp_SetConnectedCB(enatp p, int type, enats_ConnectedCB cb, void* closure)
{
    is0_ret(p, );
    enats_ht_node nt_node; ejson itr;
    if(type == LAZY_TRANS || type == ALL_TRANS)
    {
        ejso_itr(p->lazy_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt_node->connected_cb      = cb;
            nt_node->connected_closure = closure;
        }
    }
}

void        enatp_SetClosedCB(enatp p, int type, enats_ClosedCB cb, void* closure)
{
    is0_ret(p, );
    enats_ht_node nt_node; ejson itr;
    if(type == CONN_TRANS || type == ALL_TRANS)
    {
        ejso_itr(p->conn_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt_node->t->conn.closed_cb      = cb;
            nt_node->t->conn.closed_closure = closure;
        }
    }
    if(type == LAZY_TRANS || type == ALL_TRANS)
    {
        ejso_itr(p->lazy_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt_node->closed_cb      = cb;
            nt_node->closed_closure = closure;
        }
    }
}
void        enatp_SetDisconnectedCB(enatp p, int type, enats_DisconnectedCB cb, void* closure)
{
    is0_ret(p, );
    enats_ht_node nt_node; ejson itr;
    if(type == CONN_TRANS || type == ALL_TRANS)
    {
        ejso_itr(p->conn_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt_node->t->conn.disconnected_cb      = cb;
            nt_node->t->conn.disconnected_closure = closure;
        }
    }
    if(type == LAZY_TRANS || type == ALL_TRANS)
    {
        ejso_itr(p->lazy_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt_node->disconnected_cb      = cb;
            nt_node->disconnected_closure = closure;
        }
    }
}
void        enatp_SetReconnectedCB(enatp p, int type, enats_ReconnectedCB cb, void* closure)
{
    is0_ret(p, );
    enats_ht_node nt_node; ejson itr;
    if(type == CONN_TRANS || type == ALL_TRANS)
    {
        ejso_itr(p->conn_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt_node->t->conn.reconnected_cb      = cb;
            nt_node->t->conn.reconnected_closure = closure;
        }
    }
    if(type == LAZY_TRANS || type == ALL_TRANS)
    {
        ejso_itr(p->lazy_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt_node->reconnected_cb      = cb;
            nt_node->reconnected_closure = closure;
        }
    }
}
void        enatp_SetErrHandler(enatp p, int type, enats_ErrHandler cb, void* closure)
{
    is0_ret(p, );
    enats_ht_node nt_node; ejson itr;
    if(type == CONN_TRANS || type == ALL_TRANS)
    {
        ejso_itr(p->conn_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt_node->t->conn.err_handler = cb;
            nt_node->t->conn.err_closure = closure;
        }
    }
    if(type == LAZY_TRANS || type == ALL_TRANS)
    {
        ejso_itr(p->lazy_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt_node->err_handler = cb;
            nt_node->err_closure = closure;
        }
    }
}

void        enatp_SetConnectedCBByName(enatp p, constr name, enats_ConnectedCB cb, void* closure)
{
    enats_ht_node n = __enatp_GetNode(p, name);
    is0_ret(n, );

    n->connected_cb      = cb;
    n->connected_closure = closure;
}

inline void enatp_SetClosedCBByName(enatp p, constr name, enats_ClosedCB cb, void* closure)
{
    enats_ht_node n = __enatp_GetNode(p, name);
    is0_ret(n, );

    enats_SetClosedCB(n->t, cb, closure);
    if(!n->t)
    {
        n->closed_cb      = cb;
        n->closed_closure = closure;
    }
}

inline void enatp_SetDisconnectedCBByName(enatp p, constr name, enats_DisconnectedCB cb, void* closure)
{
    enats_ht_node n = __enatp_GetNode(p, name);
    is0_ret(n, );

    enats_SetDisconnectedCB(n->t, cb, closure);
    if(!n->t)
    {
        n->disconnected_cb      = cb;
        n->disconnected_closure = closure;
    }
}

inline void enatp_SetReconnectedCBByName(enatp p, constr name, enats_ReconnectedCB cb, void* closure)
{
    enats_ht_node n = __enatp_GetNode(p, name);
    is0_ret(n, );

    enats_SetReconnectedCB(n->t, cb, closure);
    if(!n->t)
    {
        n->reconnected_cb      = cb;
        n->reconnected_closure = closure;
    }
}

inline void enatp_SetErrHandlerByName(enatp p, constr name, enats_ErrHandler cb, void* closure)
{
    enats_ht_node n = __enatp_GetNode(p, name);
    is0_ret(n, );

    enats_SetErrHandler(n->t, cb, closure);
    if(!n->t)
    {
        n->err_handler = cb;
        n->err_closure = closure;
    }
}

inline natsStatus  enatp_Pub(enatp p, constr subj, conptr data, int dataLen)
{
    natsStatus     s = NATS_ERR;
    enats_ht_node nt_node; ejson itr;

    // -- check args
    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), NATS_ERR);

    // -- travles
    enatp_lock(p);

    ejso_itr(p->conn_transs, itr)
    {
        nt_node = ejso_valP(itr);

        if(natsConnection_Status(nt_node->t->conn.nc) == CONNECTED)
        {
            s = enats_Pub(nt_node->t, subj, data, dataLen);
            if(s == NATS_OK)
                break;
        }
    }

    enatp_unlock(p);
    return p->s = s;
}

inline natsStatus  enatp_PollPub(enatp p, constr subj, conptr data, int dataLen)
{
    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), NATS_ERR);

    enatp_lock(p);

    is0_exeret(p->polling_now, errset(p, "have no polling enats in enatp");enatp_unlock(p);, p->s);

    p->s = enats_Pub(p->polling_now, subj, data, dataLen);
    _enatp_polling_next(p, 0);

    enatp_unlock(p);

    return p->s;
}

static void  _enatp_polling_next(enatp p, enats reject)
{
    if(reject)
    {
        if(p->polling_now != reject) return;
        else
        {
            p->polling_itr = ejso_next(p->polling_itr);
            if(!p->polling_itr) p->polling_itr = ejso_first(p->poll_transs);
            p->polling_now = p->polling_itr ? ((enats_ht_node)ejso_valP(p->polling_itr))->t
                                            : 0;

            if(p->polling_now == reject)
            {
                p->polling_itr = 0;
                p->polling_now = 0;
            }
        }
    }

    if(!p->polling_itr)
    {
        p->polling_itr = ejso_first(p->poll_transs);
    }
    else
    {
        p->polling_itr = ejso_next(p->polling_itr);
        if(!p->polling_itr) p->polling_itr = ejso_first(p->poll_transs);
    }

    p->polling_now = p->polling_itr ? ((enats_ht_node)ejso_valP(p->polling_itr))->t
                                    : 0;
}

natsStatus  enatp_PubReq    (enatp p, constr subj, conptr data, int dataLen, constr reply)
{
    natsStatus     s = NATS_ERR;
    enats_ht_node nt_node; ejson itr;

    // -- check args
    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), NATS_ERR);

    // -- travles
    enatp_lock(p);

    ejso_itr(p->conn_transs, itr)
    {
        nt_node = ejso_valP(itr);
        if(natsConnection_Status(nt_node->t->conn.nc) == CONNECTED)
        {
            s = enats_PubReq(nt_node->t, subj, data, dataLen, reply);
            if(s == NATS_OK)
                break;
        }
    }

    enatp_unlock(p);
    return p->s = s;
}
natsStatus  enatp_PollPubReq(enatp p, constr subj, conptr data, int dataLen, constr reply)
{
    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), NATS_ERR);

    enatp_lock(p);

    is0_exeret(p->polling_now, errset(p, "have no polling enats in enatp");enatp_unlock(p);, p->s);

    p->s = enats_PubReq(p->polling_now, subj, data, dataLen, reply);
    _enatp_polling_next(p, 0);

    enatp_unlock(p);

    return p->s;
}

natsStatus  enatp_Req    (enatp p, constr subj, conptr data, int dataLen, constr reply, natsMsg**replyMsg, int64_t timeout)
{
    enats_ht_node nt_node; ejson itr;
    enats         nt = 0;

    // -- check args
    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), NATS_ERR);

    // -- travles
    enatp_lock(p);

    ejso_itr(p->conn_transs, itr)
    {
        nt_node = ejso_valP(itr);

        if(natsConnection_Status(nt_node->t->conn.nc) == CONNECTED)
        {
            nt = nt_node->t;
            break;
        }
    }
    enatp_unlock(p);

    p->s = enats_Req(nt, subj, data, dataLen, reply, replyMsg, timeout);

    return p->s;
}

natsStatus  enatp_PollReq(enatp p, constr subj, conptr data, int dataLen, constr reply, natsMsg**replyMsg, int64_t timeout)
{
    enats         nt;
    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), NATS_ERR);

    enatp_lock(p);

    is0_exeret(p->polling_now, errset(p, "have no polling enats in enatp");enatp_unlock(p);, p->s);
    nt = p->polling_now;

    _enatp_polling_next(p, 0);

    enatp_unlock(p);

    p->s = enats_Req(nt, subj, data, dataLen, reply, replyMsg, timeout);

    return p->s;
}

natsStatus  enatp_Sub(enatp p, constr name, constr subj, enats_MsgHandler onMsg, void* closure)
{
    enats_ht_node nt_node; ejson itr; enats nt = 0;
    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), NATS_ERR);
    is1_ret(!name || !subj || !*subj, NATS_ERR);

    enatp_lock(p);

    if(name == _ALL_enats_)
    {
        ejso_itr(p->conn_transs, itr)
        {
            nt_node = ejso_valP(itr);

            nt = nt_node->t;
            p->s = enats_Sub(nt, subj, onMsg, closure);
        }
    }
    else
    {
        nt_node = enatp_get_conenats(p, name);
        p->s = nt_node ? enats_Sub(nt_node->t, subj, onMsg, closure) : NATS_ERR;
    }

    enatp_unlock(p);

    return p->s;
}

natsStatus  enatp_Unsub(enatp p, constr name, constr subj)
{
    enats_ht_node nt_node; ejson itr; enats nt = 0;
    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), NATS_ERR);

    enatp_lock(p);

    if(name == _ALL_enats_)
    {
        ejso_itr(p->conn_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt = nt_node->t;
            enats_unSub(nt, subj);
        }
    }
    else
    {
        nt = enatp_Get(p, name);
        enats_unSub(nt, subj);
    }

    enatp_unlock(p);

    return nt ? nt->conn.s : NATS_ERR;
}

ntStatistics enatp_GetStats(enatp p, constr subj)
{
    ntStatistics   sum = ZERO_STATS;
    enats_ht_node nt_node; ejson itr;
    ntStatistics*  stats;

    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), sum);
    is0_exeret(p->conn_transs, errset(p, "have no trans in enatp"), sum);

    enatp_lock(p);

    p->s = NATS_OK;
    ejso_itr(p->conn_transs, itr)
    {
        nt_node = ejso_valP(itr);
        enats_GetStats(nt_node->t, subj);
        if (nt_node->t->conn.s == NATS_OK)
        {
            stats                = &nt_node->t->conn.stats;
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
            errfmt(p, "faild when get statistics for trans [%s]", nt_node->name);
            break;
        }

    }

    enatp_unlock(p);

    return sum;
}

constr enatp_GetStatsStr(enatp p, int mode, constr subj)
{
    enats_ht_node nt_node; ejson itr;
    ntStatistics*  stats  , * sum     = &p->stats;
    char*                     buf     =  p->stats_buf;
    int            len = 0,   buf_len =  2048;

    is0_exeret(p, errset(p, "invalid enatp (nullptr)"), "");
    is0_exeret(p->conn_transs, errset(p, "have no trans in enatp"), "");

    enatp_lock(p);
    p->s = NATS_OK;
    memset(&p->stats, 0, sizeof(ntStatistics));
    ejso_itr(p->conn_transs, itr)
    {
        nt_node = ejso_valP(itr);
        enats_GetStats(nt_node->t, subj);
        if (nt_node->t->conn.s == NATS_OK)
        {
            stats                 = &nt_node->t->conn.stats;
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
                    len += snprintf(buf + len, buf_len - len, "[%s]\tIn Msgs: %9" PRIu64 " - In Bytes: %9" PRIu64 " - ", nt_node->name, stats->inMsgs, stats->inBytes);
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
            errfmt(p, "faild when get statistics for trans [%s]", nt_node->name);
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
    enatp_unlock(p);

    return p->stats_buf;
}

inline int enatp_IsErr(enatp p)
{
    return p->s == NATS_OK ? 0 : 1;
}

inline constr enatp_LastErr(enatp p)
{
    return p ? p->last_err : last_err;
}


static void __destroySubs(enats t){
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

static void __on_closed(natsConnection* nc __unused, void* trans)
{
    enatp p = NULL; enats t; int quit_wait_thread = 0;
    t = (enats)trans;

    if(!t->conn.urls)
    {
        strncpy(t->conn.conn_urls, nc->opts->url, 512);
        t->conn.urls = t->conn.conn_urls;
    }

    if(t->self_node)
    {
        p = t->self_node->p;
        enatp_lock(p);

        if(!p->quit)
        {
            _enatp_polling_next(p, t);

            enatp_del_polltrans(p, t->self_node);
            enatp_del_conenats(p, t->self_node);
            enatp_add_lazytrans(p, t->self_node);

            __natsTrans_ClosedCB_DestroySubs(t);

            natsConnection* nc = t->conn.nc; t->conn.nc = 0;
            natsConnection_Destroy(nc);

            _enatp_ExeLazyThread(p);
        }

        enatp_unlock(p);
    }

    if(t->conn.closed_cb)
        t->conn.closed_cb(t, t->conn.closed_closure);

    if(t->quit)
    {   if(t->self_node) free(t->self_node); free(trans); }

    if(p && p->quit)
    {
        enatp_lock(p);
        if(p->conn_num > 0) // p->conn_num will be set only when call enatp_destroy()
            p->conn_num --;

        if(p->conn_num == 0 && p->wait_thread)
            quit_wait_thread = 1;

        if(p->conn_num == 0)
        {
            pool_count--;
            if (conn_count <= 0 && pool_count <= 0) {
                // fprintf(stderr, "exe nats_Close()\n");
                nats_Close();
                conn_count = 0;
                pool_count = 0;
            }
        }

        enatp_unlock(p);

        if(!p->wait_thread && p->conn_num == 0)
            free(p);
    }

    if(quit_wait_thread)
        __mutex_ulck(p->wait_mutex);
}

static void __on_disconnected(natsConnection* nc __unused, void* trans)
{
    enats t = (enats)trans;

    if(t->self_node)
    {
        enatp p = t->self_node->p;

        enatp_lock(p);

        _enatp_polling_next(p, t);
        enatp_del_polltrans(p, t->self_node);

        enatp_unlock(p);
    }

    if(t->conn.disconnected_cb)
        t->conn.disconnected_cb(t, t->conn.disconnected_closure);
}

static void __on_reconnected(natsConnection* nc __unused, void* trans)  // handler connect reconnected
{
    enats t = (enats)trans;

#if 1
    if(t->self_node)
    {
        enatp p = t->self_node->p;

        enatp_lock(p);
        enatp_add_polltrans(p, t->self_node);
        _enatp_polling_next(p, 0);
        enatp_unlock(p);
    }
#endif
    if(t->conn.reconnected_cb)
        t->conn.reconnected_cb(t, t->conn.reconnected_closure);
}

static void __on_erroccurred(natsConnection *nc __unused, natsSubscription *subscription, natsStatus err, void *trans __unused)
{
    enats t = (enats)trans;

    if(t->conn.err_handler)
        t->conn.err_handler(t, subscription, err, t->conn.err_closure);
}
