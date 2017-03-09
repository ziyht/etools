#include <stdio.h>

#include "test_main.h"

int main()
{
    esl_basic_test();

}

void esl_basic_test()
{
    esln itr, tmp;

    esl sl = esl_new(0);

    esl_insertO(sl, 1, "1");
    esl_insertO(sl, 2, "2");
    esl_insertO(sl, 8, "8");
    esl_insertO(sl, 3, "3");
    esl_insertO(sl, 7, "7");
    esl_insertO(sl, 5, "5");
    esl_insertO(sl, 1, "1 2");

    esl_itr2(sl, itr, tmp)
    {
        printf("%s\n", (cstr)itr->obj); fflush(stdout);
    }





    esl_free(sl);
}
