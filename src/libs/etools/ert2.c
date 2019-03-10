/// =====================================================================================
///
///       Filename:  ert.c
///
///    Description:  easy routine on threadpool, do not like routine, this is a threadpool
///                  actually
///
///        Version:  1.0
///        Created:  03/09/2017 08:51:34 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ecompat.h"
#include "ethread.h"
#include "ert2.h"
#include "ejson.h"

//#define EUTILS_LLOG 1
#include "eutils.h"

/// ----------------------- thread_pool ----------------

typedef struct task_s* TASK;

typedef struct thread_handle_s{
    char         id[16];
    ert          tp;
    thread_t     th;
    int          status;
    int          quit;

    ejson        self;
}thread_handle_t, * TH;

typedef struct task_s{
    char      tag[16];
    ert_cb    oprt;
    ert_cb    after_oprt;
    void*     data;
}task_t;

typedef struct thread_pool_s{
    int          status;

    // -- threads manager
    int          thrds_cap;     // the upper limit of threads
    int          thrds_idx;     // threads idx
    int          thrds_num;     // threads num
    int          thrds_idle;
    ejson        thrds;         // running threads
    ejson        thrds_wait;    // waiting threads, only using it in quit operation
    mutex_t      thrds_mu;
    cond_t       thrds_co;

    // -- tasks manager
    ejson        tasks;         // all tasks
    ejson        tasks_tags;    // task tags
    ejson        tasks_cache;   // task cache
    mutex_t      tasks_mu;

    // -- quit manager
    int          quit_join_ths;
    int          quit_ths_run_num;
    thread_t     quit_th;
    mutex_t      quit_mu;

    // --
    mutex_t      wait_mu;
    cond_t       wait_co;
    volatile int wait_num;
}thread_pool_t;

enum {
    // -- ert status
    _RELEASED         = 0,
    _INITIALING,
    _INITED,
    _RELEASE_REQED,
    _RELEASING,

    // -- thread status
    _WAITING,
    _RUNNING,
    _QUIT_WAITING_REQED,
    _QUIT_RUNNING_REQED,
    _QUIT_RUNNING_DETACHED,
    _QUITING,
    _QUITED,
};

#define _DFT_THREAD_NUM  4
#define _DFT_TASKS_BUFF  256

static ert _df_tp;
static int _df_thread_num = _DFT_THREAD_NUM;

static void _etp_quit_cb(void* _tp);

/// -------------------------- ert internal -----------------------

static void __ert_self_init(ert tp)
{
    mutex_init(tp->thrds_mu);
    mutex_init(tp->tasks_mu);
    mutex_init(tp->quit_mu);
    mutex_init(tp->wait_mu);

    cond_init(tp->thrds_co);
}

static void __ert_self_release(ert tp)
{
    mutex_free(tp->thrds_mu);
    mutex_free(tp->tasks_mu);
    mutex_free(tp->quit_mu);
    mutex_free(tp->wait_mu);

    cond_free(tp->thrds_co);

    free(tp);
}

static ert __ert_new(int max_thread_num)
{
    ert tp;

    is0_ret(tp = calloc(1, sizeof(*tp)), 0);

    tp->status        = _INITIALING;

    tp->thrds         = ejso_new(_OBJ_);
    tp->thrds_wait    = ejso_new(_ARR_);
    tp->tasks         = ejso_new(_ARR_);
    tp->tasks_tags    = ejso_new(_OBJ_);
    tp->tasks_cache   = ejso_new(_ARR_);

    if(!tp->thrds || !tp->tasks || !tp->thrds_wait || !tp->tasks_tags || !tp->tasks_cache)
        goto err_ret;

    __ert_self_init(tp);

    tp->thrds_cap = max_thread_num == 0 ? _DFT_THREAD_NUM : max_thread_num;

    tp->status    = _INITED;

    return tp;

err_ret:

    ejso_free(tp->thrds);
    ejso_free(tp->thrds_wait);
    ejso_free(tp->tasks);
    ejso_free(tp->tasks_tags);
    ejso_free(tp->tasks_cache);

    __ert_self_release(tp);

    return 0;
}

static inline void __ert_exeWait(ert tp)
{
    while(tp->status != _RELEASED)
    {
        mutex_lock(tp->wait_mu);
        tp->wait_num++;
        cond_wait(tp->wait_co, tp->wait_mu);
        tp->wait_num--;
        mutex_ulck(tp->wait_mu);
    }
}

