#include <stdio.h>
#include <stdlib.h>
#include "configuration.h"
#include "wifi.h"
#include "dht_driver.h"

#define ESP_MAXIMUM_RETRY  3
#define GPIO_PIN_DHT 16

const server_endpoint dht_server={
      "example.com",
      "http://example.com/sound_recorder/receive_sound/"
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
   DhtSensor dht_sensor;
   dht_sensor.dht_pin = GPIO_PIN_DHT;
   dht_sensor.dht_type = DHT_11;
   dht_config(&dht_sensor);

   // Transmission
   while (1){
      dht_read_data(&dht_sensor);
      // Transforming data from float to char array
      char chTemperature[6];
      snprintf(chTemperature, 6, "%f", dht_sensor.temperature);
      char chHumidity[6];
      snprintf(chHumidity, 6, "%f", dht_sensor.humidity);
      // Constructing message
      char post_data[32];
      strcpy(post_data, "{'temperature':");
      strcat(post_data, chTemperature);
      strcat(post_data, ",'humidity':");
      strcat(post_data, chHumidity);
      strcat(post_data, "}");
      // Sending message
      send_http_post_request(post_data, WEB_SERVER, WEB_URL);
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
