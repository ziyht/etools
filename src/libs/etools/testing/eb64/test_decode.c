#include <stdio.h>

#include "test_main.h"
#include "estr.h"

int test_decode(int argc, char* argv[])
{
    cstr src, out;

    printf("---- eb64_decode_test ----\n");

    src = estr_dupS("MA==");
    out = eb64_decb(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    src = estr_dupS("MDE=");
    out = eb64_decb(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    src = estr_dupS("MDEy");
    out = eb64_decb(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    src = estr_dupS("YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXoxMjM0NTY3ODkwIA==");
    out = eb64_decb(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    printf("\n");

    return ETEST_OK;
}
