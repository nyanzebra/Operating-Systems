/*
 * catsem.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use SEMAPHORES to solve the cat syncronization problem in 
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
#include <synch.h>


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

struct semaphore* sem_dish; //dish semaphore
struct semaphore* mutex_cat; // cat mutex
struct semaphore* mutex_mouse; // mouse mutex
struct semaphore* sem_ncats; // nocat semaphore
struct semaphore* sem_nmice; // nomouse semaphore
struct semaphore* sem_nomicewaiting;// priority queue sem
unsigned int nmice = 0; //number of mice eating
unsigned int ncats = 0; //number of cats eating
unsigned int nbowl = 0;//bowls in use
unsigned int ncatseat = 0;//total # of cats who have eaten
unsigned int nmouseeat = 0;//total # of mice who have eaten

/*
 * 
 * Function Definitions
 * 
 */


/*
 * catsem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static void catsem(void * unusedpointer, unsigned long catnumber) {
    P(sem_nomicewaiting);//begin priority queueing
    P(mutex_cat);//lock
    ncats++;//new cat
    if (ncats == 1) {//tell no mice to go
        P(sem_nmice);
    }
    V(mutex_cat);//release
    P(sem_dish);//begin the eating
    ncatseat++;//keep track of how many have eaten
    nbowl++;// bowl in use
    kprintf("Cat %d is eating at bowl %d\n", catnumber, nbowl);
    clocksleep(4);
    kprintf("Cat %d finished eating at bowl %d\n", catnumber, nbowl);
    nbowl--;//bowl free
    V(sem_dish);//finished eating
    P(mutex_cat);//lock
    ncats--;//cat leaves
    if (ncats == 0) {// signal the mice
        V(sem_ncats);
    }
    V(mutex_cat);//release
}
        

/*
 * mousesem()
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
 *      Write and comment this function using semaphores.
 *
 */

static void mousesem(void * unusedpointer, unsigned long mousenumber) {
    P(mutex_mouse);//lock
    nmice++;//mouse has come
    if (nmice == 1) {//lock cats
        P(sem_ncats);
    }
    V(mutex_mouse);//release
    V(sem_nomicewaiting);//mouse is not waiting
    P(sem_dish);//dish in use
    nmouseeat++;//mouse has eaten
    nbowl++;//bowl number
    kprintf("Mouse %d is eating at bowl %d\n", mousenumber, nbowl);
    clocksleep(4);
    kprintf("Mouse %d finished eating at bowl%d\n", mousenumber, nbowl);
    nbowl--;
    V(sem_dish);//dish free
    P(mutex_mouse);//lock
    nmice--;//mouse leaves
    if (nmice == 0) {//all mice are gone
        V(sem_nmice);
    }
    V(mutex_mouse);//release
    if (nmouseeat == NMICE) {//all mice have eaten so cats can go nuts...
        int i;
        for (i=ncatseat; i< NCATS; ++i) {
            V(sem_nomicewaiting);
        }
    }
}


/*
 * catmousesem()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catsem() and mousesem() threads.  Change this 
 *      code as necessary for your solution.
 */

int catmousesem(int nargs, char ** args) {
    int index, error;

    kprintf("Process catmousesem begins!\n");

    sem_dish = sem_create("sem_dish", NFOODBOWLS); //dish semaphore
    mutex_cat = sem_create("mutex_cat", 1); // cat mutex
    mutex_mouse = sem_create("mutex_cat", 1); // mouse mutex
    sem_ncats = sem_create("sem_ncats", 0); // nocat semaphore
    sem_nmice = sem_create("sem_nmice", 1); // nomouse semaphore
    sem_nomicewaiting = sem_create("sem_nomicewaiting", 3);// priority queue sem


    /*
     * Start NCATS catsem() threads.
     */

    for (index = 0; index < NCATS; index++) {
       
        error = thread_fork("catsem Thread", NULL, index, catsem, NULL);
            
        /*
         * panic() on error.
         */

        if (error) {         
            panic("catsem: thread_fork failed: %s\n", strerror(error));
        }
    }
    
    /*
     * Start NMICE mousesem() threads.
     */

    for (index = 0; index < NMICE; index++) {

        error = thread_fork("mousesem Thread", NULL, index, mousesem, NULL);
        
        /*
         * panic() on error.
         */

        if (error) {
            panic("mousesem: thread_fork failed: %s\n", strerror(error));
        }
    }

    return 0;
}


/*
 * End of catsem.c
 */