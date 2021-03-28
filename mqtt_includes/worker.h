/**
* @file worker.h 
* 
* @brief Defines the data types and functions required for implementing
* FIFO work queue and the worker thread for processing MQTT payloads.
*
* @date 22-Feb-2020
* @copyright GNU General Public License v3
* 
*/

#include <stdbool.h>

/**
 * @brief Maximal legth of the work queue in number of entries.
 */
#define MAXIMUM_WORKING_QUEUE_LENGTH	128

/**
 * @brief Defines a new data type for working queue.
 */
typedef struct working_queue working_queue_t;

/**
 * @brief Defines new data type for worker.
 */
typedef struct worker worker_t;

/**
 * @brief Data type do_work_f. A function pointer. Points to a function that will be called for each entry in the queue.
 */
typedef int (*do_work_f)(void *work_entry);

/**
 * @brief Represents a FIFO queue for storring payloads from received MQTT messages. 
 * 
 * Main thread will write data from one side of the queue as they arrive via MQTT, while the
 * worker thead will process them on the other end of the queue.
 */
struct working_queue {
	unsigned int max_queue_size;   /**< Muximal lenght of the queue in number of entries. */
	pthread_mutex_t access;            /**< Pthread mutex for controlling the queue access. */
	pthread_cond_t empty;               /**< Conditional variable queue is empty. Informs the waiting thread that the queue is empty. */
	pthread_cond_t not_full;            /**< Conditional variable queue is not full. Informs the waiting thread that new entries can be written in the queue. */
	pthread_cond_t not_empty;      /**< Conditional variable queue is not empty. Informs the waiting thread that there are some entries in the queue..*/
	int head;                                            /**< Head of the FIFO queue. Worker thread process the head entry first. */
	int tail;                                                /**< Tail of the FIFO queue. New entry is appended at the tail of the queue. */
	int number_of_entries;                /**< Current number of entries in the queue. */
	void *entry;                                      /**< Pointer to memory reserved for the queue. */
	int entry_size;                                 /**< Size of one queue entry in bites .*/
};

/**
 * @brief Represents the woker for processing payloads of received MQTT messages.
 */
struct worker {
	working_queue_t working_queue;     /**< FIFO queue for storing the received payloads from MQTT messages */
	pthread_t working_thread;                  /**< Worker will run in this thread */
	bool stop_working;                                 /**< Stop the processing of the working queue items */
	do_work_f do_work;                               /**< Function that will be called for each queue entry. MQTT payload processor. */
};

/**
 * @brief Creates a new FIFO queue and starts a new thread that will process the entries from the queue.
 * Processing will be done by calling a function specified in the do_work_t argument for each entry.
 * 
 * @param[in, out] worker worker object containing all worker items and properties
 * @param[in] working_queue_size maximum number of entries in the queue
 * @param[in] working_queue_entry_size size of the queue entries
 * @param[in] do_work function pointer. This function will be called for each queue elements
 *
 * @return 0 in case woker is created successfully, -1 in case of error
 */
extern int create_worker(worker_t **worker, unsigned int working_queue_size, unsigned int working_queue_entry_size, do_work_f do_work);

/**
 * @brief Writes entry in the working queue
 *
 * @param[in] working_queue pointer to the working queue
 * @param[in] working_entry pointer to the new entry
 */
extern void add_work_entry(working_queue_t *working_queue, void *working_entry);

/**
 * @brief Ends worker thread execution.
 *
 * @param[in, out] worker to be stopped
 * @return 0
 */
extern int stop_worker(worker_t *worker);

/**
 *  @brief Free up the resources allocated for the worker and the working queue.
 *  
 *  @param[in, out] worker points to a poiner of the worker structure
 * 
 */
extern void worker_clean_up(worker_t **worker);
