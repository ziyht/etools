#include <stdio.h>
#include "etype.h"
#include "test_main.h"



int main()
{
    eatom_test_set();
    eatom_test_get();
    eatom_test_add();
    eatom_test_sub();
    eatom_test_inc();
    eatom_test_dec();
    eatom_test_cas();

    return 0;
}

#include "eobj.h"


void eatom_test_set()
{
    eval val; eval ret;

    // -- 8
    val.i8 = 1; ret.i8   = eatom_set8 (val. i8,  0); etest_equal_num(val. i8,  0); etest_equal_num(ret. i8, 1);
    val.i8 = 1; ret.i8   = eatom_set8 (val. i8, -1); etest_equal_num(val. i8, -1); etest_equal_num(ret. i8, 1);

    // -- 16
    val.i16 = 1; ret.i16 = eatom_set16(val.i16,  0); etest_equal_num(val.i16,  0); etest_equal_num(ret.i16, 1);
    val.i16 = 1; ret.i16 = eatom_set16(val.i16, -1); etest_equal_num(val.i16, -1); etest_equal_num(ret.i16, 1);

    // -- 32
    val.i32 = 1; ret.i32 = eatom_set32(val.i32,  0); etest_equal_num(val.i32,  0); etest_equal_num(ret.i32, 1);
    val.i32 = 1; ret.i32 = eatom_set32(val.i32, -1); etest_equal_num(val.i32, -1); etest_equal_num(ret.i32, 1);

    // -- 64
    val.i64 = 1; ret.i64 = eatom_set64(val.i64,  0); etest_equal_num(val.i64,  0); etest_equal_num(ret.i64, 1);
    val.i64 = 1; ret.i64 = eatom_set64(val.i64, -1); etest_equal_num(val.i64, -1); etest_equal_num(ret.i64, 1);
}

void eatom_test_get()
{
    eval_t val;

    // -- 8
    val.i8 =  1; etest_equal_num(eatom_get8(val.i8),   1);
    val.i8 = -1; etest_equal_num(eatom_get8(val.i8),  -1);

    // -- 16
    val.i16 =  1; etest_equal_num(eatom_get16(val.i16),   1);
    val.i16 = -1; etest_equal_num(eatom_get16(val.i16),  -1);

    // -- 32
    val.i32 =  1; etest_equal_num(eatom_get32(val.i32),   1);
    val.i32 = -1; etest_equal_num(eatom_get32(val.i32),  -1);

    // -- 64
    val.i64 =  1; etest_equal_num(eatom_get64(val.i64),   1);
    val.i64 = -1; etest_equal_num(eatom_get64(val.i64),  -1);

}

void eatom_test_add()
{
    eval val; eval ret;

    // -- 8

    // -- 16
    val.i16 = 1; ret.i16 = eatom_xadd32(val.i16, 2); etest_equal_num(val.i16, 3); etest_equal_num(ret.i16, 1);

    // -- 32
    val.i32 = 1; ret.i32 = eatom_add32 (val.i32, 2); etest_equal_num(val.i32, 3); etest_equal_num(ret.i32, 3);

    val.i32 = 1; ret.i32 = eatom_xadd32(val.i32, 2); etest_equal_num(val.i32, 3); etest_equal_num(ret.i32, 1);


    // -- 64
    val.i64 = 1; ret.i64 = eatom_add32 (val.i64, 2); etest_equal_num(val.i64, 3); etest_equal_num(ret.i64, 3);

    val.i64 = 1; ret.i64 = eatom_xadd32(val.i64, 2); etest_equal_num(val.i64, 3); etest_equal_num(ret.i64, 1);

}

void eatom_test_sub()
{
    eval val; eval ret;

    // -- 8

    // -- 16

    // -- 32
    val.i32 = 1; ret.i32 = eatom_sub32 (val.i32, 2); etest_equal_num(val.i32, -1); etest_equal_num(ret.i32, -1);

    // -- 64
    val.i64 = 1; ret.i64 = eatom_sub32 (val.i64, 2); etest_equal_num(val.i64, -1); etest_equal_num(ret.i64, -1);
}

void eatom_test_inc()
{
    eval val; eval ret;

    // -- 8

    // -- 16
    val.i16 = 1; ret.i16 = eatom_inc16 (val.i16); etest_equal_num(val.i16, 2); etest_equal_num(ret.i16, 2);

    // -- 32
    val.i32 = 1; ret.i32 = eatom_inc32 (val.i32); etest_equal_num(val.i32, 2); etest_equal_num(ret.i32, 2);

    // -- 64
    val.i64 = 1; ret.i64 = eatom_inc64 (val.i64); etest_equal_num(val.i64, 2); etest_equal_num(ret.i64, 2);
}

void eatom_test_dec()
{
    eval val; eval ret;

    // -- 8

    // -- 16
    val.i16 = 0; ret.i16 = eatom_dec16 (val.i16); etest_equal_num(val.i16, -1); etest_equal_num(ret.i16, -1);

    // -- 32
    val.i32 = 0; ret.i32 = eatom_dec32 (val.i32); etest_equal_num(val.i32, -1); etest_equal_num(ret.i32, -1);

    // -- 64
    val.i64 = 0; ret.i64 = eatom_dec64 (val.i64); etest_equal_num(val.i64, -1); etest_equal_num(ret.i64, -1);
}

void eatom_test_cas()
{
    eval val; eval ret;

    // -- 8

    // -- 16
    val.i16 = 3; ret.i16 = eatom_cas16(val.i16, 3, 4); etest_equal_num(val.i16, 4); etest_equal_num(ret.i16, 3);
    val.i16 = 5; ret.i16 = eatom_cas16(val.i16, 3, 4); etest_equal_num(val.i16, 5); etest_equal_num(ret.i16, 5);

    // -- 32
    val.i32 = 3; ret.i32 = eatom_cas16(val.i32, 3, 4); etest_equal_num(val.i32, 4); etest_equal_num(ret.i32, 3);
    val.i32 = 5; ret.i32 = eatom_cas16(val.i32, 3, 4); etest_equal_num(val.i32, 5); etest_equal_num(ret.i32, 5);

    // -- 64
    val.i64 = 3; ret.i64 = eatom_cas16(val.i64, 3, 4); etest_equal_num(val.i64, 4); etest_equal_num(ret.i64, 3);
    val.i64 = 5; ret.i64 = eatom_cas16(val.i64, 3, 4); etest_equal_num(val.i64, 5); etest_equal_num(ret.i64, 5);

}
