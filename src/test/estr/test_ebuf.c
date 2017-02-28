#include <stdio.h>

#include "test_main.h"

void ebuf_basic_test()
{
    printf("--------- ebuf basic test ------------\n"); fflush(stdout);

    ebuf b = ebuf_new("123"); ebuf_showr(b);

    ebuf_cats(b, "Hello estr!"); ebuf_showr(b);

    ebuf_catf(b, "%s", " an append str");ebuf_showr(b);

    char news[10] = "add";
    ebuf_catb(b, news, 10); ebuf_shows(b);

    ebuf_free(b);

    b = ebuf_newLen(0, 16); ebuf_shows(b);
    ebuf_free(b);

    b = ebuf_newLen(0, 31); ebuf_shows(b);
    ebuf_free(b);

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
    ebuf_shows(e0);
    ebuf_shows(e1);
    ebuf_shows(e2);
    ebuf_shows(e3);

    from = "${PATH}"; to = "${}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    ebuf_subs(e0, from, to); ebuf_shows(e0);
    ebuf_subs(e1, from, to); ebuf_shows(e1);
    ebuf_subs(e2, from, to); ebuf_shows(e2);
    ebuf_subs(e3, from, to); ebuf_shows(e3);

    from = "${}"; to = "${PATH}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    ebuf_subs(e0, from, to); ebuf_shows(e0);
    ebuf_subs(e1, from, to); ebuf_shows(e1);
    ebuf_subs(e2, from, to); ebuf_shows(e2);
    ebuf_subs(e3, from, to); ebuf_shows(e3);

    from = "${PATH}"; to = "${abcd}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    ebuf_subs(e0, from, to); ebuf_shows(e0);
    ebuf_subs(e1, from, to); ebuf_shows(e1);
    ebuf_subs(e2, from, to); ebuf_shows(e2);
    ebuf_subs(e3, from, to); ebuf_shows(e3);

    from = "${abcd}"; to = "${abcde}";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    ebuf_subs(e0, from, to); ebuf_shows(e0);
    ebuf_subs(e1, from, to); ebuf_shows(e1);
    ebuf_subs(e2, from, to); ebuf_shows(e2);
    ebuf_subs(e3, from, to); ebuf_shows(e3);

    from = "${abcde}"; to = "";
    printf("\n\"%s\" -> \"%s\":\n", from, to); fflush(stdout);
    ebuf_subs(e0, from, to); ebuf_shows(e0);
    ebuf_subs(e1, from, to); ebuf_shows(e1);
    ebuf_subs(e2, from, to); ebuf_shows(e2);
    ebuf_subs(e3, from, to); ebuf_shows(e3);

    ebuf_free(e0);
    ebuf_free(e1);
    ebuf_free(e2);
    ebuf_free(e3);

    printf("\n\n"); fflush(stdout);
}
