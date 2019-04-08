#include "test_main.h"
#include <assert.h>

void edict_clearI_test()
{
    edict d = edict_new(EKEY_I);
    ekey  k;

    for(int i = 0; i < 1000; i++)
    {
        k.i = i;

        edict_addR(d, k, 4);

        if(i % 23 == 0 || i % 29 == 0)
        {
            edict_clear(d);
            assert(edict_len(d) == 0);
        }
    }

    edict_clear(d);
    edict_free(d);
}

void edict_clearS_test()
{
    edict d = edict_new(EKEY_S);
    ekey  k;

    char key[16];
    k.s = key;

    for(int i = 0; i < 1000; i++)
    {
        sprintf(key, "%d", i);

        edict_addR(d, k, 4);

        if(i % 7 == 0 || i % 13 == 0)
        {
            edict_clear(d);
            assert(edict_len(d) == 0);

        }
    }

    edict_clear(d);
    edict_free(d);
}

static void edict_clear_test()
{
    edict_clearI_test();
    edict_clearS_test();
}

int test_clear(int argc, char* argv[])
{
    edict_clear_test();

    return ETEST_OK;
}
