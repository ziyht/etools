#include <stdio.h>
#include <etest.h>

#include "test_main.h"

int estr_basic_test()
{
    printf("--------- estr basic test ------------\n"); fflush(stdout);

    estr s = estr_dupS("123"); estr_show(s);

    estr_catS(s, "Hello estr!"); estr_show(s);

    estr_catF(s, "%s", " an append str");estr_show(s);

    char news[100] = "123";
    estr_catB(s, news, 100); estr_show(s);

    estr_free(s);

    s = estr_newLen(0, 16); estr_show(s);
    estr_free(s);

    s = estr_newLen(0, 31); estr_show(s);
    estr_free(s);

    ///  word
    s = 0;
    estr_wrtW(s, "abc");        eexpect_str(s, "abc");
    estr_wrtW(s, "bcd abc ");   eexpect_str(s, "bcd");

    estr_catW(s, "abc bcd ");   eexpect_str(s, "bcdabc");

    estr_free(s);

    ///  line
    s = 0;
    estr_wrtL(s, "abc\n");      eexpect_str(s, "abc");
    estr_wrtL(s, "bcd\nabc");   eexpect_str(s, "bcd");

    estr_catL(s, "abc\n");      eexpect_str(s, "bcdabc");
    estr_catL(s, "abc");        eexpect_str(s, "bcdabcabc");

    printf("\n\n"); fflush(stdout);

    return ETEST_OK;
}

int estr_subc_test()
{
    estr e0, e1; cstr from, to; int cnt;

    printf("--------- estr subc test ------------\n"); fflush(stdout);

    e0 = estr_dupS("abcdcbd");
    e1 = estr_dupS("aascdasdabcsbcabbccabcdf");

    estr_show(e0);
    estr_show(e1);

    from = "abc"; to = "1234";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    cnt = estr_subc(e0, from, to); estr_show(e0); eexpect_num(cnt, 2);
    cnt = estr_subc(e1, from, to); estr_show(e1); eexpect_num(cnt, 5);

    from = "1234"; to = "*";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    cnt = estr_subc(e0, from, to); estr_show(e0); eexpect_num(cnt, 2);
    cnt = estr_subc(e1, from, to); estr_show(e1); eexpect_num(cnt, 5);

    estr_free(e0);
    estr_free(e1);

    printf("\n\n"); fflush(stdout);

    return ETEST_OK;
}

int estr_subs_test()
{
    estr e0 = 0, e1 = 0, e2 = 0, e3 = 0; cstr from, to; int cnt;

    printf("--------- estr subs test ------------\n"); fflush(stdout);

    e0 = estr_dupS("abcdasd");
    e1 = estr_dupS("abcd${PATH}asdasdf");
    e2 = estr_dupS("abcd${PATH}asdasdf${PATH}");
    e3 = estr_dupS("abcd${PATH}asdasdf${PATH}.../sdf///${PATH}fd%asd");
    estr_show(e0);
    estr_show(e1);
    estr_show(e2);
    estr_show(e3);

    from = "${PATH}"; to = "${}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    cnt = estr_subs(e0, from, to); estr_show(e0); eexpect_num(cnt, 0);
    cnt = estr_subs(e1, from, to); estr_show(e1); eexpect_num(cnt, 1);
    cnt = estr_subs(e2, from, to); estr_show(e2); eexpect_num(cnt, 2);
    cnt = estr_subs(e3, from, to); estr_show(e3); eexpect_num(cnt, 3);

    from = "${}"; to = "${PATH}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    cnt = estr_subs(e0, from, to); estr_show(e0); eexpect_num(cnt, 0);
    cnt = estr_subs(e1, from, to); estr_show(e1); eexpect_num(cnt, 1);
    cnt = estr_subs(e2, from, to); estr_show(e2); eexpect_num(cnt, 2);
    cnt = estr_subs(e3, from, to); estr_show(e3); eexpect_num(cnt, 3);

    from = "${PATH}"; to = "${abcd}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    cnt = estr_subs(e0, from, to); estr_show(e0); eexpect_num(cnt, 0);
    cnt = estr_subs(e1, from, to); estr_show(e1); eexpect_num(cnt, 1);
    cnt = estr_subs(e2, from, to); estr_show(e2); eexpect_num(cnt, 2);
    cnt = estr_subs(e3, from, to); estr_show(e3); eexpect_num(cnt, 3);

    from = "${abcd}"; to = "${abcde}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    cnt = estr_subs(e0, from, to); estr_show(e0); eexpect_num(cnt, 0);
    cnt = estr_subs(e1, from, to); estr_show(e1); eexpect_num(cnt, 10);
    cnt = estr_subs(e2, from, to); estr_show(e2); eexpect_num(cnt, 2);
    cnt = estr_subs(e3, from, to); estr_show(e3); eexpect_num(cnt, 3);

    from = "${abcde}"; to = "";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    cnt = estr_subs(e0, from, to); estr_show(e0); eexpect_num(cnt, 0);
    cnt = estr_subs(e1, from, to); estr_show(e1); eexpect_num(cnt, 1);
    cnt = estr_subs(e2, from, to); estr_show(e2); eexpect_num(cnt, 2);
    cnt = estr_subs(e3, from, to); estr_show(e3); eexpect_num(cnt, 3);

    estr_free(e0);
    estr_free(e1);
    estr_free(e2);
    estr_free(e3);

    printf("\n\n"); fflush(stdout);

    return ETEST_OK;
}

