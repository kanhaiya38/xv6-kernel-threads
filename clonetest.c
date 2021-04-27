#include "types.h"
#include "stat.h"
#include "user.h"

#define N  1000

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
  uint flags = 1;
  int tid;
  printf(1, "arg1 is %d\n", a);
  printf(1, "arg2 is %d\n", b);
  tid = clone(&thread_func, (void *)&a, (void *)&b, stack+4096, flags);
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

int
main(void)
{
  clonetest();
  exit();
}
