#include <stdio.h>
#include <inttypes.h>

#include "etype.h"
#include "etest.h"
#include "ecompat.h"

#include "test_main.h"

erb ti;
erb ts;

int erb_new_test()
{
    ti = erb_new(ERB_KEYI, 0);
    eunexpc_ptr(ti, 0);
    eexpect_num(eobj_typec((eobj)ti), ERB);
    eexpect_num(eobj_typeo((eobj)ti), EOBJ);
    eexpect_num(erb_len(ti), 0);

    ts = erb_new(ERB_KEYS, 0);
    eunexpc_ptr(ts, 0);
    eexpect_num(eobj_typec((eobj)ts), ERB);
    eexpect_num(eobj_typeo((eobj)ts), EOBJ);
    eexpect_num(erb_len(ts), 0);

    return ETEST_OK;
}

int erb_add_test()
{
    char key[] = "012345678901234567890";

    {
        erb t = ti;

        for(int i = 0; i < 10; i++)
            erb_addI(t, ekey_i(i), i);
        eexpect_num(erb_len(t), 10);

        for(int i = 0; i < 10; i++)
            erb_addI(t, ekey_i(i), i);
        eexpect_num(erb_len(t), 10);
    }

    {
        erb t = ts;

        for(int i = 0; i < 10; i++)
            erb_addI(t, ekey_s(&key[i]), i);
        eexpect_num(erb_len(t), 10);

        for(int i = 0; i < 10; i++)
            erb_addI(t, ekey_s(&key[i]), i);
        eexpect_num(erb_len(t), 10);
    }

    return ETEST_OK;
}

int erb_clear_test()
{
    erb_clear(ti);
    eexpect_num(erb_len(ti), 0);

    erb_clear(ts);
    eexpect_num(erb_len(ts), 0);

    return ETEST_OK;
}

