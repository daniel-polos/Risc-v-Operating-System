
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


//User defined sigals
#define SIG_2 2
#define SIG_3 3
#define SIG_4 4

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

//User defined handlers
void handler2(int i){
    sleep(2);
    printf("SIG_2 handled by %d\n",getpid());
}

void handler3(int i){
    sleep(2);
    printf("SIG_3 handled by %d with i=%d\n",getpid(),i);
}

void test_kill(void){
    int child_pid,cstatus;
    if((child_pid = fork())>0){
        sleep(30);
        kill(child_pid, SIGKILL);
        printf("after calling kill\n");
        wait(&cstatus);
        printf("killed child!\n");
    }else{
        while(1){
            sleep(4);
        }
    }
}

// expected result: print child, stops, cotinue, and killed.
void test_stop_cont(void){
    int child_pid,cstatus;
    if((child_pid = fork())>0){
        sleep(30);
        kill(child_pid, SIGSTOP);
        printf("sigstop sent to child\n");
        sleep(40);
        kill(child_pid, SIGCONT);
        printf("sigcont sent to child\n");
        sleep(30);
        kill(child_pid, SIGKILL);
        printf("killed child\n");

        wait(&cstatus);
        printf("cstat:%d\n",cstatus);
    }else{
        while(1){
            sleep(4);
            printf("child still sleeping!\n");
        }
    }
}

// expected result: print child, stops, cotinue is blocked , and killed.
// verifying blocking sigkill and sigcont is impossible +  ignore sigcont
void test_stop_cont_2(void){
    int child_pid,cstatus;
    int mask = (1 << SIGCONT) | (1 << SIGKILL) | (1 << SIGSTOP);
    sigprocmask(mask);

    if((child_pid = fork())>0){
        sleep(30);
        kill(child_pid, SIGSTOP);
        printf("sigstop sent to child\n");
        sleep(30);
        kill(child_pid, SIGCONT);
        printf("sigcont sent to child (should be ignored)\n");
        sleep(40);
        kill(child_pid, SIGKILL);
        printf("killed child\n");
        wait(&cstatus);
        printf("cstat:%d\n",cstatus);
    }else{
        while(1){
            sleep(4);
            printf("child still sleeping!\n");
        }
    }
}

// expected result: print child, stops, cotinue is blocked , and killed.
// verifying blocking sigkill and sigcont is impossible +  ignore sigcont
void test_stop_cont_3(void){
    int child_pid,cstatus;
    //int mask = (1 << SIGCONT) | (1 << SIGKILL) | (1 << SIGSTOP);

    if((child_pid = fork())>0){
        sleep(30);
        kill(child_pid, SIGSTOP);
        printf("sigstop sent to child\n");
        sleep(30);
        kill(child_pid, SIGCONT);
        printf("sigcont sent to child (should be ignored)\n");
        sleep(40);
        kill(child_pid, SIGKILL);
        printf("killed child\n");

        wait(&cstatus);
        printf("cstat:%d\n",cstatus);
    }else{
        int si = SIG_IGN;
        void* p;
        sigaction(SIGCONT,(struct sigaction*)&si,(struct sigaction*)&p);
        printf("p:%d\n",p);
        while(1){

            sleep(4);
            printf("child still sleeping!\n");
        }
    }
}

//verifying successful handler's behaviour changed + handling user handler
void test_sigaction(void){
    int cpid,cstatus;
    if((cpid = fork())>0){
        sleep(40);
        kill(cpid,SIG_2);
        sleep(5);
        printf("after sleep\n");

        kill(cpid,SIG_3);
        sleep(5);
        sleep(10);
        kill(cpid,SIG_4);

        wait(&cstatus);
        printf("cstat:%d\n",cstatus);
    }else{
        struct sigaction sa;
        sa.sa_handler = handler3;
        sa.sigmask = 0;

        sigaction(SIG_2,&sa,0);
        sigaction(SIG_3,&sa,0);
        while(1){
            sleep(10);
            printf("Child ");
        }
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
            printf("inside child");
            sleep(10);
            kill(cpid,SIG_2);
            sleep(5);
            kill(cpid,SIGCONT);
            sleep(5);
            printf("Child ");
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
