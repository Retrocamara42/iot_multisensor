/*
 * configuration.h
 * @description: Defines useful structs for main program
 * @author: @Retrocamara42
 *
 */
#ifndef IOT_CONFIGURATION
#define IOT_CONFIGURATION

/*
 * http_server_configuration: Organize http endpoints where device will send data
 *    - temperature_url: char*. Endpoint for temperature url
 *    - humidity_url: char*. Endpoint for humidity url
 */
typedef struct {
   char* temperature_url;
   char* humidity_url;
}http_server_configuration;

/*
 * active_devices: Indicates which devices will be active
 *    - device_name: char*. Device name. To differentiate between devices
 *    - dhtActive: uint8_t. Indicates if dht sensor will be used
 */
typedef struct {
   char* device_name;
   uint8_t dhtActive;
}active_devices;

#endif
