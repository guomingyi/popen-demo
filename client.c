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

int main() 
{
    FILE *fp = NULL;
    int ret;

    log("enter");
    fp = popen("./test2", "w");
    /*
        需要注意，fputs内容一定要加上换行符\n，
        否则管道对端只能接收第一个cmd而导致出错.
        命令顺序必须跟目标执行函数的执行命令一致才新.
    */
    fputs("1\n",fp);
    fputs("2\n",fp);
    fputs("3\n",fp);

    fflush(fp);

    log("exit");
    pclose(fp);
    return 0;
}




