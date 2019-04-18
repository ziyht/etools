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

#define ETEST_RUN(func) __ETEST_EXEC(func, __FILE__, __LINE__)

#define eexpect_num(a, b)       __etest_equal_num(a, b, __FILE__, __LINE__, __FUNCTION__)
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

#define __VAL_FMT_

#define __etest_equal_num(a, b, f, l, F)                            \
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
                 printf("etest check num equal faild:\n"            \
                        "    %s: %f\n"                              \
                        "    %s: %f\n"                              \
                        "  %s(%d) %s\n", #a, fa, #b, fb, f, l, F);  \
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
    const void* p1, * p2;                                           \
    p1 = a; p2 = b;                                                 \
                                                                    \
    if(p1 != p2)                                                    \
    {                                                               \
        printf("etest check ptr equal faild: %s(%d)\n"              \
            "    %s: %p\n"                                          \
            "    %s: %p\n",                                         \
            f, l, #a, p1, #b, p2);                                  \
        fflush(stdout);                                             \
        return ETEST_ERR;                                           \
    }                                                               \
}while(0)

#define __etest_unequ_ptr(a, b, f, l)                               \
do{                                                                 \
    const void* p1, * p2;                                           \
    p1 = a; p2 = b;                                                 \
                                                                    \
    if(p1 == p2)                                                    \
    {                                                               \
        printf("etest check ptr unequ faild: %s(%d)\n"              \
            "    %s: %p\n"                                          \
            "    %s: %p\n",                                         \
            f, l, #a, p1, #b, p2);                                  \
        fflush(stdout);                                             \
        return ETEST_ERR;                                           \
    }                                                               \
}while(0)

#define __etest_equal_str(a, b, f, l)                               \
do{                                                                 \
    const char* s1, * s2;                                           \
    s1 = a; s2 = b;                                                 \
                                                                    \
    if(0 != strcmp(s1, s2))                                         \
    {                                                               \
        printf("etest check str equal faild: %s(%d)\n"              \
            "    %s: %s\n"                                          \
            "    %s: %s\n",                                         \
            f, l, #a, s1, #b, s2);                                  \
        fflush(stdout);                                             \
        return ETEST_ERR;                                           \
    }                                                               \
}while(0)

#define __etest_unequ_str(a, b, f, l)                               \
do{                                                                 \
    const char* s1, * s2;                                           \
    s1 = a; s2 = b;                                                 \
                                                                    \
    if(0 == strcmp(s1, s2))                                         \
    {                                                               \
        printf("etest check str unequ faild: %s(%d)\n"              \
            "    %s: %s\n"                                          \
            "    %s: %s\n",                                         \
            f, l, #a, s1, #b, s2);                                  \
        fflush(stdout);                                             \
        return ETEST_ERR;                                           \
    }                                                               \
}while(0)

#define __etest_equal_raw(a, b, f, l, L)                            \
do{                                                                 \
    const char* s1, * s2; int len;                                  \
    s1 = a; s2 = b; len = L;                                        \
                                                                    \
    if(0 != memcmp(s1, s2, len))                                    \
    {                                                               \
        printf("etest check raw equal faild: %s(%d)\n"              \
            "    %s: %s\n"                                          \
            "    %s: %s\n",                                         \
            f, l, #a, s1, #b, s2);                                  \
        fflush(stdout);                                             \
        return ETEST_ERR;                                           \
    }                                                               \
}while(0)

#define __etest_unequ_raw(a, b, f, l, L)                            \
do{                                                                 \
    const char* s1, * s2; uint len;                                 \
    s1 = a; s2 = b; len = L;                                        \
                                                                    \
    if(0 == memcmp(s1, s2, len))                                    \
    {                                                               \
        printf("etest check str unequ faild: %s(%d)\n"              \
            "    %s: %s\n"                                          \
            "    %s: %s\n",                                         \
            f, l, #a, s1, #b, s2);                                  \
        fflush(stdout);                                             \
        return ETEST_ERR;                                           \
    }                                                               \
}while(0)

#define __ETEST_EXEC(func, f, l)                                    \
    if((func) != ETEST_OK)                                          \
    {                                                               \
        printf("  %s(%d): %s\n", f, l, #func);                      \
        return ETEST_ERR;                                           \
    }else                                                           \
    {                                                               \
        printf("%s PASSED\n", #func);                               \
    }                                                               \

#endif