static inline void __ert_quitWait(ert tp)
{
    while(tp->wait_num)
    {
        cond_all(tp->wait_co);
        usleep(10000);
    }
}

static inline void __ert_task_cache(ert tp, ejson te, TASK t)
{
    mutex_lock(tp->thrds_mu);

    if(t->tag[0])
    {
#if _TP_DEBUG_
        if(ejso_len(tp->tasks_tag) == 0)   // this should not happen
        {
            llog("--------------- err ----------------");   // make a breakpoint here
        }

        ejso_freeR(tp->tasks_tag, tag);

        cstr s;
        if(ejsr(tp->tasks_tag, tag))
        {    llog("[thread%s]: rm %s failed, %d, %s", th->id, tag, ejso_len(tp->tasks_tag), s = ejso_toUStr(tp->tasks_tag));ejss_free(s);}
        else
        {    llog("[thread%s]: rm %s ok, %d, %s", th->id, tag, ejso_len(tp->tasks_tag), s = ejso_toUStr(tp->tasks_tag));ejss_free(s);}
#else
        ejso_freeR(tp->tasks_tags, t->tag);
#endif
    }

    if(ejso_len(tp->tasks_cache) >= _DFT_TASKS_BUFF)
        ejso_free(te);
    else
    {
        memset(t, 0, sizeof(*t));
        ejso_addO(tp->tasks_cache, 0, te);
    }

    mutex_ulck(tp->thrds_mu);
}

static inline int __ert_task_add(ert tp, constr tag, ert_cb oprt, ert_cb after_oprt, cptr arg)
{
    ejson te, rete; TASK t;

    //mutex_lock(tp->tasks_mu);
    mutex_lock(tp->thrds_mu);

    if(tag && *tag)
    {
        rete = ejso_addT(tp->tasks_tags, tag, _TRUE_);
        if(0 == rete)
        {
            llog("[threadpool]: have a task named \"%s\" already, %s, %d", tag, ejson_err(), ejso_len(tp->tasks_tags));
            //mutex_ulck(tp->tasks_mu);
            mutex_ulck(tp->thrds_mu);
            return 0;
        }
    }

    if(ejso_len(tp->tasks_cache)) te = ejso_pop(tp->tasks_cache);
    else                          te = ejso_new(EJSON_TYPE(_RAW_, sizeof(task_t)));

    t = ejso_valR(te);
    t->oprt       = oprt;
    t->after_oprt = after_oprt;
    t->data       = arg;

    if(tag) strncpy(t->tag, tag, 16);

    te = ejso_addO(tp->tasks, 0, te);
    assert(te);
    llog("[threadpool]: add new task [%s] ok, %d tasks now", (tag && *tag) ? tag : "", ejso_len(tp->tasks));

    //mutex_ulck(tp->tasks_mu);
    mutex_ulck(tp->thrds_mu);

    return 1;
}

static inline ejson __ert_task_pop(ert tp)
{
    ejson te;

    mutex_lock(tp->thrds_mu);

    te = ejso_pop(tp->tasks);

    mutex_ulck(tp->thrds_mu);

    return te;
}

static inline void __ert_task_release(ert tp)
{
    TASK t; ejson itr;

    llog("[threadpool]: %s", "releasing tasks");

    mutex_lock(tp->thrds_mu);

    ejso_itr(tp->tasks, itr)
    {
        t = ejso_valP(itr);
        free(t);
    }
    ejso_itr(tp->tasks_cache, itr)
    {
        t = ejso_valP(itr);
        free(t);
    }
    ejso_free(tp->tasks);      tp->tasks       = 0;
    ejso_free(tp->tasks_tags); tp->tasks_tags  = 0;
    ejso_free(tp->tasks_cache);tp->tasks_cache = 0;

    mutex_lock(tp->thrds_mu);
}

static inline void __ert_thread_run_task(ert tp, TH th)
{
    ejson te; TASK t;

    te = __ert_task_pop(tp);

    while(te)
    {
        t = ejso_valR(te);

        llog("[thread%s]: run %s.oprt", th->id, t->tag);
        t->oprt(t->data);

        if(t->after_oprt)
        {
            llog("[thread%s]: run %s.after_oprt", th->id, t->tag);
            t->after_oprt(t->data);
        }

        llog("[thread%s]: run %s over", th->id, t->tag);

        if(th->quit)
        {
            ejso_free(te);
            break;
        }
        else
            __ert_task_cache(tp, te, t);

        te = __ert_task_pop(tp);
#if _DEBUG_
        if(te) llog("[thread%s]: continue task %s", th->id, ((TASK)ejso_valP(te))->tag);
#endif
    }
}

