#include "types.h"
#include "mmu.h"
#include "user.h"
#include "clone.h"

int 
kthread_create(kthread_t *thread, void (*start_routine)(void *, void *),
                   void *arg1, void *arg2) {
  if((thread->stack = malloc(PGSIZE)) < 0) {
      printf(2, "malloc failed\n");
      return -1;
  }

  if((thread->tid = clone(start_routine, arg1, arg2, thread->stack + PGSIZE,
          CLONE_THREAD | CLONE_VM)) < 0) {
    free(thread->stack);
    printf(2, "malloc failed\n");
    return -1;
  }

  return 0;
}

int 
kthread_join(kthread_t *thread)
{
  if(join(thread->tid) != thread->tid) {
    printf(2, "join failed\n");
    free(thread->stack);
    return -1;
  }
  free(thread->stack);
  return 0;
}

int 
kthread_kill(kthread_t *thread)
{
  if(tkill(thread->tid) < 0) {
    printf(2, "tkill failed\n");
    free(thread->stack);
    return -1;
  }
  free(thread->stack);
  return 0;
}

void 
kthread_exit()
{
  exit();
}

static inline uint
fetch_and_add(uint* variable, uint value)
{
  __asm__ volatile("lock; xaddl %0, %1"
                  : "+r" (value), "+m" (*variable) // input + output
                  : // No input-only
                  : "memory");

    return value;
}

void
kthread_init_lock(kthread_lock_t *lk)
{
  lk->ticket = 0;
  lk->turn   = 0;
}

void
kthread_lock(kthread_lock_t *lk)
{
  // printf(1, "locking\n");
  uint myticket = fetch_and_add(&lk->ticket, 1);

  __sync_synchronize();

  while(lk->turn != myticket)
    ;

}

void
kthread_unlock(kthread_lock_t *lk)
{
  __sync_synchronize();

  lk->turn = lk->turn + 1;
  // printf(1, "unlocking\n");
}
