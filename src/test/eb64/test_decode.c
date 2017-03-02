#include <stdio.h>

#include "test_main.h"
#include "estr.h"

void eb64_decode_test()
{
    constr src; cstr out;

    printf("---- eb64_decode_test ----\n");

    src = estr_new("MA==");
    out = eb64_decode(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    src = estr_new("MDE=");
    out = eb64_decode(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    src = estr_new("MDEy");
    out = eb64_decode(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    src = estr_new("YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXoxMjM0NTY3ODkwIA==");
    out = eb64_decode(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    printf("\n\n");
}
