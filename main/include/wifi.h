/*
 * wifi.h
 * @description: Definition of functions to connect to wifi.
 *    Based on esp8266's documentation wifi_sta
 * @uthor: @Retrocamara42
 *
 */
#ifndef IOT_WIFI
#define IOT_WIFI

#include <string.h>
#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_task_wdt.h"

#include "esp_system.h"
#include "esp_netif.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

// Program continues after semaphore is released
SemaphoreHandle_t wifi_semaphore;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define ESP_MAXIMUM_RETRY  3

/*
 * wifi_init_sta: Setup wifi connection
 *    Arguments:
 *       - wifi_config: wifi_config_t. Struct with information
 *          to start wifi connection
 */
void wifi_init_sta(wifi_config_t wifi_config);


/*
 * create_wifi_semaphore: Create wifi semaphore. Used to stop the program to
 *    continue if wifi isn't connected yet
 */
void create_wifi_semaphore();


/*
 * take_from_wifi_semaphore: Increases semaphore count by one. Should be run after
 *   function wifi_init_sta
 *    Arguments:
 *       - ticks: TickType_t. Timeout before the semaphore releases by itself
 */
void take_from_wifi_semaphore(TickType_t ticks);


/*
 * delete_wifi_semaphore: Delete wifi semaphore. Used after controller
 *   connected to wifi succesfully
 */
void delete_wifi_semaphore();

#endif
