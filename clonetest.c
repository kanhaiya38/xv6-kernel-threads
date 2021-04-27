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
  // printf(1, "sleeping...\n");
  // sleep(10000);
  // printf(1, "awake...\n");
  exit();
}

void
clonetest(void)
{
  int a = 1;
  int b = 2;
  void *stack = malloc(4096);
  uint flags = 1;
  printf(1, "fn is %p\n", thread_func);
  printf(1, "arg1 is %p\n", &a);
  printf(1, "arg2 is %p\n", &b);
  printf(1, "stack is %p\n", stack);
  printf(1, "flags is %p\n", flags);
  clone(&thread_func, (void *)&a, (void *)&b, stack+4096, flags);
  sleep(1000);
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
