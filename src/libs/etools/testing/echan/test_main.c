#include <stdio.h>
#include <time.h>

#include "test_main.h"

int main()
{
#if 1
    echan_new_test();
    echan_close_test();
    echan_send_test();

    echan_bin_test();
    echan_int_test();
    echan_double_test();
    echan_srt_test();
    echan_ptr_test();
    echan_obj_test();

    echan_sigs_test();

    test_chan_multi();
    test_chan_multi2();

    echan_time_recv_test();
#else
    echan_performance_test();
#endif
    show_pass();
#ifdef _WIN32
    getchar();
#endif
}

