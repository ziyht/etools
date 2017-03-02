#include <string.h>
#include <stdio.h>

#include "test_main.h"
#include "estr.h"


void edes_decode_test()
{
    cstr src; cstr enc, dec;

    printf("---- edes_decode_test ----\n");

    char key[] = "12345678";

    src = estr_new("12");
    enc = edes_encb(key, src, estr_len(src));
    dec = edes_decb(key, enc, estr_len(enc));
    printf("src:");edes_show(src);edes_free(src);
    printf("enc:");edes_show(enc);edes_free(enc);
    printf("dec:");edes_show(dec);edes_free(dec); printf("\n");

    src = estr_new("12345");
    enc = edes_encb(key, src, estr_len(src));
    dec = edes_decb(key, enc, estr_len(enc));
    printf("src:");edes_show(src);edes_free(src);
    printf("enc:");edes_show(enc);edes_free(enc);
    printf("dec:");edes_show(dec);edes_free(dec); printf("\n");

    src = estr_new("12345678");
    enc = edes_encb(key, src, estr_len(src));
    dec = edes_decb(key, enc, estr_len(enc));
    printf("src:");edes_show(src);edes_free(src);
    printf("enc:");edes_show(enc);edes_free(enc);
    printf("dec:");edes_show(dec);edes_free(dec); printf("\n");

    src = estr_new("1234567890");
    enc = edes_encb(key, src, estr_len(src));
    dec = edes_decb(key, enc, estr_len(enc));
    printf("src:");edes_show(src);edes_free(src);
    printf("enc:");edes_show(enc);edes_free(enc);
    printf("dec:");edes_show(dec);edes_free(dec); printf("\n");

    printf("\n\n");
}
