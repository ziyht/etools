#include <stdio.h>

#include "etype.h"
#include "eutils.h"

#pragma pack(1)
typedef struct obj_header_s{
    union {
        struct {
            uint o_type   : 4;
            uint c_type   : 4;
            uint is_float : 1;
        }         t091;
        uint      type2   : 9;

        uint reserved :  5;
        uint is_array :  1;
        uint is_ref   :  1;
    }    type;
    uint ref     : 16;
    uint len     : 32;
}obj_header_t, * OBJ;

void using_struct(int cnt)
{
    obj_header_t head; int j;

    head.type.t091.c_type = 2;
    head.type.t091.o_type = 3;
    head.type.t091.is_float  = 1;

    int s = sizeof(obj_header_t);

    for(int i = 0; i <= cnt; i++)
    {
        if(head.type.t091.c_type == 2)
        {
            j = 2;
        }


//        switch (head.type) {
//        case 2:
//            j = 2;
//            break;
//        default:
//            break;
//        }
    }
}

void using_xor(int cnt)
{
    i64 head; int j;

    head = 0;
    head = 0x2;

    for(int i = 0; i <= cnt; i++)
    {
        if((head & 0xff) == 0x2)
        {
            j = 2;
        }
    }
}

void test_xor_perfermance()
{
    int cnt = 1000000; i64 t;

    t = eutils_nowns();
    using_struct(cnt);
    printf("cost %ld us\n", eutils_nowns() - t); fflush(stdout);

    t = eutils_nowns();
    using_xor(cnt);
    printf("cost %ld us\n", eutils_nowns() - t); fflush(stdout);


}
