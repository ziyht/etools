#include "test_main.h"
#include "etype.h"

static void on_timer(ev_timer_t* t)
{
    static int conut = 0;

    printf("on_timer %2d: \n", ++conut); fflush(stdout);

    ev_timer_start(t, on_timer, 1, 0);

    if(conut == 10000)
    {
        ev_timer_stop(t);
        printf("timer stop...\n"); fflush(stdout);
    }
}

static void on_timer2(ev_timer_t* t)
{

}

void ev_timer_test2()
{
    ev_timer_t t, t2;

    ev_timer_init(&t, ev_default_loop());

    ev_timer_init(&t2, ev_default_loop());

    ev_timer_start(&t, on_timer, 10, 0);
    ev_timer_start(&t2, on_timer2, 100000, 1000);

    ev_run(ev_default_loop(), EV_RUN_DEFAULT);
}

