#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <lib.h>
#include <array.h>
#include <clock.h>
#include <thread.h>
#include <synch.h>
#include <pid.h>
#include <curthread.h>

// Create lock for pid table
static struct lock* pid_lock;

// Number of allocated processes
static size_t pids;

// Next PID to be assigned
static pid_t nextpid;

// Table of PID information
static struct pidinfo *pidinfotable[MAX_PROCESSES]; 

// Create PID Struct
static struct pidinfo * createPIDInfo(pid_t pid, pid_t parent_pid)
{
    struct pidinfo *pInfo;

    assert(pid != INVALID_PID);

    pInfo = kmalloc(sizeof(struct pidinfo));
    if (pInfo == NULL) {
        return NULL;
    }

    pInfo->process_condition = cv_create("pidinfo cv");
    if (pInfo->process_condition == NULL) {
        kfree(pInfo);
        return NULL;
    }

    pInfo->t_pid = pid;
    pInfo->t_parent_pid = parent_pid;
    pInfo->isExited = 0;
    pInfo->exitStatus = -1;

    return pInfo;
}

// Remove PID Struct
static void destoryPIDInfo(struct pidinfo *pi)
{
    assert(pi->isExited == 1);
    assert(pi->t_parent_pid == INVALID_PID);
    cv_destroy(pi->process_condition);
    kfree(pi);

}

// Get pidinfo from the pid_infoTable
static struct pidinfo *getPI(pid_t pid)
{
    struct pidinfo *pi;

    assert(pid >= 0);
    assert(pid != INVALID_PID);
    assert( lock_do_i_hold(pid_lock) );

    pi = pidinfotable[pid % MAX_PROCESSES];

    if (pi == NULL) {
        return NULL;
    }
    if (pi->t_pid != pid) {
        return NULL;
    }
    return pi;
}

// Add a pidinfo to pid_infoTable
static void addPI(pid_t pid, struct pidinfo *pi)
{
    assert( lock_do_i_hold(pid_lock) );

    assert(pid != INVALID_PID);

    assert( pidinfotable[pid % MAX_PROCESSES] == NULL);

    pidinfotable[pid % MAX_PROCESSES] = pi;

    pids++;
}

// Remove pidinfo from pid_infoTable
static void removePI(pid_t pid)
{
    struct pidinfo *pi;

    assert( lock_do_i_hold(pid_lock) );

    pi = pidinfotable[pid % MAX_PROCESSES];
    assert(pi != NULL);
    assert(pi->t_pid == pid);

    destoryPIDInfo(pi);
    pidinfotable[pid % MAX_PROCESSES] = NULL;
    pids--;
}

void initializePID(void)
{
    // Create PID lock
    pid_lock = lock_create("pidlock");
    if (pid_lock == NULL) {
        panic("Could not create PID Lock\n");
    }

    // 
    pidinfotable[BOOTUP_PID] = createPIDInfo(BOOTUP_PID, INVALID_PID);
    if (pidinfotable[BOOTUP_PID]==NULL) {
        panic("Out of memory creating bootup pid data\n");
    }

    nextpid = MIN_PROCESSES;
    pids = 1;
}

int allocatePID(pid_t *retval)
{
    struct pidinfo *pi;
    pid_t pid;
    int count;

    assert(curthread->t_pid != INVALID_PID);

    /* lock the table */
    lock_acquire(pid_lock);

    if (pids == MAX_PROCESSES) {
        lock_release(pid_lock);
        return EAGAIN;
    }

    /*
     * The above test guarantees that this loop terminates, unless
     * our pids count is off. Even so, KASSERT we aren't looping
     * forever.
     */
    count = 0;
    while (pidinfotable[nextpid % MAX_PROCESSES] != NULL) {

        /* avoid various boundary cases by allowing extra loops */
        assert(count < MAX_PROCESSES*2+5);
        count++;

        nextpid++;
        if (nextpid > MAX_PROCESSES) {
                nextpid = MIN_PROCESSES;
        }       
    }

    pid = nextpid;

    pi = createPIDInfo(pid, curthread->t_pid);
    if (pi == NULL) {
        lock_release(pid_lock);
        return ENOMEM;
    }

    addPI(pid, pi);

    nextpid++;
    if (nextpid > MAX_PROCESSES) {
        nextpid = MIN_PROCESSES;
    }

    lock_release(pid_lock);

    *retval = pid;
    return 0;
}

void unallocatePID(pid_t theirpid)
{
    struct pidinfo *them;

    assert(theirpid >= MIN_PID && theirpid <= MAX_PID);

    lock_acquire(pid_lock);

    them = getPI(theirpid);
    assert(them != NULL);
    assert(them->isExited == 0);
    assert(them->t_parent_pid == curthread->t_pid);

    /* keep pidinfo_destroy from complaining */
    them->exitStatus = 0xdead;
    them->isExited = 1;
    them->t_parent_pid = INVALID_PID;

    removePI(theirpid);

    lock_release(pid_lock);
}

void setPIDexitStatus(int status)
{
    struct pidinfo *us;
    int i;

    assert(curthread->t_pid != INVALID_PID);

    lock_acquire(pid_lock);

    /* First, disown all children */
    for (i=0; i < MAX_PROCESSES; i++) {
        if (pidinfotable[i] == NULL) {
            continue;
        }
        if (pidinfotable[i]->t_parent_pid == curthread->t_pid) {
            pidinfotable[i]->t_parent_pid = INVALID_PID;
            if (pidinfotable[i]->isExited) {
                removePI(pidinfotable[i]->t_pid);
            }
        }
    }

    /* Now, wake up our parent */
    us = getPI(curthread->t_pid);
    assert(us != NULL);

    us->exitStatus = status;
    us->isExited = 1;

    if (us->t_parent_pid == INVALID_PID) {
        /* no parent */
        removePI(curthread->t_pid);
    }
    else {
        cv_broadcast(us->process_condition, pid_lock);
    }

    lock_release(pid_lock);
}

int PIDwait(pid_t targetpid, int *status, int flags, pid_t *retpid)
{
    struct pidinfo *them;

    assert(curthread->t_pid != INVALID_PID);

    /* Don't let a process wait for itself. */
    if (targetpid == curthread->t_pid) {
        return EINVAL;
    }

    /* 
     * We don't support the Unix meanings of negative pids or 0
     * (0 is INVALID_PID) and other code may break on them, so
     * check now.
     */
    if (targetpid == INVALID_PID || targetpid < 0) {
        return EINVAL;
    }

    /* Only valid options */
    if (flags != 0) {
        return EINVAL;
    }

    lock_acquire(pid_lock);

    them = getPI(targetpid);
    if (them == NULL) {
        lock_release(pid_lock);
        return ESRCH;
    }

    assert(them->t_pid == targetpid);

    /* Only allow waiting for own children. */
    if (them->t_parent_pid != curthread->t_pid) {
        lock_release(pid_lock);
        return EINVAL;
    }

    if (them->isExited == 0) {
        cv_wait(them->process_condition, pid_lock);
        assert(them->isExited == 1);
    }

    if (status != NULL) {
        *status = them->exitStatus;
    }
    if (retpid != NULL) {
        /* 
         * In Unix you can wait for any of several possible
         * processes by passing particular magic values of
         * pid. wait then returns the pid you actually
         * found. We don't support this, so always return the
         * pid we looked for.
         */
        *retpid = targetpid;
    }

    them->t_parent_pid = 0;
    removePI(them->t_pid);

    lock_release(pid_lock);
    return 0;
}
