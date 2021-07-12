/*
 * configuration.h
 * Description: Defines server_endpoint
 * Author: Juan Manuel Neyra
 *
 */
#ifndef IOT_CONFIGURATION
#define IOT_CONFIGURATION

typedef struct {
   char* temperature_url;
   char* humidity_url;
}http_server_configuration;

typedef struct {
   char* device_name;
   uint8_t dhtActive;
}active_devices;

#endif
