/*

this compatiter are almost make from libuv

*/


#ifndef COMPAT_H_VERSION
#define COMPAT_H_VERSION  "0.0.2"		// fix confict of uv.h
#endif

//#define COMPAT_MAIN
//#define COMPAT_ALL

#ifdef  COMPAT_ALL_MAIN
#ifndef __COMPAT_ALL_MAIN__
#ifndef COMPAT_ALL
#define COMPAT_ALL
#endif  // !COMPAT_ALL          // link with: linux         win32
#define COMPAT_THREAD_MAIN      //            pthread       kernel32.lib
#define COMPAT_UNISTD_MAIN      //                          kernel32.lib
#endif  // !__COMPAT_ALL_MAIN__
#endif  // !COMPAT_ALL_MAIN

#ifdef  COMPAT_ALL
#ifndef __COMPAT_ALL__
#define __COMPAT_ALL__
#define COMPAT_THREAD
#define COMPAT_UNISTD
#endif  // __COMPAT_ALL__
#endif  // COMPAT_ALL

#ifdef  COMPAT_THREAD_MAIN
#ifndef COMPAT_THREAD
#define COMPAT_THREAD
#endif
#define __COMPAT_THREAD_DF 1
#endif  // COMPAT_THREAD_MAIN

#ifdef  COMPAT_THREAD
#define __COMPAT_THREAD_DC 1
#endif  // COMPAT_THREAD

#ifdef  COMPAT_UNISTD_MAIN
#ifndef COMPAT_UNISTD
#define COMPAT_UNISTD
#endif
#define __COMPAT_UNISTD_DF 1
#endif  // COMPAT_UNISTD_MAIN

#ifdef  COMPAT_UNISTD
#define __COMPAT_UNISTD_DC 1
#endif  // COMPAT_UNISTD


/**********************************************************
* CO_ERRNO                                                *
**********************************************************/

#ifndef CO_ERRNO_H_
#define CO_ERRNO_H_

#include <errno.h>

#define CO__EOF     (-4095)
#define CO__UNKNOWN (-4094)

#define CO__EAI_ADDRFAMILY  (-3000)
#define CO__EAI_AGAIN       (-3001)
#define CO__EAI_BADFLAGS    (-3002)
#define CO__EAI_CANCELED    (-3003)
#define CO__EAI_FAIL        (-3004)
#define CO__EAI_FAMILY      (-3005)
#define CO__EAI_MEMORY      (-3006)
#define CO__EAI_NODATA      (-3007)
#define CO__EAI_NONAME      (-3008)
#define CO__EAI_OVERFLOW    (-3009)
#define CO__EAI_SERVICE     (-3010)
#define CO__EAI_SOCKTYPE    (-3011)
#define CO__EAI_BADHINTS    (-3013)
#define CO__EAI_PROTOCOL    (-3014)

/* Only map to the system errno on non-Windows platforms. It's apparently
 * a fairly common practice for Windows programmers to redefine errno codes.
 */
#if defined(E2BIG) && !defined(_WIN32)
# define CO__E2BIG (-E2BIG)
#else
# define CO__E2BIG (-4093)
#endif

#if defined(EACCES) && !defined(_WIN32)
# define CO__EACCES (-EACCES)
#else
# define CO__EACCES (-4092)
#endif

#if defined(EADDRINUSE) && !defined(_WIN32)
# define CO__EADDRINUSE (-EADDRINUSE)
#else
# define CO__EADDRINUSE (-4091)
#endif

#if defined(EADDRNOTAVAIL) && !defined(_WIN32)
# define CO__EADDRNOTAVAIL (-EADDRNOTAVAIL)
#else
# define CO__EADDRNOTAVAIL (-4090)
#endif

#if defined(EAFNOSUPPORT) && !defined(_WIN32)
# define CO__EAFNOSUPPORT (-EAFNOSUPPORT)
#else
# define CO__EAFNOSUPPORT (-4089)
#endif

