#include "test_main.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "ecompat.h"

static FILE * fp;

void popen_worker1(void* d)
{
    char buf[1024];

    fp = popen("sleep 2; echo sleep over", "r");

    fgets(buf, 1024, fp);
    printf(buf);

    pclose(fp);
    fp = 0;
}

void popen_kill_test1()
{
    ert rt = ert_new(10);

    ert_run(rt, "popen", popen_worker1, 0, 0);


    sleep(1);
    pclose(fp);
    fp = 0;

    ert_destroy(rt, ERT_WAITING_RUNNING_TASKS);
    ert_join(rt);

}

// ------------------------------------------------

void system_kill_pid(pid_t pid)
{
    char buf[256];

    snprintf(buf, 1024, "ps -ef | awk '{print $2,$3}' | grep %d", pid);

    FILE* fp = popen(buf, "r");

    if(!fp)
        return;

    while(fgets(buf, 1024, fp))
    {
        pid_t pid;
        char kill_cmd[128];

        sscanf(buf, "%d %*d", &pid);
        snprintf(kill_cmd, 128, "kill -9 %d", pid);

        system(kill_cmd);
    }
    pclose(fp);
}

pid_t rw_popen(char* cmd, FILE **rfile, FILE **wfile) {
    int pipefd[2],pipefd2[2]; //管道描述符
    pid_t pid; //进程描述符

    if (pipe(pipefd) < 0) //建立管道
    {
        printf("rw_popen() pipe create error/n");
        return 0;
    }
    if (pipe(pipefd2) < 0) //建立管道
    {
        printf("rw_popen() pipe create error/n");
        return 0;
    }

    pid = fork(); //建立子进程

    if (pid < 0)
    return 0;

    if (0 == pid) //子进程中
    {
        close(pipefd[0]);
        dup2(pipefd[1], 1);
        close(pipefd[1]);
        dup2(pipefd2[0], 0);
        close(pipefd2[0]);
        close(pipefd2[1]);
        char *argv[] = { "/bin/sh", "-c", cmd, NULL };
        if (execvp("/bin/sh", argv) < 0) //用exec族函数执行命令
        exit(1);
    }

    close(pipefd[1]);
    *rfile = fdopen(pipefd[0], "r");
    close(pipefd2[0]);
    *wfile = fdopen(pipefd2[1], "w");
    return pid;
}
void rw_pclose(pid_t pid, FILE *rfile, FILE *wfile) {
    int status;
    waitpid(pid, &status, 0);
    fclose(rfile);
    fclose(wfile);
}
void rw_pkillclose(pid_t pid, FILE *rfile, FILE *wfile) {
    int status;


    system_kill_pid(pid);

//    waitpid(pid, &status, 0);
//    fclose(rfile);
//    fclose(wfile);
}



pid_t pid;
FILE *file1, *file2;

void popen_worker2(void* d)
{
    char buf[1024];

    pid = rw_popen("echo \"ab\";"
                   "sleep 1;"
                   "sleep 10;"
                   "sleep 10;"
                   "sleep 100;"
                   , &file1, &file2);


    while(fgets(buf, 1024, file1) != NULL)
    {
        printf(buf);
    }
    rw_pclose(pid, file1, file2);fp = 0;
}

void popen_kill_test2()
{
    ert rt = ert_new(10);

    ert_run(rt, "popen", popen_worker2, 0, 0);

    sleep(1);
    printf("pid: %d\n", pid); fflush(stdout);

    rw_pkillclose(pid, file1, file2);fp = 0;

    ert_destroy(rt, ERT_WAITING_RUNNING_TASKS);

    sleep(1);
}

void popen_kill_test()
{
    //popen_kill_test1();
    popen_kill_test2();
}

int test_popen_kill(int argc, char* argv[])
{
    popen_kill_test();

    return ETEST_OK;
}
