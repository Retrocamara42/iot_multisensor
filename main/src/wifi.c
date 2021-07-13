/*
 * wifi.h
 * @description: Implementation of functions to connect to wifi.
 *    Based on esp8266's documentation wifi_sta
 * @author: @Retrocamara42
 *
 */
#include "wifi.h"

static int wifi_retry_num = 0;
static const char *WIFI_TAG = "wifi station";
static EventGroupHandle_t s_wifi_event_group;


/*
 * _wifi_event_handler: Event handler to react after wifi connection
 *    Arguments:
 *       - arg: void*. Generic arguments
 *       - evt: esp_event_base_t. Esp event.
 *       - event_id: int32_t. Event id
 *       - event_data: void*. Event data
 */
static void _wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        // Wifi ready to connect
        esp_task_wdt_reset();
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (wifi_retry_num < ESP_MAXIMUM_RETRY){
           // Reconnect to wifi
            esp_task_wdt_reset();
            esp_wifi_connect();
            wifi_retry_num++;
        } else {
            // Wifi connection failed
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        // Wifi connected succesfully
        wifi_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        // Release semaphore
        esp_task_wdt_reset();
        xSemaphoreGive(wifi_semaphore);
    }
}


/*
 * wifi_init_sta: Setup wifi connection
 *    Arguments:
 *       - wifi_config: wifi_config_t. Struct with information
 *          to start wifi connection
 */
void wifi_init_sta(wifi_config_t wifi_config){
   esp_task_wdt_reset();
   // Disconnect before init
   esp_wifi_disconnect();

   s_wifi_event_group = xEventGroupCreate();
   // Init tcp/ip protocol
   esp_task_wdt_reset();
   tcpip_adapter_init();

   // Init configuration
   esp_task_wdt_reset();
   wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
   ESP_ERROR_CHECK(esp_wifi_init(&cfg));

   // Register to wifi event handler
   ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &_wifi_event_handler, NULL));
   ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &_wifi_event_handler, NULL));

   //ESP_LOGI(WIFI_TAG, "init wifi auth wpa2");
   if (strlen((char *)wifi_config.sta.password)) {
      wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
   }

   ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
   ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
   ESP_ERROR_CHECK(esp_wifi_start());
   //ESP_LOGI(WIFI_TAG, "wifi_init_sta finished.");

   EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

   if (bits & WIFI_CONNECTED_BIT) {
         ESP_LOGI(WIFI_TAG, "connected to ap SSID:%s succesfully",
                 wifi_config.sta.ssid);
   } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(WIFI_TAG, "Failed to connect to SSID:%s",
                 wifi_config.sta.ssid);
   } else {
        ESP_LOGE(WIFI_TAG, "UNEXPECTED EVENT");
   }

   esp_task_wdt_reset();
   ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &_wifi_event_handler));
   ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &_wifi_event_handler));
   vEventGroupDelete(s_wifi_event_group);
}


/*
 * create_wifi_semaphore: Create wifi semaphore. Used to stop the program to
 *    continue if wifi isn't connected yet
 */
void create_wifi_semaphore(){
   wifi_semaphore = xSemaphoreCreateBinary();
}


/*
 * take_from_wifi_semaphore: Increases semaphore count by one. Should be run after
 *   function wifi_init_sta
 *    Arguments:
 *       - ticks: TickType_t. Timeout before the semaphore releases by itself
 */
void take_from_wifi_semaphore(TickType_t ticks){
   xSemaphoreTake(wifi_semaphore, ticks);
}



/*
 * delete_wifi_semaphore: Delete wifi semaphore. Used after controller
 *   connected to wifi succesfully
 */
void delete_wifi_semaphore(){
   vSemaphoreDelete(wifi_semaphore);
}
