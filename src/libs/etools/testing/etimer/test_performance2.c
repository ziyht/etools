#include "test_main.h"

#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>

#define  ECOMPAT_UNISTD
#define  EUTILS_LLOG 1
#include "eutils.h"
#include "ecompat.h"
#include "ethread.h"

typedef struct{
    cstr tag;
    uint cnt;
}DATA;

static etloop loop;
static mutex_t wait_mu;

static void on_timer1(etimer t)
{
    //char buf[19];

    DATA* d = (DATA*)t->data;

    d->cnt ++;

    printf("%s %d: %"PRIi64"\n", d->tag, d->cnt, etimer_now());  fflush(stdout);

    //usleep(0000);

    if(d->cnt == 10000)
        mutex_ulck(wait_mu);

    if(!etimer_start(t, on_timer1, 0, 0))
    {
        llog("etimer start faild: .....");
    }

    //usleep(000);
}

static void on_timer2(etimer t)
{
    //char buf[19];

    DATA* d = (DATA*)t->data;

    d->cnt ++;

    printf("%s %d: %"PRIi64"\n", d->tag, d->cnt, etimer_now());  fflush(stdout);
    //printf("%s %d: %s\n", d->tag, d->cnt, etimer_nowS(buf, 19));  fflush(stdout);

    if(d->cnt == 10000)
        mutex_ulck(wait_mu);
}

static void sig_handler(int i) {
    switch (i) {

#if (!WIN32)
    case SIGHUP:    ;
#endif
    case SIGINT:    ;
    case SIGTERM:   break;
    default:             return;
    }

    static int quit;

    if(quit)
    {
        exit(0);
    }

    quit++;

    mutex_ulck(wait_mu);
}

void etimer_test_performance2()
{
    etimer t1, t2;

   DATA d[] = {
       {"timer1", 0,},
       {"timer2", 0}
   };

   loop = etloop_df(1);

   t1 = etimer_new(loop);
   t2 = etimer_new(loop);

   t1->data = &d[0];
   t2->data = &d[1];

#ifndef _WIN32
   signal(SIGHUP,  sig_handler);
#endif
   signal(SIGINT,  sig_handler);
   signal(SIGTERM, sig_handler);

   char buf[30];
   printf("%s\n", etimer_nowstr(buf, 30));  fflush(stdout);

   etimer_start(t1, on_timer1, 1000, 0);
   usleep(500000);
   etimer_start(t2, on_timer2, 0, 1000);

#if defined(_WIN32)
   Sleep(10000);
#else

   mutex_init(wait_mu);
   mutex_lock(wait_mu);
   mutex_lock(wait_mu);
#endif

   etloop_stop(loop);
   etimer_destroy(t1);
   etimer_destroy(t2);

   sleep(2);

#if defined(_WIN32)
   puts("press any key to quit");
   getchar();
#endif

   return ;
}

int test_performance2(int argc, char* argv[])
{
    etimer_test_performance2();

    return ETEST_OK;
}
