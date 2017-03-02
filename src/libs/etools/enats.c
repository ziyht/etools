#include "enats.h"

// -- local
#include <string.h>
#include <time.h>
#include <assert.h>

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

#define exe_ret(expr, ret ) { expr;      return ret;}
#define is0_ret(cond, ret ) if(!(cond)){ return ret;}
#define is1_ret(cond, ret ) if( (cond)){ return ret;}
#define is0_exe(cond, expr) if(!(cond)){ expr;}
#define is1_exe(cond, expr) if( (cond)){ expr;}

#define is0_exeret(cond, expr, ret) if(!(cond)){ expr;        return ret;}
#define is1_exeret(cond, expr, ret) if( (cond)){ expr;        return ret;}
#define is0_elsret(cond, expr, ret) if(!(cond)){ expr;} else{ return ret;}
#define is1_elsret(cond, expr, ret) if( (cond)){ expr;} else{ return ret;}

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
        llog(stderr, "exe nats_Close()\n");
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
        llog(stderr, "exe nats_Close()\n");
        nats_Close();
        __cnt_conn = 0;
        __cnt_pool = 0;
    }
    __mutex_ulck(__cnt_mut);
}

/// ---------------------------- enats  ---------------------
typedef struct __conn_s{
    natsConnection*     nc;             // connect handler of cnats
    natsOptions*        opts;           // back up of opts in t
    natsStatus          s;              // last status

    enats_evtHandler    closed_cb;
    void*               closed_closure;
    enats_evtHandler    disconnected_cb;
    void*               disconnected_closure;
    enats_evtHandler    reconnected_cb;
    void*               reconnected_closure;
    enats_evtHandler    err_handler;
    void*               err_closure;
}__conn_t, * __conn;

typedef struct __subs_s{
    natsSubscription*  sub;

    enats              t;             // father enats
    enats_msgHandler   msg_handler;
    void*              msg_closure;
}__subs_t, * __subs;

typedef struct enatp_node_s* enatp_node;
typedef struct enats_s{
    // -- connection
    __conn_t        conn;           // only one connector in natsTrans

    // -- subscriber
    ejson           sub_dic;        // subscriber in hash table
    mutex_t         sub_mu;

    // -- for quit control
    int             quit;
    int             wait_num;
    thread_t        wait_thread;    // only for wait when needed
    mutex_t		    wait_mutex;

    enatp_node      self_node;      // point to the enats_ht_node which have this enats in a enatp, or NULL if it is not a enatp enats

    natsStatus      es;
    estr            err;
    estr            stats;          // buf to store statistic info
    estr            connurls;       // buf to store connected urls
    estr            urls;
}enats_t;

/// ------------------ enats Pool ---------------------------
struct enatp_node_s{
    char                name[64];
    enats               t;
    enatp               p;                     // point to the enatp which have this node

    enats_evtHandler    connected_cb;
    void*               connected_closure;

    enats_evtHandler    closed_cb;
    void*               closed_closure;
    enats_evtHandler    disconnected_cb;
    void*               disconnected_closure;
    enats_evtHandler    reconnected_cb;
    void*               reconnected_closure;
    enats_evtHandler    err_handler;
    void*               err_closure;
}enatp_node_t;

typedef struct __attribute__ ((__packed__)) enatp_s{
    natsStatus      s;                  // last status

    // -- for enats manager
    ejson           conn_transs;        // transs connected to server
    ejson           poll_transs;        // transs in polling queue, sure connected
    ejson           lazy_transs;        // transs have not connected to server yet

    ejson           polling_itr;        // point to the ejson obj in poll_transs who is polling now
    enats           polling_now;        // the enats in polling_itr

    ejson           urls;               // to store all the connected urls

    __thread_t      lazy_thread;
    __mutex_t		mutex;

    // -- for quit control
    __thread_t      wait_thread;        // only for wait when needed
    __mutex_t		wait_mutex;
    int             conn_num;           // the num of connected trans
    int             quit;

    // -- other
    natsStatus      es;
    estr            err;                // last err
    estr            stats;              // to store statistic info
    estr            connurls;
}enatp_t;

static constr last_err;
static char   last_err_buf[1024] __unused;

// -- helpler
typedef natsOptions* natsOptions_p;
static enats       __enats_connectTo(natsOptions_p opts);
static estr        __enats_makeUrls(constr user, constr pass, constr url, int *c);
static natsStatus  __processUrlString(natsOptions *opts, constr urls);   // parse cnats url, copy from cnats src

