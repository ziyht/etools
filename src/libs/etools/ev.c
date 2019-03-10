
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <sys/ioctl.h>
#include <event.h>

#include "ev.h"
#include "ert.h"
#include "ejson.h"

#define _DEBUG_          0

#define exe_ret(expr, ret ) { expr;      return ret;}
#define is0_ret(cond, ret ) if(!(cond)){ return ret;}
#define is1_ret(cond, ret ) if( (cond)){ return ret;}
#define is0_exe(cond, expr) if(!(cond)){ expr;}
#define is1_exe(cond, expr) if( (cond)){ expr;}

#define is0_exeret(cond, expr, ret) if(!(cond)){ expr;        return ret;}
#define is1_exeret(cond, expr, ret) if( (cond)){ expr;        return ret;}
#define is0_elsret(cond, expr, ret) if(!(cond)){ expr;} else{ return ret;}
#define is1_elsret(cond, expr, ret) if( (cond)){ expr;} else{ return ret;}

#if _DEBUG_
static constr _llog_basename(constr path){static constr slash; if (slash) {return slash + 1;}else{slash = strrchr(path, '/');}if (slash) {return slash + 1;}return 0;}
#define log(fmt, ...)   fprintf(stdout, "%s(%d):" fmt "%s", _llog_basename(__FILE__), __LINE__, __VA_ARGS__)
#define llog(...)       log(__VA_ARGS__, "\n");fflush(stdout)
#else
#define llog(...)
#endif

enum {
    _RELEASED         = 0,
    _INITIALING,
    _INITED,
    _RELEASING
};

typedef struct _iwatcher_s{
    int    wd;           // inotify watcher fd
    cstr   path;         // watcher path
    ejson  handles;      // dict of handles who watche this path
    int    iterating;
}_iwatcher_t, * _iwatcher_node;

struct ev_handle_s{
    ev_loop         loop;
    int             active;
    void*           data;
};

typedef struct event_base* event_base;
typedef struct event       event_t;
typedef struct ev_loop_s{
    event_base eb;          // for timer
    int        i_fd;        // inotify fd
    event_t    i_event;     // inotify event in libevent
    ejson      i_watchers;  // dict of inotify wathers

    ert        tp;
}ev_loop_t;

static event_base _df_eb_;
static ev_loop    _df_ev_loop_;
static int        _ev_loop_num_;
static int        _df_thread_num = 4;


#define _find_watcher(loop, wd) dict_getp(loop->i_watchers, wd)
#define _add_watcher(loop, w)   dict_addp(loop->i_watchers, w->wd, w)
#define _del_watcher(loop, w)   dict_del(loop->i_watchers, w)

int     ev_set_thread_num(uint num)
{
    _df_thread_num = num > 1024 ? 1024 : num;
    return 1;
}

ev_loop ev_default_loop()
{
    if(!_df_ev_loop_)
    {
        _df_ev_loop_ = ev_loop_new(_df_thread_num);
    }

    return _df_ev_loop_;
}

ev_loop ev_loop_new(int thread_num)
{
    ev_loop out;

    if((out = calloc(1, sizeof(*out))))
    {
        if(!(out->eb = event_init()))
        {
            free(out);
            return 0;
        }

        out->tp   = ert_new(thread_num);
        out->i_fd = -1;

        _ev_loop_num_++;
    }

    return out;
}

int ev_run(ev_loop loop, int mode)
{
    int out;

    is0_ret(loop, -2);

    out = event_base_loop(loop->eb, mode);

    return out;
}

int ev_stop(ev_loop loop)
{
    is0_ret(loop, -1);  ejson itr; _iwatcher_node w;

    if(loop->i_fd > 0)
    {
        event_del(&loop->i_event);
        close(loop->i_fd);
        loop->i_fd = -1;
    }

    if(loop->i_watchers)
    {
        ejso_itr(loop->i_watchers, itr)
        {
            w = ejso_valP(itr);
            ejso_free(w->handles);
            free(w);
        }

        ejso_free(loop->i_watchers);
        loop->i_watchers = 0;
    }

    event_base_loopexit(loop->eb, 0);
    _ev_loop_num_--;

    ert_destroy(loop->tp, 0);
    ert_join(loop->tp);

    free(loop);
    if(loop == _df_ev_loop_)
    {
        _df_eb_      = 0;
        _df_ev_loop_ = 0;
    }

    return 0;
}

