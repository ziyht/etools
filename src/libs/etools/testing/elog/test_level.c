#include "test_main.h"


void elog_level_test()
{
    elog l1 = elog_new("level_test", "./level.log");

    int grade;

    int c = ELOG(l1, ELOG_DBG, 3, 0);

    elog_dbg(l1, "set grade to %d", 3);
    elog_setLevel(ELOG(l1, ELOG_DBG, 3, 0));
    grade = 0; elog_dbg(ELOG(l1, 0, grade, 0), "level%2d", grade);
    grade = 1; elog_dbg(ELOG(l1, 0, grade, 0), "level%2d", grade);
    grade = 2; elog_dbg(ELOG(l1, 0, grade, 0), "level%2d", grade);
    grade = 3; elog_dbg(ELOG(l1, 0, grade, 0), "level%2d", grade);
    grade = 4; elog_dbg(ELOG(l1, 0, grade, 0), "level%2d", grade);
    grade = 5; elog_dbg(ELOG(l1, 0, grade, 0), "level%2d", grade);
    grade = 6; elog_dbg(ELOG(l1, 0, grade, 0), "level%2d", grade);

    grade =16; elog_dbg(ELOG_G(l1, grade), "level%2d", grade);

    elog_dbg(l1, "set grade to %d", 15);
    elog_setLevel(ELOG_G(l1, 15));
    grade = 7; elog_dbg(ELOG_G(l1, grade), "level%2d", grade);
    grade = 8; elog_dbg(ELOG_G(l1, grade), "level%2d", grade);
    grade = 9; elog_dbg(ELOG_G(l1, grade), "level%2d", grade);
    grade =10; elog_dbg(ELOG_G(l1, grade), "level%2d", grade);
    grade =11; elog_dbg(ELOG_G(l1, grade), "level%2d", grade);
    grade =12; elog_dbg(ELOG_G(l1, grade), "level%2d", grade);
    grade =13; elog_dbg(ELOG_G(l1, grade), "level%2d", grade);
    grade =14; elog_dbg(ELOG_G(l1, grade), "level%2d", grade);
    grade =15; elog_dbg(ELOG_G(l1, grade), "level%2d", grade);
    grade =16; elog_dbg(ELOG_G(l1, grade), "level%2d", grade);

    elog_free(l1);
}

int test_level(int argc, char* argv[])
{
    elog_level_test();

    return ETEST_OK;
}
