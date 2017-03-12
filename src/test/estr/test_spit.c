#include "test_main.h"


void estr_split_str_test()
{
    char s[] = "abcsdfhasdjflkjklsadfjklasdjkflasdfaaaasdasdfjksadf";

    estr* split1 = estr_splitLen(s, sizeof(s) - 1, "aa", 1, 0);
    estr* split2 = estr_splitS(s, "a", 0);

    estr_showSplit(split1, 0);
    estr_showSplit(split2, -1);

    estr_freeSplit(split1, 0);
    estr_freeSplit(split2, 0);

}

void estr_split_arg_test()
{
    char args[] = "abc  -t \"123\" \'\"second\" three\' -x four -s five six";

    estr* argv = estr_splitArgs(args, 0);

    estr_showSplit(argv, -1);

    estr_freeSplit(argv, 0);
}

void estr_split_test()
{
    estr_split_str_test();
    estr_split_arg_test();
}
