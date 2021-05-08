#define MAX_STACK_SIZE 4000

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void start_func1(){
    sleep(20);
    printf("after first sleep\n");
    sleep(20);
    printf("after second sleep\n");
    sleep(5);
    printf("after thread sleep\n");
    kthread_exit(5);
   
}


void kcreate_test(){
    void *stack = malloc(MAX_STACK_SIZE);
    
    printf("calling to kthread_create\n");
    int tid = kthread_create(&start_func1, stack);
    //int tidd = kthread_create(start_func2,stack);
    int status;
    //printf("%d\n", tidd);
    printf("before kthread_join\n");
    printf("%d\n", tid);

    kthread_join(tid,&status);
    // free(stack);
    printf("finished kthread_join with status %d\n", status);
}

int main(void){
    kcreate_test();
    exit(0);
}