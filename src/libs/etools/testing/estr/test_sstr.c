#include <stdio.h>

#include "test_main.h"

int sstr_basic_test()
{
    printf("--------- sstr basic test ------------\n"); fflush(stdout);
    char buf[50];  char news[10] = "add"; sstr s;

    s = sstr_init(buf, 10); estr_show(s);
    sstr_catS(s, "Hello estr!"); estr_show(s);
    sstr_catF(s, "%s", " an append str");estr_show(s);
    sstr_catB(s, news, 10); estr_show(s);
    puts("");

    s = sstr_init(buf, 12); sstr_show(s);
    sstr_catS(s, "Hello estr!"); sstr_show(s);
    sstr_catF(s, "%s", " an append str");sstr_show(s);
    sstr_catB(s, news, 10); sstr_show(s);
    puts("");

    s = sstr_init(buf, 50); sstr_show(s);
    sstr_catS(s, "Hello estr!"); sstr_show(s);
    sstr_catF(s, "%s", " an append str");sstr_show(s);
    sstr_catB(s, news, 10); sstr_show(s);

    printf("\n\n"); fflush(stdout);

    return ETEST_OK;
}

int sstr_subc_test()
{
    estr e0, e1; cstr from, to; char buf0[100]; char buf1[100];

    printf("--------- sstr subc test ------------\n"); fflush(stdout);

    e0 = sstr_init(buf0, 100);
    e1 = sstr_init(buf1, 100);

    sstr_wrtS(e0, "abcdcbd");
    sstr_wrtS(e1, "aascdasdabcsbcabbccabcdf");
    estr_show(e0);
    estr_show(e1);

    from = "abc"; to = "1234";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    sstr_subc(e0, from, to); estr_show(e0);
    sstr_subc(e1, from, to); estr_show(e1);

    from = "1234"; to = "*";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    sstr_subc(e0, from, to); estr_show(e0);
    sstr_subc(e1, from, to); estr_show(e1);

    printf("\n\n"); fflush(stdout);

    return ETEST_OK;
}

int sstr_subs_test()
{
    estr e0 = 0, e1 = 0, e2 = 0, e3 = 0; cstr from, to;

    char buf0[100]; char buf1[100]; char buf2[100]; char buf3[100];

    printf("--------- sstr subs test ------------\n"); fflush(stdout);

    e0 = sstr_init(buf0, 11);
    e1 = sstr_init(buf1, 22);
    e2 = sstr_init(buf2, 29);
    e3 = sstr_init(buf3, 80);

    sstr_wrtS(e0, "abcdasd");
    sstr_wrtS(e1, "abcd${PATH}asdasdf");
    sstr_wrtS(e2, "abcd${PATH}asdasdf${PATH}");
    sstr_wrtS(e3, "abcd${PATH}asdasdf${PATH}.../sdf///${PATH}fd%asd");
    estr_show(e0);
    estr_show(e1);
    estr_show(e2);
    estr_show(e3);

    from = "${PATH}"; to = "${}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    sstr_subs(e0, from, to); sstr_show(e0);
    sstr_subs(e1, from, to); sstr_show(e1);
    sstr_subs(e2, from, to); sstr_show(e2);
    sstr_subs(e3, from, to); sstr_show(e3);

    from = "${}"; to = "${PATH}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    sstr_subs(e0, from, to); sstr_show(e0);
    sstr_subs(e1, from, to); sstr_show(e1);
    sstr_subs(e2, from, to); sstr_show(e2);
    sstr_subs(e3, from, to); sstr_show(e3);

    from = "${PATH}"; to = "${abcd}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    sstr_subs(e0, from, to); sstr_show(e0);
    sstr_subs(e1, from, to); sstr_show(e1);
    sstr_subs(e2, from, to); sstr_show(e2);
    sstr_subs(e3, from, to); sstr_show(e3);

    from = "${abcd}"; to = "${abcde}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    sstr_subs(e0, from, to); sstr_show(e0);
    sstr_subs(e1, from, to); sstr_show(e1);
    sstr_subs(e2, from, to); sstr_show(e2);
    sstr_subs(e3, from, to); sstr_show(e3);

    from = "${abcde}"; to = "";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    sstr_subs(e0, from, to); sstr_show(e0);
    sstr_subs(e1, from, to); sstr_show(e1);
    sstr_subs(e2, from, to); sstr_show(e2);
    sstr_subs(e3, from, to); sstr_show(e3);

    printf("\n\n"); fflush(stdout);

    return ETEST_OK;
}

int test_sstr(int argc, char* argv[])
{
    ETEST_RUN(sstr_basic_test());
    ETEST_RUN(sstr_subc_test());
    ETEST_RUN(sstr_subs_test());

    return ETEST_OK;
}
