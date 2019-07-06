#include <stdio.h>
#include<unistd.h> 
#include<string.h> 

#define log(fmt, args...) \ 
do { \
    printf("\n[%s:%s()]: " fmt "\n", __FILE__, __func__, ##args); \
}while(0)

/*
目标执行函数（交互式）,此处简化为三个输入.
*/
int main()
{
    int i = 0;
    int val;
    int count = 0;

    log("enter");

    scanf("%d", &val);
    if (val == 1) {
        log("success 1");
        count++;
    }

    scanf("%d", &val);
    if (val == 2) {
        log("success 2");
        count++;
    }

    scanf("%d", &val);
    if (val == 3) {
        log("success 3");
        count++;
    }

    if (count == 3) 
        goto ext;

    while(1) {
        sleep(1);
        log("...");
    }

ext:
    log("exit");
    return 0;
}


