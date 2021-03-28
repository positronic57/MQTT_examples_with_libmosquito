 /**
  * @file mqtt_sub.c 
  *
  * @brief Mqtt client based on libmosquitto. Subscribes to topic 
  * /home/+/ambient_data for collecting environment data from the 
  * data publishers.
  * 
  * @date 10-Feb-2020
  * @copyright GNU General Public License v3
  *
  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <stdint.h>
#include <time.h>

#include "mosquitto.h"

#include "mqtt_userdefs.h"
#include "worker.h"


/**
 * @brief This function will be called by the worker thread for processing each entry from the 
 * queue of MQTT message payloads.
 * 
 * @param[in] process this entry from the queue
 */
int process_message(void *message)
{
    time_t local_time;
    struct tm tm_result;
    char time_stamp[32];

    //Build the timestamp header
    local_time = time(NULL);
    localtime_r(&local_time, &tm_result);
    strftime(time_stamp, sizeof(time_stamp), "%d.%h.%Y %H:%M:%S", &tm_result);

    ambient_t *ambient_data = (ambient_t *) message;

    printf("%s [%s] t = %.2f[°C], p = %.2f[hPa], H = %.2f[%%rH]\n",
                    time_stamp,
                    ambient_data->location,
                    ambient_data->temperature,
                    ambient_data->pressure,
                    ambient_data->humidity
                );

    fflush(stdout);
	
    return 0;
}


/**
 * @brief Call back function for received MQTT message.
 * 
 * Libmosquitto thread will call this function for every received MQTT message.
 * It extracts the payload info from the MQTT message and writes it into the
 * working FIFO queue.
 * 
 * @param[in] pointer to libmoquitto MQTT client instance
 * @param[in,out] pointer to the data defined by the Libmosquitto user/caller
 * @param[in] points to the received MQTT message 
 */
void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    ambient_t *ambient_data = calloc(1, sizeof(ambient_t));

    memcpy(ambient_data, message->payload, message->payloadlen);

    working_queue_t *mqtt_message_queue = (working_queue_t *)userdata;

    //Append the message payload at the tail of the FIFO queue
    add_work_entry(mqtt_message_queue, (void *)ambient_data);

    free(ambient_data);
}

static void clean_up_libmosquitto(struct mosquitto *mosq)
{
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
}

int main(int argc, char *argv[])
{
    struct mosquitto *mosq = NULL;  /**< Libmosquito MQTT client instance. */

    sem_t blocking_sem;                         /**< Semaphore for blocking the main thread execution. */

    start_arg_t start_arg = {                   /**< Command line arguments will be stored here. */
        .broker_hostname = "localhost",
        .broker_port = 1883
    };
	
    worker_t *mqtt_message_processor = NULL;                   /**< Thread for processing received payload from all publishers. */
    working_queue_t *mqtt_message_queue = NULL;         /**< FIFO queue for payloads.*/

#ifdef __SHOW_MOSQUITTO_INFO__    
    int major, minor, revision;
    
    //Get libmosquitto version info
    mosquitto_lib_version(&major, &minor, &revision);
    printf("\nLibmosquitto version: %d.%d.%d\n", major, minor, revision);
#endif

    //Processing command line arguments if any
    process_arguments(argc, argv, &start_arg);

    //Create a working thread used by the subscriber for processing the received MQTT messages. 
    if (create_worker(&mqtt_message_processor, 32, sizeof(ambient_t), process_message))
    {
        printf("Error: creating worker thread for processing MQTT messages failed\n");
        return -1;
    }

    //FIFO queue for the payload from MQTT messages received by the subscriber. Worker thread will process the items as they land in the queue.
    mqtt_message_queue = &mqtt_message_processor->working_queue;

    //libmosquitto initialization
    mosquitto_lib_init();

    //Init the semaphore
    sem_init(&blocking_sem, 0, 0);
	
    //Create new libmosquitto client instance
    mosq = mosquitto_new(NULL, true, mqtt_message_queue);

    if (!mosq)
    {
        printf("Error: failed to create mosquitto client\n");
    }

    //Define a function which will be called by libmosquitto client every time when there is a new MQTT message
    mosquitto_message_callback_set(mosq, my_message_callback);
	
    //Connect to MQTT broker
    if (mosquitto_connect(mosq, start_arg.broker_hostname, start_arg.broker_port, 60) != MOSQ_ERR_SUCCESS)
    {
        printf("Error: connecting to MQTT broker failed\n");

        stop_worker(mqtt_message_processor);
        worker_clean_up(&mqtt_message_processor);

        clean_up_libmosquitto(mosq);

        exit(-1);
    }

    //Subscribe to any channel that ends with ambient_data
    mosquitto_subscribe(mosq, NULL, "home/+/ambient_data", 0);

    //Run libmosquitto client in a separate thread
    mosquitto_loop_start(mosq);
	
    while(1)
    {
        //Block the execution of the main thread
        sem_wait(&blocking_sem);
        break;
    }

    //Stop the worker thread
    stop_worker(mqtt_message_processor);

    //Stop libmosquitto cliet thread
    mosquitto_loop_stop(mosq, true);

    //Worker clean up
    worker_clean_up(&mqtt_message_processor);

    //Clean up/destroy objects created by libmosquitto
    clean_up_libmosquitto(mosq);
}
