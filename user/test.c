#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"

int main(int argc, char** argv){
    //int mask =  (1<< SYS_kill) | (1<< SYS_sbrk) | (1<< SYS_write); 
    int mask = (1<< SYS_fork) | (1<< SYS_kill) | (1<< SYS_sleep);
    int mask2 = (1<< SYS_sbrk);
    trace(mask, getpid());
    int c_pid =  fork();
    trace(mask2, c_pid);

    if(c_pid == 0) {
        sleep(1);
        sbrk(200);
    }
    else
    {
        trace(mask2, getpid());
        sleep(1);
        sbrk(100);
        kill(c_pid);
    }


    

    // int c1_pid = fork();
    // int c2_pid = fork();
    
    // kill(c2_pid);
    // kill(c1_pid);
    // sbrk(1200);
    exit(0);
}

