#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"


void start_func1();
void kcreate_test();


void start_func1(){
    sleep(20);
    printf("after first sleep\n");
    sleep(20);
    printf("after second sleep\n");
    sleep(20);
    printf("after thread sleep\n");
    kthread_exit(5);
   
}

void kcreate_test(){
    void *stack = malloc(MAX_STACK_SIZE);
    int tid = kthread_create(start_func1,stack);
    printf("tid is: %d\n", tid);
    int status;
    printf("before kthread_join\n");
    kthread_join(tid,&status);
    // free(stack);
    printf("finished kthread_join with status %d\n", status);

}

int
main(int argc, char *argv[])
{
    kcreate_test();
    exit(0);
}