#if defined(EAGAIN) && !defined(_WIN32)
# define CO__EAGAIN (-EAGAIN)
#else
# define CO__EAGAIN (-4088)
#endif

#if defined(EALREADY) && !defined(_WIN32)
# define CO__EALREADY (-EALREADY)
#else
# define CO__EALREADY (-4084)
#endif

#if defined(EBADF) && !defined(_WIN32)
# define CO__EBADF (-EBADF)
#else
# define CO__EBADF (-4083)
#endif

#if defined(EBUSY) && !defined(_WIN32)
# define CO__EBUSY (-EBUSY)
#else
# define CO__EBUSY (-4082)
#endif

#if defined(ECANCELED) && !defined(_WIN32)
# define CO__ECANCELED (-ECANCELED)
#else
# define CO__ECANCELED (-4081)
#endif

#if defined(ECHARSET) && !defined(_WIN32)
# define CO__ECHARSET (-ECHARSET)
#else
# define CO__ECHARSET (-4080)
#endif

#if defined(ECONNABORTED) && !defined(_WIN32)
# define CO__ECONNABORTED (-ECONNABORTED)
#else
# define CO__ECONNABORTED (-4079)
#endif

#if defined(ECONNREFUSED) && !defined(_WIN32)
# define CO__ECONNREFUSED (-ECONNREFUSED)
#else
# define CO__ECONNREFUSED (-4078)
#endif

#if defined(ECONNRESET) && !defined(_WIN32)
# define CO__ECONNRESET (-ECONNRESET)
#else
# define CO__ECONNRESET (-4077)
#endif

#if defined(EDESTADDRREQ) && !defined(_WIN32)
# define CO__EDESTADDRREQ (-EDESTADDRREQ)
#else
# define CO__EDESTADDRREQ (-4076)
#endif

#if defined(EEXIST) && !defined(_WIN32)
# define CO__EEXIST (-EEXIST)
#else
# define CO__EEXIST (-4075)
#endif

#if defined(EFAULT) && !defined(_WIN32)
# define CO__EFAULT (-EFAULT)
#else
# define CO__EFAULT (-4074)
#endif

#if defined(EHOSTUNREACH) && !defined(_WIN32)
# define CO__EHOSTUNREACH (-EHOSTUNREACH)
#else
# define CO__EHOSTUNREACH (-4073)
#endif

#if defined(EINTR) && !defined(_WIN32)
# define CO__EINTR (-EINTR)
#else
# define CO__EINTR (-4072)
#endif

#if defined(EINVAL) && !defined(_WIN32)
# define CO__EINVAL (-EINVAL)
#else
# define CO__EINVAL (-4071)
#endif

#if defined(EIO) && !defined(_WIN32)
# define CO__EIO (-EIO)
#else
# define CO__EIO (-4070)
#endif

#if defined(EISCONN) && !defined(_WIN32)
# define CO__EISCONN (-EISCONN)
#else
# define CO__EISCONN (-4069)
#endif

#if defined(EISDIR) && !defined(_WIN32)
# define CO__EISDIR (-EISDIR)
#else
# define CO__EISDIR (-4068)
#endif

#if defined(ELOOP) && !defined(_WIN32)
# define CO__ELOOP (-ELOOP)
#else
# define CO__ELOOP (-4067)
#endif

#if defined(EMFILE) && !defined(_WIN32)
# define CO__EMFILE (-EMFILE)
#else
# define CO__EMFILE (-4066)
#endif

#if defined(EMSGSIZE) && !defined(_WIN32)
# define CO__EMSGSIZE (-EMSGSIZE)
#else
# define CO__EMSGSIZE (-4065)
#endif

#if defined(ENAMETOOLONG) && !defined(_WIN32)
# define CO__ENAMETOOLONG (-ENAMETOOLONG)
#else
# define CO__ENAMETOOLONG (-4064)
#endif

