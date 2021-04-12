#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

//funcs
void test_for_bursttime_when_son_just_dies();
void testing_trace();
void extra_complicated_long_test();
void test_for_bursttime_when_son_does_long_stuff();
void test_for_bursttime_when_proces_with_lots_short_running_time(int num);
void test_with_lots_of_processes_for_order_checks();
void test_for_FCFS();
void test_for_FCFS_2childs_different_creation_time();
void test_for_SRT_preemptive();

struct perf {
    int ctime;
    int ttime;
    int stime;
    int retime;
    int rutime;
    int average_bursttime; //average of bursstimes in 100ths (so average*100)

};


int main(int argc, char** argv){
    test_for_bursttime_when_son_just_dies(); // not ok?
    //testing_trace();
    //extra_complicated_long_test(); //not ok
    //test_for_bursttime_when_son_does_long_stuff(); // ok
    //test_for_bursttime_when_proces_with_lots_short_running_time(1); //ok
    //test_with_lots_of_processes_for_order_checks(); // test not work
    //test_for_FCFS(); //ok
    //test_for_FCFS_2childs_different_creation_time(); //ok
    //test_for_SRT_preemptive(); // ok, but wtf?!?!?!?!
    exit(0);

}

void test_for_SRT_preemptive(){
    int i= 0;
    while(i<300000000){
        i++;
    }
    exit(0);
}//works fine

void test_for_FCFS_2childs_different_creation_time(){
    int cpid=fork();
    fprintf(1, "cpid is: %d\n", cpid);
    if(cpid==0){//son like sunshine
        fprintf(1, "son going to sleep...\n");
        sleep(2);
        fprintf(1, "son waking up & exiting...\n");
        exit(0);
    }
    else{//father
        int i = 0;
        fprintf(1, "father start wasting time...\n");
        while(i<1000000000){
            i++;
            //if(i==100000000){sleep(1);}
        }
        int cpid2=fork();
        fprintf(1, "cpid2 is: %d\n", cpid2);
        if(cpid2==0){//son like sunshine
            fprintf(1, "son going to sleep...\n");
            sleep(2);
            fprintf(1, "son waking up & exiting...\n");
            exit(0);
        }
        else{
            fprintf(1, "father going to sleep...\n");
            sleep(2);
            fprintf(1, "father waking up & waiting...\n");
            wait(0);
            wait(0);
        }
    }
}
// cpid is: 4
// father start wasting time...
// cpid2 is: 5
// father going to sleep...
// cpid is: 0
// son going to sleep...
// cpid2 is: 0
// son going to sleep...
// son waking up & exiting...
// father waking up & waiting...
// son waking up & exiting...

void test_for_FCFS(){
    int cpid=fork();
    fprintf(1, "cpid is: %d\n", cpid);
    if(cpid==0){//son like sunshine
    int i = 0;
            while(i<1000000000){
            i++;
        }
        fprintf(1, "son going to sleep...\n");
        sleep(2);
        fprintf(1, "son waking up & exiting...\n");
        exit(0);
    }
    else{//father
        //int i = 0;
        fprintf(1, "father start wasting time...\n");
        /*
        while(i<1000000000){
            i++;
            if(i==100000000){sleep(1);}
        }
        */
        fprintf(1, "father going to sleep...\n");
        sleep(2);
        fprintf(1, "father waking up & waiting...\n");
        wait(0);
    }
}
// cpid is: 4
// father start wasting time...
// father going to sleep...
// cpid is: 0
// son going to sleep...
// father waking up & waiting...
// son waking up & exiting...


void test_with_lots_of_processes_for_order_checks(){
    int i=0;
    struct perf* performance = malloc(sizeof(struct perf));
    int cpid=fork();
    if(cpid==0){//son like sunshine
        while(i<5){
            int cpid2=fork();
            if(cpid2==0){//grandchild
                if(i%2==0){
                    int k=0;
                    while(k<10000000000000){
                        k++;
                    }
                }
                else{
                    sleep(1);
                    sbrk(2);
                    int k=0;
                    while(k<10000000000000){
                        k++;
                    }
                }
                exit(0); //so grandchild won't make kids
            }
            else{//father (child1)
                wait(0);
            }
            i++;
        }
    }
    else{//father
    int t_pid = wait_stat(0, performance);
    fprintf(1, "terminated pid: %d, ctime: %d, ttime: %d, stime: %d, retime: %d, rutime: %d average_bursttime: %d \n",
                t_pid, performance->ctime, performance->ttime, performance->stime, performance->retime, performance->rutime,
                 performance->average_bursttime);
    }
}//does not work, not even in a fresh copy of the git repository->test is wrong.(tried wait instead of wait_stat in the copy of the git repo, so the test should run the same)

