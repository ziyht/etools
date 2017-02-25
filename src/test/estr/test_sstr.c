#include <stdio.h>

#include "test_sstr.h"

void sstr_basic_test()
{
    printf("--------- sstr basic test ------------\n"); fflush(stdout);
    char buf[50];  char news[10] = "add"; sstr s;

    s = sstr_init(buf, 10); sstr_showr(s);
    sstr_cats(s, "Hello estr!"); sstr_showr(s);
    sstr_catf(s, "%s", " an append str");sstr_showr(s);
    sstr_catb(s, news, 10); sstr_shows(s);
    puts("");

    s = sstr_init(buf, 12); sstr_showr(s);
    sstr_cats(s, "Hello estr!"); sstr_showr(s);
    sstr_catf(s, "%s", " an append str");sstr_showr(s);
    sstr_catb(s, news, 10); sstr_shows(s);
    puts("");

    s = sstr_init(buf, 50); sstr_showr(s);
    sstr_cats(s, "Hello estr!"); sstr_showr(s);
    sstr_catf(s, "%s", " an append str");sstr_showr(s);
    sstr_catb(s, news, 10); sstr_shows(s);

    printf("\n\n"); fflush(stdout);
}

void sstr_subs_test()
{
    estr e0 = 0, e1 = 0, e2 = 0, e3 = 0; cstr from, to;

    char buf0[100]; char buf1[100]; char buf2[100]; char buf3[100];

    printf("--------- sstr subs test ------------\n"); fflush(stdout);

    e0 = sstr_init(buf0, 11);
    e1 = sstr_init(buf1, 22);
    e2 = sstr_init(buf2, 29);
    e3 = sstr_init(buf3, 80);

    sstr_wrs(e0, "abcdasd");
    sstr_wrs(e1, "abcd${PATH}asdasdf");
    sstr_wrs(e2, "abcd${PATH}asdasdf${PATH}");
    sstr_wrs(e3, "abcd${PATH}asdasdf${PATH}.../sdf///${PATH}fd%asd");
    estr_shows(e0);
    estr_shows(e1);
    estr_shows(e2);
    estr_shows(e3);

    from = "${PATH}"; to = "${}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    sstr_subs(e0, from, to); sstr_shows(e0);
    sstr_subs(e1, from, to); sstr_shows(e1);
    sstr_subs(e2, from, to); sstr_shows(e2);
    sstr_subs(e3, from, to); sstr_shows(e3);

    from = "${}"; to = "${PATH}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    sstr_subs(e0, from, to); sstr_shows(e0);
    sstr_subs(e1, from, to); sstr_shows(e1);
    sstr_subs(e2, from, to); sstr_shows(e2);
    sstr_subs(e3, from, to); sstr_shows(e3);

    from = "${PATH}"; to = "${abcd}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    sstr_subs(e0, from, to); sstr_shows(e0);
    sstr_subs(e1, from, to); sstr_shows(e1);
    sstr_subs(e2, from, to); sstr_shows(e2);
    sstr_subs(e3, from, to); sstr_shows(e3);

    from = "${abcd}"; to = "${abcde}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    sstr_subs(e0, from, to); sstr_shows(e0);
    sstr_subs(e1, from, to); sstr_shows(e1);
    sstr_subs(e2, from, to); sstr_shows(e2);
    sstr_subs(e3, from, to); sstr_shows(e3);

    from = "${abcde}"; to = "";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    sstr_subs(e0, from, to); sstr_shows(e0);
    sstr_subs(e1, from, to); sstr_shows(e1);
    sstr_subs(e2, from, to); sstr_shows(e2);
    sstr_subs(e3, from, to); sstr_shows(e3);

    printf("\n\n"); fflush(stdout);
}