// -- callbacks for events
static void __on_closed      (natsConnection* nc, void* trans);         // handler connection closed
static void __on_disconnected(natsConnection* nc, void* trans);         // handler connection lost
static void __on_reconnected (natsConnection* nc, void* trans);         // handler connection reconnected
static void __on_erroccurred (natsConnection* nc, natsSubscription* subscription, natsStatus err, void* closure);      // errors encountered while processing inbound messages

// -- callbacks for subscriber
static void __on_msg(natsConnection* nc, natsSubscription* sub, natsMsg* msg, void *trans);

static void* __enats_waitThread(void* trans_);

// -- static err handle for enats
#define G ((enats)0)     // globel err
#define errset(h, str) do{if(h) {h->err = estr_wrs(h->err, str);}else{last_err = str;}}while(0)
#define errfmt(h, ...) do{if(h) {h->err = estr_wrp(h->err, ##__VA_ARGS__);}else{last_err = last_err_buf; sprintf(last_err_buf, ##__VA_ARGS__);}}while(0)

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
static inline estr __enats_makeUrls(constr user, constr pass, constr url, int* c)
{
    int     s;
    char  * url_next, * url_dump;
    int     count = 0;
    estr    urls;

    is1_exeret(!url || !*url, errset(G, "url is null or empty"), NULL);
    is1_ret((s = __enats_CheckUserPass(user, pass)) == USER_ERR, NULL);

    urls = estr_newLen(0, strlen(url) * 2);

    while(*url == ',') url++;

    url_dump = strdup(url);
    url = url_dump;
    url_next = strchr(url, ',');

    do{
        url_next ? (*url_next = '\0') : (0);

        if(0 == strncmp(url, "nats://", 7))
            url += 7;

        if     (s == USERPASS_OK )  urls = estr_catf(urls, "nats://%s:%s@%s,", user, pass, url);
        else if(s == PASS_ERR    )  urls = estr_catf(urls, "nats://%s@%s,"   , user,       url);
        else if(s == USERPASS_ERR)  urls = estr_catf(urls, "nats://%s,"      ,             url);
        count ++;

        url = url_next ? url_next + 1 : 0;
        url_next = url_next ? strchr(url, ',') : 0;
    }while(url);

    free(url_dump);

    c ? *c = count : 0;
    return urls;
}

static inline enats __enats_newHandle(constr urls, enats_opts e_opts)
{
    natsStatus      s     = NATS_OK;
    natsOptions*    opts  = NULL;
    estr            nurls = NULL;
    enats e;

    if(e_opts && e_opts->tls.enanle)
    {
       // is1_exeret(access(_opts->tls.ca  , F_OK), errfmt("ca file: %s not exist", _opts->tls.ca), NULL);
        is1_exeret(access(e_opts->tls.key , F_OK), errfmt(G, "client key  file: %s not exist", e_opts->tls.key ), NULL);
        is1_exeret(access(e_opts->tls.cert, F_OK), errfmt(G, "client cert file: %s not exist", e_opts->tls.cert), NULL);
    }

    // -- for urls parser
    if(!urls)
    {
        char*    auth        = e_opts->auth;
        char*    user        = e_opts->username;
        char*    pass        = e_opts->password;
        char*    url         = e_opts->conn_string;

        nurls = ((user && *user) || (pass && *pass)) ? __enats_makeUrls(user, pass, url, 0)
                                                     : __enats_makeUrls(auth, 0   , url, 0);
        is0_ret(nurls, 0);

        urls  = nurls;
    }

    is1_exe((s = natsOptions_Create(&opts))      != NATS_OK, errset(G, "alloc for new nats opts faild"    ); goto err_ret;);
    is1_exe((s = __processUrlString(opts, urls)) != NATS_OK, errset(G, "__processUrlString for opts faild"); goto err_ret;);
    is1_exe((e = calloc(1, sizeof(*e)))          == 0      , errset(G, "alloc for new enats faild"        ); goto err_ret;);

    e->conn.opts = opts;
    natsOptions_SetClosedCB      (opts, __on_closed,       e);
    natsOptions_SetDisconnectedCB(opts, __on_disconnected, e);
    natsOptions_SetReconnectedCB (opts, __on_reconnected,  e);
    natsOptions_SetErrorHandler  (opts, __on_erroccurred,  e);
    natsOptions_SetMaxReconnect  (opts, -1                  );
    natsOptions_SetNoRandomize   (opts, true                );

    if(e_opts && e_opts->tls.enanle)
    {
        is1_exe((s = natsOptions_SetSecure(opts, true))!= NATS_OK, errset(G, nats_GetLastError(&s)); goto err_ret;);

        if(e_opts->tls.ca)
        {
            is1_exe((s = natsOptions_LoadCATrustedCertificates(opts, e_opts->tls.ca))           != NATS_OK, errset(G, nats_GetLastError(&s)); goto err_ret;);
        }
        is1_exe((s = natsOptions_LoadCertificatesChain(opts, e_opts->tls.cert, e_opts->tls.key))!= NATS_OK, errset(G, nats_GetLastError(&s)); goto err_ret;);

    }

    mutex_init(e->sub_mu);
    assert(e->sub_dic = ejso_new(_OBJ_));

    e->conn.s = NATS_OK;

    return e;

err_ret:
    estr_free(nurls);
    natsOptions_Destroy(opts);

    return NULL;
}