void test_for_bursttime_when_proces_with_lots_short_running_time(int num){
    int i=0;
    struct perf* performance = malloc(sizeof(struct perf));
    int cpid=fork();
    if(cpid==0){//son like sunshine
        while(i<num){
            i++;
            sleep(1);
        }
    }
    else{//father
    int t_pid = wait_stat(0, performance);
    fprintf(1, "terminated pid: %d, ctime: %d, ttime: %d, stime: %d, retime: %d, rutime: %d average_bursttime: %d \n",
                t_pid, performance->ctime, performance->ttime, performance->stime, performance->retime, performance->rutime,
                 performance->average_bursttime);
    }
}
//terminated pid: 14, ctime: 170, ttime: 171, stime: 1, retime: 0, rutime: 0 average_bursttime: 100 


void test_for_bursttime_when_son_does_long_stuff(){
    int i=0;
    struct perf* performance = malloc(sizeof(struct perf));
    int cpid=fork();
    if(cpid==0){//son like sunshine
        while(i<1000000000){
            i++;
        }
    }
    else{//father
    int t_pid = wait_stat(0, performance);
    fprintf(1, "terminated pid: %d, ctime: %d, ttime: %d, stime: %d, retime: %d, rutime: %d average_bursttime: %d \n",
                t_pid, performance->ctime, performance->ttime, performance->stime, performance->retime, performance->rutime,
                 performance->average_bursttime);
    }
}
//terminated pid: 4, ctime: 19, ttime: 37, stime: 0, retime: 0, rutime: 18 average_bursttime: 300


void test_for_bursttime_when_son_just_dies(){
    struct perf* performance = malloc(sizeof(struct perf));
    int cpid=fork();
    if(cpid==0){//son like sunshine
        exit(0);
    }
    else{//father
    int t_pid = wait_stat(0, performance);
    //printf("ctime:\n", performance->ctime);
    //printf("here!!!\n");
    printf("terminated pid: %d, ctime: %d, ttime: %d, stime: %d, retime: %d, rutime: %d average_bursttime: %d \n",
                t_pid, performance->ctime, performance->ttime, performance->stime, performance->retime, performance->rutime,
                 performance->average_bursttime);
    }
}
// terminated pid: 4, ctime: 19, ttime: 19, stime: 0, retime: 0, rutime: 0 average_bursttime: 250
//as should be. 50 * (ticks - ticks) + 50 * (500 / 100) = 250.
//became_running_at = ticks because rutime = 0.  

void testing_trace(){
    //mask=(1<< SYS_fork)|( 1<< SYS_kill)| ( 1<< SYS_sbrk) | ( 1<< SYS_write);
    int mask=(1<< 1);
    sleep(1); //doesn't print this sleep
    trace(mask, getpid());
    int cpid=fork();//prints fork once
    if (cpid==0){
        fork();// prints fork for the second time - the first son forks
        //mask= (1<< 13); //to turn on only the sleep bit
        mask= (1<< 1)|(1<< 13); //you can uncomment this inorder to check you print for both fork and sleep syscalls
        trace(mask, getpid()); //the first son and the grandchilde changes mask to print sleep
        sleep(1);
        fork();//should print nothing
        exit(0);//shold print nothing
    }
    else {
        sleep(10);// the father doesnt pring it - has original mask
    }
    mask= (1<< 12)|( 1<< 2) | (1<<6); //sbrk & exit & kill
    trace(mask, getpid());
    cpid= fork();
    kill(cpid);
    sbrk(4096);
}
// 3: syscall fork NULL -> 4
// 4: syscall fork NULL -> 5
// 4: syscall sleep -> 0
// 4: syscall fork NULL -> 6
// 5: syscall sleep -> 0
// 5: syscall fork NULL -> 7
// 3: syscall kill 8 -> 0
// 3: syscall sbrk 4096 -> 12288


