#include <stdio.h>
#include <unistd.h> 
#include <string.h> 
#include <sys/wait.h>  
#include <errno.h>  
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>


#define log(fmt, args...) \ 
do { \
    printf("\n[%s:%s()]: " fmt "\n", __FILE__, __func__, ##args); \
}while(0)

/*
   利用popen的管道控制外部可交互程序的demo代码.

   另外，用脚步其实这样操作也是一个效果.
   echo 1 > tmp
   echo 2 >> tmp
   echo 3 >> tmp
   cat tmp|./test2
   */
/*
   此种方式利用管道的原理，可以方便的达到执行程序的目的
   而且可以拿到执行结果，适用于需要解析执行结果的场景
   但是需要提前准备好tmp临时文件便于cat取出.
   */
int exe_cmd(const char *cmd, char *result) 
{
    FILE *fp = NULL;
    int ret = 0;
    char buf[256] = {0};
    char *p = result;
    int len = 0;

    if (cmd == NULL || result == NULL) {
        log("err");
        goto err;
    }

    fp = popen(cmd, "r");
    if (fp == NULL) {
        log("popen err");
        goto err;
    }

    while (fgets(buf, sizeof(buf)-1, fp) != 0) {
        len = strlen(buf); 
        strncpy(p, buf, len);
        p += len;
    }

    pclose(fp);

err:
    return ret;
}

/*
   此种方式不需要写入tmp临时文件，可以执行成功目的
   但是拿不到终端的结果显示，对于需要解析结果的需求
   不适合.
   */
int exe_cmd2(const char *cmd, const char **in, const int inLen)
{
    FILE *fp = NULL;
    int ret = -1;
    int i = 0;

    if (cmd == NULL || in == NULL || inLen == 0) {
        log("err");
        goto err;
    }

    fp = popen(cmd, "w");
    if (fp == NULL) {
        log("err");
        goto err;
    }
    /*
       需要注意，fputs内容一定要加上换行符\n，
       否则管道对端只能接收第一个cmd而导致出错.
       命令顺序必须跟目标执行函数的执行命令一致才新.
       */
    for (i = 0; i < inLen; i++) {
        const char *c = in[i];
        /** log("%s",c); */
        fputs(c, fp);
        fflush(fp);
    }

    pclose(fp);
    ret = 0;

err:
    return ret;
}

void *thread_callback(void *args) {
    int *pid = (int *)args;
    int i = 0;

    for (i = 0; i < 3; i++) {
        log("wait timeout..");
        sleep(1);
    }

    for (i = 0; i < 2; i++) {
        log("timeout kill - %d", pid[i]);
        kill(pid[i], SIGKILL);
    }
    return NULL;
}

/*
   综合了以上两个方法的优点,可以不用tmp文件，也可以返回执行结果
   是最佳选择.
   */
int exe_cmd3(const char *cmd, const char **in, const int inLen, char *out)
{
    FILE *fp = NULL;
    int pfd[2];
    int pid = -1;
    int len = 0;
    int ret = -1;
    int stat;
    int fd;
    char buf[256] = {0};
    char *p = out;
    pthread_t tid;
    int pidsave[2];

    /* 创建管道. */
    if (pipe(pfd) < 0) {
        log("pipe err");
        goto err;
    }

    /* fork 子进程 */
    if ((pid = fork()) < 0) {
        log("fork err");
        goto err;
    }
    else if (pid == 0) {
        log("子进程开始");
        close(pfd[0]);
        if (pfd[1] != STDOUT_FILENO) {
            dup2(pfd[1], STDOUT_FILENO);
            close(pfd[1]);
        }
        exe_cmd2(cmd, in, inLen);
        log("子进程退出");
        _exit(127);
    }

    log("父进程开始");
    close(pfd[1]);
    if ((fp = fdopen(pfd[0], "r")) == NULL) {
        log("fdopen err");
        goto err;
    }

    /* 超时处理 */
    pidsave[0] = pid;
    pidsave[1] = getpid();
    log("%d,%d",pidsave[0], pidsave[1]);
    ret = pthread_create(&tid, NULL, (void*)thread_callback, pidsave);
    if (ret) {
        log("pthread create err");
        goto err;
    }

    /* 注意p指向内存需要足够大， 否则会被踩! */
    while (fgets(buf, sizeof(buf)-1, fp) != 0) {
        len = strlen(buf);
        memcpy(p, buf, len);
        p += len;
        memset(buf, 0, sizeof(buf));
    }

    fd = fileno(fp);
    if (fclose(fp) == EOF) {
        log("fclose err");
        goto err;
    }

    while (waitpid(pid, &stat, 0) < 0)  
        if (errno != EINTR) {
            log("waitpid err");
            goto err;
        }  

    log("父进程退出 ok.");
    return ret;

err:
    if (pid > 0) {
        kill(pid, SIGKILL); 
    }
    log("has err.");
    return -1;
}


int main() 
{
    char buf[2048] = {0};
    const char *parms[5] = {
        "1\n",
        "2\n",
        "3\n",
    };

    exe_cmd3("./a0", parms, 3, buf);
    /** exe_cmd("cat ./tmp|./test2", buf); */
    log("执行结果是:%s", buf);
    return 0;
}




