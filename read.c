#include <stdio.h>
#include<unistd.h> 
#include<string.h> 

#define log(fmt, args...) \ 
do { \
    printf("\n[%s:%s()]: " fmt "\n", __FILE__, __func__, ##args); \
}while(0)

/*
读取test1程序的执行终端返回结果.
*/

int main() 
{
    FILE *fp = NULL;
    int ret;
    char buf[1024] = {0};

    log("enter");
    fp = popen("./test1", "r");
    while(fgets(buf, sizeof(buf) - 1, fp) != 0) {
        log("%s", buf);
    }

    fflush(fp);

    log("exit");
    pclose(fp);
    return 0;
}




