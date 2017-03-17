/// =====================================================================================
///
///       Filename:  etimer.c
///
///    Description:  a easier timer to run task
///
///        Version:  1.0
///        Created:  03/13/2017 11:00:34 AM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "etimer.h"
#include "ejson.h"
#include "ert.h"
#include "esl.h"

#define  COMPAT_THREAD
#include "compat.h"

#define ETIMER_VERSION "etimer 1.0.0"       // new tool

#define __DEBUG__ 0

#if __DEBUG__
static constr _llog_basename(constr path){static constr slash; if (slash) {return slash + 1;}else{slash = strrchr(path, '/');}if (slash) {return slash + 1;}return 0;}
#define log(fmt, ...)   fprintf(stdout, "%s(%d):" fmt "%s", _llog_basename(__FILE__), __LINE__, __VA_ARGS__)
#define llog(...)       log(__VA_ARGS__, "\n");fflush(stdout)
#else
#define llog(...)
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

/// -- helper -------------------------------------------

typedef enum {
  _CLOCK_PRECISE = 0,  /* Use the highest resolution clock available. */
  _CLOCK_FAST = 1      /* Use the fastest clock with <= 1ms granularity. */
} clocktype_t;

/* Available from 2.6.32 onwards. */
#ifndef CLOCK_MONOTONIC_COARSE
# define CLOCK_MONOTONIC_COARSE 6
#endif

static inline u64 __hrtime_ns(clocktype_t type)
{
    static clock_t fast_clock_id = -1;
    struct timespec t;
    clock_t clock_id;

    /* Prefer CLOCK_MONOTONIC_COARSE if available but only when it has
     * millisecond granularity or better.  CLOCK_MONOTONIC_COARSE is
     * serviced entirely from the vDSO, whereas CLOCK_MONOTONIC may
     * decide to make a costly system call.
     */
    /* TODO(bnoordhuis) Use CLOCK_MONOTONIC_COARSE for UV_CLOCK_PRECISE
     * when it has microsecond granularity or better (unlikely).
     */
    if (type == _CLOCK_FAST && fast_clock_id == -1) {
      if (clock_getres(CLOCK_MONOTONIC_COARSE, &t) == 0 &&
          t.tv_nsec <= 1 * 1000 * 1000) {
        fast_clock_id = CLOCK_MONOTONIC_COARSE;
      } else {
        fast_clock_id = CLOCK_MONOTONIC;
      }
    }

    clock_id = CLOCK_MONOTONIC;
    if (type == _CLOCK_FAST)
      clock_id = fast_clock_id;

    if (clock_gettime(clock_id, &t))
    {
        fast_clock_id = CLOCK_REALTIME;

        if(clock_gettime(CLOCK_REALTIME, &t))
        {
            perror("clock_gettime not working correctly");
            abort();        /* Not really possible. */
        }
    }

    llog("%lu: %ld", clock_id, t.tv_sec * 1000000000 + t.tv_nsec);

    return t.tv_sec * 1000000000 + t.tv_nsec;
}

static inline u64 __hrtime_ms(clocktype_t type)
{
    return __hrtime_ns(type) / 1000000;
}

/// -- struct -------------------------------------------

struct etloop_s
{
    ert         tp;         // threadpool for running tasks

    int         quit;

    cond_t      co_wait;
    mutex_t     mu_wait;

    mutex_t     mu;
    thread_t    tl_th;
    esl         tl;         // timeline of tasks
   // u64         nextdl;     // next deadline to run tasks
};

typedef struct _etimer_s{
    cptr    arg;

    etloop  loop;

    u64     mydl;
    u64     repeat;
    etm_cb  cb;

    uint    active   : 1;
    uint    running  : 1;
    uint    free     : 1;
    uint    cancle   : 1;
}_etimer_t, * _etimer;


static etloop _df_loop;
static int    _df_thrdsnum = 4;

