/*
 * configuration.h
 * Description: Defines server_endpoint
 * Author: Juan Manuel Neyra
 *
 */
#ifndef IOT_CONFIGURATION
#define IOT_CONFIGURATION

typedef struct {
   uint8_t dhtActive;
   char* temperature_url;
   char* humidity_url;
}http_server_configuration;

#endif
