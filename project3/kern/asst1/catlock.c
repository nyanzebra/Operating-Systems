/*
 * catlock.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use LOCKS/CV'S to solve the cat syncronization problem in 
 * this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>


/*
 * 
 * Constants
 *
 */

/*
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6

/*
 * Number of mice.
 */

#define NMICE 2

// Define max number of times an animal can eat
#define MAX_MEALS 3

struct lock* lock_catmutex;
struct lock* lock_mousemutex;
struct lock* lock_bowlmutex;
unsigned int n_mice = 0; //number of mice eating
unsigned int n_cats = 0; //number of cats eating
unsigned int n_bowl = 0;//bowls in use
unsigned int n_catseat = 0;//total # of cats who have eaten
unsigned int n_mouseeat = 0;//total # of mice who have eaten

/*
 * 
 * Function Definitions
 * 
 */


/*
 * catlock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS -
 *      1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static void catlock(void * unusedpointer, unsigned long catnumber) {
    if (n_catseat > 0 && n_catseat % 3 == 0 && n_mouseeat < NMICE) {
        lock_acquire(lock_catmutex);
    }
    if (n_bowl >= NFOODBOWLS - 1) {
        lock_acquire(lock_bowlmutex);
    }
    lock_acquire(lock_catmutex);//lock
    n_cats++;//new cat
    if (n_cats == 1) {//tell no mice to go
        lock_acquire(lock_mousemutex);
    }
    lock_release(lock_catmutex);//release
    n_bowl++;// bowl in use
    n_catseat++;//keep track of how many have eaten
    kprintf("Cat %d is eating at bowl %d\n", catnumber, n_bowl);
    clocksleep(4);
    kprintf("Cat %d finished eating at bowl %d\n", catnumber, n_bowl);
    n_bowl--;//bowl free
    if (n_bowl < NFOODBOWLS) {
        lock_release(lock_bowlmutex);
    }
    lock_acquire(lock_catmutex);//lock
    n_cats--;//cat leaves
    if (n_cats == 0) {// signal the mice
        lock_release(lock_mousemutex);
    }
    lock_release(lock_catmutex);//release
}
        

/*
 * mouselock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static void mouselock(void * unusedpointer,unsigned long mousenumber) {  
    if (n_bowl == NFOODBOWLS) {
        lock_acquire(lock_bowlmutex);
    }
    lock_acquire(lock_mousemutex);//lock
    n_mice++;//mouse has come
    if (n_mice == 1) {//lock cats
        lock_acquire(lock_catmutex);
    }
    lock_release(lock_mousemutex);//release
    //lock_release(sem_nomicewaiting);//mouse is not waiting
    n_bowl++;//bowl number
    n_mouseeat++;//mouse has eaten
    kprintf("Mouse %d is eating at bowl %d\n", mousenumber, n_bowl);
    clocksleep(4);
    kprintf("Mouse %d finished eating at bowl%d\n", mousenumber, n_bowl);
    n_bowl--;
    if (n_bowl < NFOODBOWLS) {
        lock_release(lock_bowlmutex);//dish free
    }
    lock_acquire(lock_mousemutex);//lock
    n_mice--;//mouse leaves
    if (n_mice == 0) {//all mice are gone
        lock_release(lock_catmutex);
    }
    lock_release(lock_mousemutex);//release
    if (n_mouseeat == NMICE) {//all mice have eaten so cats can go nuts...
        lock_release(lock_catmutex);
    }
}


/*
 * catmouselock()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catlock() and mouselock() threads.  Change
 *      this code as necessary for your solution.
 */

int
catmouselock(int nargs, char ** args) {       
    lock_catmutex = lock_create("catmutex");
    lock_mousemutex = lock_create("mousemutex");
    lock_bowlmutex = lock_create("bowlmutex");     
    int index, error;
 
    /*
     * Avoid unused variable warnings.
     */

    (void) nargs;
    (void) args;

    /*
     * Start NCATS catlock() threads.
     */

    for (index = 0; index < NCATS; index++) {
         
        error = thread_fork("catlock thread",NULL,index,catlock,NULL);
     
        /*
         * panic() on error.
         */

        if (error) {
            panic("catlock: thread_fork failed: %s\n",strerror(error));
        }
    }

    /*
     * Start NMICE mouselock() threads.
     */

    for (index = 0; index < NMICE; index++) {

        error = thread_fork("mouselock thread",NULL,index,mouselock,NULL);

        /*
         * panic() on error.
         */

        if (error) {
            panic("mouselock: thread_fork failed: %s\n", strerror(error));
        }
    }

    return 0;
}

/*
 * End of catlock.c
 */