int     ev_is_active(ev_handle h)
{
    return h ? h->active : 0;
}

int ev_fs_event_init(ev_fs_event handle, ev_loop loop)
{
    if(!loop || !handle)    return -1;

    memset(handle, 0, sizeof(*handle));
    handle->loop = loop;
    handle->active = 0;

    return 0;
}

#define swap(type, a, b) {type t; t = a; a = b; b = t;}
enum {
    _IN_CHANGE  = IN_MODIFY|IN_ATTRIB,
    _IN_DELETED = IN_MOVED_FROM|IN_MOVED_TO|IN_CREATE|IN_DELETE|IN_DELETE_SELF|IN_MOVE_SELF|IN_UNMOUNT
};
static void __ev_inotify_read_cleanup(void** _d)
{
    ejson d, itr; _iwatcher_node w;
    d = *_d;

    ejso_itr(d, itr)
    {
        w = ejso_valP(itr);
        ejso_free(w->handles);
        free(w);
    }

    ejso_free(d);
}

static void ev_inotify_read( void* _loop)
{
    char buf[4096]; ssize_t size;_iwatcher_node w;int events; char key[16];
    const struct inotify_event* e;
    const char* p, * path;
    ev_fs_event h;
    ejson handles = 0; ejson itr;
    ev_loop loop = (ev_loop)_loop;

    pthread_cleanup_push(__ev_inotify_read_cleanup, &handles);
    while(1)
    {
        do
            size = read(loop->i_fd, buf, sizeof(buf));
        while(size == -1 && errno == EINTR);

        if (size == -1)
        {
            assert(errno == EAGAIN || errno == EWOULDBLOCK);
            break;
        }

        assert(size > 0);

        if(!handles && !(handles = ejso_new(_OBJ_)))  continue;

        for(p = buf; p < buf + size; p += sizeof(*e) + e->len)
        {
            e = (const struct inotify_event*)p;

            snprintf(key, 16, "%d", e->wd);
            w = ejsr_valP(loop->i_watchers, key);
            if(!w)  continue;

            events = 0;
            if(e->mask & _IN_CHANGE )   events |= EV_CHANGE;
            if(e->mask & _IN_DELETED)   events |= EV_RENAME;

            path = e->len ? (char*)(e + 1) : (path = strrchr(w->path, '/')) ? path + 1 : w->path;

            w->iterating = 1;

            swap(ejson, w->handles, handles);

            ejso_itr(handles, itr)
            {
                h = ejso_valP(itr);
                ejso_addO(w->handles, 0, ejso_rmO(handles, itr));

                if(h->cb) h->cb(h, path, events, 0);
            }

            w->iterating = 0;

            if(ejso_len(w->handles) == 0)   // user may stop watcher handle in cb, so this maybe 0 in sometime, in this case, we need to free this watcher
            {
                ejso_free(w->handles);                  // free dict in this watcher
                inotify_rm_watch(loop->i_fd, w->wd);    // rm watch event from loop

                snprintf(key, 16, "%d", w->wd);
                ejso_freeR(loop->i_watchers, key);
                free(w);                                // free self
            }
        }
    }
    pthread_cleanup_pop(1);
    return ;
}

static void i_event_cb(int i_fd , short type , void* _loop)
{
    ert_run(((ev_loop)_loop)->tp, 0, ev_inotify_read, 0, _loop);
}

