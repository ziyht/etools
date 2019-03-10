#include "test_main.h"

int main()
{
    elog_basic_test();
    //elog_level_test();
    //elog_buffer_test();
    //elog_rtt_test();
    //elog_opts_test();

    elog_multi_thread_test();

#if(_WIN32)
    getchar();
#endif

}
