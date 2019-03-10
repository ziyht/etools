#include <string.h>
#include <stdio.h>

#include "test_main.h"
#include "estr.h"


void edes_encode_test()
{
    cstr src; cstr out;

    printf("---- edes_encode_test ----\n");

    char key[] = "12345678";

    src = estr_dupS("0");
    out = edes_encb(key, src, estr_len(src));
    printf("src:");edes_show(src);edes_free(src);
    printf("out:");edes_show(out);edes_free(out); printf("\n");

    src = estr_dupS("01");
    out = edes_encb(key, src, estr_len(src));
    printf("src:");edes_show(src);edes_free(src);
    printf("out:");edes_show(out);edes_free(out); printf("\n");

    src = estr_dupS("012");
    out = edes_encb(key, src, estr_len(src));
    printf("src:");edes_show(src);edes_free(src);
    printf("out:");edes_show(out);edes_free(out); printf("\n");

    src = estr_dupS("abcdefghijklmnopqrstuvwxyz1234567890 ");
    out = edes_encb(key, src, estr_len(src));
    printf("src:");edes_show(src);edes_free(src);
    printf("out:");edes_show(out);edes_free(out); printf("\n");

    printf("\n\n");
}

int test_encode(int argc, char* argv[])
{
    edes_encode_test();

    return ETEST_OK;
}
