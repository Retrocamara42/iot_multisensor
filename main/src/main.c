#include <stdio.h>
#include <stdlib.h>
#include "configuration.h"
#include "wifi.h"
#include "dht_driver.h"

#define ESP_MAXIMUM_RETRY  3
#define GPIO_PIN_DHT 16

static const char *MAIN_TAG = "main";

const server_endpoint dht_server_temp={
      "ec2-18-221-10-42.us-east-2.compute.amazonaws.com",
      "http://ec2-18-221-10-42.us-east-2.compute.amazonaws.com/environment_monitor/get_temperature_data/"
}

const server_endpoint dht_server_hum={
      "ec2-18-221-10-42.us-east-2.compute.amazonaws.com",
      "http://ec2-18-221-10-42.us-east-2.compute.amazonaws.com/environment_monitor/get_humidity_data/"
}


wifi_config_t wifi_config = {
      .sta = {
         .ssid = "ZCL12",
         .password = "jUmaNU47."
      },
};

static int wifi_retry_num = 0;


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (wifi_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            wifi_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->ip_info.ip));
        wifi_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}


static void transmit_data_task(){
   // Init variables
   ESP_LOGI(MAIN_TAG, "Transmit data task started");
   DhtSensor dht_sensor;
   dht_sensor.dht_pin = GPIO_PIN_DHT;
   dht_sensor.dht_type = DHT_11;
   dht_config(&dht_sensor);

   // Transmission
   while (1){
      ESP_LOGI(MAIN_TAG, "Reading data from sensor");
      dht_read_data(&dht_sensor);
      // Transforming data from float to char array
      ESP_LOGI(MAIN_TAG, "Transforming variables into strings");
      char chTemperature[6];
      snprintf(chTemperature, 6, "%f", dht_sensor.temperature);
      char chHumidity[6];
      snprintf(chHumidity, 6, "%f", dht_sensor.humidity);
      // Constructing message
      ESP_LOGI(MAIN_TAG, "Constructing temperature message");
      char post_data_temp[24];
      strcpy(post_data_temp, "{'temperature':");
      strcat(post_data_temp, chTemperature);
      strcat(post_data_temp, "}");
      // Sending message
      ESP_LOGI(MAIN_TAG, "Sending temperature data");
      send_http_post_request(post_data_temp, dht_server_temp.web_server, dht_server_temp.web_url);
      ESP_LOGI(MAIN_TAG, "Constructing humidity message");
      char post_data_hum[24];
      strcpy(post_data_hum, "{'humidity':");
      strcat(post_data_hum, chHumidity);
      strcat(post_data_hum, "}");
      ESP_LOGI(MAIN_TAG, "Sending humidity data");
      send_http_post_request(post_data_hum, dht_server_hum.web_server, dht_server_hum.web_url);
      // Wait before sending another message
      vTaskDelay(300000/portTICK_RATE_MS);
   }
}

void app_main(){
   // Connect to wifi
   wifi_init_sta(wifi_config);
   // Init tcp/ip stack
   ESP_ERROR_CHECK(esp_netif_init());

   // Create task to transmit data
   xTaskCreate(transmit_data_task, "transmit_data_task", 1024, NULL, 0, NULL);
}
