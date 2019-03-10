#include "test_main.h"
#include "etype.h"

static void on_timer(ev_timer_t* t)
{
    static int conut = 0;

    printf("on_timer %2d: \n", ++conut); fflush(stdout);

    if(conut == 10000)
    {
        ev_timer_stop(t);
        printf("timer stop...\n"); fflush(stdout);
    }
}

void ev_timer_test()
{
    ev_timer_t t;

    ev_timer_init(&t, ev_default_loop());

    ev_timer_start(&t, on_timer, 10, 1);

    ev_run(ev_default_loop(), EV_RUN_DEFAULT);
}

