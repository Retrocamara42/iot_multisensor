/*
 * main.h
 * Description: Definitions for file main.c
 * Author: Juan Manuel Neyra
 *
 */
#ifndef IOT_MAIN
#define IOT_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <esp_task_wdt.h>
#include "configuration.h"
#include "wifi.h"
#include "dht_driver.h"
#include "mqtt_client.h"

/******************* DHT SENSOR CONFIGURATION ************************************/
#define GPIO_PIN_DHT GPIO_NUM_16
#define DEC_PLACE_MULTIPLIER 100
#define CONFIG_BROKER_URI "uri"

extern const uint8_t iot_mqtt_ms_cert_pem_start[]   asm("_binary_iot_mqtt_ms_cert_pem_start");
extern const uint8_t iot_mqtt_ms_cert_pem_end[]   asm("_binary_iot_mqtt_ms_cert_pem_end");

esp_mqtt_client_handle_t client;

/******************* CONFIGURATION OF SERVER ************************************/
const http_server_configuration server_configuration={
      1,
      "http://ec2-3-16-111-61.us-east-2.compute.amazonaws.com/environment_monitor/receive_temperature/",
      "http://ec2-3-16-111-61.us-east-2.compute.amazonaws.com/environment_monitor/receive_humidity/"
};


/******************* WIFI CONFIGURATION *****************************************/
wifi_config_t custom_wifi_config = {
   .sta = {
      .ssid = "ZCL12",
      .password = "jUmaNU47."
   },
};

/******************* MQTT CONFIGURATION *****************************************/
const esp_mqtt_client_config_t mqtt_cfg = {
    .uri = CONFIG_BROKER_URI,
    .cert_pem = (const char *)iot_mqtt_ms_cert_pem_start,
};


#endif
