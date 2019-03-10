#include <stdio.h>

int void_arr_ptr_test();

int main(int argc, char *argv[])
{
    void_arr_ptr_test();

    return 0;
}



int void_arr_ptr_test()
{

    typedef union eval_s{
        void*     p;                 // ptr

        char     r[8];              // raw data
        void*    v[1];
    }eval;

    eval val1;

    int i;

    val1.p = &i;

    i = 100 * 2;

    void* ptr1 = val1.r;
    void* ptr2 = val1.v;
    void* ptr3 = &val1;

    return 0;
}
