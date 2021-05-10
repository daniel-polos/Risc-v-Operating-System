/*#include "kernel/param.h"
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
}*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


//User defined sigals
#define SIG_FIRST 2
#define SIG_SECOND 3
#define SIG_THIRD 4

#define SIG_DFL 0
#define SIG_IGN 1
#define SIGKILL 9
#define SIGSTOP 17
#define SIGCONT 19
#define NTHREAD 8 //TRHEAD

struct sigaction {
  void (*sa_handler)(int);
  uint sigmask;
};

void func_1(int arg){
    printf("inside func_1\n");
    printf("func_1 of SIG_FIRST. Inside process:  %d \n",getpid());
    sleep(3);
}

void func_2(int i){
    printf("inside func_2\n");
    printf("func_2 of SIG_THIRD. inside process:  %d \n",getpid());
    sleep(3);
}

void kill_test(void){
    int pid;
    int status;
    if((pid = fork()) == 0){
        printf("inside child\n");
        printf("child goes to sleep\n");
        while(1){
            sleep(2);
        }
        printf("error- FAILED");
    }
    else{
        printf("inside parent\n");
        sleep(10);
        printf("parent slept, gave time to child to run\n");
        printf("killing the child!\n");
        kill(pid, SIGKILL);
        wait(&status);
        printf("=== PASS === KILL_TEST ===\n");
    }
}


void test_sigaction_cont_stop(void){
    int child_pid;
    int pid;
    int status;
    struct sigaction signal_handler;
    pid = getpid();
    sigaction signal_handler.sa_handler = handler3;
    sigaction signal_handler.sigmask = 0;
    sigaction(SIG_FIRST,&signal_handler,0);
    sigaction(SIG_SECOND,&signal_handler,0);
    if((pid = fork()) == 0){
        printf("child sending sig_stop to parent");
        kill(pid,SIGSTOP);
        printf("child goes to sleep to give parent time to handle stop parent\n");
        sleep(40);
        kill(pid, SIG_FIRST);
        printf("child sent parent sigaction func_1 but parent shoudnt handle it because of the sig stop\n");
        sleep(40);
        printf("child sending sig_cont to parent");
        kill(pid. sigcont);
        sleep(40);
        kill(pid, SIG_FIRST);
    }
    else {
        while(i<60){
            sleep(2);
            i++;
        }
        printf("finshed\n If only func_3 printed then\n ==== passed ==== test_sigaction_cont_stop ====\n")
    }
}




//verifying successful handler's behaviour changed + handling user handler
// verify handlers goes to child proc in fork
void test_sigaction_2(void){
    int cpid,cstatus;

    struct sigaction sa;
    sa.sa_handler = handler3;
    sa.sigmask = 0;
    sigaction(SIG_2,&sa,0);
    sigaction(SIG_3,&sa,0);

    if((cpid = fork())>0){
        sleep(40);
        kill(cpid,SIG_2);
        sleep(5);

        kill(cpid,SIG_3);
        sleep(5);
        kill(cpid,SIG_2);

        sleep(10);
        kill(cpid,SIG_4);

        wait(&cstatus);
        printf("cstat:%d\n",cstatus);
    }else{
        while(1){
            sleep(10);
            printf("Child ");
        }
    } 
}

//verifying successful handler's behaviour changed + handling user handler
// verify handlers goes to child proc in fork

void test_restoring_previous_handler(void){
    int cpid,cstatus;

    struct sigaction old_act;
    old_act.sa_handler = 0;
    old_act.sigmask = 0;
    
    struct sigaction sa_1;
    sa_1.sa_handler = handler3;
    sa_1.sigmask = 0;

    struct sigaction sa_2;
    sa_2.sa_handler = handler2;
    sa_2.sigmask = 0;

    printf("oldact is: %d\n", &old_act);
    sigaction(SIG_2,&sa_2,0);
    sigaction(SIG_2,&sa_1,&old_act);
    sigaction(SIG_2,&old_act, 0);

    if((cpid = fork())>0){
         sleep(40);
         printf("calling to kill from main\n");
         kill(cpid,SIG_2);
 
        wait(&cstatus);
        printf("cstat:%d\n",cstatus);
    }else{
        while(1){
            sleep(10);
            printf("Child ");
        }
    } 
}

void test_child_inherit_mask(void){
    int cpid,cstatus;
    int mask = (1 << SIGCONT);
    sigprocmask(mask);

    struct sigaction sa;
    sa.sa_handler = handler3;
    sa.sigmask = 0;
    sigaction(SIG_2,&sa,0);

    if((cpid = fork())>0){
        //printf("inside parent");
        sleep(40);
        kill(cpid,SIG_2);
        sleep(5);

        kill(cpid,SIGCONT);
        sleep(5);

        wait(&cstatus);
        printf("cstat:%d\n",cstatus);
    }else{
        while(1){
            //printf("inside child");
            sleep(10);
            kill(cpid,SIG_2);
            sleep(5);
            kill(cpid,SIGCONT);
            sleep(5);
            //printf("Child ");
        }
    } 
}


struct test {
    void (*f)(void);
    char *s;
  } tests[] = {
    // {test_kill, "test_kill"},
    // {test_stop_cont, "test_stop_cont"},
    //{test_stop_cont_2, "test_stop_cont_2"},
    // {test_stop_cont_3, "test_stop_cont_3"},
     //{test_sigaction, "test_sigaction"},
    //{test_restoring_previous_handler, "test_restoring_previous_handler"},
     //{test_sigaction_2, "test_sigaction_2"},
      {test_child_inherit_mask, "test_child_inherit_mask"},
    { 0, 0}, 
  };

int main(void){
    for (struct test *t = tests; t->s != 0; t++) {
        printf("----------- test - %s -----------\n", t->s);
        t->f();
    }

    exit(0);
}
