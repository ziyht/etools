#include "ethp.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ejson.h"

/// -- debug info

#define VERSION     "1.2.1"         // fix compat.h on win32

#define _TP_DEBUG_ 0

#if _TP_DEBUG_
static constr _llog_basename(constr path){static constr slash; if (slash) {return slash + 1;}else{slash = strrchr(path, '/');}if (slash) {return slash + 1;}return 0;}
#define log(fmt, ...)   fprintf(stdout, "%s(%d):" fmt "%s", _llog_basename(__FILE__), __LINE__, __VA_ARGS__)
#define llog(...)       log(__VA_ARGS__, "\n");fflush(stdout)
#else
#define llog(...)
#endif

#if (_WIN32)
#define usleep(us) Sleep(us/1000)
#define memccpy    _memccpy
#endif

#if 0
/// --------------- using uv.h to make compatible mutex and thread
#include "uv.h"
typedef uv_mutex_t  mutex_t;
typedef uv_thread_t thread_t;
typedef uv_cond_t   cond_t;

#define mutex_init(m)   uv_mutex_init(&m)
#define mutex_free(m)   uv_mutex_destroy(&m)
#define mutex_lock(m)   uv_mutex_lock(&m)
#define mutex_ulck(m)   uv_mutex_unlock(&m)

#define thread_init(t, cb, d)  uv_thread_create(&t, cb, d)
#define thread_join(t)         uv_thread_join(&t)

#define cond_init(c)    uv_cond_init(&c)
#define cond_wait(c, m) uv_cond_wait(&c, &m)
#define cond_one(c)     uv_cond_signal(&c)
#define cond_all(c)     uv_cond_broadcast(&c)
#define cond_free(c)    uv_cond_destroy(&c)

#if (_WIN32)
#define thread_detach(t)
#define thread_free(t)          CloseHandle(t)
#else
#include <unistd.h>
#define thread_detach(t)       pthread_detach(t)
#endif
#else
#define  COMPAT_THREAD_MAIN
#include "compat.h"
#endif

/// -- 
#define is0_ret(cond, ret) if(!(cond)) return ret
#define is1_ret(cond, ret) if( (cond)) return ret

#define is0_exeret(cond, expr, ret) if(!(cond)){ expr; return ret;}
#define is1_exeret(cond, expr, ret) if( (cond)){ expr; return ret;}
#define is0_elsret(cond, expr, ret) if(!(cond)){expr;} else return ret
#define is1_elsret(cond, expr, ret) if( (cond)){expr;} else return ret

/// ----------------------- thread_pool ---------------- 

typedef struct task_s* TASK;

typedef struct thread_handle_s{
    char         id[16];
    TP           tp;
    thread_t     th;
    int          status;
    int          quit;
    
    ejson        self;
}thread_handle_t, * TH;

typedef struct task_s{
    char      tag[16];
    TP_cb     oprt;
    TP_cb     after_oprt;
    void*     data;
}task_t;

typedef struct thread_pool_s{
    int          status;
    
    int          thrds_cap;     // the upper limit of threads
    int          thrds_idx;     // threads idx
    int          thrds_num;     // threads num
    int          thrds_idle;
    ejson        thrds;         // running threads
    ejson        thrds_wait;    // waiting threads, only for quit thread using
    mutex_t      thrds_mu;
    cond_t       thrds_co;
    
    ejson        tasks;         // all tasks
    ejson        tasks_tag;     // task tags
    ejson        tasks_buf;     // task buffer
    
    int          quit_join;
    int          quit_join_ths;
    int          quit_ths_run_num;
    thread_t     quit_th;
    mutex_t      quit_mu;
    mutex_t      free_mu;

}thread_pool_t;

enum {
    _RELEASED         = 0,
    _INITIALING,
    _INITED,
    _RELEASE_REQED,
    _RELEASING,
    _WAITING,
    _RUNNING,
    _QUIT_REQED,
    _QUITING,
    _QUITED,
    _DETACHED
};

