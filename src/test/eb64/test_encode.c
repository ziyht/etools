#include <string.h>
#include <stdio.h>

#include "test_main.h"
#include "estr.h"


void eb64_encode_test()
{
    cstr src, out;

    printf("---- eb64_encode_test ----\n");

    src = estr_new("0");
    out = eb64_encb(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    src = estr_new("01");
    out = eb64_encb(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    src = estr_new("012");
    out = eb64_encb(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    src = estr_new("abcdefghijklmnopqrstuvwxyz1234567890 ");
    out = eb64_encb(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    printf("\n");
}
