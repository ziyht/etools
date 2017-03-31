#include <stdio.h>

#include "test_main.h"

void ebuf_basic_test()
{
    printf("--------- ebuf basic test ------------\n"); fflush(stdout);

    ebuf b = ebuf_new("123"); ebuf_show(b);

    ebuf_catS(b, "Hello estr!"); ebuf_show(b);

    ebuf_catF(b, "%s", " an append str");ebuf_show(b);

    char news[10] = "add";
    ebuf_catB(b, news, 10); ebuf_show(b);

    ebuf_free(b);

    b = ebuf_newLen(0, 16); ebuf_show(b);
    ebuf_free(b);

    b = ebuf_newLen(0, 31); ebuf_show(b);
    ebuf_free(b);

    printf("\n\n"); fflush(stdout);
}

void ebuf_subc_test()
{
    ebuf e0, e1; cstr from, to;

    printf("--------- ebuf subc test ------------\n"); fflush(stdout);

    e0 = ebuf_new("abcdcbd");
    e1 = ebuf_new("aascdasdabcsbcabbccabcdf");

    ebuf_show(e0);
    ebuf_show(e1);

    from = "abc"; to = "1234";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    ebuf_subc(e0, from, to); ebuf_show(e0);
    ebuf_subc(e1, from, to); ebuf_show(e1);

    from = "1234"; to = "*";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    ebuf_subc(e0, from, to); ebuf_show(e0);
    ebuf_subc(e1, from, to); ebuf_show(e1);

    ebuf_free(e0);
    ebuf_free(e1);

    printf("\n\n"); fflush(stdout);
}

void ebuf_subs_test()
{
    ebuf e0 = 0, e1 = 0, e2 = 0, e3 = 0; cstr from, to;

    printf("--------- ebuf subs test ------------\n"); fflush(stdout);

    e0 = ebuf_new("abcdasd");
    e1 = ebuf_new("abcd${PATH}asdasdf");
    e2 = ebuf_new("abcd${PATH}asdasdf${PATH}");
    e3 = ebuf_new("abcd${PATH}asdasdf${PATH}.../sdf///${PATH}fd%asd");
    ebuf_show(e0);
    ebuf_show(e1);
    ebuf_show(e2);
    ebuf_show(e3);

    from = "${PATH}"; to = "${}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    ebuf_subs(e0, from, to); ebuf_show(e0);
    ebuf_subs(e1, from, to); ebuf_show(e1);
    ebuf_subs(e2, from, to); ebuf_show(e2);
    ebuf_subs(e3, from, to); ebuf_show(e3);

    from = "${}"; to = "${PATH}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    ebuf_subs(e0, from, to); ebuf_show(e0);
    ebuf_subs(e1, from, to); ebuf_show(e1);
    ebuf_subs(e2, from, to); ebuf_show(e2);
    ebuf_subs(e3, from, to); ebuf_show(e3);

    from = "${PATH}"; to = "${abcd}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    ebuf_subs(e0, from, to); ebuf_show(e0);
    ebuf_subs(e1, from, to); ebuf_show(e1);
    ebuf_subs(e2, from, to); ebuf_show(e2);
    ebuf_subs(e3, from, to); ebuf_show(e3);

    from = "${abcd}"; to = "${abcde}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    ebuf_subs(e0, from, to); ebuf_show(e0);
    ebuf_subs(e1, from, to); ebuf_show(e1);
    ebuf_subs(e2, from, to); ebuf_show(e2);
    ebuf_subs(e3, from, to); ebuf_show(e3);

    from = "${abcde}"; to = "";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    ebuf_subs(e0, from, to); ebuf_show(e0);
    ebuf_subs(e1, from, to); ebuf_show(e1);
    ebuf_subs(e2, from, to); ebuf_show(e2);
    ebuf_subs(e3, from, to); ebuf_show(e3);

    ebuf_free(e0);
    ebuf_free(e1);
    ebuf_free(e2);
    ebuf_free(e3);

    printf("\n\n"); fflush(stdout);
}