#define MAX_THREAD_NUM  4
#define MAX_TASKS_BUFF  1024

static void _tp_quit_cb(void* data);

TP   TP_New()
{
    TP tp; 
    
    is0_ret(tp = calloc(1, sizeof(*tp)), 0);
    
    tp->status        = _INITIALING;
    
    tp->thrds         = ejso_new(_OBJ_);
    tp->thrds_wait    = ejso_new(_ARR_);
    tp->tasks         = ejso_new(_ARR_);
    tp->tasks_tag     = ejso_new(_OBJ_);
    tp->tasks_buf     = ejso_new(_ARR_);
    
    mutex_init(tp->thrds_mu);
    mutex_init(tp->quit_mu);
    mutex_init(tp->free_mu);
    
    cond_init(tp->thrds_co);
    
    mutex_lock(tp->quit_mu);
    
    if(!tp->thrds || !tp->tasks || !tp->thrds_wait || !tp->tasks_tag || !tp->tasks_buf)
        goto err_ret;
    
    thread_init(tp->quit_th, _tp_quit_cb, tp);
    if(!tp->quit_th)
        goto err_ret;
    
    tp->thrds_cap = MAX_THREAD_NUM;
    
    tp->status    = _INITED;
    
    return tp;
    
err_ret:
    
    ejso_free(tp->thrds);
    ejso_free(tp->thrds_wait);
    ejso_free(tp->tasks);
    ejso_free(tp->tasks_tag);
    ejso_free(tp->tasks_buf);
    
    mutex_free(tp->quit_mu);
    mutex_free(tp->thrds_mu);
 //   cond_free(tp->quit_co);
    cond_free(tp->thrds_co);
    
    free(tp);
    
    return 0;
}

void TP_Destroy(TP tp, int opt)
{   
    is1_ret(tp->status != _INITED, ); 
    
    //mutex_lock(tp->quit_mu);
    
    tp->status        = _RELEASE_REQED;
    tp->quit_join_ths = opt & TP_JOIN_THREADS;
        
    mutex_ulck(tp->quit_mu);
    //cond_all(tp->quit_co);
    
    llog("[threadpool]: _RELEASE_REQED");
} 

void TP_SetThreadsNum(TP tp, int max)
{
    is0_ret(tp, );
    
    if(max > 1) tp->thrds_cap = max ;
}


void TP_Join(TP tp)
{
    is0_ret(tp, );
    
    if(tp->status == _RELEASING || tp->status == _RELEASE_REQED || tp->status == _INITED)
    {
        tp->quit_join = 1;
        thread_join(tp->quit_th);
    }
}

