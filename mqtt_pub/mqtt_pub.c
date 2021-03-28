 /**
  * @file mqtt_pub.c
  *
  * @brief MQTT client based on libmosquitto. It publishes 
  * dummy environment data on MQTT topic: /home/<location_name>/ambient_data.
  *
  * @date 10-Feb-2020
  * @copyright GNU General Public License v3
  *
  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <getopt.h>
#include <sys/types.h>

#include "mosquitto.h"

#include "mqtt_userdefs.h"


int main(int argc, char *argv[])
{
    struct mosquitto *mosq;     /**< Libmosquito MQTT client instance. */

    ambient_t ambient;               /**< MQTT message payload. */
    
    char mqtt_channel_name[256];

	start_arg_t start_arg = {   /**< Command line arguments will be stored here. */
		.broker_hostname = "localhost",
		.broker_port = 1883,
        .location = "location"
	};

    //Process the program arguments
    process_arguments(argc, argv, &start_arg);

#ifdef __SHOW_MOSQUITTO_INFO__	
    int major, minor, revision;

    mosquitto_lib_version(&major, &minor, &revision);
    printf("\nLibmosquitto version: %d.%d.%d\n", major, minor, revision);
#endif

    //libmosquitto initialization
    mosquitto_lib_init();

    //Create new libmosquitto client instance
    mosq = mosquitto_new(NULL, true, NULL);

    if (!mosq)
    {
	printf("Error: failed to create mosquitto client\n");
    }

    //Connect to MQTT broker
    if (mosquitto_connect(mosq, start_arg.broker_hostname, start_arg.broker_port, 60) != MOSQ_ERR_SUCCESS)
    {
	printf("Error: connecting to MQTT broker failed\n");
    }

    snprintf(ambient.location, sizeof(ambient.location), "%s", start_arg.location);
	
    //Set dummy environment data
    ambient.temperature = 25.3;
    ambient.pressure = 995.3;
    ambient.humidity = 33;

    sprintf(mqtt_channel_name, "home/%s/ambient_data", start_arg.location);

    while(1)
    {
        //Publish the MQTT message 
	    mosquitto_publish(mosq, NULL, mqtt_channel_name, sizeof(ambient_t), &ambient, MQTT_QOS_0, false);
        sleep(1);
    }

    //Clean up/destroy objects created by libmosquitto
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
}
