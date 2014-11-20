
#include <types.h>
#include <lib.h>
#include <types.h>
#include <pid.h>
#include <machine/spl.h>

#define PID_FREE   0 
#define PID_PARENT 1 
#define PID_EXITED 2 
#define PID_NEW    3 

struct pid_clist {
   pid_t pid;
   int status;
   struct pid_clist *next;
};

struct pid_list {
   pid_t pid;
   struct pid_list *next;
};

unsigned int unused_pids = MIN_PID;
struct pid_list *recycled_pids;
struct pid_clist *unavailable_pids;

pid_t newPid() {
   int spl = splhigh();
   if (recycled_pids == NULL) {
       assert(unused_pids < 0x7FFFFFFF); 
       struct pid_clist *new_entry = kmalloc(sizeof(struct pid_clist));
       new_entry->pid = unused_pids;
       new_entry->status = PID_NEW;
       new_entry->next = unavailable_pids;
       unavailable_pids = new_entry;
       unused_pids += 1;
       splx(spl);
       return (unused_pids - 1);
   } else {
       struct pid_list *first = recycled_pids;
       recycled_pids = recycled_pids->next;
       struct pid_clist *new_entry = kmalloc(sizeof(struct pid_clist));
       new_entry->pid = first->pid;
       new_entry->status = PID_NEW;
       new_entry->next = unavailable_pids;
       unavailable_pids = new_entry;
       kfree(first);
       splx(spl);
       return (new_entry->pid);
   }
}

void pidChangeStatus(pid_t x, int and_mask) {
   int spl = splhigh();
   assert(unavailable_pids != NULL);
   if (unavailable_pids->pid == x) {
       unavailable_pids->status &= and_mask;
       if (unavailable_pids->status == PID_FREE) {
           struct pid_list *new_entry = kmalloc(sizeof(struct pid_list));
           new_entry->pid = x;
           new_entry->next = recycled_pids;
           recycled_pids = new_entry;
           struct pid_clist *temp = unavailable_pids;
           unavailable_pids = unavailable_pids->next;
           kfree(temp);
       }
   } else {
       int found = 0;
       struct pid_clist *p = unavailable_pids;
       while (p->next != NULL) {
           if (p->next->pid == x) {
               found = 1;
               p->next->status &= and_mask;
               if (p->next->status == PID_FREE) {
                   struct pid_list *new_entry = kmalloc(sizeof(struct pid_list));
                   new_entry->pid = x;
                   new_entry->next = recycled_pids;
                   recycled_pids = new_entry;
                   struct pid_clist *temp = p->next;
                   p->next = p->next->next;
                   kfree(temp);
               }
           }
           p = p->next;
       }
       assert(found);
   }
   splx(spl);
}

void pidParentDone(pid_t x) {
   pidChangeStatus(x, PID_PARENT);
}

void pidProcessExit(pid_t x) {
   pidChangeStatus(x, PID_EXITED);
}

void pidFree(pid_t x) {
   pidChangeStatus(x, PID_FREE);
}

int pidClaimed(pid_t x) {
   int spl = splhigh();
   struct pid_clist *p;
   for (p = unavailable_pids; p != NULL; p = p->next) {
       if (p->pid == x) {
           splx(spl);
           return 1;
       }
   }
   splx(spl);
   return 0;
}
