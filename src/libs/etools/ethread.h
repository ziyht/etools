/// =====================================================================================
///
///       Filename:  ethread.h
///
///    Description:  a header file to compat thread for different platform, especially
///                  between linux and windows
///
///                  the pthread lib in libs are rebuild from mingwin
///
///                  the compat thread funcs in this file are rebuild from libuv (Incompletely)
///
///        Version:  1.0
///        Created:  09/14/2017 14:47:25 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#ifndef __ETHREAD_H__
#define __ETHREAD_H__

#define ETHREAD_VERSION "ethread 1.0.3"      // add ETHREAD_NOT_USING_PTHREAD_MINGEWIN check

#ifdef   ETHREAD_NOT_USING_PTHREAD_MINGEWIN
#undef   ETHREAD_USING_PTHREAD_MINGEWIN
#endif

#ifndef ETHREAD_USING_PTHREAD_MINGEWIN

#if defined(_WIN32)
#define  _WIN32_THREAD_DEF
#define  _WIN32_THREAD
#include <process.h>
#include <Windows.h>
#include "etype.h"
typedef void *(*__start_routine) (void *);

typedef union {
    CONDITION_VARIABLE cond_var;
    struct {
        unsigned int     waiters_count;
        CRITICAL_SECTION waiters_count_lock;
        HANDLE           signal_event;
        HANDLE           broadcast_event;
    } fallback;
}co_cond_t;

typedef CRITICAL_SECTION        mutex_t;
typedef co_cond_t				cond_t;
typedef HANDLE					thread_t;

static int  co_cond_init(cond_t* c);
static void co_cond_wait(cond_t* c, mutex_t* m);
static void co_cond_twait(cond_t* c, mutex_t* m, uint ms);
static void co_cond_signal(cond_t* c);
static void co_cond_broadcast(cond_t* c);
static void co_cond_destroy(cond_t* c);

static int  co_thread_create(thread_t *tid, void* (*entry)(void *arg), void *arg);
static int  co_thread_join(thread_t* tid);

#define mutex_init(m)           (InitializeCriticalSection(&(m)), 0)
#define mutex_lock(m)           EnterCriticalSection(&(m))
#define mutex_ulck(m)           LeaveCriticalSection(&(m))
#define mutex_free(m)           DeleteCriticalSection(&(m))

#define cond_init(c)            co_cond_init(&(c))
#define cond_wait(c, m)         co_cond_wait(&(c), &(m))
#define cond_twait(c, m, t)     co_cond_twait(&(c), &(m), (t))
#define cond_one(c)             co_cond_signal(&(c))
#define cond_all(c)             co_cond_broadcast(&(c))
#define cond_free(c)            co_cond_destroy(&(c))

#define thread_init(t, cb, d)   co_thread_create(&t, (__start_routine)cb, d)
#define thread_join(t)          co_thread_join(&t)
#define thread_detach(t)
#define thread_quit(t)          CloseHandle(t)

#endif
#endif

#if defined(ETHREAD_USING_PTHREAD_MINGEWIN) || !defined(_WIN32)
#include <time.h>
#include "pthread.h"
#include "etype.h"
#include "ecompat.h"
typedef void *(*__start_routine) (void *);

typedef pthread_mutex_t         mutex_t;
typedef pthread_cond_t          cond_t;
typedef pthread_t               thread_t;

static void co_cond_twait(cond_t* c, mutex_t* m, uint abstime);

#define mutex_init(m)           pthread_mutex_init(&(m), NULL)
#define mutex_lock(m)           pthread_mutex_lock(&(m))
#define mutex_ulck(m)           pthread_mutex_unlock(&(m))
#define mutex_free(m)           pthread_mutex_destroy(&(m))