static inline void __ert_thread_run_task2(ert tp, TH th)
{
    ejson te; TASK t;

    te = __ert_task_pop(tp);

    if(te)
    {
        t = ejso_valR(te);

        llog("[thread%s]: run %s.oprt", th->id, t->tag);
        t->oprt(t->data);

        if(t->after_oprt)
        {
            llog("[thread%s]: run %s.after_oprt", th->id, t->tag);
            t->after_oprt(t->data);
        }

        llog("[thread%s]: run %s over", th->id, t->tag);

        if(th->quit)
            ejso_free(te);
        else
            __ert_task_cache(tp, te, t);
    }
}

static void _task_thread(void* _th)
{
    TH th; ert tp; char id[16];

#ifdef _WIN32_THREAD
    thread_t _th_backup = 0;
#endif

    th = _th;
    tp = th->tp;

    while(1)
    {
        mutex_lock(tp->thrds_mu);
        if(th->quit)
        {
            mutex_ulck(tp->thrds_mu);
            goto quiting;
        }

        // -- waiting signal
        while(0 == ejso_len(tp->tasks))
        {
            th->status = _WAITING;
            tp->thrds_idle++;

            llog("[thread%s]: waiting", th->id);
            cond_wait(tp->thrds_co, tp->thrds_mu);
            llog("[thread%s]: wake up", th->id);

            if(th->quit)
            {
                mutex_ulck(tp->thrds_mu);
                goto quiting;
            }

            tp->thrds_idle--;
        }

        th->status = _RUNNING;
        mutex_ulck(tp->thrds_mu);

        __ert_thread_run_task2(tp, th);
    }

quiting:

    th->status = _QUITING;

    mutex_lock(tp->quit_mu);

    strncpy(id, th->id, 16);

    switch (th->quit) {
        case _QUIT_WAITING_REQED   : llog("[thread%s]: waiting free self and quited", th->id);
                                     tp->quit_ths_run_num--;
#ifdef _WIN32_THREAD
                                     _th_backup = th->th;
#endif
                                     free(th);
                                     break;
        case _QUIT_RUNNING_REQED   :
        case _QUIT_RUNNING_DETACHED: tp->quit_ths_run_num--;

                                     if(th->quit == _QUIT_RUNNING_REQED)
                                     {
                                         llog("[thread%s]: running quited", th->id);
                                     }
                                     else
                                     {
                                         llog("[thread%s]: running free self and quited", th->id);
#ifdef _WIN32_THREAD
                                         _th_backup = th->th;
#endif
                                         free(th);
                                     }

                                     break;
    }

    if(0 == tp->quit_ths_run_num)
    {
        if(!tp->quit_join_ths)
        {
            __ert_task_release(tp);
            __ert_quitWait(tp);

            llog("[threadpool] free tp in thread%s \n", id);
            __ert_self_release(tp);
        }
        else
            mutex_ulck(tp->quit_mu);
    }
    else
        mutex_ulck(tp->quit_mu);

#ifdef _WIN32_THREAD
    if(_th_backup) thread_quit(_th_backup);
#endif
}

static inline void __ert_thread_create_if_need(ert tp)
{
    TH th;

    if(tp->thrds_idle)
        return;

    if(tp->thrds_num >= tp->thrds_cap)
    {
        llog("[threadpool]: create new thread failed, reach max threads cnt");
        return ;
    }

    if((th = calloc(1, sizeof(*th))))
    {
        th->tp     = tp;

        if(thread_init(th->th, _task_thread, th) != 0)
        {
            free(th);
            llog("[threadpool]: create new thread failed, %s", strerror(errno));
            return;
        }

        tp->thrds_num++;
        tp->thrds_idx++;
        snprintf(th->id, 16, "%d", tp->thrds_idx);
        ejso_addP(tp->thrds, th->id, th);

        llog("[threadpool]: create new thread%s", th->id);
    }
}

static inline void __ert_thread_wakeup_one(ert tp)
{
    cond_one(tp->thrds_co);
}

static inline void __ert_release_waiting_threads(ert tp)
{
    ejson itr; TH th;

    tp->quit_ths_run_num = ejso_len(tp->thrds);

    // -- seperate waiting threads
    ejso_itr(tp->thrds, itr)
    {
        th = ejso_valP(itr);

        if(th->status == _WAITING)
        {
            th->quit = _QUIT_WAITING_REQED;
            assert(ejso_addO(tp->thrds_wait, th->id, ejso_rmR(tp->thrds, th->id)));
        }
    }

    // -- release waiting threads
    llog("[threadpool]: %s", "releasing waiting threads");
    cond_all(tp->thrds_co);
    ejso_itr(tp->thrds_wait, itr)
    {
        th = ejso_valP(itr);
        thread_detach(th->th);
    }

    ejso_free(tp->thrds_wait);    tp->thrds_wait = 0;
}

