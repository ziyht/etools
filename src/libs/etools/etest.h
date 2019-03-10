/// =====================================================================================
///
///       Filename:  etest.h
///
///    Description:  a test helper for etools
///
///        Version:  1.0
///        Created:  2018.07.07 04:03:34 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================


#ifndef __E_TEST_H__
#define __E_TEST_H__

#include <stdio.h>
#include <string.h>

#include "etype.h"

#define ETEST_VERSION "etest 1.0.2"      // Implemented etest_equal_raw()

#ifndef ETEST_OK
#define ETEST_ERR 1
#define ETEST_OK  0
#endif

#define ETEST_RUN(func) __ETEST_EXEC((func), __FILE__, __LINE__)

#define eexpect_num(a, b)       __etest_equal_num(a, b, __FILE__, __LINE__)
#define eexpect_ptr(a, b)       __etest_equal_ptr(a, b, __FILE__, __LINE__)
#define eexpect_str(a, b)       __etest_equal_str(a, b, __FILE__, __LINE__)
#define eexpect_raw(a, b, l)    __etest_equal_raw(a, b, __FILE__, __LINE__, l)

#define eunexpc_num(a, b)       __etest_unequ_num(a, b, __FILE__, __LINE__)
#define eunexpc_ptr(a, b)       __etest_unequ_ptr(a, b, __FILE__, __LINE__)
#define eunexpc_str(a, b)       __etest_unequ_str(a, b, __FILE__, __LINE__)
#define eunexpc_raw(a, b, l)    __etest_unequ_raw(a, b, __FILE__, __LINE__, l)

//! -----------------------------------------------------------
//! etest definition
//!

#define __etest_equal_num(a, b, f, l)                               \
do{                                                                 \
    int sizea = sizeof(a);                                          \
    int sizeb = sizeof(b);                                          \
    int size  = sizea < sizeb ? sizeb : sizea;                      \
                                                                    \
    f64 fa = (a), fb = (b), xz = fa - fb;                           \
    if(xz < 0) xz = -xz;                                            \
                                                                    \
    switch (size) {                                                 \
    case  1:                                                        \
    case  2:                                                        \
    case  4:                                                        \
    case  8: if(xz > 0.00000001)                                    \
             {                                                      \
                 printf("etest check num equal faild: %s(%d)\n"     \
                        "    %s: %f\n"                              \
                        "    %s: %f\n", f, l, #a, fa, #b, fb);      \
                 fflush(stdout);                                    \
                 return ETEST_ERR;                                  \
             }                                                      \
             break;                                                 \
    default: printf("etest check num equal faild: %s(%d)\n"         \
                     "    maybe not a num of %s or %s??\n"          \
                     "    %d %d", f, l, #a, #b, sizea, sizeb);      \
             fflush(stdout);                                        \
             return ETEST_ERR;                                      \
             break;                                                 \
    }                                                               \
}while(0)

#define __etest_unequ_num(a, b, f, l)                               \
do{                                                                 \
    int sizea = sizeof(a);                                          \
    int sizeb = sizeof(b);                                          \
    int size  = sizea < sizeb ? sizeb : sizea;                      \
                                                                    \
    f64 fa = a, fb =b , xz = fa - fb;                               \
    if(xz < 0) xz = -xz;                                            \
                                                                    \
    switch (size) {                                                 \
    case  1:                                                        \
    case  2:                                                        \
    case  4:                                                        \
    case  8: if(xz < 0.00000001)                                    \
             {                                                      \
                 printf("etest check num equal faild: %s(%d)\n"     \
                        "    %s: %f\n"                              \
                        "    %s: %f\n", f, l, #a, fa, #b, fb);      \
                 fflush(stdout);                                    \
                 return ETEST_ERR;                                  \
             }                                                      \
             break;                                                 \
    default: printf("etest check num equal faild: %s(%d)\n"         \
                     "    maybe not a num of %s or %s??\n"          \
                     "    %d %d", f, l, #a, #b, sizea, sizeb);      \
             fflush(stdout);                                        \
             return ETEST_ERR;                                      \
             break;                                                 \
    }                                                               \
}while(0)

#define __etest_equal_ptr(a, b, f, l)                               \
do{                                                                 \
    if((void*)a != (void*)b)                                        \
    {                                                               \
        printf("etest check ptr equal faild: %s(%d)\n"              \
            "    %s: %p\n"                                          \
            "    %s: %p\n",                                         \
            f, l, #a, (void*)a, #b, (void*)b);                      \
        fflush(stdout);                                             \
        return ETEST_ERR;                                           \
    }                                                               \
}while(0)

#define __etest_unequ_ptr(a, b, f, l)                               \
do{                                                                 \
    if((void*)a == (void*)b)                                        \
    {                                                               \
        printf("etest check ptr unequ faild: %s(%d)\n"              \
            "    %s: %p\n"                                          \
            "    %s: %p\n",                                         \
            f, l, #a, (void*)a, #b, (void*)b);                      \
        fflush(stdout);                                             \
        return ETEST_ERR;                                           \
    }                                                               \
}while(0)

#define __etest_equal_str(a, b, f, l)                               \
do{                                                                 \
    if(0 != strcmp(a, b))                                           \
    {                                                               \
        printf("etest check str equal faild: %s(%d)\n"              \
            "    %s: %s\n"                                          \
            "    %s: %s\n",                                         \
            f, l, #a, (char*)a, #b, (char*)b);                      \
        fflush(stdout);                                             \
        return ETEST_ERR;                                           \
    }                                                               \
}while(0)

#define __etest_unequ_str(a, b, f, l)                               \
do{                                                                 \
    if(0 == strcmp(a, b))                                           \
    {                                                               \
        printf("etest check str unequ faild: %s(%d)\n"              \
            "    %s: %s\n"                                          \
            "    %s: %s\n",                                         \
            f, l, #a, (char*)a, #b, (char*)b);                      \
        fflush(stdout);                                             \
        return ETEST_ERR;                                           \
    }                                                               \
}while(0)

#define __etest_equal_raw(a, b, f, l, L)                            \
do{                                                                 \
    if(0 != memcmp(a, b, L))                                        \
    {                                                               \
        printf("etest check raw equal faild: %s(%d)\n"              \
            "    %s: %s\n"                                          \
            "    %s: %s\n",                                         \
            f, l, #a, (char*)a, #b, (char*)b);                      \
        fflush(stdout);                                             \
        return ETEST_ERR;                                           \
    }                                                               \
}while(0)

#define __etest_unequ_raw(a, b, f, l, L)                            \
do{                                                                 \
    if(0 == memcmp(a, b, L))                                        \
    {                                                               \
        printf("etest check str unequ faild: %s(%d)\n"              \
            "    %s: %s\n"                                          \
            "    %s: %s\n",                                         \
            f, l, #a, (char*)a, #b, (char*)b);                      \
        fflush(stdout);                                             \
        return ETEST_ERR;                                           \
    }                                                               \
}while(0)

#define __ETEST_EXEC(func, f, l)                                    \
    if((func) != ETEST_OK)                                          \
    {                                                               \
        printf("%s(%d): %s FAILED\n", f, l, #func);                 \
    }else                                                           \
    {                                                               \
        printf("%s(%d): %s PASSED\n", f, l, #func);                 \
    }                                                               \

#endif
