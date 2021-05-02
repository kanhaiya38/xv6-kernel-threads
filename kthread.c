#include "types.h"
#include "mmu.h"
#include "user.h"
#include "clone.h"

int kthread_create(kthread_t *thread, void (*start_routine)(void *, void *),
                   void *arg1, void *arg2) {
  if((thread->stack = malloc(PGSIZE)) < 0) {
      printf(2, "malloc failed\n");
      return -1;
  }

  if((thread->tid = clone(start_routine, arg1, arg2, thread->stack + PGSIZE,
          CLONE_THREAD | CLONE_FILES)) < 0) {
    free(thread->stack);
    printf(2, "malloc failed\n");
    return -1;
  }

  return 0;
}

int kthread_join(kthread_t *thread)
{
  if(join(thread->tid) != thread->tid) {
    printf(2, "join failed\n");
    free(thread->stack);
    return -1;
  }
  free(thread->stack);
  return 0;
}

int kthread_kill(kthread_t *thread)
{
  if(tkill(thread->tid) < 0) {
    printf(2, "tkill failed\n");
    free(thread->stack);
    return -1;
  }
  free(thread->stack);
  return 0;
}

void kthread_exit()
{
  exit();
}
