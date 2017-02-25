#include <stdio.h>
#include "estr.h"
#include "test.h"


#include <string.h>

int main(int argc, char *argv[])
{

    estr_basic_test();
    estr_subs_test();

    sstr_basic_test();
    sstr_subs_test();

    ebuf_basic_test();
    ebuf_subs_test();

    return 0;
}




static cptr __memmem(conptr haystack, size_t haystacklen, conptr needle, size_t needlelen)
{
    char match; const unsigned char* a, * b, *ja, *jb, *itr, *end; char tag[256] = {0};

    if(haystacklen < needlelen) return 0;

    a = haystack; b = needle;

    for(ja = b, jb = b + needlelen; ja < jb; ja++)
        tag[*(ja)] = 1;

    end = a + haystacklen - needlelen--;
    for (itr = a; itr <= end;)
    {
        for(ja = itr + needlelen, jb = b + needlelen, match = 1; jb >= b; --ja, --jb)
        {
            if (!tag[*ja])
            {
                itr = ja;
                match = 0;
                break;
            }
            if (match && *ja != *jb)
            {
                match = 0;
            }
        }
        if (match)
        {
            return (cptr)itr;
        }
        itr++;
    }

    return 0;
}

static int _memmem(unsigned char * a, int alen, unsigned char * b, int blen)
{
    int i, ja, jb, match, off; char tag[256] = {0};

    for (i = 0; i < blen; ++ i)
    {
        tag[*(b+i)] = 1;
    }

    off = alen - blen--;
    for (i = 0; i <= off;)
    {
        for(ja = i + blen, jb = blen, match = 1; jb >= 0; --ja, --jb)
        {
            if (!tag[a[ja]])
            {
                i = ja;
                match = 0;
                break;
            }
            if (match && a[ja] != b[jb])
            {
                match = 0;
            }
        }
        if (match)
        {
            return i;
        }
        ++ i;
    }
    return -1;
}
