/*
 * main.c
 * Description: This program connects to wifi, then sends sensor data in
 *    a predefined interval
 * Author: Juan Manuel Neyra
 *
 */
#include "main.h"

static const char *MAIN_TAG = "main";

/*
 * transmit_data_task
 *   Description: Reads data from sensors and send them to server
 */
static void transmit_data_task(){
   // Init variables
   //ESP_LOGI(MAIN_TAG, "Creating data pointer with size %d",sizeof(DhtSensor));
   DhtSensor *dht_sensor;
   if(active_devices.dhtActive){
      dht_sensor = (DhtSensor*)malloc(sizeof(DhtSensor));
      dht_sensor->dht_pin = GPIO_PIN_DHT;
      dht_sensor->dht_type = DHT_11;
      dht_sensor->decimal_place = DEC_PLACE_MULTIPLIER;
      dht_config(&dht_sensor);
   }

   // Transmission
   while (1){
      esp_task_wdt_reset();
      ESP_LOGI(MAIN_TAG, "Reading data from sensors");
      /******** DHT ***********/
      if(active_devices.dhtActive){
         dht_read_data(&dht_sensor);
         send_dht_data_with_mqtt(dht_sensor, mqtt_cfg);
      }

      /********** SLEEP ************/
      ESP_LOGI(MAIN_TAG, "Going to sleep");
      esp_task_wdt_reset();
      vTaskDelay(600000/portTICK_RATE_MS);
   }
}


/*
 * app_main
 *   Description: Starts wifi connection and creates transmit_data_task
 */
void app_main(){
   // Start watchdog
   esp_task_wdt_init();
   /********************* WIFI CONNECT ********************************/
   create_wifi_semaphore();
   wifi_init_sta(custom_wifi_config);
   // Waits indefenitely for wifi to connect
   take_from_wifi_semaphore(portMAX_DELAY);
   delete_wifi_semaphore();

   /********************* MQTT SETUP **********************************/
   ESP_ERROR_CHECK(nvs_flash_init());
   ESP_ERROR_CHECK(esp_netif_init());
   ESP_ERROR_CHECK(esp_event_loop_create_default());
   client = mqtt_app_start(mqtt_cfg);

   // Create task to transmit data
   xTaskCreate(transmit_data_task, "transmit_data_task", 2048*2, NULL, 0, NULL);
}
