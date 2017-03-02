#include <string.h>
#include <stdio.h>

#include "test_main.h"
#include "estr.h"


void edes_encode_test()
{
    cstr src; cstr out;

    printf("---- edes_encode_test ----\n");

    char key[] = "12345678";

    src = estr_new("0");
    out = edes_encb(key, src, estr_len(src));
    printf("src:");edes_show(src);edes_free(src);
    printf("out:");edes_show(out);edes_free(out); printf("\n");

    src = estr_new("01");
    out = edes_encb(key, src, estr_len(src));
    printf("src:");edes_show(src);edes_free(src);
    printf("out:");edes_show(out);edes_free(out); printf("\n");

    src = estr_new("012");
    out = edes_encb(key, src, estr_len(src));
    printf("src:");edes_show(src);edes_free(src);
    printf("out:");edes_show(out);edes_free(out); printf("\n");

    src = estr_new("abcdefghijklmnopqrstuvwxyz1234567890 ");
    out = edes_encb(key, src, estr_len(src));
    printf("src:");edes_show(src);edes_free(src);
    printf("out:");edes_show(out);edes_free(out); printf("\n");

    printf("\n\n");
}
