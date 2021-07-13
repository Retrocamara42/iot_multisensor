/*
 * mqtt_ssl.h
 * @description: Definition of functions to connect to mqtt ssl.
 *    Based on esp8266's documentation mqtt_ssl
 * @author: @Retrocamara42
 *
 */
#ifndef IOT_MQTT_SSL
#define IOT_MQTT_SSL

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_event.h"

#include "esp_log.h"
#include "mqtt_client.h"


/*
 * mqtt_on_event_data_cb: Callback function that acts when event data received is
 *   active
 *    Arguments:
 *       - topic_len: uint8_t. Length of topic
 *       - topic: char*. Topic's name
 *       - data_len: uint8_t. Length of data received
 *       - data: char*. Data
 */
typedef void (*mqtt_on_event_data_cb)(uint8_t topic_len, char* topic, uint8_t data_len, char* data);


/*
 * mqtt_on_event_data_cb: Function that runs when the event MQTT_EVENT_DATA is active
 *    Arguments:
 *       - on_event_data_cb: void*. Custom function to run when data is received
 */
void set_mqtt_on_event_data_cb(void (*on_event_data_cb)(uint8_t topic_len, char* topic, uint8_t data_len, char* data));


/*
 * mqtt_app_start: Configure and start mqtt
 *    Arguments:
 *       - mqtt_cfg: esp_mqtt_client_config_t. Mqtt configuration struct.
 *    Returns:
 *       - client: esp_mqtt_client_handle_t. Mqtt client.
 */
esp_mqtt_client_handle_t mqtt_app_start(const esp_mqtt_client_config_t* mqtt_cfg);


/*
 * mqtt_subscribe: Subscribe to mqtt topic
 *    Arguments:
 *       - client: esp_mqtt_client_handle_t. Mqtt client.
 *       - topic: char*. Topic to subscribe to.
 *       - qos: uint8_t. Quality of service.
 */
void mqtt_subscribe(esp_mqtt_client_handle_t client, char* topic, uint8_t qos);


/*
 * mqtt_unsubscribe: Unsubscribe to mqtt topic
 *    Arguments:
 *       - client: esp_mqtt_client_handle_t. Mqtt client.
 *       - topic: char*. Topic to unsubscribe to.
 *       - qos: uint8_t. Quality of service.
 */
void mqtt_unsubscribe(esp_mqtt_client_handle_t client, char* topic);

#endif
