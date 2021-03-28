#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"

int main(int argc, char** argv){
    //int mask = (1<< SYS_fork) | (1<< SYS_kill) | (1<< SYS_sbrk) | (1<< SYS_write); 
    int pid = getpid();
    trace(1<<SYS_sbrk, pid );
    sbrk(4096);
    exit(0);
}