#if defined(ENETDOWN) && !defined(_WIN32)
# define CO__ENETDOWN (-ENETDOWN)
#else
# define CO__ENETDOWN (-4063)
#endif

#if defined(ENETUNREACH) && !defined(_WIN32)
# define CO__ENETUNREACH (-ENETUNREACH)
#else
# define CO__ENETUNREACH (-4062)
#endif

#if defined(ENFILE) && !defined(_WIN32)
# define CO__ENFILE (-ENFILE)
#else
# define CO__ENFILE (-4061)
#endif

#if defined(ENOBUFS) && !defined(_WIN32)
# define CO__ENOBUFS (-ENOBUFS)
#else
# define CO__ENOBUFS (-4060)
#endif

#if defined(ENODEV) && !defined(_WIN32)
# define CO__ENODEV (-ENODEV)
#else
# define CO__ENODEV (-4059)
#endif

#if defined(ENOENT) && !defined(_WIN32)
# define CO__ENOENT (-ENOENT)
#else
# define CO__ENOENT (-4058)
#endif

#if defined(ENOMEM) && !defined(_WIN32)
# define CO__ENOMEM (-ENOMEM)
#else
# define CO__ENOMEM (-4057)
#endif

#if defined(ENONET) && !defined(_WIN32)
# define CO__ENONET (-ENONET)
#else
# define CO__ENONET (-4056)
#endif

#if defined(ENOSPC) && !defined(_WIN32)
# define CO__ENOSPC (-ENOSPC)
#else
# define CO__ENOSPC (-4055)
#endif

#if defined(ENOSYS) && !defined(_WIN32)
# define CO__ENOSYS (-ENOSYS)
#else
# define CO__ENOSYS (-4054)
#endif

#if defined(ENOTCONN) && !defined(_WIN32)
# define CO__ENOTCONN (-ENOTCONN)
#else
# define CO__ENOTCONN (-4053)
#endif

#if defined(ENOTDIR) && !defined(_WIN32)
# define CO__ENOTDIR (-ENOTDIR)
#else
# define CO__ENOTDIR (-4052)
#endif

#if defined(ENOTEMPTY) && !defined(_WIN32)
# define CO__ENOTEMPTY (-ENOTEMPTY)
#else
# define CO__ENOTEMPTY (-4051)
#endif

#if defined(ENOTSOCK) && !defined(_WIN32)
# define CO__ENOTSOCK (-ENOTSOCK)
#else
# define CO__ENOTSOCK (-4050)
#endif

#if defined(ENOTSUP) && !defined(_WIN32)
# define CO__ENOTSUP (-ENOTSUP)
#else
# define CO__ENOTSUP (-4049)
#endif

#if defined(EPERM) && !defined(_WIN32)
# define CO__EPERM (-EPERM)
#else
# define CO__EPERM (-4048)
#endif

#if defined(EPIPE) && !defined(_WIN32)
# define CO__EPIPE (-EPIPE)
#else
# define CO__EPIPE (-4047)
#endif

#if defined(EPROTO) && !defined(_WIN32)
# define CO__EPROTO (-EPROTO)
#else
# define CO__EPROTO (-4046)
#endif

#if defined(EPROTONOSUPPORT) && !defined(_WIN32)
# define CO__EPROTONOSUPPORT (-EPROTONOSUPPORT)
#else
# define CO__EPROTONOSUPPORT (-4045)
#endif

#if defined(EPROTOTYPE) && !defined(_WIN32)
# define CO__EPROTOTYPE (-EPROTOTYPE)
#else
# define CO__EPROTOTYPE (-4044)
#endif

#if defined(EROFS) && !defined(_WIN32)
# define CO__EROFS (-EROFS)
#else
# define CO__EROFS (-4043)
#endif

#if defined(ESHUTDOWN) && !defined(_WIN32)
# define CO__ESHUTDOWN (-ESHUTDOWN)
#else
# define CO__ESHUTDOWN (-4042)
#endif

