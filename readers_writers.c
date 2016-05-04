/* Reader/Writer implementation with writer priority
 * using condition variables 
 * Author: Sherri Goings
 * Last Modified: 5/1/2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

void* Reader(void*);
void* Writer(void*);

// initialize lock and 2 condition vars
pthread_mutex_t lock;
pthread_cond_t okToRead;
pthread_cond_t okToWrite;

// keep track of how many readers/writers are both active and waiting
int activeReaders = 0;
int activeWriters = 0;
int waitingReaders = 0;
int waitingWriters = 0;

int main() {
    /* Initialize mutex and condition variable objects */
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&okToRead, NULL);
    pthread_cond_init(&okToWrite, NULL);
 
    // create threads for testing here, R readers & W writers
    // you will want to mix up the order of creation for testing!
    int R = 5;
    int W = 2;
    pthread_t threads[R+W];
    int i;
    for (i=0; i<R; i++) {
        long j = (long)i;
        pthread_create(&threads[i], NULL, Reader, (void*) j);
    }
    for (i=R; i<R+W; i++) {
        long j = (long)i;
        pthread_create(&threads[i], NULL, Writer, (void*) j);
    }

    // join all threads before quitting
    for (i=0; i<R+W; i++) {
        pthread_join(threads[i], NULL);    
        }     
    return 0;
}

/* Algorithm for reader
 * gives priority to writers, if any are currently writing or waiting,
 * reader will wait. If on finish read there are any waiting writers and
 * this was last active reader, wake one writer
 * args - long id of this thread
 * return 0 on exit
 */
void* Reader(void* args) {
    printf("reader %ld created\n", (long) args);
    fflush(stdout);

    // give writers priority by waiting if any are active or waiting
    pthread_mutex_lock(&lock);
    while (activeWriters + waitingWriters > 0) {
        waitingReaders++;
        pthread_cond_wait(&okToRead, &lock);
        waitingReaders--;
    }
    // if here going to read so increment activeReaders
    activeReaders++;
    pthread_mutex_unlock(&lock);

    printf("reader %ld begin reading\n", (long) args);
    fflush(stdout);
   // you should put a delay here for testing to make things more interesting
    printf("reader %ld finished reading\n", (long) args);
    fflush(stdout);

    // finish reading so decrement activeReaders, wake one writer if needed
    pthread_mutex_lock(&lock);
    activeReaders--;
    if (activeReaders==0 && waitingWriters>0) {
        pthread_cond_signal(&okToWrite);
    }
    pthread_mutex_unlock(&lock);

    return (void*) 0;
}

/* Algorithm for writer
 * writer has priority, only waits if a reader or writer is already
 * actively reading/writing.  Upon finish write, wakes one other waiting
 * writer unless there are none, only then wakes all waiting readers
 * args - long id of this thread
 * return 0 on exit
 */
void* Writer(void* args) {
    printf("writer %ld created\n", (long) args);
    fflush(stdout);

    // wait if anything else currently active
    while (activeWriters + activeReaders > 0) {
        waitingWriters++;
        pthread_cond_wait(&okToWrite, &lock);
        waitingWriters--;
    }
    // if here going to write so increment activeWriters
    activeWriters++;
    pthread_mutex_unlock(&lock);

    printf("writer %ld begin writing\n", (long) args);
    fflush(stdout);
   // you should put a delay here for testing to make things more interesting
    printf("writer %ld finished writing\n", (long) args);
    fflush(stdout);

    // done writing so decrement activeWriters, signal appropriate thread(s)
    pthread_mutex_lock(&lock);
    activeWriters--;
    if (waitingWriters>0) 
        pthread_cond_signal(&okToWrite);
    else if (waitingReaders>0)
        pthread_cond_broadcast(&okToRead);
    pthread_mutex_unlock(&lock);

    return (void*) 0;
}