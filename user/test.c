
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"

#include "kernel/fcntl.h"
#include "kernel/syscall.h"
//#include "kernel/perf.h"

struct perf {
  int ctime;                   //creation time
  int ttime;                   //termination time
  int stime;                   //sleeping time
  int retime;                  //tunnable time
  int rutime;                  //running time
  int average_bursttime;
};

int main (int argc, char**argv){

    struct perf pe;
    int status;
    int id;
    int sum;
    int i;
    // printf("trace.c. status addr = %d, performance addr = %d\n", &status, &pe);

    int mask= (1<< SYS_fork) | (1<<SYS_wait) | (1<<SYS_set_priority);
    trace(mask,getpid());
    set_priority(1);

    // trace(mask,2);
    if(fork()==0){
        // sleep(3);
        set_priority(2);

        if(fork()==0){
            set_priority(3);
            // sleep(3);
            sum = 0;
            for (i=0; i<1000000000; i++) 
                sum += i;

            sleep(1);
            
            for (i=0; i<1000000000; i++) 
                sum += i;
        }
        else {
            // wait(0);
            // sleep(3);
        
            sum = 0;
            for (i=0; i<1000000000; i++) 
                sum += i;
            sleep(1);
            for (i=0; i<1000000000; i++) 
                sum += i;

            id = wait_stat(&status,&pe);
            printf("pid %d\n", id);
            printf("ctime: %d\n", pe.ctime);
            printf("ttime: %d\n", pe.ttime);
            printf("stime: %d\n", pe.stime);
            printf("retime: %d\n", pe.retime);
            printf("rutime: %d\n", pe.rutime);
            printf("average_bursttime: %d\n", pe.average_bursttime);
            // printf("%lf\n", pe.bursttime); //@TODO
        }
    }
    else {
        // wait(0);
        // sleep(3);

        sum = 0;
        for (i=0; i<1000000000; i++) 
            sum += i;

        sleep(1);

        for (i=0; i<1000000000; i++) 
            sum += i;
            
        id = wait_stat(&status,&pe);
        printf("pid %d\n", id);
        printf("ctime: %d\n", pe.ctime);
        printf("ttime: %d\n", pe.ttime);
        printf("stime: %d\n", pe.stime);
        printf("retime: %d\n", pe.retime);
        printf("rutime: %d\n", pe.rutime);
        printf("average_bursttime: %d\n", pe.average_bursttime);

        // printf("bursttime: %d\n", pe.bursttime); //@TODO
    }
    exit(0);
}