int erb_val_test()
{
    eobj o; int i; char key[32] = "012345678901234567890";

    {
        erb t = ti;

        erb_addI(t, ekey_i(1), 1);
        erb_addF(t, ekey_i(2), 2);
        erb_addP(t, ekey_i(3), (cptr)3);
        erb_addS(t, ekey_i(4), "4");
        erb_addR(t, ekey_i(5), 5);
        eexpect_num(erb_len(t), 5);

        /// -- test 1
        i = 1;
        o = erb_val(t, ekey_i(i));
        eexpect_num(eobj_typeo(o), ENUM);
        eexpect_num(eobj_valI(o), 1);
        eexpect_num(eobj_valF(o), 1.0);

        i = 2;
        o = erb_val(t, ekey_i(i));
        eexpect_num(eobj_typeo(o), ENUM);
        eexpect_num(eobj_valI(o), 2);
        eexpect_num(eobj_valF(o), 2.0);

        i = 3;
        o = erb_val(t, ekey_i(i));
        eexpect_num(eobj_typeo(o), EPTR);
        eexpect_ptr(eobj_valP(o), (cptr)3);

        i = 4;
        o = erb_val(t, ekey_i(i));
        eexpect_num(eobj_typeo(o), ESTR);
        eexpect_str(eobj_valS(o), "4");
        eexpect_num(eobj_len(o), 1);

        i = 5;
        o = erb_val(t, ekey_i(i));
        eexpect_num(eobj_typeo(o), ERAW);
        eexpect_raw(eobj_valR(o), "\0\0\0\0\0", 5);
        eexpect_num(eobj_len(o), 5);

        i = 1;
        eexpect_num(eobj_typeo(erb_val(t, ekey_i(i))), ENUM);
        eexpect_num(erb_valI(t, ekey_i(i)), 1);
        eexpect_num(erb_valF(t, ekey_i(i)), 1.0);

        i = 2;
        eexpect_num(eobj_typeo(erb_val(t, ekey_i(i))), ENUM);
        eexpect_num(erb_valI(t, ekey_i(i)), 2);
        eexpect_num(erb_valF(t, ekey_i(i)), 2.0);

        i = 3;
        eexpect_num(eobj_typeo(erb_val(t, ekey_i(i))), EPTR);
        eexpect_ptr(erb_valP(t, ekey_i(i)), (cptr)3);

        i = 4;
        eexpect_num(eobj_typeo(erb_val(t, ekey_i(i))), ESTR);
        eexpect_str(erb_valS(t, ekey_i(i)), "4");

        i = 5;
        eexpect_num(eobj_typeo(erb_val(t, ekey_i(i))), ERAW);
        eexpect_raw(erb_valR(t, ekey_i(i)), "\0\0\0\0\0", 5);

        ETEST_RUN(erb_clear_test());

        /// -- test 3
        for(i = 0; i < 100; i++)
        {
            erb_addI(t, ekey_i(i), i);
        }
        for(i = 0; i < 100; i++)
        {
            eexpect_num(erb_valI(t, ekey_i(i)), i);
        }
    }

    {
        erb t = ts;

        erb_addI(t, ekey_s(&key[1]), 1);
        erb_addF(t, ekey_s(&key[2]), 2);
        erb_addP(t, ekey_s(&key[3]), (cptr)3);
        erb_addS(t, ekey_s(&key[4]), "4");
        erb_addR(t, ekey_s(&key[5]), 5);
        eexpect_num(erb_len(t), 5);

        /// -- test 1
        i = 1;
        o = erb_val(t, ekey_s(&key[i]));
        eexpect_num(eobj_typeo(o), ENUM);
        eexpect_num(eobj_valI(o), 1);
        eexpect_num(eobj_valF(o), 1.0);

        i = 2;
        o = erb_val(ts, ekey_s(&key[i]));
        eexpect_num(eobj_typeo(o), ENUM);
        eexpect_num(eobj_valI(o), 2);
        eexpect_num(eobj_valF(o), 2.0);

        i = 3;
        o = erb_val(t, ekey_s(&key[i]));
        eexpect_num(eobj_typeo(o), EPTR);
        eexpect_ptr(eobj_valP(o), (cptr)3);

        i = 4;
        o = erb_val(t, ekey_s(&key[i]));
        eexpect_num(eobj_typeo(o), ESTR);
        eexpect_str(eobj_valS(o), "4");
        eexpect_num(eobj_len(o), 1);

        i = 5;
        o = erb_val(ts, ekey_s(&key[i]));
        eexpect_num(eobj_typeo(o), ERAW);
        eexpect_raw(eobj_valR(o), "\0\0\0\0\0", 5);
        eexpect_num(eobj_len(o), 5);

        /// -- test 2
        i = 1;
        eexpect_num(eobj_typeo(erb_val(t, ekey_s(&key[i]))), ENUM);
        eexpect_num(erb_valI(t, ekey_s(&key[i])), 1);
        eexpect_num(erb_valF(t, ekey_s(&key[i])), 1.0);

        i = 2;
        eexpect_num(eobj_typeo(erb_val(t, ekey_s(&key[i]))), ENUM);
        eexpect_num(erb_valI(t, ekey_s(&key[i])), 2);
        eexpect_num(erb_valF(t, ekey_s(&key[i])), 2.0);

        i = 3;
        eexpect_num(eobj_typeo(erb_val(t, ekey_s(&key[i]))), EPTR);
        eexpect_ptr(erb_valP(t, ekey_s(&key[i])), (cptr)3);
        i = 4;
        eexpect_num(eobj_typeo(erb_val(t, ekey_s(&key[i]))), ESTR);
        eexpect_str(erb_valS(t, ekey_s(&key[i])), "4");

        i = 5;
        eexpect_num(eobj_typeo(erb_val(t, ekey_s(&key[i]))), ERAW);
        eexpect_raw(erb_valR(t, ekey_s(&key[i])), "\0\0\0\0\0", 5);

        ETEST_RUN(erb_clear_test());

        /// -- test 3
        for(i = 0; i < 100; i++)
        {
            sprintf(key, "%d", i);
            erb_addI(t, ekey_s(key), i);
        }
        for(i = 0; i < 100; i++)
        {
            sprintf(key, "%d", i);
            eexpect_num(erb_valI(t, ekey_s(key)), i);
        }
    }

    ETEST_RUN(erb_clear_test());

    return ETEST_OK;
}

int erb_free_test()
{
    if(ti)
    {
        eexpect_num(erb_free(ti) > 0, 1);
        ti = 0;
    }

    if(ts)
    {
        eexpect_num(erb_free(ts) > 0, 1);
        ts = 0;
    }

    return ETEST_OK;
}

