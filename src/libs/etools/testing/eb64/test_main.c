#include <stdio.h>

#include "test_main.h"

int main()
{
    eb64_encode_test();
    eb64_decode_test();

#ifdef _WIN32
    getchar();
#endif
}
