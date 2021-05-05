#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

extern void* callsigret(void);
extern void* endcallsigret(void);

struct cpu cpus[NCPU];

struct proc proc[NPROC];

struct proc *initproc;

int nextpid = 1;
int next_t_id= 1; //THREAD
struct spinlock t_id_lock; // THREAD
struct spinlock pid_lock;

extern void forkret(void);
static void freeproc(struct proc *p);

extern char trampoline[]; // trampoline.S

// helps ensure that wakeups of wait()ing
// parents are not lost. helps obey the
// memory model when using p->parent.
// must be acquired before any p->lock.
struct spinlock wait_lock;

// Allocate a page for each process's kernel stack.
// Map it high in memory, followed by an invalid
// guard page.
void
proc_mapstacks(pagetable_t kpgtbl) {
  struct proc *p;
  for(p = proc; p < &proc[NPROC]; p++) {
    char *pa = kalloc();
    if(pa == 0)
      panic("kalloc");
    //uint64 va = KSTACK((int) (p - proc), );
    //uint64 va = KSTACK((int) (p - proc), (int) (th - p->threads_Table)); // TODO ????
      uint64 va = KSTACK((int) (p - proc));
      kvmmap(kpgtbl, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
  }
}

// initialize the proc table at boot time.
void
procinit(void)
{
  //printf("inside procinit\n");
  struct proc *p;
  struct thread *th;
  initlock(&pid_lock, "nextpid");
  initlock(&t_id_lock, "next_t_id"); //THREAD
  initlock(&wait_lock, "wait_lock");
  for(p = proc; p < &proc[NPROC]; p++) {
      initlock(&p->lock, "proc");
      //p->kstack = KSTACK((int) (p - proc));
      for(th=p->threads_Table; th<&p->threads_Table[NTHREAD]; th++){ //thats how to write the for?
        initlock(&th->t_lock, "thread");
           
        //th->kstack = KSTACK((int) (th - p->threads_Table), (int) (th - p->threads_Table)); //TODO ????
     }
  }
}

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
int
cpuid() //??? TODO
{
  int id = r_tp();
  return id;
}

// Return this CPU's cpu struct.
// Interrupts must be disabled.
struct cpu*
mycpu(void) {
  int id = cpuid();
  struct cpu *c = &cpus[id];
  return c;
}



// Return the current struct proc *, or zero if none.
struct proc*
myproc(void) {
  push_off();
  struct cpu *c = mycpu();
  struct proc *p = c->proc;
  pop_off();
  return p;
}

struct thread*    //TREAD dual function to myproc
mythread(void) {
  push_off();
  struct cpu *c = mycpu();
  struct thread *t = c->thread;
  pop_off();
  return t;
}

int
allocpid() {
  printf("inside allocpid\n");
  int pid;
  
  acquire(&pid_lock);
  pid = nextpid;
  nextpid = nextpid + 1;
  release(&pid_lock);

  return pid;
}
int
alloc_t_id() { //THREAD
  int tid;
  
  acquire(&t_id_lock);
  tid = next_t_id;
  next_t_id = next_t_id + 1;
  release(&t_id_lock);

  return tid;
}

static void
freethread (struct thread *th)
{
  //printf("inside freethread\n");
  //if(th->trapframe)
  //  kfree((void*)th->trapframe);
  th->trapframe = 0;
  th->tid= 0;
  th->tstate = TUNUSED;
  th->chan = 0;
}


static struct thread*
allocthread (struct proc *p){ 
                       
  printf("inside allocthread\n");
  struct thread *th;
  int ind = 0;
  for(th=p->threads_Table; th<&p->threads_Table[NTHREAD]; th++){ //thats how to write the for?
    acquire(&th->t_lock);
    if(th->tstate == TUNUSED){
      goto found;
    }
    else{
      ind++;
      release(&th->t_lock);
    }
  }
  return 0;

  found:
  th->tid = alloc_t_id();
  th->tstate = TUSED;
  th->ind = ind;
  th->trapframe = (struct trapframe*)((uint64)(p->trapframe) + (uint64)(th->ind*sizeof(struct trapframe)));
  th->parent_proc = p;
  th->killed=0;
  memset(&th->context, 0, sizeof(th->context)); //THREAD
  if((th->kstack = (uint64)kalloc()) == 0){
    freethread(th);
    release(&th->t_lock);
    return 0;

  } //TEST!!!!
  /*if((th->trapframe = (struct trapframe *)kalloc()) == 0){ 
    freethread(th);
    release(&th->t_lock);
    return 0;
  }*/
  
  // Set up new context to start executing at forkret,
  // which returns to user space.
  th->context.ra = (uint64)forkret;
  th->context.sp = th->kstack + PGSIZE;
  return th;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
static struct thread*
allocproc(void)
{
  printf("inside allocproc\n");
  struct proc *p;
  //struct thread *th;
  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if(p->state == UNUSED) {
      goto found;
    } else {
      release(&p->lock);
    }
  }
  return 0;

found:
  p->pid = allocpid();
  p->state = USED;

  // Initialize handlers to be default
  
  for (int i = 0; i < 32; i++) {
    p->signal_handlers[i] = (void*)SIG_DFL;
  }

  p->signals_mask = 0;
  p->pending_signals = 0;
  p->stopped = 0;
  p->signal_handling = 0;
  //struct trapframe *trapf;
  // Allocate a trapframe page.
  //if((p->trapframe = (struct trapframe *)kalloc()) == 0){
  if((p->trapframe = (struct trapframe *)kalloc()) == 0){
    freeproc(p);
    release(&p->lock);
    return 0;
  }

//are we sure th was alredy allocated?
  //p->trapframe = trapf;   //move inside for with dynamic i addition
  //for(th=p->threads_Table; th<&p->threads_Table[NTHREAD]; th++, trapf++){ //thats how to write the for?
  //  th->trapframe = trapf;
    //debug
  //  printf("trapframe: %p\n",trapf);
    //th->trapframe = p->trapframe +(sizeof (struct trapframe))* (th - p->threads_Table);
 // }
  //printf("p->trapframe points to: %p\n",p->trapframe);
  

  // An empty user page table.
  p->pagetable = proc_pagetable(p);
  if(p->pagetable == 0){
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // Set up new context to start executing at forkret,
  // which returns to user space.
 /* memset(&p->context, 0, sizeof(p->context)); THREAD
  p->context.ra = (uint64)forkret;
  p->context.sp = p->kstack + PGSIZE;*/

  return allocthread(p);
}


// free a proc structure and the data hanging from it,
// including user pages.
// p->lock must be held.
static void
freeproc(struct proc *p)
{
  //printf("inside freeproc\n");
  struct thread *th;
  /*if(p->trapframe)
    kfree((void*)p->trapframe);*/ //thread
  //p->trapframe = 0;
  if(p->pagetable)
    proc_freepagetable(p->pagetable, p->sz);
  p->pagetable = 0;
  p->sz = 0; 
  p->pid = 0;
  p->parent = 0;
  p->name[0] = 0;
 // p->chan = 0;
  p->killed = 0;
  p->xstate = 0;
  p->state = UNUSED;
  for(th=p->threads_Table; th<&p->threads_Table[NTHREAD]; th++){ //thats how to write the for?
    freethread(th);
  }
  if(p->trapframe)
    kfree((void*)p->trapframe);
  p->trapframe = 0;
}


// Create a user page table for a given process,
// with no user memory, but with trampoline pages.
pagetable_t
proc_pagetable(struct proc *p)
{
  pagetable_t pagetable;

  // An empty page table.
  pagetable = uvmcreate();
  if(pagetable == 0)
    return 0;

  // map the trampoline code (for system call return)
  // at the highest user virtual address.
  // only the supervisor uses it, on the way
  // to/from user space, so not PTE_U.
  if(mappages(pagetable, TRAMPOLINE, PGSIZE,
              (uint64)trampoline, PTE_R | PTE_X) < 0){
    uvmfree(pagetable, 0);
    return 0;
  }

  // map the trapframe just below TRAMPOLINE, for trampoline.S.
  if(mappages(pagetable, TRAPFRAME, PGSIZE,
              (uint64)(p->trapframe), PTE_R | PTE_W) < 0){  //TODO ???
    uvmunmap(pagetable, TRAMPOLINE, 1, 0);
    uvmfree(pagetable, 0);
    return 0;
  }

  return pagetable;
}

// Free a process's page table, and free the
// physical memory it refers to.
void
proc_freepagetable(pagetable_t pagetable, uint64 sz)
{
  uvmunmap(pagetable, TRAMPOLINE, 1, 0);
  uvmunmap(pagetable, TRAPFRAME, 1, 0);
  uvmfree(pagetable, sz);
}

// a user program that calls exec("/init")
// od -t xC initcode
uchar initcode[] = {
  0x17, 0x05, 0x00, 0x00, 0x13, 0x05, 0x45, 0x02,
  0x97, 0x05, 0x00, 0x00, 0x93, 0x85, 0x35, 0x02,
  0x93, 0x08, 0x70, 0x00, 0x73, 0x00, 0x00, 0x00,
  0x93, 0x08, 0x20, 0x00, 0x73, 0x00, 0x00, 0x00,
  0xef, 0xf0, 0x9f, 0xff, 0x2f, 0x69, 0x6e, 0x69,
  0x74, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

// Set up first user process.
void
userinit(void)
{
  //printf("inside userinit#########\n");
  struct thread *th; //THREAD
  struct proc *p;

  th = allocproc();         //THREAD
  p = th->parent_proc;
  initproc = p; //THREAD
  
  // allocate one user page and copy init's instructions
  // and data into it.
  uvminit(p->pagetable, initcode, sizeof(initcode));
  p->sz = PGSIZE;

  // prepare for the very first "return" from kernel to user.
  //p->trapframe->epc = 0;      // user program counter //THREAD
  //p->trapframe->sp = PGSIZE;  // user stack pointer   //THREAD
  th->trapframe->epc = 0;      // user program counter  //THREAD
  th->trapframe->sp = PGSIZE;  // user stack pointer    //THREAD

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = USED;       //THREAD
  th->tstate= TRUNNABLE; //THREAD
  release(&th->t_lock);
  release(&p->lock);
}

// Grow or shrink user memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  //printf("inside growproc\n");
  uint sz;
  struct proc *p = myproc();


  acquire(&p->lock); //TODO ???
  sz = p->sz;
  if(n > 0){
    if((sz = uvmalloc(p->pagetable, sz, sz + n)) == 0) {
      release(&p->lock);
      return -1;
    }
  } else if(n < 0){
    sz = uvmdealloc(p->pagetable, sz, sz + n);
  }
  p->sz = sz;
  release(&p->lock);
  return 0;
}

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int
fork(void)
{
  //printf("inside fork\n");
  int i, pid;
  struct proc *np;
  struct proc *p = myproc();
  struct thread *th = mythread(); //THREAD
  struct thread *nt;              //THREAD
  // Allocate process.
  if((nt = allocproc()) == 0){  //THREAD nt instead of np
    return -1;
  }
  np= nt->parent_proc; //THREAD

  // Copy user memory from parent to child.
  if(uvmcopy(p->pagetable, np->pagetable, p->sz) < 0){
    freethread(nt);       //CHECK! we are calling to freethread inside freeproc
    release(&nt->t_lock); //THREAD
    freeproc(np);
    release(&np->lock);
    return -1;
  }
  np->sz = p->sz;

  //copy signals_mask and signals_handlers to child proc
  //memmove(&np->signal_handlers, &p->signal_handlers, sizeof(p->signal_handlers));
  for (int j = 0; j < 32; j++) {
    np->signal_handlers[j] = p->signal_handlers[j];
    //debug
    //printf("j: %d, process pid: %d, signal_handler[i]: %d\n",j, np->pid, np->signal_handlers[j]);
  }

  np->signals_mask = p->signals_mask;
  np->pending_signals = 0;

  // copy saved user registers.
  *(nt->trapframe) = *(th->trapframe); //THREAD

  // Cause fork to return 0 in the child.
  nt->trapframe->a0 = 0; //THREAD, neet to understand the comment in the line above

  // increment reference counts on open file descriptors.
  for(i = 0; i < NOFILE; i++)
    if(p->ofile[i])
      np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = idup(p->cwd);

  safestrcpy(np->name, p->name, sizeof(p->name));

  pid = np->pid;
  release(&nt->t_lock);
  release(&np->lock);

  acquire(&wait_lock);
  np->parent = p;
  release(&wait_lock);

  acquire(&nt->t_lock); //CHECK! I've changed from np->lock to nt->lock
  nt->tstate = TRUNNABLE; //THREAD
  release(&nt->t_lock);

  return pid;
}

// Pass p's abandoned children to init.
// Caller must hold wait_lock.
void
reparent(struct proc *p)
{
  //printf("inside reparent\n");
  struct proc *pp;

  for(pp = proc; pp < &proc[NPROC]; pp++){
    if(pp->parent == p){
      pp->parent = initproc;
      wakeup(initproc);
    }
  }
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
void
exit(int status)
{
  printf("inside exit\n");
  struct proc *p = myproc();
  struct thread *th;
  struct thread *thisth = mythread();
  if(p == initproc)
    panic("init exiting");

  //kill all threads
  for(th=p->threads_Table; th<&p->threads_Table[NTHREAD]; th++){
    if(th!=thisth){
          acquire(&th->t_lock);
          th->killed = 1;
          if (th->tstate == TSLEEPING) {
            th->tstate = TRUNNABLE;
          }
          release(&th->t_lock);

      }
  }

  for(th=p->threads_Table; th<&p->threads_Table[NTHREAD]; th++){
        if(th!=thisth) {
          acquire(&th->t_lock);
          kthread_join(th->tid, 0);
          release(&th->t_lock);
        }       
  }

  //kthread_exit(thisth->pid, 0);
  

  // Close all open files.
  for(int fd = 0; fd < NOFILE; fd++){
    if(p->ofile[fd]){
      struct file *f = p->ofile[fd];
      fileclose(f);
      p->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(p->cwd);
  end_op();
  p->cwd = 0;



  acquire(&wait_lock);

  // Give any children to init.
  reparent(p);


  // Parent might be sleeping in wait().
  wakeup(p->parent);
  acquire(&thisth->t_lock);

  acquire(&p->lock);

  p->xstate = status;
  p->state = ZOMBIE;
  release(&p->lock);
  thisth->tstate = TZOMBIE;

  release(&wait_lock);

  // Jump into the scheduler, never to return.
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(uint64 addr)
{
  //printf("inside wait\n");
  struct proc *np;
  int havekids, pid;
  struct proc *p = myproc();


  acquire(&wait_lock);


  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(np = proc; np < &proc[NPROC]; np++){
      if(np->parent == p){
        // make sure the child isn't still in exit() or swtch().
        acquire(&np->lock);

        havekids = 1;
        if(np->state == ZOMBIE){
          // Found one.
          pid = np->pid;
          if(addr != 0 && copyout(p->pagetable, addr, (char *)&np->xstate,
                                  sizeof(np->xstate)) < 0){
          release(&np->lock);
          release(&wait_lock);
          return -1;
          }
          freeproc(np);
          release(&np->lock);
          release(&wait_lock);
          return pid;
        }
        release(&np->lock);
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || p->killed){
      release(&wait_lock);
      return -1;
    }
    
    // Wait for a child to exit.
    sleep(p, &wait_lock);  //DOC: wait-sleep
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void
scheduler(void)
{
  //printf("inside scheduler\n");
  struct proc *p;
  struct thread *th;
  struct cpu *c = mycpu(); //sjo==hould
  

  c->proc = 0;
  for(;;){
    // Avoid deadlock by ensuring that devices can interrupt.
    intr_on();

    for(p = proc; p < &proc[NPROC]; p++) {
      //printf("in for 1\n");

      //acquire(&p->lock);
      //printf("in for 2\n");
      if(p->state == USED) { //THREAD USED instead od RUNNABLE
        // Switch to chosen process.  It is the process's job
        // to release its lock and then reacquire it
        // before jumping back to us.
        //printf("pState == USED\n");
        for(th=p->threads_Table; th<&p->threads_Table[NTHREAD]; th++){
          acquire(&th->t_lock);
          if(th->tstate == TRUNNABLE){
            //printf("tState runnable\n");
            th->tstate = TRUNNING;
            c->proc = p;
            c->thread= th;
            //printf("trapframe: %d\n", (uint64)th->trapframe);
            //printf("calling to swtch\n");
            swtch(&c->context, &th->context);
            //printf("right after swtch\n");
            // Process is done running for now.
            // It should have changed its p->state before coming back.
            c->proc = 0;
            c->thread = 0;
          }
          release(&th->t_lock);
        }
      }
      //release(&p->lock);
   }
  }
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  //debug
  //printf("inside sched, nnof: %d\n", mycpu()->noff);
  //printf("inside sched\n");
  int intena;
  struct thread *th = mythread();
  if(!holding(&th->t_lock))
    panic("sched th->t_lock");
  if(mycpu()->noff != 1)
    panic("sched locks");
  if(th->tstate == TRUNNING)
    panic("sched running");
  if(intr_get())
    panic("sched interruptible");

  intena = mycpu()->intena;
  //swtch(&p->context, &mycpu()->context);
  swtch(&th->context, &mycpu()->context);  //thread
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  //printf("inside yield\n");
  //struct proc *p = myproc();
  struct thread *th = mythread();
  acquire(&th->t_lock);
  th->tstate = TRUNNABLE;
  sched();
  release(&th->t_lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch to forkret.
void
forkret(void)
{
  //printf("inside forkret\n");
  static int first = 1;

  // Still holding p->lock from scheduler.
  release(&mythread()->t_lock);
  //release(&mythread()->t_lock);


  if (first) {
    // File system initialization must be run in the context of a
    // regular process (e.g., because it calls sleep), and thus cannot
    // be run from main().
    first = 0;
    fsinit(ROOTDEV);
  }

  usertrapret();
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  //printf("inside sleep\n");
   //struct proc *p = myproc(); //THREAD
  struct thread *th = mythread(); //THREAD
  
  // Must acquire p->lock in order to
  // change p->state and then call sched.
  // Once we hold p->lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup locks p->lock),
  // so it's okay to release lk.

  //acquire(&p->lock);  //DOC: sleeplock1 //THREAD
  acquire(&th->t_lock);  //DOC: sleeplock1
  release(lk);

  // Go to sleep.
  th->chan = chan;       //THREAD
  th->tstate = TSLEEPING; //THREAD

  sched();

  // Tidy up.
  th->chan = 0; //THREAD

  // Reacquire original lock.
  release(&th->t_lock); //THREAD
  acquire(lk);
}

// Wake up all processes sleeping on chan.
// Must be called without any p->lock.
void
wakeup(void *chan)
{
  //printf("inside wakeup\n");
  struct proc *p;
  struct thread *th;
  
  for(p = proc; p < &proc[NPROC]; p++) {
    if(p != myproc()){
      //acquire(&p->lock);
      for(th=p->threads_Table; th<&p->threads_Table[NTHREAD]; th++){ //thats how to write the for?
        //acquire(&th->t_lock);
        if(th->tstate == TSLEEPING && th->chan == chan) {
          th->tstate = TRUNNABLE;
        }
        //release(&th->t_lock);
      }
      //release(&p->lock);
    }
  }
  //printf("finished wakeup\n");
}

// Kill the process with the given pid.
// The victim won't exit until it tries to return
// to user space (see usertrap() in trap.c).
int
kill(int pid, int signum)
{
  printf("inside kill\n");
  struct proc *p;
  uint op = 1 << signum;

  if(signum < 0 || signum > 31)
    return -1;

  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->pid == pid){
      //debug
      //printf("%d: got %d\n",p->pid,signum);
      //printf("inside kill ! i: %d, process pid: %d, signal_handler[i]: %d\n",i, p->pid, p->signal_handlers[i]);

      if(p->killed || p->state==ZOMBIE || p->state==UNUSED){
        release(&p->lock); //TODO ??
        return -1;
      }

      p->pending_signals = p->pending_signals|op;
      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

uint
sigprocmask(uint mask)
{
  struct proc *p = myproc();
  uint old_mask = p->signals_mask;
  p->signals_mask = mask & ~(1 << SIGKILL | 1 << SIGSTOP);
  return old_mask;
}

int
sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
  //debug
  printf("act: %d\n", act);
  struct proc *p = myproc();

  if(signum < 0 || signum > 31)
    return -1;

  if(signum == SIGKILL || signum == SIGSTOP)
    return -1;

  if(act == 0) 
    return -1;

  //struct sigaction *var;

  if(act == (void*)SIG_DFL) {
    //debug
    printf("inside SIG_DFL case\n");
    if( oldact != 0)
      memmove(&oldact, &p->signal_handlers[signum], sizeof(void*));
      //copyout(p->pagetable, (uint64)&oldact, (char*)&p->signal_handlers[signum], sizeof(struct sigaction));
    p->signal_handlers[signum] = &act;
  }
  else {
    //debug
    printf("user handler case\n");
    if (oldact != 0) {
      //debut
      printf("memmove to oldact\n");
      memmove(&oldact, &p->signal_handlers[signum], sizeof(void*));
    }
    //copyout(p->pagetable, (uint64)&oldact, (char*)&p->signal_handlers[signum], sizeof(struct sigaction));
    //debug
    //printf("after copyout\n");
    memmove(&p->signal_handlers[signum], &act, sizeof(void*));
    //copyin(p->pagetable, (char*)&p->signal_handlers[signum], (uint64)&act, sizeof(struct sigaction));
    //debug
    //printf("after copyin\n");
  }
  //debug
  //printf("returning 0\n");
  return 0;
}

void
sigret(void)
{
  //debug
  //printf("sigret!!!\n");
  //printf("inside sigret!!!!!!\n");
  struct proc *p = myproc();
  struct thread *th= mythread();
  acquire(&p->lock);
  acquire(&th->t_lock);
  //copyin(p->pagetable, (char*)p->trapframe, (uint64)p->user_tf_backup, sizeof(struct trapframe));
  copyin(p->pagetable, (char*)th->trapframe, (uint64)th->user_tf_backup, sizeof(struct trapframe));

  p->signals_mask = p->signals_mask_backup;
  th->user_tf_backup = 0;
  p->signal_handling = 0;
  release(&th->t_lock);
  release(&p->lock);
  //debug
  printf("end of sigret func\n");
  //return p->trapframe->eax; // ?????
}

// Copy to either a user address, or kernel address,
// depending on usr_dst.
// Returns 0 on success, -1 on error.
int
either_copyout(int user_dst, uint64 dst, void *src, uint64 len)
{
  struct proc *p = myproc();
  if(user_dst){
    return copyout(p->pagetable, dst, src, len);
  } else {
    memmove((char *)dst, src, len);
    return 0;
  }
}

// Copy from either a user address, or kernel address,
// depending on usr_src.
// Returns 0 on success, -1 on error.
int
either_copyin(void *dst, int user_src, uint64 src, uint64 len)
{
  struct proc *p = myproc();
  if(user_src){
    return copyin(p->pagetable, dst, src, len);
  } else {
    memmove(dst, (char*)src, len);
    return 0;
  }
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  //printf("inside procdump\n");
  static char *states[] = {
  [UNUSED]    "unused",
  //[SLEEPING]  "sleep ",
  //[RUNNABLE]  "runble",
  //[RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  struct proc *p;
  char *state;

  printf("\n");
  for(p = proc; p < &proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    printf("%d %s %s", p->pid, state, p->name);
    printf("\n");
  }
}

void 
sigcont_func(void)
{
  struct proc *p = myproc();
  //acquire(&p->lock);
  p->stopped = 0;
  //release(&p->lock);
}

void
sigstop_func(void)
{
  struct proc *p = myproc();
  //acquire(&p->lock);
  p->stopped = 1;
  //release(&p->lock);
}

void
sigkill_func(void)
{
  struct proc *p = myproc();
  struct thread *th;
  acquire(&p->lock);
  p->killed = 1;
  for(th=p->threads_Table; th<&p->threads_Table[NTHREAD]; th++){ //thats how to write the for?
      acquire(&th->t_lock);
      th->killed = 1;
      if(th->tstate == TSLEEPING){
        th->tstate = TRUNNABLE;
      }
      release(&th->t_lock);
  }
  release(&p->lock);
}

int
is_pending_and_not_masked(int signum) {
  struct proc *p = myproc();

  uint64 non_maskable = ~(1 << SIGKILL) & ~(1 << SIGSTOP);
  
  if((p->pending_signals & (1 << signum)) && !((p->signals_mask & non_maskable) & (1 << signum)))
    return 1;
  
  else
    return 0;
  
}


void
turnoff_sigbit(struct proc *p, int i) {
  
  //int op = 1 << i;   UNCOMMENT LATER
  //uint pending_sigs = p->pending_signals;   UNCOMMENT LATER
  acquire(&p->lock);
  p->pending_signals = p->pending_signals & ~(1 << i);
  release(&p->lock);
  //TO ADD - after implemenation of CAS
  /*
  while(!cas(&p->pending_signals, pending_sigs, pending_sigs&op)){
    pending_sigs = p->pending_signals;
  }
  */
}
    //p->pending_signals = p->pending_signals & ~(1 << i); 


int
search_cont_signals(void) {
  struct proc *p = myproc();

  if(is_pending_and_not_masked(SIGCONT)) {
    sigcont_func();
    turnoff_sigbit(p, SIGCONT);
    return 1;
  }

  if(is_pending_and_not_masked(SIGKILL)) {
    sigkill_func();
    turnoff_sigbit(p, SIGKILL);
    return 1;
  }

  for(int i = 0; i < 32; i++) {
    if(is_pending_and_not_masked(i)) {
      if(p->signal_handlers[i] == (void*)SIGCONT){
        sigcont_func();
        turnoff_sigbit(p, i);
        return 1;
      }
    }
  }

  return 0;
}

void
signalhandler(void)
{
  struct proc *p = myproc();
  struct thread *th = mythread();
  
  if(p == 0)
    return;
  
  if(p->signal_handling)
    return;
  

  //make sure its not a kernel trap???????

  for(int i = 0; i < 32; i++) {
    //printf("i: %d, process pid: %d, signal_handler[i]: %d\n",i, p->pid, p->signal_handlers[i]);

    //int signal_ptr= 1 << i;
    //check p->killed????
    //if(p->killed)
    //  return;
    if(th->killed){
      return;
    }
    while(p->stopped) {
      if(search_cont_signals())
      {
        break;
      }
      //yield CPU back to the scheduler
      yield();
    }

    
    if(is_pending_and_not_masked(i))
    {
      // printf("process pid: %d, signal_handler[i]: %d\n", p->pid, p->signal_handlers[i]);
      //printf("%d: handeling %d\n",p->pid,i);
      //printf("signum: %d, in signalhandler, signal_handling is: %d\n", i, p->signal_handling);
      //debug
      if(p->signal_handling)
        return;
      //printf("handler: %d\n", p->signal_handlers[i]);
      //debug
      printf("handle signal: %d\n", i);

      if(p->signal_handlers[i] == (void*)SIG_IGN){
        continue;
      }
      //kernel space handler
      else if(p->signal_handlers[i] == (void*)SIG_DFL)
      { 
      //debug
      // printf("inside kernel space handler\n");
      
      switch (i){
        case SIGSTOP:
          sigstop_func();
          break;
        
        case SIGCONT:
          sigcont_func();
          break;

        case SIGKILL:
          sigkill_func();
          break;
        
        default:
          sigkill_func();
          break;       
        }
      }

      else if(p->signal_handlers[i] == (void*)SIGSTOP){
        sigstop_func();
      }

      else if(p->signal_handlers[i] == (void*)SIGCONT){
        sigcont_func();
      }

      else if(p->signal_handlers[i] == (void*)SIGKILL){
        sigkill_func();
      }

      //User signal handler
      else{ 
        //debug
        // printf("\ncalling to usersignalshandler!\n");
        usersignalhandler(p, th ,i);
      }    
       
    turnoff_sigbit(p, i);

    }
    //p->pending_signals = p->pending_signals & ~(1 << i); 
  }
  
  //release(&p->lock);

}

void
usersignalhandler(struct proc *p,  struct thread *th, int signum) {
  printf("inside signaluserhandler\n");
  //acquire(&p->lock);
  acquire(&th->t_lock);

  //debug
  printf("user handeling %d\n",signum);
  //1
    //debug
  //printf("on stage 1\n");
  uint64 dst; 
  struct sigaction *handler = (struct sigaction*) p->signal_handlers[signum];
  copyin(p->pagetable, (char*)&dst, (uint64)&handler->sa_handler, sizeof(void*));
  
  //2
  //debug
  //printf("on stage 2\n");
  //backup mask
  uint sigmask;
  copyin(p->pagetable, (char*)&sigmask, (uint64)&handler->sigmask, sizeof(uint));
  uint backup_mask = sigprocmask(sigmask);
  p->signals_mask_backup = backup_mask;
  
  //3
  //debug
  //printf("on stage 3\n");
  //turn on flag
  //acquire(&p->lock);
  //debug
  //printf("signum: %d, in userhandler, signal_handling is: %d\n", signum, p->signal_handling);
  p->signal_handling = 1;
  //printf("signum: %d, in userhandler, signal_handling is: %d\n", signum, p->signal_handling);

  //release(&p->lock);
 
  //4
    //debug
  //printf("on stage 4\n");
  //reduce the process trapframe stack pointer by the size of trapframe
  uint64 sp_n = th->trapframe->sp - sizeof(struct trapframe);
  th->user_tf_backup = (struct trapframe*)sp_n;
   
  //5 
    //debug
  //printf("on stage 5\n");
  //backup the process trap frame 
  copyout(p->pagetable, (uint64)th->user_tf_backup, (char*)th->trapframe, sizeof(struct trapframe));
  
  //6    
  //debug
  //printf("on stage 6\n");
  th->trapframe->epc = (uint64)dst;
  
  //7
    //debug
  //printf("on stage 7\n");
  int func_size = (endcallsigret - callsigret);
  sp_n -= func_size;
  //reduce the trapframe stack pointer by func size
  th->trapframe->sp = sp_n;
  
  //8
    //debug
  // printf("on stage 8\n");
  //copy call_sigret to the process trapframe stack pointer 
  copyout(p->pagetable, (uint64)(th->trapframe->sp), (char*)&callsigret, func_size); // second arg?????
  
  //9
    //debug
  // printf("on stage 9\n");
  //debug
  //printf("inside user handler, signum: %d\n", signum);
  th->trapframe->a0 = signum;
  //put at the process return address register the new trapframe sp
  //debug
  th->trapframe->ra = sp_n;
  //p->signals_mask = backup_mask;
  //p->signal_handling = 1;
  //debug
  // printf("after last stage\n");
  release(&th->t_lock);
  //release(&p->lock);
}
int
kthread_join(int thread_id, int* status){
  printf("inside kthread_join\n");
  struct thread *thisth = mythread();
  struct proc *p = myproc();
  struct thread *th;
  acquire(&wait_lock);
  if(thisth->tid == thread_id){
    return -1;
  }
  //acquire(&p->lock);
  for(th=p->threads_Table; th<&p->threads_Table[NTHREAD]; th++){ //thats how to write the for?
    if(th == thisth || thread_id != th->tid) {
      continue;
    }
    //acquire(&th->t_lock);
    if(th->tstate == TZOMBIE){
      if(status != 0 && copyout(p->pagetable, (uint64)status, (char *)&p->xstate,
                                  sizeof(p->xstate)) < 0){
          freethread(th);
          //release(&th->t_lock);
          release(&wait_lock);
          //release(&p->lock);
          return -1;
      }
      else {
        freethread(th);
        //release(&th->t_lock);
        release(&wait_lock);
        //release(&p->lock);
        return 0;
      }
    }
    if (th->tstate == TUNUSED){
      //release(&th->t_lock);
      release(&wait_lock);
      //release(&p->lock);
      return 0; 
    }
    else{
      sleep(th,&wait_lock);
       if(status != 0 && copyout(p->pagetable, (uint64)status, (char *)&p->xstate,
                                  sizeof(p->xstate)) < 0){
        freethread(th);
        //release(&th->t_lock);
        release(&wait_lock);
        //release(&p->lock);
        return -1;
      }
      else{
        freethread(th);
        //release(&th->t_lock);
        release(&wait_lock);
        //release(&p->lock);
        return 0;
      }
    }
  }
  //release (&thisth->t_lock);
  release(&wait_lock);
  //release(&p->lock);
  return -1;
}
void
wakethreads(void *chan){
  printf("inside wakethreads\n");
  //struct thread *thisth = mythread();
  struct proc *p = myproc();
  struct thread *th;
  acquire(&p->lock);
  for(th = p->threads_Table; th < &p->threads_Table[NTHREAD]; th++){
    acquire(&th->t_lock);
    if(th->tstate == TSLEEPING && th->chan == chan){
      th->tstate = TRUNNABLE;
    }
    release(&th->t_lock);
  }
  release(&p->lock);
}

void kthread_exit(int status){
  printf("inside kthread_exit\n");
  struct thread *th;
  struct thread *thisth= mythread();
  struct proc *p = myproc();
  //acquire(&thisth->t_lock);
  thisth->tstate = TZOMBIE;
  //release(thisth->t_lock);
  int hasThreadsInside = 0;
  acquire(&p->lock);
  for(th=p->threads_Table; th<&p->threads_Table[NTHREAD]; th++){ //thats how to write the for?
    acquire(&th->t_lock);
    if(th->tstate != TUNUSED && th->tstate != TZOMBIE){
      hasThreadsInside=1;
    }
    release(&th->t_lock);
  }
  release(&p->lock);
  wakethreads(thisth);
  if(hasThreadsInside == 0){
    //release(&thisth->t_lock);
    //debug
    printf("calling exit");
    exit(status); //status?
  }
  sched();
}

int
kthread_create(void (*start_func)(),void *stack) {
  printf("inside kthread_create\n");
  struct thread *th;
  struct thread *thisth = mythread();
  struct proc *p = myproc();
  //debug
  printf("create1\n");
  acquire(&p->lock);
  if((th = allocthread(p)) == 0) {
    release(&p->lock);
    return -1;
  }
  //debug
  printf("create2\n");
  memmove(th->trapframe, thisth->trapframe, sizeof(struct trapframe));
  th->tstate = TRUNNABLE;
  th->trapframe->sp = (uint64)(stack + MAX_STACK_SIZE -16);
  th->trapframe->epc = (uint64) start_func;
  //debug
  printf("start_func adder: %p\n", start_func);
  release(&p->lock);

  //debug
  printf("inside kthread_create, tid: %d\n", th->tid);
  return th->tid; 
}

int
kthread_id() {
  struct thread *t = mythread();
  return t->tid;
}