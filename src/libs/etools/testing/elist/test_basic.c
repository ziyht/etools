#include "test_main.h"
#include "etest.h"

static elist l;

typedef struct obj_s{
    int  key;
    char val[16];
}obj_t, * obj;



static int __alloc_cnt;
static int __free_cnt;

static void __resetCnt()    { __alloc_cnt = __free_cnt = 0;}
static int  __checkCnt()    { eexpect_num(__alloc_cnt, __free_cnt); return ETEST_OK; }
static cptr __new()         { __alloc_cnt++; return emalloc(4); }
static void __free(eobj o)  { __free_cnt++ ; efree(o->p);   }

static int elist_new_test();
static int elist_push_test();
static int elist_appd_test();
static int elist_at_test();
static int elist_val_test();
static int elist_take_test();
static int elist_clear_test();
static int elist_free_test();

int elist_etest_basic()
{
    ETEST_RUN(elist_new_test());
    ETEST_RUN(elist_push_test());
    ETEST_RUN(elist_appd_test());
    ETEST_RUN(elist_at_test());
    ETEST_RUN(elist_val_test());
    ETEST_RUN(elist_take_test());
    ETEST_RUN(elist_clear_test());
    ETEST_RUN(elist_free_test());

    return ETEST_OK;
}

static int elist_new_test()
{
    l = elist_new();

    eunexpc_ptr(l, 0);
    eexpect_num(elist_len(l), 0);

    return ETEST_OK;
}

static int elist_push_test()
{
    int i;

    for(i = 0; i < 1000; i++)
    {
        elist_pushI(l, i);
    }
    eexpect_num(elist_len(l), 1000);
    elist_clear(l);


    // push 1 -> [1]
    eobj oi = elist_pushI(l, 1);
    eunexpc_ptr(oi, 0);
    eexpect_num(elist_len(l), 1);

    oi = elist_at(l, 0);
    eunexpc_ptr(oi, 0);
    eexpect_num(EOBJ_VALI(oi), 1);

    eexpect_num(elist_valI(l, 0), 1);
    eexpect_num(elist_valF(l, 0), 1.0);

    // push 2.0 -> [2.0, 1]
    oi = elist_pushF(l, 2);
    eunexpc_ptr(oi, 0);
    eexpect_num(elist_len(l), 2);

    oi = elist_at(l, 0);
    eunexpc_ptr(oi, 0);
    eexpect_num(EOBJ_VALF(oi), 2);

    eexpect_num(elist_valI(l, 0), 2);
    eexpect_num(elist_valF(l, 0), 2.0);

    elist_clear(l);

    return ETEST_OK;
}

static int elist_appd_test()
{
    int i;

    for(i = 0; i < 1000; i++)
    {
        elist_appdI(l, i);
    }
    eexpect_num(elist_len(l), 1000);
    elist_clear(l);

    // appd 1 -> [1]
    eobj oi = elist_appdI(l, 1);
    eunexpc_ptr(oi, 0);
    eexpect_num(elist_len(l), 1);

    oi = elist_at(l, 0);
    eunexpc_ptr(oi, 0);
    eexpect_num(EOBJ_VALI(oi), 1);

    eexpect_num(elist_valI(l, 0), 1);
    eexpect_num(elist_valF(l, 0), 1.0);

    // appd 2.0 -> [1, 2.0]
    eobj of = elist_appdF(l, 2);
    eunexpc_ptr(of, 0);
    eexpect_num(elist_len(l), 2);

    of = elist_at(l, 1);
    eunexpc_ptr(of, 0);
    eexpect_num(EOBJ_VALF(of), 2.0);

    eexpect_num(elist_valI(l, 1), 2);
    eexpect_num(elist_valF(l, 1), 2.0);

    elist_clear(l);

    return ETEST_OK;
}