#define cond_init(c)            pthread_cond_init(&(c), NULL)
#define cond_wait(c, m)         pthread_cond_wait(&(c), &(m))
#define cond_twait(c, m, t)     co_cond_twait(&(c), &(m), (t));
#define cond_one(c)             pthread_cond_signal(&(c))
#define cond_all(c)             pthread_cond_broadcast(&(c))
#define cond_free(c)            pthread_cond_destroy(&(c))

#define thread_init(t, cb, d)   pthread_create(&(t), NULL, (__start_routine)(cb), (d))
#define thread_join(t)          pthread_join((t), NULL)
#define thread_detach(t)        pthread_detach((t))
#define thread_quit(t)          pthread_cancel(t)

static inline void co_cond_twait(cond_t* c, mutex_t* m, uint ms)
{
    struct timespec abstime;

    clock_gettime(CLOCK_REALTIME, &abstime);

    abstime.tv_sec  += ms / 1000;
    abstime.tv_nsec += ms % 1000 * 1000000;

    if(abstime.tv_nsec > 1000000000)
    {
        abstime.tv_sec  += 1;
        abstime.tv_nsec %= 1000000000;
    }

    pthread_cond_timedwait(c, m, &abstime);
}

#endif  // _WIN32

#if defined(_WIN32_THREAD_DEF)

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "eerrno.h"

#ifndef __IMPORT_KERNEL32_LIB
#define __IMPORT_KERNEL32_LIB
#pragma comment(lib, "Kernel32.lib")
#endif

static void co_fatal_error(const int errorno, const char* syscall) {
    char* buf = NULL;
    const char* errmsg;

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorno,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, NULL);

    if (buf) {	errmsg = buf;}
    else	 {	errmsg = "Unknown error";}

    /* FormatMessage messages include a newline character already, */
    /* so don't add another. */
    if (syscall) {	fprintf(stderr, "%s: (%d) %s", syscall, errorno, errmsg);}
    else		 {	fprintf(stderr, "(%d) %s", errorno, errmsg);}

    if (buf) {	LocalFree(buf);	}

    *((char*)NULL) = 0xff; /* Force debug break */
    abort();
}

/* Kernel32 function typedefs */
typedef BOOL (WINAPI *sGetQueuedCompletionStatusEx)
             (HANDLE CompletionPort,
              LPOVERLAPPED_ENTRY lpCompletionPortEntries,
              ULONG ulCount,
              PULONG ulNumEntriesRemoved,
              DWORD dwMilliseconds,
              BOOL fAlertable);

typedef BOOL (WINAPI* sSetFileCompletionNotificationModes)
             (HANDLE FileHandle,
              UCHAR Flags);

typedef BOOLEAN (WINAPI* sCreateSymbolicLinkW)
                (LPCWSTR lpSymlinkFileName,
                 LPCWSTR lpTargetFileName,
                 DWORD dwFlags);

typedef BOOL (WINAPI* sCancelIoEx)
             (HANDLE hFile,
              LPOVERLAPPED lpOverlapped);

typedef VOID (WINAPI* sInitializeConditionVariable)
             (PCONDITION_VARIABLE ConditionVariable);

typedef BOOL (WINAPI* sSleepConditionVariableCS)
             (PCONDITION_VARIABLE ConditionVariable,
              PCRITICAL_SECTION CriticalSection,
              DWORD dwMilliseconds);

typedef BOOL (WINAPI* sSleepConditionVariableSRW)
             (PCONDITION_VARIABLE ConditionVariable,
              PSRWLOCK SRWLock,
              DWORD dwMilliseconds,
              ULONG Flags);

typedef VOID (WINAPI* sWakeAllConditionVariable)
             (PCONDITION_VARIABLE ConditionVariable);

typedef VOID (WINAPI* sWakeConditionVariable)
             (PCONDITION_VARIABLE ConditionVariable);

typedef BOOL (WINAPI* sCancelSynchronousIo)
             (HANDLE hThread);

typedef DWORD (WINAPI* sGetFinalPathNameByHandleW)
             (HANDLE hFile,
              LPWSTR lpszFilePath,
              DWORD cchFilePath,
              DWORD dwFlags);