/// -- etloop -------------------------------------------

static inline u64 __nowms()
{
    return __hrtime_ns(_CLOCK_FAST) / 1000000;
}

static inline u64 __nextdl(s64 ms)
{
    return ms < 1 ? __hrtime_ns(_CLOCK_FAST) / 1000000
                  : __hrtime_ns(_CLOCK_FAST) / 1000000 + ms;
}

static inline void __nextspec(struct timespec* abstime, s64 ms)
{
    clock_gettime(CLOCK_REALTIME, abstime);

    abstime->tv_sec  += ms / 1000;
    abstime->tv_nsec += ms % 1000 * 1000000;

    if(abstime->tv_nsec > 1000000000)
    {
        abstime->tv_sec  += 1;
        abstime->tv_nsec %= 1000000000;
    }
}

static void __task_after_cb(_etimer e)
{
    e->running = 0;

    if(e->cancle)
    {
        if(e->free) free(e);

        return;
    }

    if(e->mydl)
    {
        etloop loop = e->loop;

        mutex_lock(loop->mu);

        e->active = 1;
        esl_insertO(loop->tl, e->mydl, e);
        llog("readd timer: %d %p", esl_len(loop->tl), (void*)e);
        llog("ulck and cond_all");
        mutex_ulck(loop->mu_wait);
        cond_all(loop->co_wait);
        mutex_ulck(loop->mu);
    }
}

static void* __tl_th_cb(void* arg)
{
    s64 wait_ms; esln first; _etimer e; struct timespec abstime; u64 now;

    etloop loop = (etloop)arg;

    while(!loop->quit)
    {
        if(!(first = esl_first(loop->tl)))
        {
            llog("lock");
            //mutex_lock(loop->mu_wait);
            __nextspec(&abstime, 100);
            cond_twait(loop->co_wait, loop->mu_wait, abstime);
            //cond_wait(loop->co_wait, loop->mu_wait);
            //mutex_ulck(loop->mu_wait);
            llog("lock over");
            mutex_ulck(loop->mu_wait);
            continue;
        }

        do{
            wait_ms = first->score - __nowms();

            llog("first: %ld now: %ld, break!", first->score, __nowms());

            if(wait_ms > 0)
            {
                __nextspec(&abstime, wait_ms);
                llog("cond_twait: wait %ld ms", wait_ms);
                cond_twait(loop->co_wait, loop->mu_wait, abstime);
                llog("cond_twait over");
                if(loop->quit) return 0;
            }
            else
                break;

        }while((first = esl_first(loop->tl)));

        //if(!first)
          //  continue;

        mutex_lock(loop->mu);

        now    = __nowms();
        do{
            if((u64)first->score > now)
            {
                llog("first: %ld now: %ld, break!", first->score, now);
                break;
            }

            e = first->obj;

            llog("----- timer: %d %p", esl_len(loop->tl), (void*)e);

            e->active = 0;
            if(!e->cancle)
            {
                if(!e->running)
                {
                    int repeat = e->repeat;

                    e->mydl    = repeat ? first->score + (((now - first->score) / repeat) + 1) * repeat : 0;
                    e->running = 1;
                    llog("run timer: %p", (void*)e);
                    ert_run(loop->tp, 0, (ert_cb)e->cb, (ert_cb)__task_after_cb, e);

                }
                else
                {
                    llog("skip  timer: %d %p", esl_len(loop->tl), (void*)e);
                }
            }
            else if(e->free)
            {
                free(e);
            }

            esl_freeH(loop->tl);

        }while((first = esl_first(loop->tl)));

        mutex_ulck(loop->mu);
    }

    return 0;
}

