#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "clone.h"

#define N  0
char *echoargv[] = { "echo", "ALL", "TESTS", "PASSED", 0 };
int a, b;
void *stack, *stack1, *stack2;
int tid, tid1, tid2;

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
  printf(1, "basic clone test\n");
  a = 1;
  b = 2;
  stack = malloc(4096);
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
    printf(1, "basic clone test FAILED\n");
    exit();
  }

  printf(1, "arg1 is %d\n", a);
  printf(1, "arg2 is %d\n", b);
  printf(1, "basic clone test OK\n");
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
  a = 1;
  b = 2;
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
  if(join(tid1) < 0) {
    printf(1, "join failed\n");
    exit();
  }
  if(join(tid2) < 0) {
    printf(1, "join failed\n");
    exit();
  }

  printf(1, "clone child fork test OK\n");
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
  int n;
  stack = malloc(4096);
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

void
clone_files_test_func(void *a, void *b)
{
  int *fd = (int *)a;
  if (write(*fd, "test", 4) != 4) {
    printf(1, "write failed\n");
  }
  // close(*fd);
  exit();
}

void
clone_files_test(void)
{
  printf(1, "clone files test\n");
  int fd;
  char str[10];
  stack = malloc(4096);

  fd = open("clone_files_test.txt", O_CREATE | O_RDWR);
  if(fd < 0) {
    printf(1, "open failed\n");
    exit();
  }

  tid = clone(&clone_files_test_func, (void *)&fd, (void *)&b, stack+4096, CLONE_THREAD | CLONE_FILES);
  if(tid < 0) {
    printf(1, "clone failed\n");
    close(fd);
    exit();
  }
  if(join(tid)< 0) {
    printf(1, "join failed\n");
    close(fd);
    exit();
  }

  if(write(fd, "file", 4) != 4) {
    printf(1, "write failed\n");
    close(fd);
    exit();
  }
  close(fd);

  fd = open("clone_files_test.txt", O_RDONLY);
  if(fd < 0) {
    printf(1, "open failed\n");
    exit();
  }

  if(read(fd, str, 8) != 8 || strcmp(str, "testfile")) {
    printf(1, "clone files test FAILED\n");
    close(fd);
    exit();
  }
  close(fd);
  if(unlink("clone_files_test.txt") < 0){
    printf(1, "unlink failed\n");
    exit();
  }

  printf(1, "clone files test OK\n");
}

void
clone_fs_test_func(void *a, void *b)
{
  if(mkdir("test") < 0) {
    printf(1, "mkdir failed\n");
    exit();
  }
  if(chdir("test") < 0) {
    printf(1, "chdir failed\n");
    exit();
  }
  exit();
}

void
clone_fs_test(void)
{
  printf(1, "clone fs test\n");
  stack = malloc(4096);

  tid = clone(&clone_fs_test_func, (void *)&a, (void *)&b, stack+4096, CLONE_THREAD | CLONE_FS);
  if(tid < 0) {
    printf(1, "clone failed\n");
    exit();
  }
  if(join(tid)< 0) {
    printf(1, "join failed\n");
    exit();
  }
  if (chdir("../") < 0){
    printf(1, "clone fs test FAILED\n");
    exit();
  }
  if (unlink("test") < 0){
    printf(1, "unlink failed\n");
    exit();
  }
  printf(1, "clone fs test OK\n");
}

int glob = 5;

void
clone_vm_test_func(void *a, void *b)
{
  glob++;
  exit();
}

void
clone_vm_test()
{
  printf(1, "clone vm test\n");
  stack = malloc(4096);

  tid = clone(&clone_vm_test_func, (void *)&a, (void *)&b, stack+4096, CLONE_THREAD | CLONE_VM);
  if(tid < 0) {
    printf(1, "clone failed\n");
    exit();
  }
  if(join(tid)< 0) {
    printf(1, "join failed\n");
    exit();
  }
  if(glob != 6) {
    printf(1, "clone vm test FAILED\n");
    exit();
  }
  printf(1, "clone vm test OK\n");
}

void tkill_test_func(void *a, void *b)
{
  sleep(10);
  int pid;
  pid = fork();
  if(pid < 0){
    printf(1, "fork failed\n");
    exit();
  } 
  printf(1, "tkill test FAILED\n");
  exit();
}

void
tkill_test()
{
  printf(1, "tkill test\n");
  tid = clone(&tkill_test_func, (void *)&a, (void *)&b, stack+4096, CLONE_THREAD);
  if(tid < 0) {
    printf(1, "clone failed\n");
    exit();
  }
  tkill(tid);
  if(join(tid) < 0) {
    printf(1, "join failed\n");
    exit();
  }
  printf(1, "tkill test OK\n");
}

void
gettid_test_func(void *a, void *b)
{
  *(int *)a = gettid();
  exit();
}

void
gettid_test()
{
  printf(1, "gettid test\n");
  tid = clone(&gettid_test_func, (void *)&a, (void *)&b, stack+4096, CLONE_THREAD | CLONE_VM);
  if(tid < 0) {
    printf(1, "clone failed\n");
    exit();
  }
  // sleep(1000);
  if(join(tid) < 0) {
    printf(1, "join failed\n");
    exit();
  }
  if(tid != a) {
    printf(1, "gettid test FAILED\n");
    exit();
  }
  printf(1, "gettid test OK\n");
}

int
main(void)
{
  // clonetest();
  // child_fork_test();
  // child_exec_test();
  // clone_vm_test();
  gettid_test();
  // tkill_test();
  // clone_fs_test();
  // clone_files_test();
  exit();
}