/* Kernel32 function pointers */
static sGetQueuedCompletionStatusEx			pGetQueuedCompletionStatusEx;
static sSetFileCompletionNotificationModes	pSetFileCompletionNotificationModes;
static sCreateSymbolicLinkW					pCreateSymbolicLinkW;
static sCancelIoEx							pCancelIoEx;
static sInitializeConditionVariable			pInitializeConditionVariable;
static sSleepConditionVariableCS			pSleepConditionVariableCS;
static sSleepConditionVariableSRW			pSleepConditionVariableSRW;
static sWakeAllConditionVariable			pWakeAllConditionVariable;
static sWakeConditionVariable				pWakeConditionVariable;
static sCancelSynchronousIo					pCancelSynchronousIo;
static sGetFinalPathNameByHandleW			pGetFinalPathNameByHandleW;

static void co__thread_once_init()
{
    static int _co_thread_init_guard = 0;	HMODULE kernel32_module;

    if(_co_thread_init_guard)	return; _co_thread_init_guard = 1;

    kernel32_module = GetModuleHandleA("kernel32.dll");
    if (kernel32_module == NULL)
        co_fatal_error(GetLastError(), "GetModuleHandleA");

    pGetQueuedCompletionStatusEx        = (sGetQueuedCompletionStatusEx)       GetProcAddress(kernel32_module, "GetQueuedCompletionStatusEx");
    pSetFileCompletionNotificationModes = (sSetFileCompletionNotificationModes)GetProcAddress(kernel32_module, "SetFileCompletionNotificationModes");
    pCreateSymbolicLinkW				= (sCreateSymbolicLinkW)               GetProcAddress(kernel32_module, "CreateSymbolicLinkW");
    pCancelIoEx							= (sCancelIoEx)                        GetProcAddress(kernel32_module, "CancelIoEx");
    pInitializeConditionVariable		= (sInitializeConditionVariable)       GetProcAddress(kernel32_module, "InitializeConditionVariable");
    pSleepConditionVariableCS			= (sSleepConditionVariableCS)          GetProcAddress(kernel32_module, "SleepConditionVariableCS");
    pSleepConditionVariableSRW			= (sSleepConditionVariableSRW)         GetProcAddress(kernel32_module, "SleepConditionVariableSRW");
    pWakeAllConditionVariable			= (sWakeAllConditionVariable)          GetProcAddress(kernel32_module, "WakeAllConditionVariable");
    pWakeConditionVariable				= (sWakeConditionVariable)             GetProcAddress(kernel32_module, "WakeConditionVariable");
    pCancelSynchronousIo				= (sCancelSynchronousIo)               GetProcAddress(kernel32_module, "CancelSynchronousIo");
    pGetFinalPathNameByHandleW			= (sGetFinalPathNameByHandleW)         GetProcAddress(kernel32_module, "GetFinalPathNameByHandleW");
}

static int co_cond_fallback_init(cond_t* cond)
{
    int err;

    /* Initialize the count to 0. */
    cond->fallback.waiters_count = 0;

    InitializeCriticalSection(&cond->fallback.waiters_count_lock);

    /* Create an auto-reset event. */
    cond->fallback.signal_event = CreateEvent(NULL,  /* no security */
                                            FALSE, /* auto-reset event */
                                            FALSE, /* non-signaled initially */
                                            NULL); /* unnamed */
    if (!cond->fallback.signal_event) {
        err = GetLastError();
        goto error2;
    }

    /* Create a manual-reset event. */
    cond->fallback.broadcast_event = CreateEvent(NULL,  /* no security */
                                               TRUE,  /* manual-reset */
                                               FALSE, /* non-signaled */
                                               NULL); /* unnamed */
    if (!cond->fallback.broadcast_event) {
        err = GetLastError();
        goto error;
    }

    return 0;

error:
    CloseHandle(cond->fallback.signal_event);
error2:
    DeleteCriticalSection(&cond->fallback.waiters_count_lock);
    return err;
}

