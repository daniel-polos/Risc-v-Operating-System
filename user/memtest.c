#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"

void memory_test(void){
    char* array[22]; //// more than 16 so some will be in swapFile
    int i=0;
	int pid;
    // allocating 22 pages
    //going throuh all pages and alloc them
   
	while (i < 22){
		array[i] = sbrk(PGSIZE);
		array[i][5] = i; //Placement in the i'th page
		i++;
	}
	printf("====Checking if all 22 pages at [i][5] still good=====\n");
	i=0;
	while (i < 22){
		if (array[i][5] == i){
			printf("nice! page: %d\n", i);
		}
		i++;
	}
	printf("fork test\n");
	int status;
	if((pid=fork())==0){
		printf("son starting!\n son changing all pages [i][5] to something else\n");
		i=0;
		while (i < 22){
			if (array[i][5] == i){
				printf("nice! copy is good to son page: %d\n", i);
				array[i][5]= i+7;
			}
			i++;
		}
		printf("son exiting...\n");
		exit(1);
	}
	else{
		wait(&status); //let son change
		printf("son finished!\n now checking if parent kept as before son changed the pages\n");
		i=0;
		while (i < 22){
			if (array[i][5] == i){
				printf("nice! fork did'nt changed parent's data!\n: %d\n", i);
			}
			i++;
		}
		//printf("now iterte in all kinds of oreders, to stimulate page faults (swappings)\n");
		while (i<4){
			array[i][0]= i; //access pages 0-3
			i++;
		}
		//printf("level up\n");
		//sleep(3);
		i=4;
		while(i<8){
			array[i][0]= i; //access pages 4-7
			i++;
		}
		//sleep(3);
		i=4;
		while(i<8){
			array[i][0]= i; //access pages 4-7
			i++;
		}
		//printf("level up\n");
		//sleep(3);
		i=4;
		while(i<8){
			array[i][0]= i; //access pages 4-7
			i++;
		}
		//printf("level up\n");
		sleep(3);
		i=4;
		while(i<8){
			array[i][0]= i; //access pages 4-7
			i++;
		}
		sleep(3);
		i=4;
		while(i<8){
			array[i][0]= i; //access pages 4-7
			i++;
		}
		sleep(3);
		//printf("level up\n");
		i=7;
		while(i>=0){
			array[i][0]= i; //access pages 0-7 from 7 to 0
			i--;
		}
		sleep(3);
		//printf("level up\n");
		i=8;
		while(i<12){
			array[i][0]= i; //access pages 8-11
			i++;
		}
		//printf("level up\n");
		sleep(3);
		i=8;
		while(i<12){
			array[i][0]= i; //access pages 8-11
			i++;
		}
		sleep(3);
		i=12;
		while(i<16){
			array[i][0]= i; //access pages 12-15
			i++;
		}    
		//printf("level up\n");
		sleep(3);
		i=16;
		while(i<22){
			array[i][0]= i; //access pages 16-21
			i++;
		}
		sleep(3); 
		//printf("level up\n");  
		i=0;
		while(i<4){
			int t=0;
			while(t<4){
				//printf("here? when t== %d\n", t);
				array[t][0]= t;
				t++;
			}
			//printf("level up1111111111\n");
			sleep(3);
			int j= i*4;
			int k= i*4;
			while(j<k+4){
				//printf("level up222222222\n j: %d i: %d\n", j,i);
				array[j][0]= j;
				j++;
			}
			//printf("level uppupupupupup\n j: %d i: %d\n", j,i);
			sleep(3);
			i++;
		}
		printf("DONE!\n");
	}
}
/* 
RESUTS:
num of pagefaults		Page replacement algorithms
97 						SCFIFO
52						LAPA
49						NFUA*/

struct test {
    void (*f)(void);
    char *s;
  } tests[] = {
     {memory_test, "memory_test"},
    { 0, 0}, 
  };

int main(int argc, char *argv[]){
    for (struct test *t = tests; t->s != 0; t++) {
        printf("########## TEST: %s ###########\n", t->s);
        t->f();
    }
    exit(0);
};
	
