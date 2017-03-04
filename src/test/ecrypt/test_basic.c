#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "test_main.h"

void test1();
void test2();

void ecrypt_basic_test()
{
    test1();
    test2();
}

void test1()
{
    clock_t before;
    clock_t after;
    char salt[ECRYPT_SIZE];
    char hash[ECRYPT_SIZE];
    int ret;

    const char pass[] = "hi,mom";
    const char hash1[] = "$2a$10$VEVmGHy4F4XQMJ3eOZJAUeb.MedU0W10pTPCuf53eHdKJPiSE8sMK";
    const char hash2[] = "$2a$10$3F0BVk5t8/aoS.3ddaB3l.fxg5qvafQ9NybxcpXLzMeAt.nVWn.NO";

    ret = ecrypt_gensalt(12, salt);
    assert(ret == 1);
    printf("Generated salt: %s\n", salt);
    before = clock();
    ret = ecrypt_hashs2s("testtesttest", salt, hash);
    assert(ret >= 1);
    after = clock();
    printf("Hashed password: %s\n", hash);
    printf("Time taken: %f seconds\n",
           (double)(after - before) / CLOCKS_PER_SEC);

    ret = ecrypt_hashs2s(pass, hash1, hash);
    assert(ret >= 1);
    printf("First hash check: %s\n", (strcmp(hash1, hash) == 0)?"OK":"FAIL");
    ret = ecrypt_hashs2s(pass, hash2, hash);
    assert(ret >= 1);
    printf("Second hash check: %s\n", (strcmp(hash2, hash) == 0)?"OK":"FAIL");

    before = clock();
    ret = (ecrypt_check(pass, hash1) == 1);
    after = clock();
    printf("First hash check with bcrypt_checkpw: %s\n", ret?"OK":"FAIL");
    printf("Time taken: %f seconds\n",
           (double)(after - before) / CLOCKS_PER_SEC);

    before = clock();
    ret = (ecrypt_check(pass, hash2) == 1);
    after = clock();
    printf("Second hash check with bcrypt_checkpw: %s\n", ret?"OK":"FAIL");
    printf("Time taken: %f seconds\n",
           (double)(after - before) / CLOCKS_PER_SEC);

    return ;
}

void test2()
{
    char passwd[] = "ziyht";

    estr hash = ecrypt_encs(passwd);

    ecrypt_show(hash);

    printf("check: %s\n", ecrypt_check(passwd, hash) ? "ok" : "fail");
    ecrypt_free(hash);




}
