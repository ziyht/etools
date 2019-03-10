#include "unit_test.h"
#include "threadpool.h"

#if test_threadpool

static const char* _llog_basename(const char* path){static const char* slash; if (slash) {return slash + 1;}else{slash = strrchr(path, '/');}if (slash) {return slash + 1;}return 0;}
#define log(fmt, ...)   fprintf(stdout, "%s(%d):" fmt "%s", _llog_basename(__FILE__), __LINE__, __VA_ARGS__)
#define llog(...)       log(__VA_ARGS__, "\n");fflush(stdout)


void* cb1(void* _d)
{
    char* d = (char*) _d;
    
    llog("this is in cb1, data: \"%s\"", d);
    return 0;
}

void* after_cb1(void* _d)
{
    char* d = (char*) _d;
    
    llog("this is in after_cb1, data: \"%s\"", d);
    return 0;
}

char d1[] = "this is d1 data";

void* cb2(void* _d)
{
    char* d = (char*) _d;
    
    llog("this is in cb2, data: \"%s\"", d);
    sleep(1);
    return 0;
}

void* after_cb2(void* _d)
{
    char* d = (char*) _d;
    
    llog("this is in after_cb2, data: \"%s\"", d);
    return 0;
}

char d2[] = "this is d2 data";


void ev_threadpool_test1()
{
    TP tp = TP_New();
    
    TP_SetThreadsNum(tp, 16);
    for(int i = 0; i < 100; i++)
    {
        TP_AddTask(tp, 0, cb1, after_cb1, d1);
        TP_AddTask(tp, 0, cb2, after_cb2, d2);
        TP_AddTask(tp, "tag1", cb1, after_cb1, d1);
        TP_AddTask(tp, "tag1", cb1, after_cb1, d1);
        TP_AddTask(tp, "tag2", cb2, after_cb2, d2);
        TP_AddTask(tp, "tag2", cb2, after_cb2, d2);
        
        usleep(1000);
    }
    
    sleep(1);
    
    TP_Destroy(tp, TP_JOIN_THREADS);
    TP_Join(tp);
    
    sleep(2);
}


void* cb_all(void* _d){return 0;}
void* after_cb_all(void* _d){return 0;}
void ev_threadpool_test2()
{
    TP tp = TP_New();
    
    TP_SetThreadsNum(tp, 16);
    
    TP_SetThreadsNum(tp, 16);
    for(int i = 0; i < 1000000; i++)
    {
        TP_AddTask(tp, "cpu", cb_all, after_cb_all, 0);
        TP_AddTask(tp, "df", cb_all, after_cb_all, 0);
        TP_AddTask(tp, "dskio", cb_all, after_cb_all, 0);
        TP_AddTask(tp, "ipmi", cb_all, after_cb_all, 0);
        TP_AddTask(tp, "meminfo", cb_all, after_cb_all, 0);
        TP_AddTask(tp, "net", cb_all, after_cb_all, 0);
        TP_AddTask(tp, "nfs", cb_all, after_cb_all, 0);
        TP_AddTask(tp, "proc", cb_all, after_cb_all, 0);
        TP_AddTask(tp, "microarch2", cb_all, after_cb_all, 0);
        TP_AddTask(tp, "sysinfo", cb_all, after_cb_all, 0);
        
        usleep(1000000);
        
        puts("");
    }
    
    TP_Destroy(tp, 0);
    TP_Join(tp);
    
    sleep(2);
}

void ev_threadpool_test()
{
    ev_threadpool_test1();
}
#else
void ev_threadpool_test(){}
#endif