#if defined(ESPIPE) && !defined(_WIN32)
# define CO__ESPIPE (-ESPIPE)
#else
# define CO__ESPIPE (-4041)
#endif

#if defined(ESRCH) && !defined(_WIN32)
# define CO__ESRCH (-ESRCH)
#else
# define CO__ESRCH (-4040)
#endif

#if defined(ETIMEDOUT) && !defined(_WIN32)
# define CO__ETIMEDOUT (-ETIMEDOUT)
#else
# define CO__ETIMEDOUT (-4039)
#endif

#if defined(ETXTBSY) && !defined(_WIN32)
# define CO__ETXTBSY (-ETXTBSY)
#else
# define CO__ETXTBSY (-4038)
#endif

#if defined(EXDEV) && !defined(_WIN32)
# define CO__EXDEV (-EXDEV)
#else
# define CO__EXDEV (-4037)
#endif

#if defined(EFBIG) && !defined(_WIN32)
# define CO__EFBIG (-EFBIG)
#else
# define CO__EFBIG (-4036)
#endif

#if defined(ENOPROTOOPT) && !defined(_WIN32)
# define CO__ENOPROTOOPT (-ENOPROTOOPT)
#else
# define CO__ENOPROTOOPT (-4035)
#endif

#if defined(ERANGE) && !defined(_WIN32)
# define CO__ERANGE (-ERANGE)
#else
# define CO__ERANGE (-4034)
#endif

#if defined(ENXIO) && !defined(_WIN32)
# define CO__ENXIO (-ENXIO)
#else
# define CO__ENXIO (-4033)
#endif

#if defined(EMLINK) && !defined(_WIN32)
# define CO__EMLINK (-EMLINK)
#else
# define CO__EMLINK (-4032)
#endif

/* EHOSTDOWN is not visible on BSD-like systems when _POSIX_C_SOURCE is
 * defined. Fortunately, its value is always 64 so it's possible albeit
 * icky to hard-code it.
 */
#if defined(EHOSTDOWN) && !defined(_WIN32)
# define CO__EHOSTDOWN (-EHOSTDOWN)
#elif defined(__APPLE__) || \
      defined(__DragonFly__) || \
      defined(__FreeBSD__) || \
      defined(__FreeBSD_kernel__) || \
      defined(__NetBSD__) || \
      defined(__OpenBSD__)
# define CO__EHOSTDOWN (-64)
#else
# define CO__EHOSTDOWN (-4031)
#endif

#if (!_WIN32)
#include <netdb.h>
#endif

