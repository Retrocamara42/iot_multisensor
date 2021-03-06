/*
 * main.h
 * @description: Definitions for file main.c
 * @author: Juan Manuel Neyra
 *
 */
#ifndef IOT_MAIN
#define IOT_MAIN

#include <stdio.h>
#include <esp_task_wdt.h>
#include "esp_sleep.h"
#include "driver/hw_timer.h"
#include "credentials.h"
#include "configuration.h"
#include "wifi.h"
#include "dht_driver.h"
#include "mqtt_client.h"
#include "mqtt_ssl.h"
#include <dht.h>

/******************* DHT SENSOR CONFIGURATION ************************************/
#define DEC_PLACE_MULTIPLIER 100
#define CONFIG_BROKER_URI IOT_CORE_MQTT_URI
#define DEVICE_NAME "iot_ms"
#define TEMPERATURE_TOPIC "temperature"
#define HUMIDITY_TOPIC "humidity"
#define SUBSCRIBE_TOPIC "remote_action"
// Sleep time in minutes
#define SLEEP_TIME 15

// Certificate for AWS IoT Core
extern const uint8_t iot_cert_pem_start[]   asm("_binary_AmazonRootCA1_pem_start");
extern const uint8_t iot_cert_pem_end[]   asm("_binary_AmazonRootCA1_pem_end");

extern const uint8_t iot_client_cert_pem_start[]   asm("_binary_iot_multisensor_certificate_pem_crt_start");
extern const uint8_t iot_client_cert_pem_end[]   asm("_binary_iot_multisensor_certificate_pem_crt_end");

extern const uint8_t iot_key_pem_start[]   asm("_binary_iot_multisensor_private_pem_key_start");
extern const uint8_t iot_key_pem_end[]   asm("_binary_iot_multisensor_private_pem_key_end");

esp_mqtt_client_handle_t client;

/******************* AVAILABLE DEVICES ************************************/
const active_devices iot_active_devices={
   DEVICE_NAME,
   1,
};

/******************* WIFI CONFIGURATION *****************************************/
wifi_config_t custom_wifi_config = {
   .sta = {
      .ssid = WIFI_SSID,
      .password = WIFI_PWD
   },
};

/******************* MQTT CONFIGURATION *****************************************/
const esp_mqtt_client_config_t mqtt_cfg = {
    .uri = CONFIG_BROKER_URI,
    .cert_pem = (const char *)iot_cert_pem_start,
    .client_cert_pem = (const char *)iot_client_cert_pem_start,
    .client_key_pem = (const char *)iot_key_pem_start,
};


/******************* FUNCTION DEFINITIONS *****************************************/
/*
 * hw_timer_sleep
 *   Description: Sends task to sleep
 */
void hw_timer_sleep(void *arg);


/*
 * my_custom_mqtt_on_event_data_cb
 *   Description: Mqtt function that executes when receiving data
 */
void my_custom_mqtt_on_event_data_cb(uint8_t topic_len, char* topic, uint8_t data_len, char* data);



/*
 * transmit_data_task
 *   Description: Reads data from sensors and send them to server
 */
static void transmit_data_task();


#endif
