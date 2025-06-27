/**
*  @file mqtt_pub_sense_hat.cpp
*  
*  @brief MQTT publisher code based on libmosquitto.
*  Readings from Pi Sense HAT sensors are used as 
*  MQTT message payload in JSON fomar so it can
*  be processed by Home Assistant which plays the
*  role of a subscriber to MQTT topics.
*
*  Exernal dependences:
*   - libmosquitto (https://mosquitto.org)
*   - libsetila v0.5.7 or newer (https://github.com/positronic57)
*   - libjsoncpp (https://github.com/open-source-parsers/jsoncpp)
* 
*  @date 10-Jun-2025
*  @copyright GNU General Public License v3
*/

#include <iostream>
#include <cstdint>
#include <string>

#include <unistd.h> // For sleep()

#include "setila/setila_i2c.h"
#include "setila/LPS25H.h"
#include "setila/HTS221.h"

#include "json/json.h"

extern "C" {
#include "mosquitto.h"
}


// MQTT auhtentication with a user name and password in clear text.
// Only for demo purpose, it's a worst security practice.
#define MQTT_BROKER_HOST "some_ip_or_hostname"
#define MQTT_BROKER_USER "some_mqtt_user"
#define MQTT_BROKER_PASSWORD "some_mqtt_password"

#define MQTT_HA_AMBIENT_TOPIC "home/ambient_data/living_room"


struct AmbientData {
    std::string location;
    double temperature = 0.0;
    double pressure = 0.0;
    double humidity = 0.0;
    std::string as_json_string();
};


std::string AmbientData::as_json_string()
{
    Json::Value ha_json;
    Json::StreamWriterBuilder builder;
    
    // Configure the JSON as string writer
    builder["indentation"] = "";
    builder["precision"] = 2;
    builder["precisionType"] = "decimal";

    // Build the JSON document
    ha_json["temperature"] = temperature;
    ha_json["pressure"] = pressure;
    ha_json["humidity"] = humidity;

    // Convert the JSON to a string
    return Json::writeString(builder, ha_json);
}


int init_ambient_sensors(LPS25H *lps25h_sensor, HTS221 *hts221_sensor)
{
    // Set LPS25H internal temperature average to 16 and pressure to 32
    int status = lps25h_sensor->set_resolution(0x01, 0x01);
    if (status)
    {
        std::cout << "HTS221 sensor set resolution failed." << std::endl;
        return status;
    }

    status = lps25h_sensor->set_mode_of_operation(
                ST_Sensor::MODE_OF_OPERATION::OP_FIFO_MEAN_MODE,
                ST_Sensor::OUTPUT_DATA_RATE::ODR_1_Hz,
                LPS25H_NBR_AVERAGED_SAMPLES::AVER_SAMPLES_4
              );

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

    return status;
}


int prepare_MQTT_connection(struct mosquitto **mosq)
{
    mosquitto_lib_init();

    *mosq = mosquitto_new("iot_mqtt", true, NULL);

    if (!*mosq) {
        std::cout << "Error: failed to create mosquitto client" << std::endl;
        return -1;
    }

    mosquitto_username_pw_set(*mosq, MQTT_BROKER_USER, MQTT_BROKER_PASSWORD);

    if (mosquitto_connect(*mosq, MQTT_BROKER_HOST, 1883, 60) != MOSQ_ERR_SUCCESS)
    {
        std::cout << "Error: connecting to MQTT broker failed" << std::endl;
	return -1;
    }

    return 0;
}


int main(int argc, char *argv[])
{
    int status = 0;

    struct mosquitto *mosq = nullptr;

    Bus_Master_Device *i2c_bus_master = new Bus_Master_Device("/dev/i2c-1", BUS_TYPE::I2C_BUS);

    LPS25H *lps25h_sensor = new LPS25H(Slave_Device_Type::I2C_SLAVE_DEVICE, i2c_bus_master, 0x5C);
    HTS221 *hts221_sensor = new HTS221(Slave_Device_Type::I2C_SLAVE_DEVICE, i2c_bus_master, 0x5F);

    struct AmbientData ambient_data;

    do //only once
    {
        status = i2c_bus_master->open_bus();
        if (status) {
            std::cout << "Failed to open master bus" << std::endl;
            break;
        }

        status = init_ambient_sensors(lps25h_sensor, hts221_sensor);
        if (status) {
            break;
        }

        status = prepare_MQTT_connection(&mosq);
        if (status) {
            break;
        }

        ambient_data.location = "living_room";

        std::string mqtt_payload;
        int loop = 0;
        do // Do 10 measurements with period of 60s
        {
            status = lps25h_sensor->get_sensor_readings();
            if (status) {
                std::cout << "LPS25H pressure/temperature measurement failed." << std::endl;
                break;
            }

            status = hts221_sensor->get_sensor_readings();
            if (status) {
                std::cout << "HTS221 humidity/temperature measurement failed." << std::endl;
                break;
            }

            std::cout << std::endl << "Pi Sense Hat sensor reading #" << loop + 1 << ":" << std::endl;
            std::cout << "Pressure P = " << lps25h_sensor->pressure_reading() << "[hPa]" << std::endl;
            std::cout << "Temperature T = " << lps25h_sensor->temperature_reading() << "[°C]" << std::endl;
            std::cout << "Relative Humidity R = " << hts221_sensor->humidity_reading() << "[%rH]" << std::endl;
            std::cout << std::endl;

            ambient_data.temperature = lps25h_sensor->temperature_reading();
            ambient_data.pressure = lps25h_sensor->pressure_reading();
            ambient_data.humidity = hts221_sensor->humidity_reading();

            mqtt_payload = ambient_data.as_json_string();
            std::cout << "JSON payload for reading number #" << loop + 1 << std::endl;
            std::cout << mqtt_payload << std::endl;

            mosquitto_publish(mosq, NULL, MQTT_HA_AMBIENT_TOPIC, mqtt_payload.length(), mqtt_payload.c_str(), 0, false);

            // Sleep for about a minute
            sleep(60);
        }while(++loop < 10);

    }while(0);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    delete lps25h_sensor;
    delete hts221_sensor;
    delete i2c_bus_master;

    return status;
}