static int co_cond_wait_helper(cond_t* cond, mutex_t* mutex, DWORD dwMilliseconds)
{
    DWORD result;
    int last_waiter;
    HANDLE handles[2] = {
        cond->fallback.signal_event,
        cond->fallback.broadcast_event
    };

    /* Avoid race conditions. */
    EnterCriticalSection(&cond->fallback.waiters_count_lock);
    cond->fallback.waiters_count++;
    LeaveCriticalSection(&cond->fallback.waiters_count_lock);

    /* It's ok to release the <mutex> here since Win32 manual-reset events */
    /* maintain state when used with <SetEvent>. This avoids the "lost wakeup" */
    /* bug. */
    mutex_ulck(*mutex);

    /* Wait for either event to become signaled due to <uv_cond_signal> being */
    /* called or <uv_cond_broadcast> being called. */
    result = WaitForMultipleObjects(2, handles, FALSE, dwMilliseconds);

    EnterCriticalSection(&cond->fallback.waiters_count_lock);
    cond->fallback.waiters_count--;
    last_waiter = result == WAIT_OBJECT_0 + 1 && cond->fallback.waiters_count == 0;
    LeaveCriticalSection(&cond->fallback.waiters_count_lock);

    /* Some thread called <pthread_cond_broadcast>. */
    if (last_waiter) {
        /* We're the last waiter to be notified or to stop waiting, so reset the */
        /* the manual-reset event. */
        ResetEvent(cond->fallback.broadcast_event);
    }

    /* Reacquire the <mutex>. */
    mutex_lock(*mutex);

    if (result == WAIT_OBJECT_0 || result == WAIT_OBJECT_0 + 1)
        return 0;

    if (result == WAIT_TIMEOUT)
        return CO_ETIMEDOUT;

    abort();
    return -1; /* Satisfy the compiler. */
}


static void co_cond_fallback_wait(cond_t* cond, mutex_t* mutex) {
    if (co_cond_wait_helper(cond, mutex, INFINITE)) abort();
}

static void co_cond_fallback_twait(cond_t* cond, mutex_t* mutex, int time) {
    if (co_cond_wait_helper(cond, mutex, time)) abort();
}

static void co_cond_fallback_signal(cond_t* cond) {
    int have_waiters;

    /* Avoid race conditions. */
    EnterCriticalSection(&cond->fallback.waiters_count_lock);
    have_waiters = cond->fallback.waiters_count > 0;
    LeaveCriticalSection(&cond->fallback.waiters_count_lock);

    if (have_waiters)
        SetEvent(cond->fallback.signal_event);
}

static void co_cond_fallback_broadcast(cond_t* cond) {
    int have_waiters;

    /* Avoid race conditions. */
    EnterCriticalSection(&cond->fallback.waiters_count_lock);
    have_waiters = cond->fallback.waiters_count > 0;
    LeaveCriticalSection(&cond->fallback.waiters_count_lock);

    if (have_waiters)
        SetEvent(cond->fallback.broadcast_event);
}

static void co_cond_fallback_destroy(cond_t* cond) {
  if (!CloseHandle(cond->fallback.broadcast_event))	abort();
  if (!CloseHandle(cond->fallback.signal_event))	abort();
  DeleteCriticalSection(&cond->fallback.waiters_count_lock);
}

#define HAVE_CONDVAR_API() (pInitializeConditionVariable != NULL)

static inline int co_cond_init(cond_t* c)
{
    co__thread_once_init();
    if (HAVE_CONDVAR_API())	{pInitializeConditionVariable(&c->cond_var);	return 0;}
    else					return co_cond_fallback_init(c);
}

static inline void co_cond_wait(cond_t* c, mutex_t* m)
{
    if (HAVE_CONDVAR_API()) {if (!pSleepConditionVariableCS(&c->cond_var, m, INFINITE))abort();}
    else					co_cond_fallback_wait(c, m);
}