static inline int   __enats_tryConnect(enats e)
{
    natsConnection* nc   = NULL;
    natsStatus      s    = NATS_OK;

    is1_ret(e->conn.nc, 1);

    is1_exeret((s = natsConnection_Connect(&nc, e->conn.opts)) != NATS_OK, e->es =s, 0);

    e->conn.s  = s;
    e->conn.nc = nc;

    __cnt_connInc();

    return 1;
}

static inline void __enats_destroySubDic(enats e)
{
    ejson itr; __subs esub;

    is0_ret(e, )

    mutex_lock(e->sub_mu);

    ejso_itr(e->sub_dic, itr)
    {
        esub = ejso_valR(itr);
        natsSubscription_Unsubscribe(esub->sub);
        natsSubscription_Destroy(esub->sub);
    }
    ejso_free(e->sub_dic); e->sub_dic = 0;

    mutex_ulck(e->sub_mu);
    mutex_free(e->sub_mu);
}

static inline void __enats_destroyNc(enats e)
{
    is0_ret(e, );

    if(e->conn.nc)
    {
        natsCondition_Destroy(e->conn.nc);
        e->conn.nc = 0;

        __cnt_connDec();
    }
}

static inline void __enats_destroyOpts(enats e)
{
    is0_ret(e, );

    natsOptions_Destroy(e->conn.opts);
    e->conn.opts = 0;
}

static inline void __enats_freeHandle(enats e)
{
    is0_ret(e, );

    if(e->err)      estr_free(e->err);
    if(e->stats)    estr_free(e->stats);
    if(e->connurls) estr_free(e->connurls);
    if(e->urls)     estr_free(e->urls);

    free(e);
}

static inline void __enats_release(enats e)
{
    __enats_destroySubDic(e);
    __enats_destroyNc(e);
    __enats_destroyOpts(e);
    __enats_freeHandle(e);
}

static inline void* __enats_waitThread(void* d)
{
    enats e = (enats)d;

    e->wait_num++;

    mutex_lock(e->wait_mutex);
    mutex_ulck(e->wait_mutex);

    e->wait_num--;

    return 0;
}

static inline void __enats_exeWaitThread(enats e)
{
    is0_ret(e, ); is1_ret(e->quit, ); is1_ret(e->self_node, );

    if(!e->wait_thread)
    {
        mutex_init(e->wait_mutex);
        mutex_lock(e->wait_mutex);
        is1_exeret(thread_init(e->wait_thread, __enats_waitThread, e), errfmt(e, "__enats_exeWaitThread faild: %s", strerror(errno)),);
        e->wait_num++;
        thread_join(e->wait_thread);
        e->wait_num--;
    }
    else
    {
        e->wait_num++;
        mutex_lock(e->wait_mutex);
        mutex_ulck(e->wait_mutex);
        e->wait_num--;
    }
}

static inline void __enats_quitWaitThread(enats e)
{
    is0_ret(e, );
    mutex_ulck(e->wait_mutex);

    while(e->wait_num) usleep(10000);
}

static inline constr __enats_getConnUrls(enats e)
{
    natsConnection* nc = e->conn.nc;

    if(!nc)
        e->connurls = estr_wrb(e->connurls, "", 0);

    natsConn_Lock(nc);
    if ((nc->status == CONNECTED) && (nc->url->fullUrl != NULL))
        e->connurls = estr_wrs(e->connurls, nc->url->fullUrl);
    else
        e->connurls = estr_wrb(e->connurls, "", 0);

    natsConn_Unlock(nc);

    return e->connurls;
}

