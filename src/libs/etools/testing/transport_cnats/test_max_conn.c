#include "test_main.h"

#if (!_WIN32)
#include <unistd.h>

static const char* _llog_basename(const char* path){static const char* slash; if (slash) {return slash + 1;}else{slash = strrchr(path, '/');}if (slash) {return slash + 1;}return 0;}
#define log(fmt, ...)   fprintf(stdout, "%s(%d):" fmt "%s", _llog_basename(__FILE__), __LINE__, __VA_ARGS__)
#define llog(...)       log(__VA_ARGS__, "\n");fflush(stdout)

#else
static const char* _llog_basename(const char* path){static const char* slash; if (slash) {return slash + 1;}else{slash = strrchr(path, '/');}if (slash) {return slash + 1;}return 0;}
#define llog(fmt, ...)   fprintf(stdout, "%s(%d):" fmt "\n", _llog_basename(__FILE__), __LINE__, __VA_ARGS__)

#include "ecompat.h"

#endif

#define MAX_CONN 2048
#define MAX_TEST 204

void server_max_collect_test()
{
    cstr url = "nats://172.18.4.205:4242";

    int i, j;

    nTrans* ntSet = calloc(MAX_CONN, sizeof(*ntSet));

    for (i = 0; (ntSet[i] = nTrans_New(url)) && i < MAX_TEST - 1;)
    {
        nTrans_SetClosedCB(ntSet[i], ClosedCB, 0);

        i++;

        if(i % 100 == 0 )
            llog("connected: %d", i);
    }
    llog("connected: %d\n", i + 1);

    for(j = 0; j <= i ; j++)
    {
        nTrans_Destroy(&ntSet[j]);

        if(j % 100 == 0 )
            llog("destroyed: %d", j);

    }
    llog("destroyed: %d", j);

    free(ntSet);

    sleep(2);
}