int ev_fs_event_start(ev_fs_event handle, ev_fs_event_cb cb, const char* path, uint flags )
{
    ev_loop loop; int i_fd = -1, wd = -1; _iwatcher_node w; int i = 1; char key[16];

    is1_ret(!handle || !handle->loop || handle->active, -1);

    loop = handle->loop;

    if(loop->i_fd == -1)
    {
        is1_ret((i_fd = inotify_init()) == -1, -1);

        // -- make ifd nonblok
        is1_exeret(!loop->i_watchers && !(loop->i_watchers = ejso_new(_OBJ_)), close(i_fd), 0);
        loop->i_fd = i_fd;
        ioctl(i_fd, FIOCLEX);
        ioctl(i_fd, FIONBIO, &i);
        event_set(&loop->i_event, i_fd, EV_READ|EV_PERSIST, i_event_cb, loop);
        is1_ret(event_base_set(loop->eb, &loop->i_event  ) == -1, -1);
        is1_ret(evtimer_add   (&loop->i_event  , 0) == -1, -1);
    }

    is1_ret((wd = inotify_add_watch(loop->i_fd, path, _IN_CHANGE|_IN_DELETED))==-1, 0);

    sprintf(key, "%d", wd);
    if((w = ejsr_valP(loop->i_watchers, key)))   goto skip_insert;

    is0_ret(w = calloc(1, sizeof(*w) + strlen(path) + 1), -1);
    is0_exeret(w->handles = ejso_new(_OBJ_), free(w), -1);

    w->wd = wd;
    w->path = strcpy((char*)(w+1), path);

    ejso_addP(loop->i_watchers, key, w);

skip_insert:
    handle->path = w->path;
    handle->wd   = wd;
    handle->cb   = cb;

    do{
        srand(time(0));
        handle->identifier = rand();
        sprintf(key, "%d", handle->identifier);
    }while(!ejso_addP(w->handles, key, handle));

    handle->active = 1;

    return 0;
}

int ev_fs_event_stop(ev_fs_event handle)
{
    _iwatcher_node w; char key[16];

    is1_ret(!handle || !handle->active, -1);

    sprintf(key, "%d", handle->wd);
    w = ejsr_valP(handle->loop->i_watchers, key);
    assert(w != NULL);

    handle->wd   = -1;
    handle->path = NULL;
    handle->active = 0;

    sprintf(key, "%d", handle->identifier);
    ejso_freeR(w->handles, key);

    return 0;
}

int ev_fs_event_getpath(ev_fs_event handle, char* buffer, size_t* size) {
  size_t required_len;

  if (!handle->active) {
    *size = 0;
    return 0;
  }

  required_len = strlen(handle->path);
  if (required_len >= *size) {
    *size = required_len + 1;
    return 0;
  }

  memcpy(buffer, handle->path, required_len);
  *size = required_len;
  buffer[required_len] = '\0';

  return 0;
}

int ev_timer_init(ev_timer handle, ev_loop loop)
{
    is1_ret(!handle || !loop, -1);

    memset(handle, 0, sizeof(*handle));
    handle->loop   = loop;
    handle->active = 0;

    return 0;
}

static void eb_timer_run(void* _handle)
{
    ev_timer handle = _handle;
    if(handle->cb)  handle->cb(_handle);
    handle->running = 0;
    return ;
}

static void eb_timer_cb(int fd , short tag , void* _handle)
{
    ev_timer handle = _handle;

    llog("event timer: interval: %ld", handle->interval);

    if(handle->interval)    event_add(&handle->timer, &handle->timeout);
    else                    handle->active = 0;

    if(!handle->running)
    {
        handle->running = 1;
        //ert_run(handle->loop->tp, 0, eb_timer_run, 0, _handle);
        eb_timer_run(_handle);
    }
}

int ev_timer_start(ev_timer handle, ev_timer_cb cb, uint64_t timeout, uint64_t interval)
{
    is1_ret(!handle || !handle->loop || handle->active, -1);

    struct timeval tv = {0, 0};

    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = timeout % 1000 * 1000;

    handle->interval        = interval;
    handle->timeout.tv_sec  = interval / 1000;
    handle->timeout.tv_usec = interval % 1000 * 1000;
    handle->cb              = cb;

    if(!handle->timer.ev_base)
    {
        event_set(&handle->timer, -1, 0, eb_timer_cb, handle);
        is1_ret(event_base_set(handle->loop->eb, &handle->timer  ) == -1, -1);
    }

    is1_ret(event_add     (&handle->timer  , &tv             ) == -1, -1);

    llog("[timer]: %8lu, %8lu", handle->timer.ev_timeout.tv_sec, handle->timer.ev_timeout.tv_usec);

    handle->active = 1;

    return 0;
}

int ev_timer_stop(ev_timer handle)
{
    is1_ret(!handle || !handle->active, -1);

    evtimer_del(&handle->timer);
    handle->interval = 0;
    handle->active   = 0;

    return 0;
}