static inline constr __enats_getUrls(enats e)
{

}

/// ---------------------------- enats  ---------------------
enats enats_new(constr urls)
{
    enats e = NULL;

    is1_exeret(!urls || !*urls, errset(G, "enats_new faild: urls is null or empty");, NULL);

    is0_exeret(e = __enats_newHandle(urls, 0), errfmt(G, "enats_new faild: %s", last_err), 0);

    if(!__enats_tryConnect(e))
    {
        errfmt(G, "enats_new faild: %s", nats_GetLastError(&e->es));
        __enats_release(e);
        e = NULL;
    }

    return e;
}

enats enats_new2(constr user, constr pass, constr url)
{
    estr urls; enats e = 0;

    is1_exe(urls = __enats_makeUrls(user, pass, url, 0), e = enats_new(urls); estr_free(urls););

    return e;
}

enats enats_new3(constr user, constr pass, constr server, int port)
{
    char url[1024]; estr urls; enats e = 0;

    is1_exeret(!server || !*server, errset(G, "enats_new3 faild: server is null or empty");, NULL);

    is1_exeret(snprintf(url, 1024, "%s:%d", server, port) >= 1024, errset(G, "enats_new3 faild: url buf of 1024 overflow");, NULL);

    is1_exe(urls = __enats_makeUrls(user, pass, url, 0), e = enats_new(urls); estr_free(urls););

    return e;
}

enats enats_new4(enats_opts opts)
{
    enats e;

    is0_exeret(_opts, errset(G, "enats_new4 faild: opts is null or empty"), NULL);

    is0_exeret(e = __enats_newHandle(0, opts), errfmt(G, "enats_new4 faild: %s", last_err), 0);

    if(!__enats_tryConnect(e))
    {
        errfmt(G, "enats_new faild: %s", nats_GetLastError(&e->es));
        __enats_release(e);
        e = NULL;
    }

    return e;
}


void enats_join(enats e)
{
    __enats_exeWaitThread(e);
}

void enats_destroy(enats e)
{
    is0_ret(e, )

    e->quit  = 1;

    __enats_quitWaitThread(t);
    __enats_destroySubDic(t);
    __enats_destroyNc(e);           // this call will raise __on_closed, we do __enats_freeHandle() in it
}



inline constr enats_connurls(enats e)
{
    is0_ret(e, "enats_getConnUrls: nullptr");

    return __enats_getConnUrls(e);
}

inline constr enats_urls(enats e)
{
    is0_ret(e, "enats_getUrls: nullptr");

    return __enats_getUrls(e);
}

inline constr enats_name(enats e)
{
    is0_ret(trans, NULL);

    return e->self_node ? e->self_node->name : NULL;
}

inline enatp enats_pool(enats e)
{
    is0_ret(e, NULL);

    return e->self_node ? e->self_node->p : NULL;
}

inline void enats_setClosedCB(enats trans, enats_evtHandler cb, void* closure)
{
    is0_ret(trans, );

    trans->conn.closed_cb       = cb;
    trans->conn.closed_closure  = closure;
}
inline void enats_setDisconnectedCB(enats trans, enats_evtHandler cb, void* closure)
{
    is0_ret(trans, );

    trans->conn.disconnected_cb       = cb;
    trans->conn.disconnected_closure  = closure;
}
inline void enats_setReconnectedCB(enats trans, enats_evtHandler cb, void* closure)
{
    is0_ret(trans, );

    trans->conn.reconnected_cb       = cb;
    trans->conn.reconnected_closure  = closure;
}

inline void enats_setErrHandler(enats trans, enats_errHandler cb, void* closure)
{
    is0_ret(trans, );

    trans->conn.err_handler = cb;
    trans->conn.err_closure = closure;
}

inline natsStatus enats_pub(enats trans, constr subj, conptr data, int data_len)
{
    is0_exeret(trans, last_err = "invalid enats (nullptr)", NATS_ERR);
    trans->conn.s = natsConnection_Publish(trans->conn.nc, subj, data, data_len);
    return trans->conn.s;
}

inline natsStatus  enats_pubReq(enats trans, constr subj, conptr data, int dataLen, constr reply)
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