etloop etloop_new(int maxthread)
{
    etloop loop;

    if(maxthread < 4)  maxthread = 4;

    is0_ret(loop = calloc(1, sizeof(*loop)), NULL);

    loop->tp  = ert_new(maxthread);
    loop->tl  = esl_new(0);

    if(!loop->tp || !loop->tl || thread_init(loop->tl_th, __tl_th_cb, loop))
    {
        ert_release(loop->tp, 0);
        esl_free(loop->tl);

        free(loop);
        loop = 0;
    }

    mutex_init(loop->mu);
    mutex_init(loop->mu_wait);
    cond_init(loop->co_wait);

    return loop;
}

etloop etloop_df(int maxthread)
{
    if(maxthread > 4) _df_thrdsnum = maxthread;

    if(!_df_loop) return _df_loop = etloop_new(_df_thrdsnum);
    else          ert_maxThread(_df_loop->tp, _df_thrdsnum);

    return _df_loop;
}

void   etloop_stop(etloop loop)
{
    esln itr, tmp; _etimer e;

    is0_ret(loop, );

    if(loop == _df_loop) _df_loop = 0;

    loop->quit   = 1;
    ert_release(loop->tp, ERT_WAITING_TASKS);
    ert_join(loop->tp);
    llog("ulck and cond_all");
    mutex_ulck(loop->mu_wait);
    cond_all(loop->co_wait);
    mutex_lock(loop->mu);

    thread_join(loop->tl_th);

    esl_itr2(loop->tl, itr, tmp)
    {
        llog("free etimer: %p %d", itr->obj, esl_len(loop->tl));
        free(itr->obj);
    }
    mutex_ulck(loop->mu);

    esl_free(loop->tl);

    free(loop);
}

etimer etimer_new(etloop loop)
{
    _etimer e;

    is0_ret(loop, NULL);
    is0_ret(e = calloc(1, sizeof(*e)), NULL);

    e->loop = loop;

    return (etimer)e;
}

void   etimer_destroy(etimer _e)
{
    _etimer e = (_etimer)_e;

    is0_exe(e, );

    if(e->active)
    {
        e->repeat = 0;
        e->cancle = 1;
        e->free   = 1;
    }
    else
        free(e);
}

int    etimer_start(etimer _e, etm_cb cb, u64 timeout, u64 repeat)
{
    _etimer e; etloop loop;

    is1_ret(!_e || !cb, 0);

    e = (_etimer)_e;

    is1_ret(e->active, 0);

    loop = e->loop;
    mutex_lock(loop->mu);

    e->cb     = cb;
    e->mydl   = __nextdl(timeout);
    e->repeat = repeat;

    if(!e->running)
    {
        e->active = 1;
        esl_insertO(loop->tl, e->mydl, e);
    }

    llog("ulck and cond_all: %ld", e->mydl);
    mutex_ulck(loop->mu_wait);
    cond_all(e->loop->co_wait);

    mutex_ulck(e->loop->mu);

    return 1;
}

int    etimer_stop (etimer _e)
{
    _etimer e = (_etimer)_e;

    is0_ret(_e, 0);

    if(e->active)
        e->cancle = 1;

    return 1;
}

static inline u64 __etimer_ns()
{
    static int _init_;
    static s64 offset;

    if(!_init_)
    {
        _init_ = 1;

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        offset = (int64_t)ts.tv_sec * 1000000000 + (int64_t)ts.tv_nsec - __hrtime_ns(_CLOCK_FAST);
    }

    return __hrtime_ns(_CLOCK_FAST)  + offset;
}

u64    etimer_now()
{
    return __etimer_ns() / 1000000;
}

cstr   etimer_nowS(cstr buf, int len)
{
    struct tm time; u64 now_ns; time_t sec;

    now_ns = __etimer_ns();
    sec    = now_ns / 1000000000;

    localtime_r(&sec, &time);

    len < 17 ? snprintf(buf, len, "%4d%02d%02d%02d%02d%02d ", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec)
             : snprintf(buf, len, "%4d%02d%02d%02d%02d%02d.%09ld", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec, now_ns % 1000000000);

    return buf;
}
