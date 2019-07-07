#include <stdio.h>
#include<unistd.h> 
#include<string.h> 

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
int exe_cmd2(const char *cmd, const char **in, const int in_len)
{
    FILE *fp = NULL;
    int ret = -1;
    int i = 0;

    if (cmd == NULL || in == NULL || in_len == 0) {
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
    for (i = 0; i < in_len; i++) {
        char *c = in[i];
        log("%s",c);
        fputs(c, fp);
        fflush(fp);
    }

    pclose(fp);
    ret = 0;
err:
    return ret;
}

int main() 
{
    char *parms[5] = {
        "1\n",
        "2\n",
        "3\n",
    };
    char buf[2048] = {0};

    log("enter");

    exe_cmd2("./test2", parms, 3);
    exe_cmd("cat ./tmp|./test2", buf);

    log("%s", buf);
    return 0;
}