/* Expand this list if necessary. */
#define CO_ERRNO_MAP(XX)                                                      \
  XX(E2BIG, "argument list too long")                                         \
  XX(EACCES, "permission denied")                                             \
  XX(EADDRINUSE, "address already in use")                                    \
  XX(EADDRNOTAVAIL, "address not available")                                  \
  XX(EAFNOSUPPORT, "address family not supported")                            \
  XX(EAGAIN, "resource temporarily unavailable")                              \
  XX(EAI_ADDRFAMILY, "address family not supported")                          \
  XX(EAI_AGAIN, "temporary failure")                                          \
  XX(EAI_BADFLAGS, "bad ai_flags value")                                      \
  XX(EAI_BADHINTS, "invalid value for hints")                                 \
  XX(EAI_CANCELED, "request canceled")                                        \
  XX(EAI_FAIL, "permanent failure")                                           \
  XX(EAI_FAMILY, "ai_family not supported")                                   \
  XX(EAI_MEMORY, "out of memory")                                             \
  XX(EAI_NODATA, "no address")                                                \
  XX(EAI_NONAME, "unknown node or service")                                   \
  XX(EAI_OVERFLOW, "argument buffer overflow")                                \
  XX(EAI_PROTOCOL, "resolved protocol is unknown")                            \
  XX(EAI_SERVICE, "service not available for socket type")                    \
  XX(EAI_SOCKTYPE, "socket type not supported")                               \
  XX(EALREADY, "connection already in progress")                              \
  XX(EBADF, "bad file descriptor")                                            \
  XX(EBUSY, "resource busy or locked")                                        \
  XX(ECANCELED, "operation canceled")                                         \
  XX(ECHARSET, "invalid Unicode character")                                   \
  XX(ECONNABORTED, "software caused connection abort")                        \
  XX(ECONNREFUSED, "connection refused")                                      \
  XX(ECONNRESET, "connection reset by peer")                                  \
  XX(EDESTADDRREQ, "destination address required")                            \
  XX(EEXIST, "file already exists")                                           \
  XX(EFAULT, "bad address in system call argument")                           \
  XX(EFBIG, "file too large")                                                 \
  XX(EHOSTUNREACH, "host is unreachable")                                     \
  XX(EINTR, "interrupted system call")                                        \
  XX(EINVAL, "invalid argument")                                              \
  XX(EIO, "i/o error")                                                        \
  XX(EISCONN, "socket is already connected")                                  \
  XX(EISDIR, "illegal operation on a directory")                              \
  XX(ELOOP, "too many symbolic links encountered")                            \
  XX(EMFILE, "too many open files")                                           \
  XX(EMSGSIZE, "message too long")                                            \
  XX(ENAMETOOLONG, "name too long")                                           \
  XX(ENETDOWN, "network is down")                                             \
  XX(ENETUNREACH, "network is unreachable")                                   \
  XX(ENFILE, "file table overflow")                                           \
  XX(ENOBUFS, "no buffer space available")                                    \
  XX(ENODEV, "no such device")                                                \
  XX(ENOENT, "no such file or directory")                                     \
  XX(ENOMEM, "not enough memory")                                             \
  XX(ENONET, "machine is not on the network")                                 \
  XX(ENOPROTOOPT, "protocol not available")                                   \
  XX(ENOSPC, "no space left on device")                                       \
  XX(ENOSYS, "function not implemented")                                      \
  XX(ENOTCONN, "socket is not connected")                                     \
  XX(ENOTDIR, "not a directory")                                              \
  XX(ENOTEMPTY, "directory not empty")                                        \
  XX(ENOTSOCK, "socket operation on non-socket")                              \
  XX(ENOTSUP, "operation not supported on socket")                            \
  XX(EPERM, "operation not permitted")                                        \
  XX(EPIPE, "broken pipe")                                                    \
  XX(EPROTO, "protocol error")                                                \
  XX(EPROTONOSUPPORT, "protocol not supported")                               \
  XX(EPROTOTYPE, "protocol wrong type for socket")                            \
  XX(ERANGE, "result too large")                                              \
  XX(EROFS, "read-only file system")                                          \
  XX(ESHUTDOWN, "cannot send after transport endpoint shutdown")              \
  XX(ESPIPE, "invalid seek")                                                  \
  XX(ESRCH, "no such process")                                                \
  XX(ETIMEDOUT, "connection timed out")                                       \
  XX(ETXTBSY, "text file is busy")                                            \
  XX(EXDEV, "cross-device link not permitted")                                \
  XX(UNKNOWN, "unknown error")                                                \
  XX(EOF, "end of file")                                                      \
  XX(ENXIO, "no such device or address")                                      \
  XX(EMLINK, "too many links")                                                \
  XX(EHOSTDOWN, "host is down")                                               \


typedef enum {
#define XX(code, _) CO_ ## code = CO__ ## code,
    CO_ERRNO_MAP(XX)
#undef XX
    CO_ERRNO_MAX = CO__EOF - 1
}co_errno_t;