int erb_addo_test()
{
    erb t = ti;

    erb_addO(t, ekey_i(1), erb_newO(EFALSE, 0));
    erb_addO(t, ekey_i(2), erb_newO(ETRUE,  0));
    erb_addO(t, ekey_i(3), erb_newO(ENULL,  0));
    erb_addO(t, ekey_i(4), erb_newO(ENUM,   0));
    erb_addO(t, ekey_i(5), erb_newO(EPTR,   0));
    erb_addO(t, ekey_i(6), erb_newO(ESTR,   0));
    erb_addO(t, ekey_i(7), erb_newO(EOBJ,   0));

    ETEST_RUN(erb_free_test());

    return ETEST_OK;
}

int erb_itr_test()
{
    char buf[32], key[32]; eobj itr; int i;

    {
        erb t = ti = erb_new(ERB_KEYI, 0);

        for(i = 0; i < 200; i++)
        {
            sprintf(buf, "%"PRId64"", i+100000000000);
            erb_addS(t, ekey_i(i), buf);
        }
        eexpect_num(erb_len(t), 200);

        i = 0;
        erb_foreach(t, itr)
        {
            sprintf(buf, "%"PRId64"", i+100000000000);
            eexpect_str(erb_valS(t, ekey_i(i)), buf);
            i++;
        }
    }

    {
        erb t = ts = erb_new(ERB_KEYS, 0);

        for(i = 0; i < 200; i++)
        {
            sprintf(key, "%d", i);
            sprintf(buf, "%"PRId64"", i+100000000000);
            erb_addS(t, ekey_s(key), buf);
        }

        eexpect_num(erb_len(t), 200);

        i = 0;
        erb_foreach(t, itr)
        {
            sprintf(key, "%d", i);
            sprintf(buf, "%"PRId64"", i+100000000000);
            eexpect_str(erb_valS(t, ekey_s(key)), buf);
            i++;
        }
    }

    ETEST_RUN(erb_free_test());

    return ETEST_OK;
}

int erb_take_test()
{
    int i; eobj itr; char key[32];

    {
        erb t = ti = erb_new(ERB_KEYI, 0);

        for(i = 0; i < 100; i++)
            erb_addI(t, ekey_i(i), i);

        while(erb_len(t))
        {
            i = random() % 100;

            erb_freeOne(t, ekey_i(i));

            erb_foreach(t, itr)
            {
                eexpect_ptr(erb_val(t, eobj_key(itr)), itr);
            }
        }
    }

    {
        erb t = ts = erb_new(ERB_KEYS, 0);

        for(i = 0; i < 100; i++)
        {
            sprintf(key, "%d", i);
            erb_addI(t, ekey_s(key), i);
        }

        while(erb_len(t))
        {
            i = random() % 100;

            sprintf(key, "%d", i);

            erb_freeOne(t, ekey_s(key));

            erb_foreach(t, itr)
            {
                eexpect_ptr(erb_val(t, eobj_key(itr)), itr);
            }
        }
    }

    ETEST_RUN(erb_free_test());

    return ETEST_OK;
}

int erb_addM_test()
{
    int i; eobj itr;

    ti = erb_new(ERB_KEYI, 0);

    for(i = 0; i < 100; i++)
    {
        erb_addI(ti, ekey_i(i), i);
    }

    eexpect_num(erb_len(ti), 100);

    for(i = 0; i < 100; i++)
    {
        erb_addI(ti, ekey_i(i), i);
    }

    eexpect_num(erb_len(ti), 100);

    for(i = 0; i < 100; i++)
    {
        erb_addMI(ti, ekey_i(i), i);
    }

    eexpect_num(erb_len(ti), 200);

    i = 0;
    erb_foreach(ti, itr)
    {
        eexpect_num(eobj_valI(itr), (int)(i / 2));

        i++;
    }

    erb_free_test();

    return ETEST_OK;
}

int test_basic(int argc, char* argv[])
{
    ETEST_RUN(erb_new_test());
    ETEST_RUN(erb_add_test());
    ETEST_RUN(erb_clear_test());
    ETEST_RUN(erb_val_test());
    ETEST_RUN(erb_free_test());
    ETEST_RUN(erb_itr_test());
    ETEST_RUN(erb_take_test());
    ETEST_RUN(erb_addM_test());

    return ETEST_OK;
}