static int elist_at_test()
{
    char data[] = "1234567890";

    eobj o, ao = elist_newO(EOBJ, 6);

    memcpy(ao->r, data, 6);

    elist_appdF(l, 1.0);
    elist_appdI(l, 2.0);
    elist_appdP(l, 0);
    elist_appdS(l, "4");
    o =
    elist_appdR(l, 5);  memcpy(o->r, data, 5);
    elist_appdO(l, ao);

    eexpect_num(elist_len(l), 6);

    o = elist_at(l, 0);
    eexpect_num(eobj_typev(o), ENUM);
    eexpect_num(eobj_valI(o), 1);
    eexpect_num(eobj_valF(o), 1.0);

    o = elist_at(l, 1);
    eexpect_num(eobj_typev(o), ENUM);
    eexpect_num(eobj_valI(o), 2);
    eexpect_num(eobj_valF(o), 2.0);

    o = elist_at(l, 2);
    eexpect_num(eobj_typev(o), EPTR);
    eexpect_ptr(eobj_valP(o), 0);

    o = elist_at(l, 3);
    eexpect_num(eobj_typev(o), ESTR);
    eexpect_str(eobj_valS(o), "4");

    o = elist_at(l, 4);
    eexpect_num(eobj_typev(o), ERAW);
    eexpect_num(eobj_len( o), 5);
    eexpect_raw(o->r, data, eobj_len( o));

    o = elist_at(l, 5);
    eexpect_num(eobj_typev(o), EOBJ);
    eexpect_num(eobj_len( o), 6);
    eexpect_raw(o->r, data, eobj_len( o));

    elist_clear(l);

    {
        for(int i = 0; i < 1000; i++)
        {
            obj_t* o = (obj_t*)elist_appdR(l, sizeof(obj_t));

            o->key = i;
        }

        srand(0);
        int i = 0;
        while(i < 1000)
        {
            int idx = rand() % 1000;

            obj_t* o = (obj_t*)elist_at(l, idx);

            eexpect_num(o->key, idx);

            if(!o->val[0])
            {
                o->val[0] = 1;
                i++;
            }
        }

        elist_clear(l);
    }

    return ETEST_OK;
}

static int elist_val_test()
{
    int i; char data[] = "1234567890";

    eobj o, ao = elist_newO(EOBJ, 6);
    memcpy(ao->r, data, 6);

    elist_appdF(l, 1.0);
    elist_appdI(l, 2.0);
    elist_appdP(l, 0);
    elist_appdS(l, "4");
    o =
    elist_appdR(l,  5 );    memcpy(o->r, data, 5);
    elist_appdO(l, ao );

    eexpect_num(elist_len(l), 6);

    i = 0;
    o = elist_val(l, i);
    eexpect_num(eobj_typev(o), ENUM);
    eexpect_num(eobj_valI(o), 1);
    eexpect_num(eobj_valF(o), 1.0);

    i = 1;
    o = elist_val(l, i);
    eexpect_num(eobj_typev(o), ENUM);
    eexpect_num(eobj_valI(o), 2);
    eexpect_num(eobj_valF(o), 2.0);

    i = 2;
    o = elist_val(l, i);
    eexpect_num(eobj_typev(o), EPTR);
    eexpect_ptr(eobj_valP(o), 0);

    i = 3;
    o = elist_val(l, i);
    eexpect_num(eobj_typev(o), ESTR);
    eexpect_str(eobj_valS(o), "4");

    i = 4;
    o = elist_val(l, i);
    eexpect_num(eobj_typev(o), ERAW);
    eexpect_num(eobj_len( o), 5);

    i = 5;
    o = elist_val(l, i);
    eexpect_num(eobj_typev(o), EOBJ);
    eexpect_num(eobj_len( o), 6);
    eexpect_raw(o, data, 6);

    /// -- test 2
    i = 0;
    eexpect_num(eobj_typev(elist_val(l, i)), ENUM);
    eexpect_num(elist_valI(l, i), 1);
    eexpect_num(elist_valF(l, i), 1.0);

    i = 1;
    eexpect_num(eobj_typev(elist_val(l, i)), ENUM);
    eexpect_num(elist_valI(l, i), 2);
    eexpect_num(elist_valF(l, i), 2.0);

    i = 2;
    eexpect_num(eobj_typev(elist_val(l, i)), EPTR);
    eexpect_ptr(elist_valP(l, i), 0);

    i = 3;
    eexpect_num(eobj_typev(elist_val(l, i)), ESTR);
    eexpect_str(elist_valS(l, i), "4");

    i = 4;
    eexpect_num(eobj_typev(elist_val(l, i)), ERAW);
    eexpect_num(eobj_len (elist_val(l, i)), 5);
    eexpect_raw(elist_valR(l, i), data, eobj_len (elist_val(l, i)));

    i = 5;
    eexpect_num(eobj_typev(elist_val(l, i)), EOBJ);
    eexpect_num(eobj_len (elist_val(l, i)), 6);
    eexpect_raw(elist_val(l, i), data, eobj_len(elist_val(l, i)));

    elist_clear(l);

    {
        for(int i = 0; i < 1000; i++)
        {
            obj_t* o = (obj_t*)elist_appdR(l, sizeof(obj_t));

            o->key = i;
        }

        srand(0);
        int i = 0;
        while(i < 1000)
        {
            int idx = rand() % 1000;

            obj_t* o = (obj_t*)elist_val(l, idx);

            eexpect_num(o->key, idx);

            if(!o->val[0])
            {
                o->val[0] = 1;
                i++;
            }
        }

        elist_clear(l);
    }

    return ETEST_OK;
}

