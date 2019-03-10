#include "etest.h"
#include "elist.h"

int traverse1(int scale)
{
    elist l; eobj itr; int i;

    l = elist_new();

    for(i = 0; i < scale; i++)
    {
        elist_appdI(l, i);
    }

    i = 0;
    elist_foreach(l, itr)
    {
        eexpect_num(EOBJ_VALI(itr), i);
        i++;
    }

    for(itr=elist_last(l); (itr); itr = elist_prev(itr))
    {
        i--;
        eexpect_num(EOBJ_VALI(itr), i);
    }

    elist_free(l);

    return ETEST_OK;
}



int test_traverse(int argc, char* argv[])
{
    ETEST_RUN(traverse1(1000));

    return ETEST_OK;
}