static inline void co_cond_twait(cond_t* c, mutex_t* m, uint ms)
{
    if (HAVE_CONDVAR_API()) {if (!pSleepConditionVariableCS(&c->cond_var, m, ms));}
    else					co_cond_fallback_twait(c, m, ms);
}

static inline void co_cond_signal(cond_t* c)
{
    if (HAVE_CONDVAR_API())	pWakeConditionVariable(&c->cond_var);
    else					co_cond_fallback_signal(c);
}

static inline void co_cond_broadcast(cond_t* c)
{
    if (HAVE_CONDVAR_API()) pWakeAllConditionVariable(&c->cond_var);
    else					co_cond_fallback_broadcast(c);
}

static inline void co_cond_destroy(cond_t* c)
{
    if (HAVE_CONDVAR_API()) /* nothing to do */;
    else					co_cond_fallback_destroy(c);
}

struct thread_ctx {
    void*	  (*entry)(void* arg);
    void*		arg;
    thread_t	self;
};

typedef struct {
    DWORD tls_index;
}co_key_t;

typedef struct co_once_s {
    unsigned char	ran;
    HANDLE			event;
}co_once_t;

#define CO_ONCE_INIT { 0, NULL }

static void co__once_inner(co_once_t* guard, void (*callback)(void)) {
    DWORD result; HANDLE existing_event, created_event;

    created_event = CreateEvent(NULL, 1, 0, NULL);
    if (created_event == 0) {
        /* Could fail in a low-memory situation? */
        co_fatal_error(GetLastError(), "CreateEvent");
    }

    existing_event = InterlockedCompareExchangePointer(&guard->event,
                                                     created_event,
                                                     NULL);

    if (existing_event == NULL) {
        /* We won the race */
        callback();

        result = SetEvent(created_event);
        assert(result);
        guard->ran = 1;
    }
    else {
        /* We lost the race. Destroy the event we created and wait for the */
        /* existing one to become signaled. */
        CloseHandle(created_event);
        result = WaitForSingleObject(existing_event, INFINITE);
        assert(result == WAIT_OBJECT_0);
    }
}

static inline void co_once(co_once_t* guard, void (*callback)(void)) {
    /* Fast case - avoid WaitForSingleObject. */
    if (guard->ran) return;

    co__once_inner(guard, callback);
}

static inline int co_key_create(co_key_t* key) {
    key->tls_index = TlsAlloc();
    if (key->tls_index == TLS_OUT_OF_INDEXES)
        return CO_ENOMEM;
    return 0;
}

static inline void co_key_delete(co_key_t* key) {
    if (TlsFree(key->tls_index) == FALSE)
        abort();
    key->tls_index = TLS_OUT_OF_INDEXES;
}

static inline void co_key_set(co_key_t* key, void* value) {
    if (TlsSetValue(key->tls_index, value) == FALSE)
        abort();
}

static co_key_t  co__current_thread_key;
static co_once_t co__current_thread_init_guard = CO_ONCE_INIT;

static inline void co__init_current_thread_key(void) {
  if (co_key_create(&co__current_thread_key))
    abort();
}

static UINT __stdcall co__thread_start(void* arg) {
    struct thread_ctx *ctx_p;
    struct thread_ctx ctx;

    ctx_p = arg;
    ctx = *ctx_p;
    free(ctx_p);

    co_once(&co__current_thread_init_guard, co__init_current_thread_key);
    co_key_set(&co__current_thread_key, (void*) ctx.self);

    ctx.entry(ctx.arg);

    return 0;
}

