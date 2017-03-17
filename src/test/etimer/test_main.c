#include "test_main.h"

#include <stdio.h>
#include <unistd.h>
#include <time.h>

typedef struct{
    cstr tag;
    uint cnt;
}DATA;

static inline void __etimer_ns(cstr tag, int type)
{
    u64 offset;

    struct timespec ts;

    clock_gettime(type, &ts);
    offset = (int64_t)ts.tv_sec * 1000000000 + (int64_t)ts.tv_nsec;

    printf("%s: %d %ld\n", tag, type, offset);
}


static void on_timer1(etimer t)
{
    char buf[19];

    DATA* d = (DATA*)t->data;

    d->cnt ++;

    printf("%s %s %d: %ld\n", etimer_nowS(buf, 19), d->tag, d->cnt, etimer_now());  fflush(stdout);
}

static void on_timer2(etimer t)
{
    char buf[19];

    DATA* d = (DATA*)t->data;

    d->cnt ++;

    printf("%s %s %d: %ld\n", etimer_nowS(buf, 19), d->tag, d->cnt, etimer_now());  fflush(stdout);
}

int main()
{
    etloop loop; etimer t1, t2;

    DATA d[] = {
        {"timer1", 0,},
        {"timer2", 0}
    };

    loop = etloop_df(4);

    t1 = etimer_new(loop);
    t2 = etimer_new(loop);

    t1->data = &d[0];
    t2->data = &d[1];

    char buf[19];
    printf("%s\n", etimer_nowS(buf, 19));  fflush(stdout);

    etimer_start(t1, on_timer1, 1000, 100);
    //etimer_start(t2, on_timer2, 500 , 1);

    sleep(5);

    etloop_stop(loop);


    sleep(1);

    return 0;
}
