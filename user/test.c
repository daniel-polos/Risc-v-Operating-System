#include "../kernel/types.h"
#include "user.h"
#include "../kernel/fcntl.h"
#include "../kernel/syscall.h"
struct perf
{
    int ctime;
    int ttime;
    int stime;
    int retime;
    int rutime;
    int average_bursttime;
};
int main(int argc, char **argv)
{
    fprintf(2, "Hello world!\n");
    int mask = (1 << SYS_fork) | (1 << SYS_kill) | (1 << SYS_sbrk);
    // int mask = (1 << 1);
    //sleep(1); //doesn't print this sleep
    trace(mask, getpid());
    int cpid = fork(); //prints fork once
    if (cpid == 0)
    {
        //fork();// prints fork for the second time - the first son forks
        // mask = (1 << 13); //to turn on only the sleep bit
        //mask= (1<< 1)|(1<< 13); you can uncomment this inorder to check you print for both fork and sleep syscalls
        trace(mask, getpid()); //the first son and the grandchilde changes mask to print sleep
        // set_priority(1);
        for (int i = 0; i < 100000000; i++)
        {
        }
        //fork();//should print nothing
        exit(0); //should print nothing
    }
    else
    {
        //sleep(10);// the father doesnt pring it - has original mask
        int stat;
        struct perf *childperf;
        if ((childperf = (struct perf *)malloc(sizeof(struct perf))) != 0)
        {
            char *x = sbrk(10);
            x[1] = 'a';
            printf("The Pid is: %d\n", cpid);
            if (wait_stat(&stat, childperf) > -1)
            {
                fprintf(2, "child terminated!\nchild status: %d\n", stat);
                fprintf(2, "ctime: %d\nttime: %d\n", childperf->ctime, childperf->ttime);
                fprintf(2, "stime: %d\nretime: %d\nrutime: %d\n", childperf->stime, childperf->retime, childperf->rutime);
                printf("avarage brust time: %d\n", childperf->average_bursttime);
                free(childperf);
            }
            else
                fprintf(2, "wait_stat failed!\n");
        }
        else
            fprintf(2, "kalloc failed!\n");
    }
    exit(0);
}