natsStatus  enats_Sub(enats trans, constr subj, enats_msgHandler onMsg, void* closure)
{
    is0_exeret(trans, errset(G, "invalid enats (nullptr)"), NATS_ERR);
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
    trans->conn.s = natsConnection_Subscribe(&ntSub->sub, trans->conn.nc, subj, __on_msg, ntSub);

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

inline ntStatistics enats_stats(enats trans, constr subj)
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

inline constr enats_statsS(enats trans, int mode, constr subj)
{
    is0_exeret(trans, last_err = "invalid enats (nullptr)", "");

    enats_stats(trans, subj);

    if (trans->conn.s == NATS_OK)
    {
        char*           buf   = trans->conn.stats_buf;
        ntStatistics*   stats =&trans->conn.stats;
        int len = 0;

            len += snprintf(buf + len, 1024 - len, "In Msgs: %9" PRIu64 " - "\
                   "In Bytes: %9" PRIu64 " - ", stats->inMsgs, stats->inBytes);


            len += snprintf(buf + len, 1024 - len, "Out Msgs: %9" PRIu64 " - "\
                   "Out Bytes: %9" PRIu64 " - ", stats->outMsgs, stats->outBytes);


            len += snprintf(buf + len, 1024 - len, "Delivered: %9" PRId64 " - ", stats->deliveredMsgs);
            len += snprintf(buf + len, 1024 - len, "Pending: %5d - ", stats->pendingMsgs);
            len += snprintf(buf + len, 1024 - len, "Dropped: %5" PRId64 " - ", stats->droppedMsgs);

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







static inline void __on_msg(natsConnection* nc __unused, natsSubscription* sub, natsMsg* msg, void *subscriber)
{
    ntSubscriber s = (ntSubscriber)subscriber;

    s->msg_handler(s->t, sub, (eMsg)msg, s->msg_closure);
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
typedef struct enatp_node_s{
    char                 name[64];
    enats                t;
    enatp                p;                     // point to the enatp which have this node

    natsOptions*         opts;                  // back up of opts in t
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
}enatp_node_t, * enatp_node;

typedef struct __attribute__ ((__packed__)) enatp_s{

    // -- for enats manager
    ejson       conn_transs;            // transs connected to server
    ejson       poll_transs;            // transs in polling queue, sure connected
    ejson       lazy_transs;            // transs have not connected to server yet

    ejson       polling_itr;            // point to the ejson obj in poll_transs who is polling now
    enats       polling_now;            // the enats in polling_itr

    ejson       urls;                   // to store all the server urls(with user and pass) that connected or will connected to

    __mutex_t		mutex;

    natsStatus      s;                  // last status

    // -- for quit control
    __thread_t      wait_thread;        // only for wait when needed
    __mutex_t		wait_mutex;
    __thread_t      lazy_thread;
    int             conn_num;           // the num of connected trans
    int             quit;

    // -- other
    estr            err;                // last err
    estr            stats;              // to store statistic info
    estr            connurls;
}enatp_t;

// -- global
static ntStatistics ZERO_STATS;

// -- helpler
static enats_ht_node __enatp_getNode(enatp p, constr name);
static void*         __enatp_waitThread(void*);

// -- micros
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
    __mutex_init(out->mutex);

    out->conn_transs = ejso_new(_OBJ_);
    out->poll_transs = ejso_new(_OBJ_);
    out->lazy_transs = ejso_new(_OBJ_);
    out->urls        = ejso_new(_OBJ_);

    __cnt_poolInc();

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
        enats_destroy(&nt_node->t);    // release trans in CloseCB() of enats
        natsOptions_Destroy(nt_node->opts);
    }

    ejso_free(ebackup);

    // -- release urls
    ejso_free(p->urls); p->urls = 0;

    enatp_unlock(p);

    if(free_self_here)
    {
        if(p->wait_thread)  __mutex_ulck(p->wait_mutex);    // p will be free in enatp_Join()
        else                free(p);

        __cnt_poolDec();
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
        __mutex_init(p->wait_mutex);
        __mutex_lock(p->wait_mutex);
        is1_exeret(__thread_create(p->wait_thread, __enatp_waitThread, p),
                   errfmt(G, "create wait thread for enatp faild: %s", strerror(errno)),
        );

        __thread_join(p->wait_thread);
        free_enatp_here = 1;
    }

    while(p->conn_num) usleep(100000);

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

        trans->conn.s = natsConnection_Subscribe(&ntSub->sub, trans->conn.nc, ntSub->subj, __on_msg, ntSub);

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
    is1_exeret((s = natsOptions_Create(&opts)     ) != NATS_OK, errset(p, nats_GetLastError(&s)), NATS_ERR);
    is1_exeret((s = __processUrlString(opts, urls)) != NATS_OK, errset(p, nats_GetLastError(&s)); goto err_return;, NATS_ERR);

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

        enats_connurls(nt_node->t);
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

    enatp_lock(p);
    if(type == CONN_TRANS || type == ALL_TRANS)
    {
        ejso_itr(p->conn_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt_node->connected_cb      = cb;
            nt_node->connected_closure = closure;
        }
    }
    if(type == LAZY_TRANS || type == ALL_TRANS)
    {
        ejso_itr(p->lazy_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt_node->connected_cb      = cb;
            nt_node->connected_closure = closure;
        }
    }
    enatp_unlock(p);
}

void        enatp_SetClosedCB(enatp p, int type, enats_ClosedCB cb, void* closure)
{
    is0_ret(p, );
    enats_ht_node nt_node; ejson itr;

    enatp_lock(p);
    if(type == CONN_TRANS || type == ALL_TRANS)
    {
        ejso_itr(p->conn_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt_node->closed_cb              = cb;
            nt_node->closed_closure         = closure;
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
    enatp_unlock(p);
}
void        enatp_SetDisconnectedCB(enatp p, int type, enats_DisconnectedCB cb, void* closure)
{
    is0_ret(p, );
    enats_ht_node nt_node; ejson itr;

    enatp_lock(p);
    if(type == CONN_TRANS || type == ALL_TRANS)
    {
        ejso_itr(p->conn_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt_node->disconnected_cb              = cb;
            nt_node->disconnected_closure         = closure;
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
    enatp_unlock(p);
}
void        enatp_SetReconnectedCB(enatp p, int type, enats_ReconnectedCB cb, void* closure)
{
    is0_ret(p, );
    enats_ht_node nt_node; ejson itr;

    enatp_lock(p);
    if(type == CONN_TRANS || type == ALL_TRANS)
    {
        ejso_itr(p->conn_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt_node->reconnected_cb              = cb;
            nt_node->reconnected_closure         = closure;
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
    enatp_unlock(p);
}
void        enatp_SetErrHandler(enatp p, int type, enats_ErrHandler cb, void* closure)
{
    is0_ret(p, );
    enats_ht_node nt_node; ejson itr;

    enatp_lock(p);
    if(type == CONN_TRANS || type == ALL_TRANS)
    {
        ejso_itr(p->conn_transs, itr)
        {
            nt_node = ejso_valP(itr);
            nt_node->err_handler         = cb;
            nt_node->err_closure         = closure;
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
    enatp_unlock(p);
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

    n->closed_cb      = cb;
    n->closed_closure = closure;
}

inline void enatp_SetDisconnectedCBByName(enatp p, constr name, enats_DisconnectedCB cb, void* closure)
{
    enats_ht_node n = __enatp_GetNode(p, name);
    is0_ret(n, );

    enats_SetDisconnectedCB(n->t, cb, closure);

    n->disconnected_cb      = cb;
    n->disconnected_closure = closure;
}

inline void enatp_SetReconnectedCBByName(enatp p, constr name, enats_ReconnectedCB cb, void* closure)
{
    enats_ht_node n = __enatp_GetNode(p, name);
    is0_ret(n, );

    enats_SetReconnectedCB(n->t, cb, closure);

    n->reconnected_cb      = cb;
    n->reconnected_closure = closure;
}

inline void enatp_SetErrHandlerByName(enatp p, constr name, enats_ErrHandler cb, void* closure)
{
    enats_ht_node n = __enatp_GetNode(p, name);
    is0_ret(n, );

    enats_SetErrHandler(n->t, cb, closure);

    n->err_handler = cb;
    n->err_closure = closure;
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
            __conn_dec();

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
        nTPool_lock(p);
        if(p->conn_num > 0)     // p->conn_num will be set only when call nTPool_destroy()
            p->conn_num --;

        if(p->conn_num == 0)
        {
            __pool_dec();

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

static void __on_disconnected(natsConnection* nc __unused, void* trans)
{
    enats t = (enats)trans;

    if(!t->conn.urls)
    {
        strncpy(t->conn.conn_urls, nc->url->fullUrl, 512);
        t->conn.urls = t->conn.conn_urls;
    }

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