#endif /* UV_ERRNO_H_ */

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************
* THREAD  DECLARATION                                     *
**********************************************************/
#if     __COMPAT_THREAD_DC
#ifndef __COMPAT_THREAD_DECLARATION__
#define __COMPAT_THREAD_DECLARATION__

#if defined(_WIN32)
#include <process.h>
#include <Windows.h>
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

int  co_cond_init(cond_t* c);
void co_cond_wait(cond_t* c, mutex_t* m);
void co_cond_signal(cond_t* c);
void co_cond_broadcast(cond_t* c);
void co_cond_destroy(cond_t* c);

int  co_thread_create(thread_t *tid, void* (*entry)(void *arg), void *arg);
int  co_thread_join(thread_t* tid);

#define mutex_init(m)           InitializeCriticalSection(&(m))
#define mutex_lock(m)           EnterCriticalSection(&(m))
#define mutex_ulck(m)           LeaveCriticalSection(&(m))
#define mutex_free(m)           DeleteCriticalSection(&(m))

#define cond_init(c)            co_cond_init(&(c))
#define cond_wait(c, m)         co_cond_wait(&(c), &(m))
#define cond_one(c)             co_cond_signal(&(c))
#define cond_all(c)             co_cond_broadcast(&(c))
#define cond_free(c)            co_cond_destroy(&(c))

#define thread_init(t, cb, d)   co_thread_create(&t, (__start_routine)cb, d)
#define thread_join(t)          co_thread_join(&t)
#define thread_detach(t)
#define thread_quit(t)          CloseHandle(t)

#else   // ------------- UNIX
#include <pthread.h>
typedef void *(*__start_routine) (void *);

typedef pthread_mutex_t         mutex_t;
typedef pthread_cond_t          cond_t;
typedef pthread_t               thread_t;

#define mutex_init(m)           pthread_mutex_init(&m, NULL)
#define mutex_lock(m)           pthread_mutex_lock(&m)
#define mutex_ulck(m)           pthread_mutex_unlock(&m)
#define mutex_free(m)           pthread_mutex_destroy(&m)

#define cond_init(c)            pthread_cond_init(&c, NULL)
#define cond_wait(c, m)         pthread_cond_wait(&c, &m)
#define cond_twait(c, m, t)     pthread_cond_timedwait(&c, &m, &t);
#define cond_one(c)             pthread_cond_signal(&c)
#define cond_all(c)             pthread_cond_broadcast(&c)
#define cond_free(c)            pthread_cond_destroy(&c)

#define thread_init(t, cb, d)   pthread_create(&t, NULL, (__start_routine)cb, d)
#define thread_join(t)          pthread_join(t, NULL)
#define thread_detach(t)        pthread_detach(t)
#define thread_quit(t)          pthread_cancel(t)

#endif  // _WIN32

#endif  // __COMPAT_THREAD_DECLARATION__
#endif  // !__COMPAT_THREAD_DC

/**********************************************************
* UNISTD  DECLARATION                                     *
**********************************************************/
#if     __COMPAT_UNISTD_DC
#ifndef __COMPAT_UNISTD_DECLARATION__
#define __COMPAT_UNISTD_DECLARATION__

#if defined(_WIN32)
#include <stdint.h>
void usleep(int64_t microsecond);
void sleep(int64_t second);
#else   // ------------- UNIX
#include <unistd.h>
#endif  // _WIN32

#endif  // __COMPAT_UNISTD_DECLARATION__
#endif  // !__COMPAT_UNISTD_DC

/*---------------------------------------------------------
| THREAD  DIFINITION                                      |
----------------------------------------------------------*/
#if     __COMPAT_THREAD_DF
#ifndef __COMPAT_THREAD_DIFINITION__
#define __COMPAT_THREAD_DIFINITION__
#ifdef  _WIN32

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#ifndef __IMPORT_KERNEL32_LIB
#define __IMPORT_KERNEL32_LIB
#pragma comment(lib, "Kernel32.lib")
#endif

