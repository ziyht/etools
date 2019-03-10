#include <string.h>
#include <stdio.h>

#include "test_main.h"
#include "estr.h"
#include "eb64.h"

void edes_decb_test()
{
    cstr des_src; cstr des_enc, des_dec; cstr b64_enc, b64_dec;

    printf("---- edes_decb_test ----\n");

    char des_key[] = "12345678";
    memset(des_key, 0, 8);

    des_src = estr_dupS("12");
    des_enc = edes_encb(des_key, des_src, estr_len(des_src));
    b64_enc = eb64_encb(des_enc, estr_len(des_enc));
    b64_dec = eb64_decb(b64_enc, estr_len(b64_enc));
    des_dec = edes_decb(des_key, b64_dec, estr_len(b64_dec));
    printf("des_src:");edes_show(des_src);edes_free(des_src);
    printf("des_enc:");edes_show(des_enc);edes_free(des_enc);
    printf("b64_enc:");edes_show(b64_enc);edes_free(b64_enc);
    printf("b64_dec:");edes_show(b64_dec);edes_free(b64_dec);
    printf("des_dec:");edes_show(des_dec);edes_free(des_dec); printf("\n");

    des_src = estr_dupS("12345");
    des_enc = edes_encb(des_key, des_src, estr_len(des_src));
    b64_enc = eb64_encb(des_enc, estr_len(des_enc));
    b64_dec = eb64_decb(b64_enc, estr_len(b64_enc));
    des_dec = edes_decb(des_key, b64_dec, estr_len(b64_dec));
    printf("des_src:");edes_show(des_src);edes_free(des_src);
    printf("des_enc:");edes_show(des_enc);edes_free(des_enc);
    printf("b64_enc:");edes_show(b64_enc);edes_free(b64_enc);
    printf("b64_dec:");edes_show(b64_dec);edes_free(b64_dec);
    printf("des_dec:");edes_show(des_dec);edes_free(des_dec); printf("\n");

    des_src = estr_dupS("12345678");
    des_enc = edes_encb(des_key, des_src, estr_len(des_src));
    b64_enc = eb64_encb(des_enc, estr_len(des_enc));
    b64_dec = eb64_decb(b64_enc, estr_len(b64_enc));
    des_dec = edes_decb(des_key, b64_dec, estr_len(b64_dec));
    printf("des_src:");edes_show(des_src);edes_free(des_src);
    printf("des_enc:");edes_show(des_enc);edes_free(des_enc);
    printf("b64_enc:");edes_show(b64_enc);edes_free(b64_enc);
    printf("b64_dec:");edes_show(b64_dec);edes_free(b64_dec);
    printf("des_dec:");edes_show(des_dec);edes_free(des_dec); printf("\n");

    des_src = estr_dupS("1234567890");
    des_enc = edes_encb(des_key, des_src, estr_len(des_src));
    b64_enc = eb64_encb(des_enc, estr_len(des_enc));
    b64_dec = eb64_decb(b64_enc, estr_len(b64_enc));
    des_dec = edes_decb(des_key, b64_dec, estr_len(b64_dec));
    printf("des_src:");edes_show(des_src);edes_free(des_src);
    printf("des_enc:");edes_show(des_enc);edes_free(des_enc);
    printf("b64_enc:");edes_show(b64_enc);edes_free(b64_enc);
    printf("b64_dec:");edes_show(b64_dec);edes_free(b64_dec);
    printf("des_dec:");edes_show(des_dec);edes_free(des_dec); printf("\n");

    printf("\n");
}

void edes_decb2b_test()
{
    cstr src; char enc[100] = {0}, dec[100] = {0}; size outlen1, outlen2;

    printf("---- edes_decb2b_test ----\n");

    char key[] = "12345678";

    src = estr_dupS("12");
    edes_encb2b(key, src, estr_len(src), enc, &outlen1);
    edes_decb2b(key, enc, outlen1      , dec, &outlen2);
    printf("src:");edes_show(src);edes_free(src);
    printf("enc: %s\n", enc); memset(enc, 0, 100);
    printf("dec: %s\n", dec); memset(dec, 0, 100);printf("\n");

    src = estr_dupS("12345");
    edes_encb2b(key, src, estr_len(src), enc, &outlen1);
    edes_decb2b(key, enc, outlen1      , dec, &outlen2);
    printf("src:");edes_show(src);edes_free(src);
    printf("enc: %s\n", enc); memset(enc, 0, 100);
    printf("dec: %s\n", dec); memset(dec, 0, 100);printf("\n");

    src = estr_dupS("12345678");
    edes_encb2b(key, src, estr_len(src), enc, &outlen1);
    edes_decb2b(key, enc, outlen1      , dec, &outlen2);
    printf("src:");edes_show(src);edes_free(src);
    printf("enc: %s\n", enc); memset(enc, 0, 100);
    printf("dec: %s\n", dec); memset(dec, 0, 100);printf("\n");

    src = estr_dupS("1234567890");
    edes_encb2b(key, src, estr_len(src), enc, &outlen1);
    edes_decb2b(key, enc, outlen1      , dec, &outlen2);
    printf("src:");edes_show(src);edes_free(src);
    printf("enc: %s\n", enc); memset(enc, 0, 100);
    printf("dec: %s\n", dec); memset(dec, 0, 100);printf("\n");

    printf("\n");
}

int test_decode(int argc, char* argv[])
{
    edes_decb_test();
    edes_decb2b_test();

    return ETEST_OK;
}