static int  co_thread_create(thread_t *tid, void* (*entry)(void *arg), void *arg)
{
    struct thread_ctx* ctx; int err; HANDLE thread;

    ctx = malloc(sizeof(*ctx));
    if (ctx == NULL)
        return CO_ENOMEM;

    ctx->entry	= entry;
    ctx->arg	= arg;

    /* Create the thread in suspended state so we have a chance to pass
     * its own creation handle to it */
    thread = (HANDLE) _beginthreadex(NULL,
                                   0,
                                   co__thread_start,
                                   ctx,
                                   CREATE_SUSPENDED,
                                   NULL);
  if (thread == NULL) {
    err = errno;
    free(ctx);
  } else {
    err = 0;
    *tid = thread;
    ctx->self = thread;
    ResumeThread(thread);
  }

  switch (err) {
    case 0:
      return 0;
    case EACCES:
      return CO_EACCES;
    case EAGAIN:
      return CO_EAGAIN;
    case EINVAL:
      return CO_EINVAL;
  }

  return CO_EIO;
}

static inline int co_thread_join(thread_t* _tid)
{
    thread_t tid = *_tid;
    if (WaitForSingleObject(tid, INFINITE)) return GetLastError();
    else									{	CloseHandle(tid);return 0;}
}

#endif // _WIN32_THREAD_DEF

/// ---------------------- mucas -------------------------
///
///     a faster mutex
///
/// note:
///     1. can not used with cond_t
///     2. do not using it to wait, it costs a lot
///

// #define ETHREAD_MUCAS_ENABLE     // please add this before ethread.h when you want to use it

#if defined(WIN32) && !defined(__cplusplus)
#define inline
#else
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 40300
#define inline
#endif
#endif

#ifdef GCC_VERSION

#if GCC_VERSION > 40100 && defined(ETHREAD_MUCAS_ENABLE)

#define CAS __sync_bool_compare_and_swap

typedef int mucas_t;
#define __CAS_LOCK_VAL 1
#define __CAS_ULCK_VAL 0

static inline void __mucas_init(mucas_t* mu) { *mu = __CAS_ULCK_VAL;}
static inline void __mucas_free(mucas_t* mu) { *mu = __CAS_ULCK_VAL;}
static inline void __mucas_lock(mucas_t* mu) { while(!CAS(mu, __CAS_ULCK_VAL, __CAS_LOCK_VAL)) usleep(0); }
static inline void __mucas_ulck(mucas_t* mu) { while(!CAS(mu, __CAS_LOCK_VAL, __CAS_ULCK_VAL)) usleep(0); }

#define mucas_init(m) __mucas_init(&m)
#define mucas_lock(m) __mucas_lock(&m)
#define mucas_ulck(m) __mucas_ulck(&m)
#define mucas_free(m) __mucas_free(&m)

#else

typedef mutex_t mucas_t;
#define mucas_init mutex_init
#define mucas_lock mutex_lock
#define mucas_ulck mutex_ulck
#define mucas_free mutex_free

#endif

#else

#if defined(MUCAS_ENABLE)

#define CAS InterlockedCompareExchange

typedef long mucas_t;
#define __CAS_LOCK_VAL 1
#define __CAS_ULCK_VAL 0

static inline void __mucas_init(mucas_t* mu) { *mu = __CAS_ULCK_VAL; }
static inline void __mucas_free(mucas_t* mu) { *mu = __CAS_ULCK_VAL; }
static inline void __mucas_lock(mucas_t* mu) { while(!CAS(mu, __CAS_ULCK_VAL, __CAS_LOCK_VAL)) sleep(0); }
static inline void __mucas_ulck(mucas_t* mu) { while(!CAS(mu, __CAS_LOCK_VAL, __CAS_ULCK_VAL)) sleep(0); }

#define mucas_init(m) __mucas_init(&m)
#define mucas_lock(m) __mucas_lock(&m)
#define mucas_ulck(m) __mucas_ulck(&m)
#define mucas_free(m) __mucas_free(&m)

#else

typedef mutex_t mucas_t;
#define mucas_init mutex_init
#define mucas_lock mutex_lock
#define mucas_ulck mutex_ulck
#define mucas_free mutex_free

#endif
#endif


#endif  // __ETHREAD_H__
