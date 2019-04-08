/**
* this file is create by subsystem 'ETest' from EMake framework
*/

#include <etest.h>

#include "evar.h"

static int gen_auto_val()
{
    evar v; int i = 0;

    for(; i < 2; i++)
    {
        v = evar_gen(E_I8, i, 0);
        eexpect_num((uint)v.type, E_I8);
        eexpect_num(      v.cnt , 1);
        eexpect_num((uint)v.size, 1);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 0);

        v = evar_gen(E_I16, i, 0);
        eexpect_num((uint)v.type, E_I16);
        eexpect_num(      v.cnt , 1);
        eexpect_num((uint)v.size, 2);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 0);

        v = evar_gen(E_I32, i, 0);
        eexpect_num((uint)v.type, E_I32);
        eexpect_num(      v.cnt , 1);
        eexpect_num((uint)v.size, 4);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 0);

        v = evar_gen(E_I64, i, 0);
        eexpect_num((uint)v.type, E_I64);
        eexpect_num(      v.cnt , 1);
        eexpect_num((uint)v.size, 8);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 0);

        v = evar_gen(E_U8, i, 0);
        eexpect_num((uint)v.type, E_U8);
        eexpect_num(      v.cnt , 1);
        eexpect_num((uint)v.size, 1);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 0);

        v = evar_gen(E_U16, i, 0);
        eexpect_num((uint)v.type, E_U16);
        eexpect_num(      v.cnt , 1);
        eexpect_num((uint)v.size, 2);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 0);

        v = evar_gen(E_U32, i, 0);
        eexpect_num((uint)v.type, E_U32);
        eexpect_num(      v.cnt , 1);
        eexpect_num((uint)v.size, 4);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 0);

        v = evar_gen(E_U64, i, 0);
        eexpect_num((uint)v.type, E_U64);
        eexpect_num(      v.cnt , 1);
        eexpect_num((uint)v.size, 8);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 0);

        v = evar_gen(E_F32, i, 0);
        eexpect_num((uint)v.type, E_F32);
        eexpect_num(      v.cnt , 1);
        eexpect_num((uint)v.size, 4);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 0);

        v = evar_gen(E_F64, i, 0);
        eexpect_num((uint)v.type, E_F64);
        eexpect_num(      v.cnt , 1);
        eexpect_num((uint)v.size, 8);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 0);
    }

    return ETEST_OK;
}

