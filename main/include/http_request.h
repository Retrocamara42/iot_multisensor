/*
 * dht_driver.h
 * Description: Definition of functions to send an http request.
 *    Based on esp8266's documentation http_request
 * Author: Juan Manuel Neyra
 *
 */
#ifndef IOT_HTTP_REQUEST
#define IOT_HTTP_REQUEST

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_task_wdt.h"

#include "esp_http_client.h"

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048


/*
 * _http_event_handler: Event handler for http request
 *    Arguments:
 *       - evt: esp_http_client_event_t. Http event.
 */
esp_err_t _http_event_handler(esp_http_client_event_t *evt);


/*
 * send_http_post_request: Send a http request
 *    Arguments:
 *       - post_data: Char*. Body of post request
 *       - web_url: Char*. Complete url (with path) of post request
 */
void send_http_post_request(char* post_data, char* web_url);


#endif
