#include <string.h>
#include "test_main.h"

void echan_bin_test()
{
    echan chan = echan_new(ECHAN_LIST, 1);
    char s[256]; cptr r;
    strcpy(s, "hello world");
    echan_sendB(chan, s, sizeof(s));
    strcpy(s, "Hello World");
    r = echan_recvB(chan);
    assert_true(memcmp(s, r, sizeof(s)), chan, "Wrong value of buf");

    echan_freeO(r);
    echan_free(chan);
    pass();
}

void echan_int_test()
{
    echan chan = echan_new(ECHAN_LIST, 1);
    int s = 12345; int64_t r = 0;
    echan_sendI(chan, s);
    r = echan_recvI(chan);
    assert_true(s == r, chan, "Wrong value of int(12345)");

    int32_t s32 = 12345; int64_t r32 = 0;
    echan_sendI(chan, s32);
    r32 = echan_recvI(chan);
    assert_true(s32 == r32, chan, "Wrong value of int32(12345)");

    int64_t i64 = 12345, r64 = 0;
    echan_sendI(chan, i64);
    r64 =  echan_recvI(chan);
    assert_true(i64 == r64, chan, "Wrong value of int64(12345)");

    echan_free(chan);
    pass();
}

void echan_double_test()
{
    echan chan = echan_new(ECHAN_LIST, 1);
    double s = 123.45, r = 0;
    echan_sendF(chan, s);
    r = echan_recvF(chan);
    assert_true(s == r, chan, "Wrong value of double(123.45)");

    echan_free(chan);
    pass();
}

void echan_str_test()
{
    echan chan = echan_new(ECHAN_LIST, 1);
    char s[256]; cptr r;
    strcpy(s, "hello world");
    echan_sendS(chan, s);
    strcpy(s, "Hello World");
    r = echan_recvS(chan);
    assert_true(memcmp(s, r, sizeof(s)), chan, "Wrong value of buf");

    echan_freeO(r);
    echan_free(chan);
    pass();
}

void echan_ptr_test()
{
    echan chan = echan_new(ECHAN_LIST, 1);
    char s[256]; cptr r;
    strcpy(s, "hello world");
    echan_sendP(chan, s);
    strcpy(s, "Hello World");
    r = echan_recvP(chan);
    assert_true(!strcmp(s, r), chan, "Wrong value of buf");

    echan_free(chan);
    pass();
}

void echan_obj_test()
{
    echan chan = echan_new(ECHAN_LIST, 1);
    int s = 123456; cptr r;
    echan_sendI(chan, s);
    r = echan_recvO(chan);
    assert_true(eobj_valI(r) == s, chan, "Wrong value of buf");

    echan_freeO(r);
    echan_free(chan);
    pass();
}

int test_echan_type(int argc, char* argv[])
{
    echan_bin_test();
    echan_int_test();
    echan_double_test();
    echan_str_test();
    echan_ptr_test();
    echan_obj_test();

    return ETEST_OK;
}

