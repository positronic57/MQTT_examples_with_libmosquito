/**
*  @file mqtt_pub_sense_hat.cpp
*  
*  @brief MQTT publisher code based on libmosquitto. 
*  Readings from Pi Sense HAT sensors are used as 
*  MQTT message payload.
* 
*  @date 10-Feb-2020
*  @copyright GNU General Public License v3
*/

#include <iostream>

#include "setila/setila_i2c.h"
#include "setila/LPS25H.h"
#include "setila/HTS221.h"

extern "C" {
#include <unistd.h>
#include "mosquitto.h"
#include "mqtt_userdefs.h"
}

int main(int argc, char *argv[])
{
    Bus_Master_Device *i2c_bus_master = new Bus_Master_Device("/dev/i2c-1", BUS_TYPE::I2C_BUS);

    LPS25H *lps25h_sensor = new LPS25H(0x5C);
    HTS221 *hts221_sensor = new HTS221(0x5F);

    char mqtt_channel_name[256];

    struct mosquitto *mosq = nullptr;

    ambient_t ambient;

    start_arg_t start_arg = { "localhost", 1883, "location" };

    int status = 0;

    process_arguments(argc, argv, &start_arg);

    if (i2c_bus_master->open_bus() < 0)
    {
	std::cout << "Failed to open master bus" << std::endl;
	return -1;
    }

    status = lps25h_sensor->attach_to_bus(i2c_bus_master);
    status = hts221_sensor->attach_to_bus(i2c_bus_master);

    status = lps25h_sensor->init_sensor();

    if (status)
    {
        std::cout << "LPS25H sensor initialization failed." << std::endl;
	return status;
    }

    status = hts221_sensor->init_sensor(0x1B, 0x85);
    if (status)
    {
        std::cout << "LTS221 sensor initialization failed." << std::endl;
        return status;
    }

    mosquitto_lib_init();

    mosq = mosquitto_new(NULL, true, NULL);

    if (!mosq)
    {
        std::cout << "Error: failed to create mosquitto client" << std::endl;
    }

    if (mosquitto_connect(mosq, start_arg.broker_hostname, start_arg.broker_port, 60) != MOSQ_ERR_SUCCESS)
    {
        std::cout << "Error: connecting to MQTT broker failed" << std::endl;
    }

    snprintf(ambient.location, sizeof(ambient.location), "%s", start_arg.location);

    sprintf(mqtt_channel_name, "home/%s/ambient_data", start_arg.location);

    /* Get the humidity and temperature readings from the sensor and calculate the current temperature and humidity values. */
    if (hts221_sensor->get_sensor_readings())
    {
        std::cout << "HTS221 humidity/temperature measurement failed." << std::endl;
        return -1;
    }

    while(1)
    {
        if (lps25h_sensor->get_sensor_readings())
        {
            std::cout << "LPS25H pressure/temperature measurement failed." << std::endl;
            return status;
        }

        std::cout << std::endl << "Readings from Pi Sense Hat sensors:" << std::endl << std::endl;
        std::cout << "Pressure P=" << lps25h_sensor->pressure_reading() << "[hPa]" << std::endl;
        std::cout << "Temperature T=" << lps25h_sensor->temperature_reading() << "[°C]" << std::endl;
        std::cout << "Relative Humidity R=" << hts221_sensor->humidity_reading() << "[%rH]" << std::endl;
        std::cout << std::endl;

        ambient.temperature = lps25h_sensor->temperature_reading();
        ambient.pressure = lps25h_sensor->pressure_reading();
        ambient.humidity = hts221_sensor->humidity_reading();

        mosquitto_publish(mosq, NULL, mqtt_channel_name, sizeof(ambient_t), &ambient, MQTT_QOS_0, false);

        sleep(3);
    }

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    delete lps25h_sensor;
    delete hts221_sensor;
    delete i2c_bus_master;

    return 0;
}


