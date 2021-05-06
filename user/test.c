#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


#define SIG_FIRST 2
#define SIG_SECOND 3

#define SIG_DFL 0
#define SIG_IGN 1
#define SIGKILL 9
#define SIGSTOP 17
#define SIGCONT 19
#define NTHREAD 8

struct sigaction {
  void (*sa_handler)(int);
  uint sigmask;
};

void func_1(int arg){
    sleep(20);
    printf("inside func_1\n");
    printf("func_1 of SIG_FIRST. Inside process:  %d \n",getpid());
    sleep(3);
    printf("after sleeping\n");

}

void func_2(int i){
    sleep(20);
    printf("inside func_2\n");
    printf("func_2 of SIG_THIRD. inside process:  %d \n",getpid());
    sleep(3);
    printf("after sleeping\n");
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
    struct sigaction signal_handler1;
    struct sigaction signal_handler2;
    signal_handler1.sa_handler = func_1;
    signal_handler1.sigmask = 0;
    signal_handler2.sa_handler = func_2;
    signal_handler2.sigmask = 0;
    sigaction(SIG_FIRST,&signal_handler1,0);
    sigaction(SIG_SECOND,&signal_handler2,0);
    if((child_pid = fork()) == 0){
        printf("child sending sig_stop to parent");
        kill(child_pid,SIGSTOP);
        printf("child goes to sleep to give parent time to handle stop parent\n");
        sleep(40);
        kill(child_pid, SIG_FIRST);
        printf("child sent parent sigaction func_1 but parent shoudnt handle it because of the sig stop\n");
        sleep(40);
        printf("child sending sig_cont to parent");
        kill(child_pid, SIGCONT);
        sleep(40);
        kill(child_pid, SIG_FIRST);
    }
    else {
        int i = 0;
        while(i<60){
            sleep(2);
            i++;
        }
        printf("finshed\n If only func_2 printed then\n ==== passed ==== test_sigaction_cont_stop ====\n");
    }
}
void restore_handler_test(void){
    int pid;
    struct sigaction old_act;
    old_act.sa_handler = 0;
    old_act.sigmask = 0;
    
    struct sigaction signal_handler1;
    signal_handler1.sa_handler = func_1;
    signal_handler1.sigmask = 0;

    struct sigaction signal_handler2;
    signal_handler2.sa_handler = func_2;
    signal_handler2.sigmask = 0;

    sigaction(SIG_FIRST,&signal_handler1,0);
    sigaction(SIG_FIRST,&signal_handler2,&old_act);
    sigaction(SIG_FIRST,&old_act, 0);

    if((pid = fork()) == 0){
        //kill(pid, SIG_FIRST);
        printf("child goes to sleep to give time to parent to hanlde the kill\n");
        sleep(55);
    }
    else{
        kill(pid, SIG_FIRST);
        printf("parent goes to sleep to give time to the child to send him the kill\n");
        sleep(25);
        printf("if func_1 was printed then\n===PASS====restore_test\n");
    }
}

void inheritence_test(void){
    
    int child_pid;
    int mask = (1 << SIGCONT);
    int i;
    i=1;
    sigprocmask(mask);

    struct sigaction signal_handler1;
    signal_handler1.sa_handler = func_1;
    signal_handler1.sigmask = 0;
    sigaction(SIG_FIRST,&signal_handler1,0);
    
    if((child_pid = fork()) == 0){
        while(i<1000){
        sleep(7);
        }
    }
    else{
        kill(child_pid, SIGSTOP);
        sleep(30);
        kill(child_pid,SIGCONT);
        sleep(30);
        kill(child_pid, SIG_FIRST);
        sleep(60);
        printf("If func_1 was NOT printed then\n===PASS====inheritence_test\n");
        } 
}


struct test {
    void (*f)(void);
    char *s;
  } tests[] = {
    // {kill_test, "test_kill"},
    // {test_sigaction_cont_stop, "test_sigaction_cont_stop"},
    //{restore_handler_test, "restore_handler_test"},
     {inheritence_test, "test_child_inherit_mask"},
    { 0, 0}, 
  };

int main(void){
    for (struct test *t = tests; t->s != 0; t++) {
        printf("########## TEST: %s ###########\n", t->s);
        t->f();
    }

    exit(0);
}
