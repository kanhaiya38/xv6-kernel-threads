#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "clone.h"

#define N  0
#define PGSIZE 4096
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
clone_test(void)
{
  printf(1, "basic clone test\n");
  a = 1;
  b = 2;
  stack = malloc(PGSIZE);
  printf(1, "arg1 is %d\n", a);
  printf(1, "arg2 is %d\n", b);
  tid = clone(&thread_func, (void *)&a, (void *)&b,
              stack+PGSIZE, CLONE_THREAD | CLONE_VM);
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
  free(stack);
}

void
clone_fork_test_func1(void *a, void *b) {
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
clone_fork_test_func2(void *a, void *b) {
  if(wait() < 0) 
    printf(1, "clone child fork test FAILED\n");
  else
    printf(1, "clone child fork test OK\n");
  exit();
}

void
clone_fork_test(void)
{
  printf(1, "clone child fork test\n");
  a = 1;
  b = 2;
  void *stack1 = malloc(PGSIZE), *stack2 = malloc(PGSIZE);

  tid1 = clone(&clone_fork_test_func1, (void *)&a, (void *)&b,
              stack1+PGSIZE, CLONE_THREAD);
  if(tid1 < 0) {
    printf(1, "clone failed\n");
    exit();
  }
  sleep(100);
  tid2 = clone(&clone_fork_test_func2, (void *)&a, (void *)&b, stack2+PGSIZE, CLONE_THREAD);
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

  free(stack);
  printf(1, "clone child fork test OK\n");
}

char *echoargv[] = { "echo", "ALL", "TESTS", "PASSED", 0 };

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
clone_exec_test(void) {
  printf(1, "child exec test\n");
  int n;
  stack = malloc(PGSIZE);
  tid = clone(&child_exec_test_func, (void *)&a, (void *)&b,
              stack+PGSIZE, CLONE_THREAD | CLONE_VM);
  if(tid < 0) {
    printf(1, "clone failed\n");
    exit();
  }
  int tids[N];
  for(n=0; n<N; n++){
    tids[n] = clone(&thread_func, (void *)&a, (void *)&b,
                    stack+PGSIZE, CLONE_THREAD | CLONE_VM);
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

  free(stack);
  printf(1, "child exec test FAILED\n");
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
  stack = malloc(PGSIZE);

  fd = open("clone_files_test.txt", O_CREATE | O_RDWR);
  if(fd < 0) {
    printf(1, "open failed\n");
    exit();
  }

  tid = clone(&clone_files_test_func, (void *)&fd, (void *)&b,
      stack+PGSIZE, CLONE_THREAD | CLONE_VM | CLONE_FILES);
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

  if(read(fd, str, 8) != 8 && strcmp(str, "testfile")) {
    printf(1, "str is %s\n", str);
    printf(1, "clone files test FAILED\n");
    close(fd);
    exit();
  }
  // close(fd);
  if(unlink("clone_files_test.txt") < 0){
    printf(1, "unlink failed\n");
    exit();
  }

  free(stack);
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
  stack = malloc(PGSIZE);

  tid = clone(&clone_fs_test_func, (void *)&a, (void *)&b,
              stack+PGSIZE, CLONE_THREAD | CLONE_FS);
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
  free(stack);
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
  glob = 0;
  stack = malloc(PGSIZE);

  tid = clone(&clone_vm_test_func, (void *)&a, (void *)&b,
              stack+PGSIZE, CLONE_THREAD | CLONE_VM);

  if(tid < 0) {
    printf(1, "clone failed\n");
    exit();
  }

  if(join(tid) < 0) {
    printf(1, "join failed\n");
    exit();
  }

  if(glob != 1) {
    printf(1, "clone vm test FAILED\n");
    exit();
  }

  free(stack);
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
  tid = clone(&tkill_test_func, (void *)&a, (void *)&b, stack+PGSIZE, CLONE_THREAD);
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
  tid = clone(&gettid_test_func, (void *)&a, (void *)&b,
              stack+PGSIZE, CLONE_THREAD | CLONE_VM);
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

void kthread_test_func(void *a, void *b) {
  *(int *)a = *(int *)a + 100;
  *(int *)b = *(int *)b + 100;
  kthread_exit();
}

void
kthread_test()
{
  printf(1, "kthread test\n");
  kthread_t kt;
  a = 1;
  b = 2;

  if(kthread_create(&kt, &kthread_test_func, (void *)&a, (void *)&b) < 0) {
    printf(1, "kthread_create failed\n");
    exit();
  }

  if(kthread_join(&kt) < 0) {
    printf(1, "kthread_join failed\n");
    exit();
  }

  if(a != 101 || b != 102) {
    printf(1, "basic kthread test FAILED\n");
    exit();
  }
  
  printf(1, "kthread test OK\n");
}

kthread_lock_t lock;
#define L 10
#define NL 1e3
void kthread_lock_test_func(void *a, void *b) {
  int i;
  kthread_lock(&lock);
  for(i = 0; i < NL; i++) {
    glob++;
  }
  kthread_unlock(&lock);
  kthread_exit();
}

void
kthread_lock_test()
{
  printf(1, "kthread lock test\n");
  kthread_t kts[L];
  int i;
  glob = 0;

  kthread_init_lock(&lock);

  for(i = 0; i < L; i++) {
    if(kthread_create(&kts[i], &kthread_lock_test_func, (void *)&a, (void *)&b) < 0) {
      printf(1, "kthread_create failed\n");
      exit();
    }
  }
  for(i = 0; i < L; i++) {
    if(kthread_join(&kts[i]) < 0) {
      printf(1, "kthread_join failed\n");
      exit();
    }
  }

  if(glob != L * NL) {
    printf(1, "kthread lock FAILED\n");
    exit();
  }
  
  printf(1, "kthread lock test OK\n");
}


#define MAX_CLONES 62
void
clone_stress_test()
{
  printf(1, "clone stress test\n");
  int threads[MAX_CLONES];
  void *stacks[MAX_CLONES];
  int i;

  for(i = 0; i < MAX_CLONES; i++) {
    stacks[i] = malloc(PGSIZE);
    if((threads[i] = clone(&thread_func, (void *)&a, (void *)&b,
        stacks[i], CLONE_THREAD | CLONE_VM)) < 0) {
      if(i == MAX_CLONES - 1) {
        printf(1, "max threads created\n");
        break;
      } else {
        printf(1, "clone stress test FAILED\n");
      }
      printf(1, "i is %d", i);
      printf(1, "kthread_create failed\n");
      exit();
    }
  }

  for(i = 0; i < MAX_CLONES - 1; i++) {
    if(join(threads[i]) < 0) {
      printf(1, "kthread_join failed\n");
      exit();
    }
  }

  printf(1, "clone stress test OK\n");

}

typedef struct matrix {
    int **data;
    int row, col;
} matrix;

matrix mat1, mat2, mat3;
#define MAX_THREADS 3

void printMatrix(matrix *m)
{
  int row = m->row;
  int col = m->col;
  int **data = m->data;
  int i, j;
  printf(1, "%d %d\n", row, col);
  for (i = 0; i < row; i++) {
    for (j = 0; j < col; j++) printf(1, "%d ", data[i][j]);
    printf(1, "\n");
  }
}

void mallocMatrix(matrix *m, int row, int col) {
  int **data = (int **)malloc(row * sizeof(int *));
  int i;
  // assert(data != NULL);
  for (i = 0; i < row; i++) {
    data[i] = (int *)malloc(col * sizeof(int));
    // assert(data[i] != NULL);
  }
  m->row = row;
  m->col = col;
  m->data = data;
}

void multiplyMatrixThreadsUtil(void *arg, void *nouse) {
  int t = *(int *)arg;
  int m = mat1.row;
  int n = mat2.col;
  int p = mat1.col;
  int avg_work = (m * n) / MAX_THREADS;
  int rem_work = (m * n) % MAX_THREADS;
  int op_start, op_end;
  int i, j;
  if (t == 0) {
    op_start = avg_work * t;
    op_end = (avg_work * (t + 1)) + rem_work;
  } else {
    op_start = avg_work * t + rem_work;
    op_end = (avg_work * (t + 1)) + rem_work;
  }
  // printf(1, "%d - %d, %d\n", t, op_start, op_end);
  for (i = op_start; i < op_end; i++) {
    int row = i / n;
    int col = i % n;
    mat3.data[row][col] = 0;
    for (j = 0; j < p; j++)
      mat3.data[row][col] += mat1.data[row][j] * mat2.data[j][col];
    // printf(1, "%d - %d - %d %d: %d\n", t, i, row, col, mat3.data[row][col]);
  }
  // printf(1, "complete\n");
  kthread_exit();
};

#define MAT_M 4
#define MAT_N 4

void
multiply_matrices()
{
  // multiplication
  printf(1, "matrix multiplication test\n");
  int t;
  int m, n;
  kthread_init_lock(&lock);

  mallocMatrix(&mat1, MAT_M, MAT_N);
  mallocMatrix(&mat2, MAT_M, MAT_N);
  mallocMatrix(&mat3, MAT_M, MAT_N);

  // mat1.row = MAT_M;
  // mat2.col = MAT_N;
  for(m = 0; m < MAT_M; m++) {
    for(n = 0; n < MAT_N; n++) {
      mat1.data[m][n] = m + n;
    }
  }

  // mat1.row = MAT_M;
  // mat2.col = MAT_N;
  for(m = 0; m < MAT_M; m++) {
    for(n = 0; n < MAT_N; n++) {
      mat2.data[m][n] = 0;
    }
  }
  printMatrix(&mat1);
  printMatrix(&mat2);

  kthread_t threads[MAX_THREADS];
  for (t = 0; t < MAX_THREADS; t++) {
    kthread_create(&threads[t], &multiplyMatrixThreadsUtil, (void *)&t, (void*)&a);
  }
  for (t = 0; t < MAX_THREADS; t++)
    kthread_join(&threads[t]);
  printMatrix(&mat3);
  printf(1, "matrix multiplication test OK\n");
}

int
main(void)
{
  kthread_test();
  kthread_lock_test();
  multiply_matrices();
  clone_test();
  gettid_test();
  tkill_test();
  clone_stress_test();
  clone_fork_test();
  clone_vm_test();
  clone_fs_test();
  clone_files_test();
  clone_exec_test();
  exit();
}
