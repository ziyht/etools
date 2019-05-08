#include "test_main.h"

#include <stdio.h>
#include <time.h>
#include <signal.h>

#define  ECOMPAT_UNISTD
#include "ecompat.h"
#include "ethread.h"

typedef struct{
    cstr tag;
    uint cnt;
}DATA;

etloop loop;
emutex_t wait_mu;

static void on_timer1(etimer t)
{
    //char buf[19];

    DATA* d = (DATA*)t->data;

    d->cnt ++;

    printf("%s %d: %"PRIi64"\n", d->tag, d->cnt, etimer_now());  fflush(stdout);

    if(d->cnt == 10000)
        emutex_ulck(wait_mu);
}

static void on_timer2(etimer t)
{
    //char buf[19];

    DATA* d = (DATA*)t->data;

    d->cnt ++;

    printf("%s %d: %"PRIi64"\n", d->tag, d->cnt, etimer_now());  fflush(stdout);
    //printf("%s %d: %s\n", d->tag, d->cnt, etimer_nowS(buf, 19));  fflush(stdout);

    if(d->cnt == 10000)
        emutex_ulck(wait_mu);
}

void sig_handler(int i) {
    switch (i) {

#if (!WIN32)
    case SIGHUP:    ;
#endif
    case SIGINT:    ;
    case SIGTERM:   break;
    default:             return;
    }

    emutex_ulck(wait_mu);
}

void etimer_test_performance()
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
   etimer_start(t2, on_timer2, 0, 1);

#if defined(_WIN32)
   Sleep(10000);
#else

   emutex_init(wait_mu);
   emutex_lock(wait_mu);
   emutex_lock(wait_mu);
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

int test_performance(int argc, char* argv[])
{
    etimer_test_performance();

    return ETEST_OK;
}
