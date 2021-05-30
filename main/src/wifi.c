#include "wifi.h"

void wifi_init_sta(wifi_config_t wifi_config){
   ESP_LOGI(WIFI_TAG, "nvs flash init");
   ESP_ERROR_CHECK(nvs_flash_init());

   s_wifi_event_group = xEventGroupCreate();

   ESP_LOGI(WIFI_TAG, "init tcp ip adpater");
   tcpip_adapter_init();

   ESP_ERROR_CHECK(esp_event_loop_create_default());

   ESP_LOGI(WIFI_TAG, "init wifi config");
   wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
   ESP_ERROR_CHECK(esp_wifi_init(&cfg));

   ESP_LOGI(WIFI_TAG, "event handler");
   ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
   ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

   ESP_LOGI(WIFI_TAG, "init wifi auth wpa2");
   if (strlen((char *)wifi_config.sta.password)) {
      wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
   }

   ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
   ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
   ESP_ERROR_CHECK(esp_wifi_start());

   ESP_LOGI(WIFI_TAG, "wifi_init_sta finished.");

   EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

   if (bits & WIFI_CONNECTED_BIT) {
         ESP_LOGI(WIFI_TAG, "connected to ap SSID:%s password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASS);
   } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(WIFI_TAG, "Failed to connect to SSID:%s, password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASS);
   } else {
        ESP_LOGE(WIFI_TAG, "UNEXPECTED EVENT");
   }

   ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
   ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
   vEventGroupDelete(s_wifi_event_group);
}