static inline void __ert_release_running_threads(ert tp)
{
    ejson itr; TH th;

    if(tp->quit_ths_run_num && ejso_len(tp->thrds))
    {
        ejso_itr(tp->thrds, itr)
        {
            th = ejso_valP(itr);

            if(tp->quit_join_ths)
            {
                th->quit = _QUIT_RUNNING_REQED;
            }
            else
            {
                thread_detach(th->th);
                th->quit = _QUIT_RUNNING_DETACHED;
                llog("[thread%s]: detached", th->id);
            }
        }

        llog("[threadpool]: %s", "releasing running threads");
        cond_all(tp->thrds_co);
        if(tp->quit_join_ths)
        {
            llog("[threadpool]: %s", "join running threads");
            mutex_ulck(tp->quit_mu);		// unlock so all the threads(including waiting threads) would quiting now
            ejso_itr(tp->thrds, itr)
            {
                th = ejso_valP(itr);
                thread_join(th->th);
                free(th);
            }
            mutex_lock(tp->quit_mu);
        }
    }
    else
        tp->quit_join_ths = 0;				// no running threads to join

    ejso_free(tp->thrds); tp->thrds = 0;
}

static inline void __ert_releasing(ert tp)
{
    mutex_lock(tp->quit_mu);				// note: all threads will blocking until unlock this mutex

    __ert_release_waiting_threads(tp);
    __ert_release_running_threads(tp);

    mutex_ulck(tp->quit_mu);

    if(tp->quit_join_ths)
    {
        __ert_task_release(tp);
        __ert_quitWait(tp);

        llog("[threadpool] free tp in __ert_releasing() \n");
        __ert_self_release(tp);

    }
}

/// ---------------------- creator -------------------------

ert  ert_new(int max_thread_num)
{
    return __ert_new(max_thread_num);
}

void ert_join(ert tp)
{
    is1_exe(tp == DEFAULT_ERT, tp = _df_tp);
    is0_ret(tp, );

    is1_ret(tp->status != _INITED, );

    __ert_exeWait(tp);
}


void ert_destroy(ert tp, int opt)
{
    is1_exe(tp == DEFAULT_ERT, tp = _df_tp);
    is0_ret(tp, );

    is1_ret(tp->status != _INITED, );

    tp->status        = _RELEASING;
    tp->quit_join_ths = opt & ERT_WAITING_RUNNING_TASKS;

    llog("[threadpool]: _RELEASING");
    __ert_releasing(tp);

    if(tp == _df_tp) _df_tp = 0;
}

void ert_maxThread(ert tp, int num)
{
    is0_ret(tp && num > 0, );
    is1_exe(tp == DEFAULT_ERT, tp = _df_tp);
    is0_exeret(tp, _df_thread_num = num;, );

    tp->thrds_cap = num ;
}

int ert_run(ert tp, constr tag, ert_cb oprt, ert_cb after_oprt, cptr arg)
{
    is1_ret(!tp || !oprt, 0);

    if(tp == DEFAULT_ERT)
    {
        is0_exe(_df_tp, _df_tp = ert_new(_df_thread_num));
        is0_ret(_df_tp, 0);

        tp = _df_tp;
    }

    is1_ret(tp->status != _INITED, 0);
    is0_ret(__ert_task_add(tp, tag, oprt, after_oprt, arg), 0);

    __ert_thread_create_if_need(tp);
    __ert_thread_wakeup_one(tp);

    return 1;
}

/// @brief ert_query - to query a task whether is in the threadpool or not
///
/// @param tp  - the pointor to a threadpool returned by TP_New()
/// @param tag - the tag of the task you want to query
/// @return [0] - this task is running over or haven't been added to the threadpool
///         [1] - this task is running or in the waiting list
///
int  ert_query(ert tp, constr tag)
{
    int ret;

    is1_exe(tp == DEFAULT_ERT, tp = _df_tp);
    is0_ret(tp, 0);

    mutex_lock(tp->tasks_mu);

    ret = ejsr(tp->tasks_tags, tag) ? 1 : 0;

    mutex_ulck(tp->tasks_mu);

    return ret;
}