static int elist_take_test()
{
    int scale = 100;

    {
        for(int i = 0; i < scale; i++)
        {
            eobj o = elist_appdR(l, sizeof(eobj_t));

            o->v.i32_[0] = i;
        }

        int i, idx;
        while(elist_len(l))
        {
            idx = rand() % elist_len(l);

            eobj itr, o = elist_at(l, idx);

            i = 0;
            elist_foreach(l, itr)
            {
                if(itr == o)
                {
                    eexpect_num(i, idx);
                    break;
                }

                i++;
            }

            o->v.i32_[1] = 1;

            o = elist_first(l);
            while(o && o->v.i32_[1])
            {
                o = elist_takeH(l);
                elist_freeO(0, o);

                o = elist_first(l);
            }
        }
        elist_clear(l);
    }

    {
        for(int i = 0; i < scale; i++)
        {
            eobj o = elist_appdR(l, sizeof(eobj_t));

            o->v.i32_[0] = i;
        }

        int i, idx;
        while(elist_len(l))
        {
            idx = rand() % elist_len(l);

            eobj itr, o = elist_at(l, idx);

            i = 0;
            elist_foreach(l, itr)
            {
                if(itr == o)
                {
                    eexpect_num(i, idx);
                    break;
                }

                i++;
            }

            o->v.i32_[1] = 1;

            o = elist_last(l);
            while(o && o->v.i32_[1])
            {
                o = elist_takeT(l);
                elist_freeO(0, o);

                o = elist_last(l);
            }
        }
        elist_clear(l);
    }

    {
        for(int i = 0; i < scale; i++)
        {
            eobj o = elist_appdR(l, sizeof(eobj_t));

            o->v.i32_[0] = i;
        }

        int i, idx;
        while(elist_len(l))
        {
            idx = rand() % elist_len(l);

            eobj itr, o = elist_at(l, idx);

            i = 0;
            elist_foreach(l, itr)
            {
                if(itr == o)
                {
                    eexpect_num(i, idx);
                    break;
                }

                i++;
            }

            o->v.i32_[1] = 1;

            if(elist_len(l) % 2 == 0)
            {
                o = elist_first(l);
                while(o && o->v.i32_[1])
                {
                    o = elist_takeH(l);
                    elist_freeO(0, o);

                    o = elist_first(l);
                }
            }
            else
            {
                o = elist_last(l);
                while(o && o->v.i32_[1])
                {
                    o = elist_takeT(l);
                    elist_freeO(0, o);

                    o = elist_last(l);
                }
            }
        }
        elist_clear(l);
    }

    return ETEST_OK;
}

static int elist_clear_test()
{
    int scale = 100;

    /// -- elist_clear()
    {
        for(int i = 0; i < scale; i++)
        {
            elist_appdP(l, 0);
        }

        elist_clear(l);

        eexpect_num(elist_len(l), 0);
    }

    /// -- elist_clearEx()
    {
        for(int i = 0; i < scale; i++)
        {
            elist_appdP(l, __new());
        }

        elist_clearEx(l, __free);
        eexpect_num(elist_len(l), 0);
        ETEST_RUN(__checkCnt());

        __resetCnt();
    }

    return ETEST_OK;
}

