

#ifndef __EV_H__
#define __EV_H__
#include <pthread.h>
#include <sys/inotify.h>
#include <event.h>

#define _EV_THREDLIZE_        1

typedef struct ev_loop_s*     ev_loop;
typedef struct ev_fs_event_s* ev_fs_event;
typedef struct ev_timer_s*    ev_timer;
typedef struct ev_handle_s*   ev_handle, ev_handle_t;

typedef void (*ev_fs_event_cb)(ev_fs_event handle, const char* filename, int event, int status);
typedef struct ev_fs_event_s{
    ev_loop         loop;
    int             active;
    void*           data;
    ev_fs_event_cb  cb;

    int             wd;
    int             identifier;
    char*           path;
}ev_fs_event_t;

enum {
    EV_RENAME = 1,
    EV_CHANGE = 2
};

typedef void (*ev_timer_cb)(ev_timer handle);
typedef struct ev_timer_s{
    ev_loop         loop;
    int             active;
    void*           data;
    ev_timer_cb     cb;

    int             running;
    struct event    timer;
    struct timeval  timeout;
    uint64_t        interval;
}ev_timer_t;

typedef enum{
    EV_RUN_DEFAULT = 0,
    EV_RUN_ONCE,
    EV_RUN_NOWAIT
}ev_run_mode;

int     ev_set_thread_num(uint num);

ev_loop ev_default_loop();
ev_loop ev_loop_new(int thread_num);
int     ev_run(ev_loop loop, int mode);
int     ev_stop(ev_loop loop);
int     ev_is_active(ev_handle h);

int ev_fs_event_init(ev_fs_event handle, ev_loop loop);
int ev_fs_event_start(ev_fs_event handle, ev_fs_event_cb cb, const char* path, uint flags);
int ev_fs_event_stop(ev_fs_event handle);
int ev_fs_event_getpath(ev_fs_event handle, char* buffer, size_t* size);

int ev_timer_init(ev_timer handle, ev_loop loop);
int ev_timer_start(ev_timer handle, ev_timer_cb cb, uint64_t timeout, uint64_t interval);
int ev_timer_stop(ev_timer handle);

#endif
