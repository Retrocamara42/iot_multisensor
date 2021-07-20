/*
 * main.c
 * @description: This program connects to wifi, then sends sensor data in
 *    a predefined interval
 * @author: @Retrocamara42
 *
 */
#include "main.h"

static const char *MAIN_TAG = "main";
static SemaphoreHandle_t sleep_semaphore;
static uint32_t sleep_semaphore_count=0;
// Sleep time in minutes
static uint16_t sleep_time=SLEEP_TIME;
static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
static const gpio_num_t dht_gpio = GPIO_NUM_0;


/*
 * hw_timer_sleep
 *   Description: Sends task to sleep
 */
void hw_timer_sleep(void *arg){
    esp_task_wdt_reset();
    sleep_semaphore_count++;
    if(sleep_semaphore_count>(60*sleep_time)){
      sleep_semaphore_count=0;
      xSemaphoreGive(sleep_semaphore);
    }
}


/*
 * my_custom_mqtt_on_event_data_cb
 *   Description: Mqtt function that executes when receiving data
 */
void my_custom_mqtt_on_event_data_cb(uint8_t topic_len, char* topic, uint8_t data_len, char* data){
   for(uint8_t i=0; (i+2)<data_len; i++){
      if(data[i]=='q' && data[i+2]==':'){
         ESP_LOGI(MAIN_TAG, "Waking up device");
         xSemaphoreGive(sleep_semaphore);
      }
   }
}



/*
 * transmit_data_task
 *   Description: Reads data from sensors and send them to server
 */
static void transmit_data_task(){
   // Init variables
   //ESP_LOGI(MAIN_TAG, "Creating data pointer with size %d",sizeof(DhtSensor));
   DhtSensor *dht_sensor;
   if(iot_active_devices.dhtActive){
      dht_sensor = (DhtSensor*)malloc(sizeof(DhtSensor));
      dht_sensor->dht_pin = dht_gpio;
      dht_sensor->dht_type = sensor_type;
      dht_config(&dht_sensor);
   }
   sleep_semaphore = xSemaphoreCreateBinary();

   // Transmission
   while (1){
      esp_task_wdt_reset();
      ESP_LOGI(MAIN_TAG, "Reading data from sensors");
      /******** DHT ***********/
      if(iot_active_devices.dhtActive){
         dht_read_and_process_data(&dht_sensor);
         send_dht_data_with_mqtt(dht_sensor,
            iot_active_devices.device_name,
            client, mqtt_cfg, TEMPERATURE_TOPIC,HUMIDITY_TOPIC);
      }

      /********** SLEEP ************/
      ESP_LOGI(MAIN_TAG, "Going to sleep");
      esp_task_wdt_reset();
      esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
      hw_timer_init(hw_timer_sleep, NULL);
      hw_timer_alarm_us(1000000, true);
      xSemaphoreTake(sleep_semaphore, portMAX_DELAY);
      hw_timer_deinit();
      esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
   }
}


/*
 * app_main
 *   Description: Starts wifi connection and creates transmit_data_task
 */
void app_main(){
   // Start watchdog
   esp_task_wdt_init();
   /********************* DEFAULT CONFIG ******************************/
   ESP_ERROR_CHECK(nvs_flash_init());
   ESP_ERROR_CHECK(esp_netif_init());
   ESP_ERROR_CHECK(esp_event_loop_create_default());
   esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
   /********************* WIFI CONNECT ********************************/
   create_wifi_semaphore();
   wifi_init_sta(custom_wifi_config);
   // Waits indefenitely for wifi to connect
   take_from_wifi_semaphore(portMAX_DELAY);
   delete_wifi_semaphore();

   /********************* MQTT SETUP **********************************/
   client = mqtt_app_start(&mqtt_cfg);

   // Subscribe to topics
   set_mqtt_on_event_data_cb(&my_custom_mqtt_on_event_data_cb);
   mqtt_subscribe(client, SUBSCRIBE_TOPIC, 1);

   // Create task to transmit data
   xTaskCreate(transmit_data_task, "transmit_data_task", 2048*2, NULL, 15, NULL);
}
