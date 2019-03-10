#include <time.h>
#include <stdlib.h>
#include "test_main.h"

int main()
{
#if 0
    //edict_basic_test();
    //edict_itr_test();

    //edict_mutex_test();

    edict_clear_test();

    //edict_hash_test();
#else
    edict_performance_test();
    uthash_performance_test();
#endif

#if(_WIN32)
    getchar();
#endif

}

