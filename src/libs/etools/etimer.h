/// =====================================================================================
///
///       Filename:  etimer.h
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

#ifndef __ETIMER_H__
#define __ETIMER_H__

#define ETIMER_VERSION "etimer 1.0.0"       // new tool

#include "etype.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct etloop_s* etloop;
typedef struct etimer_s
{
    void* data;
}etimer_t, * etimer;

typedef void (*etm_cb)(etimer t);


etloop etloop_new(int maxthread);
etloop etloop_df (int maxthread);

void   etloop_stop(etloop loop);

etimer etimer_new(etloop loop);
void   etimer_destroy(etimer e);

int    etimer_start(etimer e, etm_cb cb, u64 timeout, u64 repeat);
int    etimer_stop (etimer e);

u64    etimer_now();
cstr   etimer_nowS(cstr buf, int len);

#ifdef __cplusplus
}
#endif

#endif