void extra_complicated_long_test(){

    struct perf* performance = malloc(sizeof(struct perf));
    int mask=(1<< 1) | (1<< 23) | (1<< 3);
    trace(mask, getpid());
    int cpid = fork();
    if (cpid != 0){
        int t_pid = wait_stat(0, performance);
        fprintf(1, "terminated pid: %d, ctime: %d, ttime: %d, stime: %d, retime: %d, rutime: %d average_bursttime: %d \n", t_pid, performance->ctime, performance->ttime, performance->stime, performance->retime, performance->rutime, performance->average_bursttime);
    }
    else{
        sleep(10);
        for(int i=1; i < 15; i++){
            int c_pid = fork();
            if(c_pid == 0){
                sleep(i);
                exit(0);
            }
            else{
                int i = 0;
                while(i<100000000){
                    i++;
                }
            }
        }
        sleep(10);
        for(int i=1; i < 15; i++){
            int c_pid = fork();
            if(c_pid == 0){
                int i = 0;
                while(i<10000000){
                    i++;
                }
                exit(0);
            }
            else{
                int t_pid = wait_stat(0, performance);
                fprintf(1, "terminated pid: %d, ctime: %d, ttime: %d, stime: %d, retime: %d, rutime: %d average_bursttime: %d \n", t_pid, performance->ctime, performance->ttime, performance->stime, performance->retime, performance->rutime, performance->average_bursttime);
                int i = 0;
                while(i<10000){
                    i++;
                }
            }
        }
    }
}
// 3: syscall fork NULL -> 4
// 4: syscall fork NULL -> 5
// 4: syscall fork NULL -> 6
// 4: syscall fork NULL -> 7
// 4: syscall fork NULL -> 8
// 4: syscall fork NULL -> 9
// 4: syscall fork NULL -> 10
// 4: syscall fork NULL -> 11
// 4: syscall fork NULL -> 12
// 4: syscall fork NULL -> 13
// 4: syscall fork NULL -> 14
// 4: syscall fork NULL -> 15
// 4: syscall fork NULL -> 16
// 4: syscall fork NULL -> 17
// 4: syscall fork NULL -> 18
// 4: syscall fork NULL -> 19
// 4: syscall wait_stat -> 5
// terminated pid: 5, ctime: 28, ttime: 35, stime: 5, retime: 6, rutime: 0 average_bursttime: 100 
// 4: syscall fork NULL -> 20
// 4: syscall wait_stat -> 6
// terminated pid: 6, ctime: 29, ttime: 35, stime: 5, retime: 5, rutime: 0 average_bursttime: 100 
// 4: syscall fork NULL -> 21
// 4: syscall wait_stat -> 7
// terminated pid: 7, ctime: 31, ttime: 40, stime: 5, retime: 8, rutime: 0 average_bursttime: 100 
// 4: syscall fork NULL -> 22
// 4: syscall wait_stat -> 8
// terminated pid: 8, ctime: 33, ttime: 40, stime: 5, retime: 6, rutime: 0 average_bursttime: 100 
// 4: syscall fork NULL -> 23
// 4: syscall wait_stat -> 20
// terminated pid: 20, ctime: 64, ttime: 65, stime: 0, retime: 1, rutime: 0 average_bursttime: 250 
// 4: syscall fork NULL -> 24
// 4: syscall wait_stat -> 21
// terminated pid: 21, ctime: 64, ttime: 65, stime: 0, retime: 1, rutime: 0 average_bursttime: 250 
// 4: syscall fork NULL -> 25
// 4: syscall wait_stat -> 24
// terminated pid: 24, ctime: 65, ttime: 66, stime: 0, retime: 1, rutime: 0 average_bursttime: 250 
// 4: syscall fork NULL -> 26
// 4: syscall wait_stat -> 22
// terminated pid: 22, ctime: 64, ttime: 65, stime: 0, retime: 1, rutime: 0 average_bursttime: 250 
// 4: syscall fork NULL -> 27
// 4: syscall wait_stat -> 23
// terminated pid: 23, ctime: 65, ttime: 66, stime: 0, retime: 1, rutime: 0 average_bursttime: 250 
// 4: syscall fork NULL -> 28
// 4: syscall wait_stat -> 26
// terminated pid: 26, ctime: 66, ttime: 67, stime: 0, retime: 1, rutime: 0 average_bursttime: 250 
// 4: syscall fork NULL -> 29
// 4: syscall wait_stat -> 25
// terminated pid: 25, ctime: 66, ttime: 67, stime: 0, retime: 1, rutime: 0 average_bursttime: 250 
// 4: syscall fork NULL -> 30
// 4: syscall wait_stat -> 29
// terminated pid: 29, ctime: 67, ttime: 68, stime: 0, retime: 1, rutime: 0 average_bursttime: 250 
// 4: syscall fork NULL -> 31
// 4: syscall wait_stat -> 27
// terminated pid: 27, ctime: 66, ttime: 67, stime: 0, retime: 1, rutime: 0 average_bursttime: 250 
// 4: syscall fork NULL -> 32
// 4: syscall wait_stat -> 31
// terminated pid: 31, ctime: 68, ttime: 69, stime: 0, retime: 1, rutime: 0 average_bursttime: 250 
// 3: syscall wait_stat -> 4
// terminated pid: 4, ctime: 18, ttime: 69, stime: 20, retime: 0, rutime: 31 average_bursttime: 0