int estr_auto_create_test()
{
    estr e0 = 0, e1 = 0, e2 = 0, e3 = 0; cstr from, to;

    printf("--------- estr auto create subs test ------------\n"); fflush(stdout);

    estr_wrtS(e0, "abcdasd");
    estr_wrtS(e1, "abcd${PATH}asdasdf");
    estr_wrtS(e2, "abcd${PATH}asdasdf${PATH}");
    estr_wrtS(e3, "abcd${PATH}asdasdf${PATH}.../sdf///${PATH}fd%asd");
    estr_show(e0);
    estr_show(e1);
    estr_show(e2);
    estr_show(e3);

    from = "${PATH}"; to = "${}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    estr_subs(e0, from, to); estr_show(e0);
    estr_subs(e1, from, to); estr_show(e1);
    estr_subs(e2, from, to); estr_show(e2);
    estr_subs(e3, from, to); estr_show(e3);

    from = "${}"; to = "${PATH}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    estr_subs(e0, from, to); estr_show(e0);
    estr_subs(e1, from, to); estr_show(e1);
    estr_subs(e2, from, to); estr_show(e2);
    estr_subs(e3, from, to); estr_show(e3);

    from = "${PATH}"; to = "${abcd}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    estr_subs(e0, from, to); estr_show(e0);
    estr_subs(e1, from, to); estr_show(e1);
    estr_subs(e2, from, to); estr_show(e2);
    estr_subs(e3, from, to); estr_show(e3);

    from = "${abcd}"; to = "${abcde}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    estr_subs(e0, from, to); estr_show(e0);
    estr_subs(e1, from, to); estr_show(e1);
    estr_subs(e2, from, to); estr_show(e2);
    estr_subs(e3, from, to); estr_show(e3);

    from = "${abcde}"; to = "";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    estr_subs(e0, from, to); estr_show(e0);
    estr_subs(e1, from, to); estr_show(e1);
    estr_subs(e2, from, to); estr_show(e2);
    estr_subs(e3, from, to); estr_show(e3);

    estr_free(e0);
    estr_free(e1);
    estr_free(e2);
    estr_free(e3);

    printf("\n\n"); fflush(stdout);

    return ETEST_OK;
}

int test_estr(int argc, char* argv[])
{
    ETEST_RUN(estr_basic_test());
    ETEST_RUN(estr_auto_create_test());

    return ETEST_OK;
}
