#include "types.h"
#include "stat.h"
#include "user.h"
#include "clone.h"

#define N  0
char *echoargv[] = { "echo", "ALL", "TESTS", "PASSED", 0 };

void thread_func(void *a, void *b) {
  int *one = (int *)a;
  int *two = (int *)b;
  *one = *two + *one;
  *two = *one - *two;
  *one = *one - *two;
  exit();
}

void
clonetest(void)
{
  int a = 1;
  int b = 2;
  void *stack = malloc(4096);
  int tid;
  printf(1, "arg1 is %d\n", a);
  printf(1, "arg2 is %d\n", b);
  tid = clone(&thread_func, (void *)&a, (void *)&b, stack+4096, CLONE_THREAD);
  if(tid < 0) {
    printf(1, "clone failed\n");
    exit();
  }
  // sleep(1000);
  if(join(tid)< 0) {
    printf(1, "join failed\n");
    exit();
  }

  printf(1, "arg1 is %d\n", a);
  printf(1, "arg2 is %d\n", b);
  exit();
}

void
child_fork_test_func1(void *a, void *b) {
  int pid;

  pid = fork();
  if(pid < 0){
    printf(1, "fork failed\n");
    exit();
  } 
  if(pid != 0) *(int *)a = pid;
  // printf(1, "child is %d\n", pid);
  // if(pid == 0) printf(1, "i am child\n");
  // else printf(1, "i am parent\n");
  exit();
}

void
child_fork_test_func2(void *a, void *b) {
  if(wait() < 0) 
    printf(1, "clone child fork test FAILED\n");
  else
    printf(1, "clone child fork test OK\n");
  exit();
}

void
child_fork_test(void)
{
  printf(1, "clone child fork test\n");
  int a = 1;
  int b = 2;
  int tid1, tid2;
  void *stack1 = malloc(4096), *stack2 = malloc(4096);

  tid1 = clone(&child_fork_test_func1, (void *)&a, (void *)&b, stack1+4096, CLONE_THREAD);
  if(tid1 < 0) {
    printf(1, "clone failed\n");
    exit();
  }
  sleep(100);
  tid2 = clone(&child_fork_test_func2, (void *)&a, (void *)&b, stack2+4096, CLONE_THREAD);
  if(tid2 < 0) {
    printf(1, "clone failed\n");
    exit();
  }

  // sleep(1000);
  if(join(tid1)< 0) {
    printf(1, "join failed\n");
    exit();
  }
  if(join(tid2)< 0) {
    printf(1, "join failed\n");
    exit();
  }

  // if()

  printf(1, "clone child fork test OK\n");
  exit();
}

void child_exec_test_func(void *a, void *b) {
  // sleep(1000);
  if(exec("echo", echoargv) < 0){
    printf(1, "exec failed\n");
    exit();
  }
  printf(1, "Child exec test FAILED\n");
  exit();
}

void
child_exec_test(void) {
  printf(1, "child exec test\n");
  int a = 1;
  int b = 2;
  int n;
  void *stack = malloc(4096);
  int tid;
  printf(1, "arg1 is %d\n", a);
  printf(1, "arg2 is %d\n", b);
  tid = clone(&child_exec_test_func, (void *)&a, (void *)&b, stack+4096, CLONE_THREAD);
  if(tid < 0) {
    printf(1, "clone failed\n");
    exit();
  }
  int tids[N];
  for(n=0; n<N; n++){
    tids[n] = clone(&thread_func, (void *)&a, (void *)&b, stack+4096, CLONE_THREAD);
    if(tids[n] < 0) {
      printf(1, "clone failed\n");
      exit();
    }
  }
  // sleep(1000);
  if(join(tid)< 0) {
    printf(1, "join failed\n");
    exit();
  }
  for(n=0; n<N; n++){
    if(join(tids[n]) < 0) {
      printf(1, "join failed\n");
      exit();
    }
  }

  printf(1, "child exec test FAILED\n");
  exit();
}

int
main(void)
{
  // clonetest();
  // child_fork_test();
  child_exec_test();
  exit();
}
