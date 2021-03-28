/**
*  @file worker.c
*  
*  @brief Implementation of the worker thread for processing payloads from
*  MQTT messages and FIFO queue for the received payloads.
* 
*  @date 22-Feb-2020
*  @copyright GNU General Public License v3
*
*  The implementation is based on the examples from the book
*
*  "Using POSIX Threads: Programming with Pthreads"
*     by Brad nichols, Dick Buttlar, Jackie Farrell
*     O'Reilly & Associates, Inc."
*
*  available at:
*
*  https://resources.oreilly.com/examples/9781565921153/
* 
*/

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>
#include <errno.h>

#include "worker.h"


/**
 * @brief Woker thread function.
 *  
 * @param[in] arguments provided to the thread when created
 * 
 * @return void pointer
 */
static void *worker_thread(void *worker_thread_arguments);


int create_worker(worker_t **worker, unsigned int working_queue_size, unsigned int working_queue_entry_size, do_work_f do_work)
{
    if (*worker)
    {
        //Worker already created/exists.
        return -1;
    }

    pthread_attr_t attr_thread;
	
    int rc;

    *worker = malloc(sizeof(worker_t));

    (*worker)->working_queue.head = 0;
    (*worker)->working_queue.number_of_entries = 0;
    (*worker)->working_queue.tail = 0;
    (*worker)->working_queue.entry_size = working_queue_entry_size;
    (*worker)->working_queue.max_queue_size = working_queue_size > MAXIMUM_WORKING_QUEUE_LENGTH ? MAXIMUM_WORKING_QUEUE_LENGTH : working_queue_size;
    (*worker)->working_queue.entry = calloc((*worker)->working_queue.max_queue_size, working_queue_entry_size);
    (*worker)->stop_working = false;
    (*worker)->do_work = do_work;

    //Init the objects for synchronisation of the threds
    pthread_mutex_init(&((*worker)->working_queue.access), NULL);
    pthread_cond_init(&((*worker)->working_queue.not_full), NULL);
    pthread_cond_init(&((*worker)->working_queue.not_empty), NULL);
    pthread_cond_init(&((*worker)->working_queue.empty), NULL);

    pthread_attr_init(&attr_thread);

    pthread_attr_setdetachstate(&attr_thread, PTHREAD_CREATE_JOINABLE);

    //Inherit all of the thread scheduling properties from the calling thread.
    pthread_attr_setinheritsched(&attr_thread, PTHREAD_EXPLICIT_SCHED);

    //Start the worker thread
    rc = pthread_create(&((*worker)->working_thread), &attr_thread, worker_thread, *worker);
    if (rc != 0)
    {
        strerror(errno);
        worker_clean_up(worker);
        rc = -1;
    }

    return 0;
}


void add_work_entry(working_queue_t *working_queue, void *working_entry)
{
    pthread_mutex_lock(&(working_queue->access));

    while((working_queue->number_of_entries) == working_queue->max_queue_size)
    {
        pthread_cond_wait(&(working_queue->not_full), &(working_queue->access));
    }

    // Place the new work entry on the working queue
    memcpy(working_queue->entry + working_queue->entry_size * working_queue->tail, working_entry, working_queue->entry_size);
    working_queue->tail = (working_queue->tail + 1) % working_queue->max_queue_size;
    working_queue->number_of_entries++;

    pthread_cond_signal(&(working_queue->not_empty));

    pthread_mutex_unlock(&working_queue->access);
}


void *worker_thread(void *worker_thread_arguments)
{
    worker_t *worker = (worker_t *) worker_thread_arguments;
    void *working_entry = NULL;

    while(1)
    {
        pthread_mutex_lock(&(worker->working_queue.access));

        /* Check if the queue is empty. */
        while ((worker->working_queue.number_of_entries == 0) && (!worker->stop_working))
        {
            /* Wait till the other side signals that there a new item on the queue. */
            pthread_cond_wait(&(worker->working_queue.not_empty), &(worker->working_queue.access));
        }

        if (worker->stop_working)
        {
            pthread_mutex_unlock(&(worker->working_queue.access));
            if (working_entry)
            {
                free(working_entry);
            }
            pthread_exit(NULL);
        }

        if (worker->working_queue.number_of_entries != 0)
        {
            working_entry = malloc(worker->working_queue.entry_size);
            memcpy(working_entry, worker->working_queue.entry + (worker->working_queue.head * worker->working_queue.entry_size), worker->working_queue.entry_size);
            worker->working_queue.head = (worker->working_queue.head + 1) % worker->working_queue.max_queue_size;
            worker->working_queue.number_of_entries--;
        }

        if (worker->working_queue.number_of_entries < worker->working_queue.max_queue_size)
        {
            pthread_cond_signal(&(worker->working_queue.not_full));
        }

        if (worker->working_queue.number_of_entries == 0)
        {
            pthread_cond_signal(&(worker->working_queue.empty));
        }

        pthread_mutex_unlock(&(worker->working_queue.access));

        //Process the entry from the working queue
        if (working_entry)
        {
            worker->do_work(working_entry);
            free(working_entry);
            working_entry = NULL;

        }
    }

    pthread_exit((void *) 0);
}


int stop_worker(worker_t *worker)
{
    pthread_mutex_lock(&worker->working_queue.access);

    if (worker->stop_working)
    {
        pthread_mutex_unlock(&worker->working_queue.access);
        return 0;
    }

    while(worker->working_queue.number_of_entries != 0)
    {
        pthread_cond_wait(&worker->working_queue.empty, &worker->working_queue.access);
    }

    //The queue is empty. Set the stop_working flag and release the mutex.
    worker->stop_working = true;
    pthread_mutex_unlock(&worker->working_queue.access);

    //Wake up the worker thread that waits for non-empty queue so it can check the stop_working parameter.
    pthread_cond_broadcast(&worker->working_queue.not_empty);

    //Wait for the worker thread to make an exit
    pthread_join(worker->working_thread, NULL);

    return 0;
}


void worker_clean_up(worker_t **worker)
{
    if (*worker)
    {
        pthread_cond_destroy(&((*worker)->working_queue.not_full));
        pthread_cond_destroy(&((*worker)->working_queue.not_empty));
        pthread_mutex_destroy(&((*worker)->working_queue.access));
        free((*worker)->working_queue.entry);
        free(*worker);
        *worker = NULL;
    }
}
