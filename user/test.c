#include "../kernel/types.h"
#include "user.h"
#include "../kernel/fcntl.h"
#include "../kernel/syscall.h"

//uint lock;

int main(int argc, char **argv)
{
    int pid1 = fork();

    if(pid1 == 0) {
        int i = 1000000;
        while(i-->0){
            //if(i%1000 == 0)
                //printf("i: %d", i);

        }
    }

    else{
        //int i = 100;
        int i = 1000000;
        while(i-->0){
            //if(i%1000 == 0)
                //printf("i: %d", i);
        }
    }

    exit(0);
}