void co_fatal_error(const int errorno, const char* syscall) {
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

void co__thread_once_init()
{
    static _co_thread_init_guard = 0;	HMODULE kernel32_module;

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

int co_cond_fallback_init(cond_t* cond)
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

int co_cond_init(cond_t* c)
{
    co__thread_once_init();
    if (HAVE_CONDVAR_API())	{pInitializeConditionVariable(&c->cond_var);	return 0;}
    else					return co_cond_fallback_init(c);
}

void co_cond_wait(cond_t* c, mutex_t* m)
{
    if (HAVE_CONDVAR_API()) {if (!pSleepConditionVariableCS(&c->cond_var, m, INFINITE))abort();}
    else					co_cond_fallback_wait(c, m);
}

void co_cond_signal(cond_t* c)
{
    if (HAVE_CONDVAR_API())	pWakeConditionVariable(&c->cond_var);
    else					co_cond_fallback_signal(c);
}

void co_cond_broadcast(cond_t* c)
{
    if (HAVE_CONDVAR_API()) pWakeAllConditionVariable(&c->cond_var);
    else					co_cond_fallback_broadcast(c);
}

void co_cond_destroy(cond_t* c)
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

void co_once(co_once_t* guard, void (*callback)(void)) {
    /* Fast case - avoid WaitForSingleObject. */
    if (guard->ran) return;

    co__once_inner(guard, callback);
}

int co_key_create(co_key_t* key) {
    key->tls_index = TlsAlloc();
    if (key->tls_index == TLS_OUT_OF_INDEXES)
        return CO_ENOMEM;
    return 0;
}

void co_key_delete(co_key_t* key) {
    if (TlsFree(key->tls_index) == FALSE)
        abort();
    key->tls_index = TLS_OUT_OF_INDEXES;
}

void co_key_set(co_key_t* key, void* value) {
    if (TlsSetValue(key->tls_index, value) == FALSE)
        abort();
}

static co_key_t  co__current_thread_key;
static co_once_t co__current_thread_init_guard = CO_ONCE_INIT;

static void co__init_current_thread_key(void) {
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

int  co_thread_create(thread_t *tid, void* (*entry)(void *arg), void *arg)
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

int co_thread_join(thread_t* _tid)
{
    thread_t tid = *_tid;
    if (WaitForSingleObject(tid, INFINITE)) return GetLastError();
    else									 {	CloseHandle(tid);return 0;}
}

#endif  // _WIN32
#endif  // __COMPAT_THREAD_DIFINITION__
#endif  // !__COMPAT_THREAD_DF

/*---------------------------------------------------------
| UNISTD  DIFINITION                                      |
----------------------------------------------------------*/
#if     __COMPAT_UNISTD_DF
#ifndef __COMPAT_UNISTD_DIFINITION__
#define __COMPAT_UNISTD_DIFINITION__
#ifdef  _WIN32
#ifndef __IMPORT_KERNEL32_LIB
#include <Windows.h>
#define __IMPORT_KERNEL32_LIB
#pragma comment(lib, "Kernel32.lib")
#endif

void usleep(int64_t delayTime)
{
    LARGE_INTEGER Freq={0};
    if (!QueryPerformanceFrequency(&Freq))
    {
        Sleep(0); return;
    }

    LARGE_INTEGER tcStart={0};
    QueryPerformanceCounter(&tcStart);
    LARGE_INTEGER tcEnd={0};
    while(1)
    {
        QueryPerformanceCounter(&tcEnd);
        double time=(((tcEnd.QuadPart - tcStart.QuadPart)*1000000)/(double)Freq.QuadPart);
        if (time >= delayTime)
            break;
    }
}

void sleep(int64_t second)
{
    usleep(second * 1000000);
}

#endif  // _WIN32
#endif  // __COMPAT_UNISTD_DIFINITION__
#endif  // !__COMPAT_UNISTD_DF

#ifdef __cplusplus
}
#endif