static int gen_auto_arr()
{
    evar v;

    for(int i = 2; i < 8; i++)
    {
        v = evar_gen(E_I8, i, 0);

        eexpect_num((uint)v.type, E_I8_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 1);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    for(int i = 2; i <= 4; i++)
    {
        v = evar_gen(E_I16, i, 0);

        eexpect_num((uint)v.type, E_I16_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 2);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    for(int i = 2; i <= 2; i++)
    {
        v = evar_gen(E_I32, i, 0);

        eexpect_num((uint)v.type, E_I32_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 4);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    for(int i = 2; i < 8; i++)
    {
        v = evar_gen(E_U8, i, 0);

        eexpect_num((uint)v.type, E_U8_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 1);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    for(int i = 2; i <= 4; i++)
    {
        v = evar_gen(E_U16, i, 0);

        eexpect_num((uint)v.type, E_U16_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 2);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    for(int i = 2; i <= 2; i++)
    {
        v = evar_gen(E_U32, i, 0);

        eexpect_num((uint)v.type, E_U32_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 4);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    for(int i = 2; i <= 2; i++)
    {
        v = evar_gen(E_F32, i, 0);

        eexpect_num((uint)v.type, E_F32_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 4);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    return ETEST_OK;
}

static int gen_arr()
{
    evar v;

    for(int i = 1; i < 8; i++)
    {
        v = evar_gen(E_I8_a, i, 0);

        eexpect_num((uint)v.type, E_I8_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 1);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    for(int i = 1; i <= 4; i++)
    {
        v = evar_gen(E_I16_a, i, 0);

        eexpect_num((uint)v.type, E_I16_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 2);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    for(int i = 1; i <= 2; i++)
    {
        v = evar_gen(E_I32_a, i, 0);

        eexpect_num((uint)v.type, E_I32_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 4);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    for(int i = 1; i <= 1; i++)
    {
        v = evar_gen(E_I64_a, i, 0);

        eexpect_num((uint)v.type, E_I64_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 8);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    for(int i = 1; i < 8; i++)
    {
        v = evar_gen(E_U8_a, i, 0);

        eexpect_num((uint)v.type, E_U8_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 1);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    for(int i = 1; i <= 4; i++)
    {
        v = evar_gen(E_U16_a, i, 0);

        eexpect_num((uint)v.type, E_U16_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 2);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    for(int i = 1; i <= 2; i++)
    {
        v = evar_gen(E_U32_a, i, 0);

        eexpect_num((uint)v.type, E_U32_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 4);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    for(int i = 1; i <= 1; i++)
    {
        v = evar_gen(E_U64_a, i, 0);

        eexpect_num((uint)v.type, E_U64_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 8);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    for(int i = 1; i <= 2; i++)
    {
        v = evar_gen(E_F32_a, i, 0);

        eexpect_num((uint)v.type, E_F32_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 4);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    for(int i = 1; i <= 1; i++)
    {
        v = evar_gen(E_F64_a, i, 0);

        eexpect_num((uint)v.type, E_F64_a);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 8);
        eexpect_num(evar_isArr(&v), 1);
        eexpect_num(evar_isPtr(&v), 0);
    }

    return ETEST_OK;
}

static int gen_ptr()
{
    evar v;

    for(int i = 1; i < 8; i++)
    {
        v = evar_gen(E_I8_p, i, 0);
        eexpect_num((uint)v.type, E_I8_p);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 1);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 1);
        eexpect_num(evar_free (&v), i);

        v = evar_gen(E_I16_p, i, 0);
        eexpect_num((uint)v.type, E_I16_p);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 2);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 1);
        eexpect_num(evar_free (&v), i);

        v = evar_gen(E_I32_p, i, 0);
        eexpect_num((uint)v.type, E_I32_p);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 4);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 1);
        eexpect_num(evar_free (&v), i);

        v = evar_gen(E_I64_p, i, 0);
        eexpect_num((uint)v.type, E_I64_p);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 8);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 1);
        eexpect_num(evar_free (&v), i);

        v = evar_gen(E_U8_p, i, 0);
        eexpect_num((uint)v.type, E_U8_p);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 1);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 1);
        eexpect_num(evar_free (&v), i);

        v = evar_gen(E_U16_p, i, 0);
        eexpect_num((uint)v.type, E_U16_p);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 2);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 1);
        eexpect_num(evar_free (&v), i);

        v = evar_gen(E_U32_p, i, 0);
        eexpect_num((uint)v.type, E_U32_p);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 4);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 1);
        eexpect_num(evar_free (&v), i);

        v = evar_gen(E_U64_p, i, 0);
        eexpect_num((uint)v.type, E_U64_p);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 8);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 1);
        eexpect_num(evar_free (&v), i);

        v = evar_gen(E_F32_p, i, 0);
        eexpect_num((uint)v.type, E_F32_p);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 4);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 1);
        eexpect_num(evar_free (&v), i);

        v = evar_gen(E_F64_p, i, 0);
        eexpect_num((uint)v.type, E_F64_p);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 8);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 1);
        eexpect_num(evar_free (&v), i);

        v = evar_gen(E_USER_p, i, 16);
        eexpect_num((uint)v.type, E_USER_p);
        eexpect_num(      v.cnt , i);
        eexpect_num((uint)v.size, 16);
        eexpect_num(evar_isArr(&v), 0);
        eexpect_num(evar_isPtr(&v), 1);
        eexpect_num(evar_free (&v), i);
    }

    return ETEST_OK;
}


int gen_test(int argc, char* argv[])
{
    ETEST_RUN( gen_auto_val() );
    ETEST_RUN( gen_auto_arr() );
    ETEST_RUN( gen_arr() );
    ETEST_RUN( gen_ptr() );

    return ETEST_OK;
}

