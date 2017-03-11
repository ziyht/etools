#include <stdio.h>

#include "test_main.h"

void estr_basic_test()
{
    printf("--------- estr basic test ------------\n"); fflush(stdout);

    estr s = estr_new("123"); estr_show(s);

    s = estr_catS(s, "Hello estr!"); estr_show(s);

    s = estr_catF(s, "%s", " an append str");estr_show(s);

    char news[100];
    s = estr_catB(s, news, 100); estr_show(s);

    estr_free(s);

    s = estr_newLen(0, 16); estr_show(s);
    estr_free(s);

    s = estr_newLen(0, 31); estr_show(s);
    estr_free(s);

    printf("\n\n"); fflush(stdout);
}

void estr_subs_test()
{
    estr e0 = 0, e1 = 0, e2 = 0, e3 = 0; cstr from, to;

    printf("--------- estr subs test ------------\n"); fflush(stdout);

    e0 = estr_new("abcdasd");
    e1 = estr_new("abcd${PATH}asdasdf");
    e2 = estr_new("abcd${PATH}asdasdf${PATH}");
    e3 = estr_new("abcd${PATH}asdasdf${PATH}.../sdf///${PATH}fd%asd");
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
    e0 = estr_subs(e0, from, to); estr_show(e0);
    e1 = estr_subs(e1, from, to); estr_show(e1);
    e2 = estr_subs(e2, from, to); estr_show(e2);
    e3 = estr_subs(e3, from, to); estr_show(e3);

    from = "${PATH}"; to = "${abcd}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    e0 = estr_subs(e0, from, to); estr_show(e0);
    e1 = estr_subs(e1, from, to); estr_show(e1);
    e2 = estr_subs(e2, from, to); estr_show(e2);
    e3 = estr_subs(e3, from, to); estr_show(e3);

    from = "${abcd}"; to = "${abcde}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    e0 = estr_subs(e0, from, to); estr_show(e0);
    e1 = estr_subs(e1, from, to); estr_show(e1);
    e2 = estr_subs(e2, from, to); estr_show(e2);
    e3 = estr_subs(e3, from, to); estr_show(e3);

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
}

void estr_auto_create_test()
{
    estr e0 = 0, e1 = 0, e2 = 0, e3 = 0; cstr from, to;

    printf("--------- estr auto create subs test ------------\n"); fflush(stdout);

    e0 = estr_wrtS(e0, "abcdasd");
    e1 = estr_wrtS(e1, "abcd${PATH}asdasdf");
    e2 = estr_wrtS(e2, "abcd${PATH}asdasdf${PATH}");
    e3 = estr_wrtS(e3, "abcd${PATH}asdasdf${PATH}.../sdf///${PATH}fd%asd");
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
    e0 = estr_subs(e0, from, to); estr_show(e0);
    e1 = estr_subs(e1, from, to); estr_show(e1);
    e2 = estr_subs(e2, from, to); estr_show(e2);
    e3 = estr_subs(e3, from, to); estr_show(e3);

    from = "${PATH}"; to = "${abcd}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    e0 = estr_subs(e0, from, to); estr_show(e0);
    e1 = estr_subs(e1, from, to); estr_show(e1);
    e2 = estr_subs(e2, from, to); estr_show(e2);
    e3 = estr_subs(e3, from, to); estr_show(e3);

    from = "${abcd}"; to = "${abcde}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    e0 = estr_subs(e0, from, to); estr_show(e0);
    e1 = estr_subs(e1, from, to); estr_show(e1);
    e2 = estr_subs(e2, from, to); estr_show(e2);
    e3 = estr_subs(e3, from, to); estr_show(e3);

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
}
