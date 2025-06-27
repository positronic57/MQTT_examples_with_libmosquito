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
#include <cstdint>

#include <unistd.h> // For sleep()

#include "setila/setila_i2c.h"
#include "setila/LPS25H.h"
#include "setila/HTS221.h"


extern "C" {
#include "mosquitto.h"
#include "mqtt_userdefs.h"
}

int main(int argc, char *argv[])
{
    Bus_Master_Device *i2c_bus_master = new Bus_Master_Device("/dev/i2c-1", BUS_TYPE::I2C_BUS);

    char mqtt_channel_name[256];

    struct mosquitto *mosq = nullptr;

    ambient_t ambient;

    start_arg_t start_arg = { "localhost", 1883, "location" };

    int status = 0;

    process_arguments(argc, argv, &start_arg);

    LPS25H *lps25h_sensor = new LPS25H(Slave_Device_Type::I2C_SLAVE_DEVICE, i2c_bus_master, 0x5C);
    HTS221 *hts221_sensor = new HTS221(Slave_Device_Type::I2C_SLAVE_DEVICE, i2c_bus_master, 0x5F);

    if (i2c_bus_master->open_bus() < 0)
    {
        std::cout << "Failed to open master bus" << std::endl;
        return -1;
    }

    // Set LPS25H internal temperature average to 16 and pressure to 32
    status = lps25h_sensor->set_resolution(0x01, 0x01);
    if (status)
    {
        std::cout << "HTS221 sensor set resolution failed." << std::endl;
        return status;
    }

    status = lps25h_sensor->set_mode_of_operation(ST_Sensor::MODE_OF_OPERATION::OP_FIFO_MEAN_MODE, ST_Sensor::OUTPUT_DATA_RATE::ODR_1_Hz, LPS25H_NBR_AVERAGED_SAMPLES::AVER_SAMPLES_4);
    if (status)
    {
        std::cout << "LPS25H sensor initialization failed." << std::endl;
        return status;
    }

    //Set HTS221 internal temperature average to 32 and humidity to 64
    status = hts221_sensor->set_resolution(0x04, 0x04);
    if (status)
    {
        std::cout << "HTS221 sensor set resolution failed." << std::endl;
        return status;
    }

    // Configure HTS221 for ONE SHOT type of measurements
    status = hts221_sensor->set_mode_of_operation(ST_Sensor::MODE_OF_OPERATION::OP_ONE_SHOT);
    if (status)
    {
        std::cout << "HTS221 sensor initialization failed." << std::endl;
        return status;
    }

    mosquitto_lib_init();

    mosq = mosquitto_new(NULL, true, NULL);

    if (!mosq) {
        std::cout << "Error: failed to create mosquitto client" << std::endl;
        return -1;
    }

    if (mosquitto_connect(mosq, start_arg.broker_hostname, start_arg.broker_port, 60) != MOSQ_ERR_SUCCESS)
    {
        std::cout << "Error: connecting to MQTT broker failed" << std::endl;
    }

    snprintf(ambient.location, sizeof(ambient.location), "%s", start_arg.location);

    sprintf(mqtt_channel_name, "home/%s/ambient_data", start_arg.location);

    while(1)
    {
        if (lps25h_sensor->get_sensor_readings())
        {
            std::cout << "LPS25H pressure/temperature measurement failed." << std::endl;
            return status;
        }

        if (hts221_sensor->get_sensor_readings())
        {
            std::cout << "HTS221 humidity/temperature measurement failed." << std::endl;
            return -1;
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