static void _tp_quit_cb(void* data)
{
    TP tp; ejson itr; TH th; TASK t; thread_t tp_quit_th;
    
    tp = (TP)data; 
    tp_quit_th = tp->quit_th;
    
    while(tp->status != _RELEASED)
    {
        mutex_lock(tp->quit_mu);
        
        llog("[threadpool]: %s", "quit thread ulck");
        if(tp->status != _RELEASE_REQED) continue;
        llog("[threadpool]: %s", "_RELEASE_REQED recieved");
        tp->status = _RELEASING;

        mutex_lock(tp->thrds_mu);

        // -- release all unruning tasks
        llog("[threadpool]: %s", "releasing tasks");
        ejso_itr(tp->tasks, itr)
        {
            t = ejso_valP(itr);
            free(t);
        }
        ejso_itr(tp->tasks_buf, itr)
        {
            t = ejso_valP(itr);
            free(t);
        }
        ejso_free(tp->tasks);      tp->tasks      = 0;
        ejso_free(tp->tasks_tag);  tp->tasks_tag = 0;
        ejso_free(tp->tasks_buf);  tp->tasks_buf = 0;
        
        // -- set quit_tag for all threads and seperate waiting threads
        ejso_itr(tp->thrds, itr)
        {
            th = ejso_valP(itr);
            
            th->quit = _QUIT_REQED;
            
            if(th->status == _WAITING)
            {
                assert(ejso_addO(tp->thrds_wait, th->id, ejso_rmR(tp->thrds, th->id)));
            }
            else if(!tp->quit_join_ths)
            {
                thread_detach(th->th);
                th->quit = _DETACHED;
                llog("[thread%s]: detached", th->id);
            }
        }
        
        tp->quit_ths_run_num = ejso_len(tp->thrds) + 1;     // all running threads and self
        
        if(!tp->quit_join_ths)
        {
            ejso_free(tp->thrds); tp->thrds = 0;
        }
        
        mutex_ulck(tp->thrds_mu);
        
        // -- release waiting threads
        llog("[threadpool]: %s", "releasing waiting threads");
        cond_all(tp->thrds_co);
        ejso_itr(tp->thrds_wait, itr)
        {
            th = ejso_valP(itr);
            thread_join(th->th);
            free(th);
        }
        ejso_free(tp->thrds_wait);    tp->thrds_wait = 0;
        
        // -- waiting for running tasks if needed
        if(tp->quit_join_ths)
        {
            llog("[threadpool]: %s", "join inner threads");
            ejso_itr(tp->thrds, itr)
            {
                th = ejso_valP(itr);
                thread_join(th->th);
                free(th);
            }
            ejso_free(tp->thrds); tp->thrds = 0;
        }
        
        tp->status = _RELEASED;
        mutex_ulck(tp->quit_mu);
        break;
    }

    mutex_lock(tp->free_mu);
    tp->quit_ths_run_num--;
    if(tp->quit_ths_run_num == 0)
    {
        // printf("free tp in quiting thread\n"); fflush(stdout);
        free(tp);
    }else
        mutex_ulck(tp->free_mu);
    
    thread_detach(tp_quit_th);
}

static void _thread_cb(void* _th)
{
    TH      th    = _th;
    TP      tp    = th->tp;
    ejson   te    = 0;
    TASK    t;
    char    tag[16];
#if (_WIN32)
	thread_t _th_backup = 0;
#endif

    while(1)
    {
        mutex_lock(tp->thrds_mu);
        if(th->quit)
        {
            th->status = _QUITING;
            mutex_ulck(tp->thrds_mu);
            break;
        }
        
        // -- waiting signal
        while(0 == ejso_len(tp->tasks))
        {
            th->status = _WAITING;
            tp->thrds_idle++;
            llog("[thread%s]: waiting", th->id);
            cond_wait(tp->thrds_co, tp->thrds_mu);
            llog("[thread%s]: wake up", th->id);
            tp->thrds_idle--;
            
            if(th->quit)
            {
                th->status = _QUITING;
                mutex_ulck(tp->thrds_mu);
                goto quit_waiting_thread;
            }
        }
        assert(te = ejso_pop(tp->tasks));
        mutex_ulck(tp->thrds_mu);
        
        // -- running tasks
        th->status = _RUNNING;

run_task:
        t = ejso_valP(te);
        strncpy(tag, t->tag, 16);
        llog("[thread%s]: run %s.oprt", th->id, t->tag);
        t->oprt(t->data);                
        if(t->after_oprt){
            llog("[thread%s]: run %s.after_oprt", th->id, t->tag);
            t->after_oprt(t->data);
        }
        llog("[thread%s]: run %s over", th->id, t->tag);
        
        // -- quit check
        if(th->quit)
        {
            free(t);
            free(te);
            th->status = _QUITING;
            break;
        }
        
        // -- delta task
        if(tp->tasks)
        {
            mutex_lock(tp->thrds_mu);
            
            if(ejso_len(tp->tasks_buf) > MAX_TASKS_BUFF)
            {
                free(t);
                free(te);
            }
            else
            {
                memset(t, 0, sizeof(*t));
                ejso_addO(tp->tasks_buf, 0, te);
            }
            
            if(tag[0])
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
                ejso_freeR(tp->tasks_tag, tag);
                #endif
            }
            
            if((te = ejso_pop(tp->tasks)))
            {
                mutex_ulck(tp->thrds_mu);
                llog("[thread%s]: get task %s", th->id, ((TASK)ejso_valP(te))->tag);
                goto run_task;
            }

            mutex_ulck(tp->thrds_mu);
        }
        else
        {
            free(t);
            free(te);
            break;
        }   
    }