static int elist_free_test()
{
    int scale = 100;

    /// -- elist_freeH()
    {
        for(int i = 0; i < scale; i++)
        {
            eobj o = elist_appdR(l, sizeof(eobj_t));

            o->v.i32_[0] = i;
        }

        int i, idx;
        while(elist_len(l))
        {
            idx = rand() % elist_len(l);

            eobj itr, o = elist_at(l, idx);

            i = 0;
            elist_foreach(l, itr)
            {
                if(itr == o)
                {
                    eexpect_num(i, idx);
                    break;
                }

                i++;
            }

            o->v.i32_[1] = 1;

            o = elist_first(l);
            while(o && o->v.i32_[1])
            {
                elist_freeH(l);

                o = elist_first(l);
            }
        }
        elist_clear(l);
    }

    /// -- elist_freeT()
    {
        for(int i = 0; i < scale; i++)
        {
            eobj o = elist_appdR(l, sizeof(eobj_t));

            o->v.i32_[0] = i;
        }

        int i, idx;
        while(elist_len(l))
        {
            idx = rand() % elist_len(l);

            eobj itr, o = elist_at(l, idx);

            i = 0;
            elist_foreach(l, itr)
            {
                if(itr == o)
                {
                    eexpect_num(i, idx);
                    break;
                }

                i++;
            }

            o->v.i32_[1] = 1;

            o = elist_last(l);
            while(o && o->v.i32_[1])
            {
                elist_freeT(l);

                o = elist_last(l);
            }
        }
        elist_clear(l);
    }

    /// -- elist_freeI() from last
    {
        for(int i = 0; i < scale; i++)
        {
            elist_appdI(l, i);
        }

        while(elist_len(l))
        {
            eexpect_num(elist_valI(l, elist_len(l) - 1), elist_len(l) - 1);

            elist_freeI(l, elist_len(l) - 1);
        }
    }

    /// -- elist_freeI() from first
    {
        for(int i = 0; i < scale; i++)
        {
            elist_appdI(l, i);
        }

        while(elist_len(l))
        {
            eexpect_num(elist_valI(l, 0), scale - elist_len(l));

            elist_freeI(l, 0);
        }
    }

    /// -- elist_freeO()
    {
        for(int i = 0; i < scale; i++)
        {
            elist_appdI(l, i);
        }

        while(elist_len(l))
        {
            eobj o = elist_at(l, rand() % elist_len(l));

            elist_freeO(l, o);
        }
    }

    /// -- elist_freeEx()
    {
        for(int i = 0; i < scale; i++)
        {
            elist_appdP(l, __new());
        }

        elist_freeEx(l, __free);
        ETEST_RUN(__checkCnt());

        __resetCnt();
    }

    return ETEST_OK;
}


void my_list_show(elist l)
{
    eobj o;

    printf("len: %d\n", elist_len(l));

    for(o = elist_first(l); o; o = elist_next(o))
    {
        printf("  %4d: %s\n", ((obj)o)->key, ((obj)o)->val);
    }

    printf("\n");
    fflush(stdout);
}

void my_list_showr(elist l)
{
    eobj o;

    printf("len: %d\n", elist_len(l));

    for(o = elist_last(l); o; o = elist_prev(o))
    {
        printf("  %4d: %s\n", ((obj)o)->key, ((obj)o)->val);
    }

    printf("\n");
    fflush(stdout);
}

int elist_basic_test()
{
    elist l; eobj o;

    l = elist_new();

    for(int i = 0; i < 10; i++)
    {
        o = elist_appdR(l, sizeof(obj_t));

        ((obj)o)->key = i + 1;

        snprintf(((obj)o)->val, 16, "%d", ((obj)o)->key + 10);
    }

    my_list_show(l);
    //my_list_showr(l);

    elist_freeH(l);
    elist_freeT(l);
    my_list_show(l);
    //my_list_showr(l);

    elist_freeI(l, 3);
    my_list_show(l);
    //my_list_showr(l);

    elist_free(l);

    return ETEST_OK;
}


int test_basic(int argc, char* argv[])
{
    return elist_etest_basic();
}


