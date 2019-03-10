#include "test_main.h"

#include "ethread.h"
#include "elog.h"

static edict d;
static u64  count;

static void* _add(void* _start)
{
    i64 start = *(i64*)_start;
    for(uint i = 0; i < count; i++)
    {
        edict_addI(d, ekey_i(i + start), i);
    }

    elog_inf(0, "add0 over: %d", count); fflush(stdout);

    return 0;
}

void edict_mutex_test()
{
    d = edict_new(EDICT_KEYI);

    thread_t th[10];

    count = 1000000;

    i64 t = eutils_nowms();

    i64 start[10] =
    {
        count*0,count*1,count*2,count*3,count*4,
    };

    thread_init(th[0], _add, &start[0]);
    thread_init(th[1], _add, &start[1]);
    thread_init(th[2], _add, &start[2]);
    thread_init(th[3], _add, &start[3]);
    thread_init(th[4], _add, &start[4]);

    thread_join(th[0]);
    thread_join(th[1]);
    thread_join(th[2]);
    thread_join(th[3]);
    thread_join(th[4]);

    elog_inf(0, "edict len: %d  %ldms", edict_len(d), eutils_nowms() - t);

    edict_free(d);
}

int test_mutex(int argc, char* argv[])
{
    edict_mutex_test();

    return ETEST_OK;
}
