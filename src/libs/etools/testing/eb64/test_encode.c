#include <string.h>
#include <stdio.h>

#include "test_main.h"
#include "estr.h"

int test_encode(int argc, char* argv[])
{
    cstr src, out;

    printf("---- eb64_encode_test ----\n");

    src = estr_dupS("0");
    out = eb64_encb(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    src = estr_dupS("01");
    out = eb64_encb(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    src = estr_dupS("012");
    out = eb64_encb(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    src = estr_dupS("abcdefghijklmnopqrstuvwxyz1234567890 ");
    out = eb64_encb(src, estr_len(src));
    printf("src:");eb64_show(src);eb64_free(src);
    printf("out:");eb64_show(out);eb64_free(out); printf("\n");

    printf("\n");

    return ETEST_OK;
}
