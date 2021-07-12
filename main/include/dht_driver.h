/*
 * dht_driver.h
 * Description: Definition of functions to read data from dht 11
 *    and dht 22 sensors.
 * Author: Juan Manuel Neyra
 *
 */
#ifndef IOT_DHT_DRIVER
#define IOT_DHT_DRIVER

#include <stdlib.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

#include "driver/gpio.h"
#include "esp_log.h"

#include "configuration.h"
#include "http_request.h"
#include "mqtt_client.h"

// Amount of cycles to be transmitted
#define CYCLES_READ 40

// Number of retrys when reading dht data
#define MAX_DHT_READING 5


// Gpio level enum
typedef enum{
    GPIO_LEVEL_LOW = 0,
    GPIO_LEVEL_HIGH = 1,
}Gpio_Level;

// Dht type enum
typedef enum{
    DHT_11 = 0,
    DHT_22 = 1,
}Dht_Type;


// Contains configuration and values read from dht sensor
typedef struct DhtSensor {
   uint8_t  dht_pin;
   Dht_Type dht_type;
   float temperature;
   float humidity;
   uint8_t decimal_place;
} DhtSensor;


/*
 * dht_config: Configure gpio port for dht sensor
 *       Arguments:
 *          -dht_sensor: DhtSensor struct. Contains configuration options
 *             for dht sensor.
 */
void dht_config(DhtSensor **dht_sensor);


/*
 * dht_read_data: Get temperature and humidity values from sensors
 *       Arguments:
 *          -dht_sensor: DhtSensor struct. Use dht_sensor.temperature and
 *             dht.humidity to retrive read data
 */
void dht_read_data(DhtSensor **dht_sensor);


/*
 * send_dht_data_with_http: Send dht data with http
 *       Arguments:
 *          -dht_sensor: DhtSensor struct. Use dht_sensor.temperature and
 *             dht.humidity to retrive read data
 *          -device_name: char*. Name of the device. To be part of the payload
 *          -http_server_configuration: http_server_configuration struct.
 *             Information of the server where to send the data to
 */
 void send_dht_data_with_http(DhtSensor *dht_sensor,
          char* device_name, http_server_configuration http_server_configuration);


/*
 * send_dht_data_with_mqtt: Send dht data with mqtt
 *       Arguments:
 *          -dht_sensor: DhtSensor struct. Use dht_sensor.temperature and
 *             dht.humidity to retrive read data
 *          -device_name: char*. Name of the device. To be part of the payload
 *          -client: esp_mqtt_client_handle_t. Mqtt client.
 *          -mqtt_configuration: esp_mqtt_client_config_t struct.
 *             Mqtt broker configuration
 *          -topic_temp: char*. Name of the topic to publish temperature data
 *          -topic_humid: char*. Name of the topic to publish humidity data
 */
void send_dht_data_with_mqtt(DhtSensor *dht_sensor,
         char* device_name,
         esp_mqtt_client_handle_t client,
         esp_mqtt_client_config_t mqtt_configuration,
         char* topic_temp, char* topic_humid);

#endif
