/**
 * @file mqtt_userdefs.h
 * 
 * @brief Header file for the data type definitions and common functions.
 * 
 * @date 10-Feb-2020
 * @copyright GNU General Public License v3
 * 
 * 
 */
#include <stdbool.h>

/**
 * @brief New enum data type. Represents the posible quality of service levels in MQTT messages
 * supported by libmosquitto.
 */
typedef enum {
	MQTT_QOS_0 = 0,
	MQTT_QOS_1,
	MQTT_QOS_2
} mqtt_qos_t;


/**
 * @brief Payload send in every MQTT message.
 */
typedef struct __attribute__ ((__packed__)){
	char location[256];            /**< Location name. */
	double temperature;        /**< Temperature value. */
	double pressure;               /**< Pressure value. */
	double humidity;               /**< Humidity value. */
} ambient_t;


/**
 * @brief Container for the command line arguments provided 
 * when starting the MQTT clients.
 */
typedef struct {
 char broker_hostname[128];     /**< Hostname/IP of the MQTT broker host. */
 uint16_t broker_port;                  /**< MQTT broker listens on this port for MQTT messages. */
 char location[64];                         /**< MQTT location string. */
} start_arg_t;


/**
 * @brief Process command line arguments.
 *
 * @param[in] argc	number of command line arguments
 * @param[in] argv  pointer to an array of given arguments
 * @param[out] start_arg output structure, filled based on the given arguments
 * 
 * @return always returns 0
 */
extern int process_arguments(int argc, char *argv[], start_arg_t *start_arg);
