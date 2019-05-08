/// =====================================================================================
///
///       Filename:  epopen.c
///
///    Description:  new popen operations in linux
///
///        Version:  1.0
///        Created:  12/14/2017 17:08:34 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#include <stdlib.h>
#include <poll.h>
#include <stdio.h>
#include <sys/wait.h>

#include "epopen.h"
#include "ethread.h"
#include "eutils.h"
#include "estr.h"
#include "elog.h"

typedef struct _eio_s{
    FILE*   ifp;
    FILE*   ofp;
    pid_t   pid;
    uint    is_closing: 1;
    uint    is_killed : 1;
    uint    non_block : 1;
    int     ifd;
    int     ofd;
    cstr    pos;
    estr    buf;
    emutex_t mu;
}_eio_t, *_eio;

static _eio _epopen(constr cmd)
{
    _eio out;
    int pipefd1[2] = {0,0},
        pipefd2[2] = {0,0},
        pid;

    out = calloc(1, sizeof(*out));

    is0_ret(out, 0);

    if(pipe(pipefd1) < 0 || pipe(pipefd2) < 0)
        goto err_ret;

    pid = fork();

    is1_ret(pid < 0, 0);

    if(0 == pid)    // child
    {
        close(pipefd1[0]); dup2(pipefd1[1], 1); close(pipefd1[1]);
        close(pipefd2[1]); dup2(pipefd2[0], 0); close(pipefd2[0]);

        char* argv[] = {"/bin/sh", "-c", (cstr)cmd, NULL };

        is1_exe(execvp("/bin/sh", argv) < 0, exit(1));
    }

    close(pipefd1[1]);
    close(pipefd2[0]);

    out->ifp = fdopen(pipefd2[1], "w");
    out->ofp = fdopen(pipefd1[0], "r");
    out->pid = pid;
    out->ifd = -1;
    out->ofd = -1;

    emutex_init(out->mu);

    return out;

err_ret:

    close(pipefd1[0]); close(pipefd1[1]);
    close(pipefd2[0]); close(pipefd2[1]);

    free(out);

    return 0;
}

static int _epclose(_eio _e)
{
    int status;

    emutex_lock(_e->mu);

    if(_e->is_closing)
    {
        emutex_ulck(_e->mu);
        return -2;
    }

    _e->is_closing = 1;
    emutex_ulck(_e->mu);

    waitpid(_e->pid, &status, 0);   _e->pid = 0;

    fclose(_e->ifp); _e->ifp = 0;
    fclose(_e->ofp); _e->ofp = 0;

    free(_e);

    return WEXITSTATUS(status);
}

static int _system_kill_pid(pid_t pid)
{

#if 0
    char kill_cmd[128];
    snprintf(kill_cmd, 128, "kill -9 %d", pid);
    system(kill_cmd);
#else
    int retval = kill(pid, SIGKILL);

    if ( retval )
    {
        perror("kill faild in _system_kill_pid()");
    }
#endif

    return 0;
}

static int _epkill(_eio _e)
{
    emutex_lock(_e->mu);

    if(_e->is_killed)
    {
        emutex_ulck(_e->mu);
        return -3;
    }

    _e->is_killed = 1;
    _system_kill_pid(_e->pid);

    emutex_ulck(_e->mu);

    return 0;
}

static int _epkillclose(_eio _e)
{
    int status; int killed;

    emutex_lock(_e->mu);

    if(_e->is_closing)
    {
        if(_e->is_killed)
        {
            emutex_ulck(_e->mu);
            return -3;
        }

        _e->is_killed = 1;

        _system_kill_pid(_e->pid);

        emutex_ulck(_e->mu);
        return -2;
    }

    killed = _e->is_killed;

    _e->is_closing = 1;
    _e->is_killed  = 1;
    emutex_ulck(_e->mu);

    if(!killed)
    {
        _system_kill_pid(_e->pid);
    }
    waitpid(_e->pid, &status, 0); _e->pid = 0;

    fclose(_e->ifp); _e->ifp = 0;
    fclose(_e->ofp); _e->ofp = 0;

    free(_e);

    return WEXITSTATUS(status);
}


