#include "test_main.h"

#include <unistd.h>
#include "uv.h"

uv_loop_t* _loop;

static void on_timer(uv_timer_t* t)
{
    static int conut = 0;

    printf("on_timer %2d: %ld\n", ++conut, uv_now(_loop)); fflush(stdout);

    if(conut == 10000)
    {
        uv_timer_stop(t);
        printf("timer stop...\n"); fflush(stdout);
    }
}

void libuv_timer_test()
{
    printf("-- test timer of libuv --\n"); fflush(stdout);

    uv_loop_t* my_loop = uv_default_loop();
    uv_loop_init(my_loop);

    _loop = my_loop;

    printf("timer start...\n"); fflush(stdout);
    uv_timer_t my_timer;
    uv_timer_init(my_loop, &my_timer);
    uv_timer_start(&my_timer, on_timer, 1000, 1);

    uv_run(my_loop, UV_RUN_DEFAULT);
}
