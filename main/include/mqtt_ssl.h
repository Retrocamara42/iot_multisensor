/*
 * mqtt_ssl.h
 * Description: Definition of functions to connect to mqtt ssl.
 *    Based on esp8266's documentation mqtt_ssl
 * Author: Juan Manuel Neyra
 *
 */
#ifndef IOT_MQTT_SSL
#define IOT_MQTT_SSL

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_ota_ops.h"


/*
 * mqtt_app_start: Configure and start mqtt
 *    Arguments:
 *       - mqtt_cfg: esp_mqtt_client_config_t. Mqtt configuration struct.
 *    Returns:
 *       - client: esp_mqtt_client_handle_t. Mqtt client.
 */
esp_mqtt_client_handle_t mqtt_app_start(esp_mqtt_client_config_t mqtt_cfg);

#endif