eio epopen(constr cmd)
{
    return (cmd && *cmd) ? (eio)_epopen(cmd)
                         : 0;
}

int epclose    (eio e)
{
    return e ? _epclose((_eio)e) : -10;
}

int epkill     (eio e)
{
    return e ? _epkill((_eio)e) : -10;
}

int epkillclose(eio e)
{
    return e ? _epkillclose((_eio)e) : -10;
}

int eio_gets(eio e, cstr buf, int len)
{
    return eio_tgets(e, buf, len, -1);
}
/**
 * @brief eio_tgets
 * @param _e
 * @param buf
 * @param len
 * @param timeout
 * @return  1 - ok
 *          0  - timeout
 *          -1 - feof
 *
 */
int eio_tgets(eio _e, cstr buf, int len, int timeout)
{
    _eio e; cstr ret; *buf = '\0';

    is0_ret(_e, 0);

    e = (_eio)_e;

    if(e->ofd < 0 && (e->ofd = fileno(e->ofp)) < 0)
    {
        elog_err(1, "fileno err: %s", strerror(errno));
    }

    if(!e->non_block && e->ofd >= 0)
    {
        if(0 == fcntl(e->ofd, F_SETFL, O_NONBLOCK))
            e->non_block = 1;
    }

    if(fgets(buf, len, e->ofp))
        return 1;

    struct pollfd pollfds = { e->ofd, POLLIN|POLLPRI, 0};

    while(1)
    {
        switch(poll(&pollfds, 1, timeout))
        {
            case  0: return 0;      // timeout
            case -1: continue;      // err

            default: break;
        }

        switch (pollfds.revents) {
            case POLLIN :
            case POLLPRI: return fgets(buf, len, e->ofp) ? 1 : 0;

            case POLLHUP | POLLIN:
            case POLLHUP | POLLPRI:
            case POLLHUP: e->is_killed = 1;
                          return fgets(buf, len, e->ofp) ? 1 : -1;

            default     : e->is_killed = 1;
                          elog_err(1, "poll unkown ret: %d", pollfds.revents);
                          return fgets(buf, len, e->ofp) ? 1 : -1;
        }
    }

    return 0;
}

//int eio_online (eio e, void (*cb)(cstr), int timeout)
//{
//    char line[40960];

//    is0_ret(e, 0);

//    while(1)
//    {
//        int ret = eio_tgets(e, line, 40960, 1000);

//        switch (ret) {
//            // ok
//            case  1:   __parse_slurm_job_from_squeue_line(line);
//                      continue;

//            // timeout
//            case  0:  if((s64)etimer_now() - query->last_start >= query->timeout)
//                      {
//                          ret = -88;
//                          query->state = TIMEOUT;
//                      }
//                      break;

//            // eof
//            case -1:  ret = -88;
//                      break;
//        }

//        if(ret == -88)
//            break;
//    }
//}

//int eio_oncline (eio e, void (*cb)(cstr), int timeout)
//{
//    query->efd = 0;
//    while(1)
//    {
//        int ret = eio_tgets(efd, line, 40960, 1000);

//        switch (ret) {
//            // ok
//            case  1: if(line[0] == '\0' || line[1] == '\0')
//                     {
//                        // skip continuous line
//                        if(estr_len(buf) == 0)
//                            continue;

//                            __parse_slurm_queue_from_scontrl_buf(buf);
//                            estr_clear(buf);
//                            continue;
//                      }

//                      buf = estr_catS(buf, line);
//                      continue;

//            // timeout
//            case  0:  if((s64)etimer_now() - query->last_start >= query->timeout)
//                      {
//                          estr_clear(buf);
//                          query->state = TIMEOUT;
//                          ret = -88;
//                      }
//                      break;

//            // eof
//            case -1:  ret = -88;
//                      break;
//        }

//        if(ret == -88)
//            break;
//    }
//    if(estr_len(buf) > 0)
//        __parse_slurm_queue_from_scontrl_buf(buf);

//    epkillclose(efd);
//}
