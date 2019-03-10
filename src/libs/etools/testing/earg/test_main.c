#include <stdio.h>

#include "test_main.h"

int main(int argc, char* argv[])
{
    earg_basic_test(argc, argv);

#if(_WIN32)
    getchar();
#endif
}