#if (_WIN32)
	_th_backup = tp->quit_ths_run_num ? 0 : th->th;
#endif
    mutex_lock(tp->free_mu);
    tp->quit_ths_run_num--;
    if(0 == tp->quit_ths_run_num)
    {
        // printf("free tp in thread%s\n", th->id); fflush(stdout);
        free(tp);
    }else
        mutex_ulck(tp->free_mu);

quit_waiting_thread:
    if(th->quit == _DETACHED)
    {
        llog("[thread%s]: free self and quited", th->id);
        free(th);
    }
    else
    {   
        llog("[thread%s]: quited", th->id);
        th->status = _QUITED;
    }

#if (_WIN32)
	if(_th_backup) thread_quit(_th_backup);
#endif
}

int  TP_AddTask(TP tp, const char* tag, TP_cb oprt, TP_cb after_oprt, void* data)
{
    TH th = 0; TASK t; ejson te, rete;
    
    is1_ret(!oprt || tp->status != _INITED, 0); 
    
    // make new task
    mutex_lock(tp->thrds_mu);
    if(!(te = ejso_pop(tp->tasks_buf)))
    {
        is0_exeret(t = calloc(1, sizeof(*t)), mutex_ulck(tp->thrds_mu);, 0);
        is0_exeret(ejso_addP(tp->tasks_buf, 0, t), mutex_ulck(tp->thrds_mu); free(t);, 0);
        te = ejso_pop(tp->tasks_buf);
    }
    t = ejso_valP(te);
    
    if(tag && *tag)
    {
        rete = ejso_addT(tp->tasks_tag, tag, _TRUE_);
        
        if(0 == rete)
        {
            llog("[threadpool]: have a task named \"%s\" already, %s, %d", tag, ejse_str(), ejso_len(tp->tasks_tag));
            if(!ejso_addO(tp->tasks_buf, 0, te)) {ejso_free(te); free(t);}
            mutex_ulck(tp->thrds_mu);
            return 0;
        }
        
        memccpy(t->tag, tag, '\0', 15);
    }
    
    t->oprt       = oprt;
    t->after_oprt = after_oprt;
    t->data       = data;
    
    assert(ejso_addO(tp->tasks, 0, te));    // add task to list
    llog("[threadpool]: add new task [%s] ok", (tag && *tag) ? tag : "");
    mutex_ulck(tp->thrds_mu);
    
    // -- check idle threads
    if(tp->thrds_idle)
    {
        cond_one(tp->thrds_co);     // wake up one thread to run the task
    }
    else
    {
        if(tp->thrds_num >= tp->thrds_cap)
        {
            llog("[threadpool]: create new thread failed, reach max threads cnt");
            return 0;
        }
        else
        {
            if((th = calloc(1, sizeof(*th))))
            {
                th->tp     = tp;
                if(thread_init(th->th, _thread_cb, th) == -1)
                {
                    free(th);
                    llog("[threadpool]: create new thread failed, %s", strerror(errno));
                    return 0;
                }
                else
                {
                    tp->thrds_num++;
                    tp->thrds_idx++;
                    snprintf(th->id, 16, "%d", tp->thrds_idx);
                    mutex_lock(tp->thrds_mu);
                    assert(ejso_addP(tp->thrds, th->id, th));
                    mutex_ulck(tp->thrds_mu);
                    cond_one(tp->thrds_co);
                    llog("[threadpool]: create new thread%s", th->id);
                }
            }
        }
    }
    
    return 1;
}

int  TP_QueryTask(TP tp, const char* tag)
{
    int ret;
    mutex_lock(tp->thrds_mu);
    
    ret = ejsr(tp->tasks_tag, tag) ? 1 : 0;    
    
    mutex_ulck(tp->thrds_mu);

    return ret;
}
