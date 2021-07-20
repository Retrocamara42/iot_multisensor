/*
 * dht_driver.c
 * @description: Implementation of functions to read data from dht 11
 *    and dht 22 sensors.
 * @author: @Retrocamara42
 *
 */
#include "dht_driver.h"

static char *DHT_TAG = "dht";

/*
 * dht_config: Configure gpio port for dht sensor. Additionally, it initializes
 *    temperature and humidity to -99
 *       Arguments:
 *          -dht_sensor: DhtSensor struct. Contains configuration options
 *             for dht sensor.
 */
void dht_config(DhtSensor **dht_sensor){
   esp_task_wdt_reset();
   //ESP_LOGI(DHT_TAG, "dht_config init");
   uint32_t dht_pin=(*dht_sensor)->dht_pin;
   //ESP_LOGI(DHT_TAG, "Configuring dht sensor on pin %d",dht_pin);
   gpio_config_t io_conf;
   io_conf.intr_type = GPIO_INTR_DISABLE;
   io_conf.mode = GPIO_MODE_INPUT;
   uint32_t gpio_mask = dht_pin;
   io_conf.pin_bit_mask = gpio_mask;
   io_conf.pull_down_en = 1;
   io_conf.pull_up_en = 0;
   gpio_config(&io_conf);

   (*dht_sensor)->temperature=-99;
   (*dht_sensor)->humidity=-99;
}


/*
 * dht_read_and_process_data: Get temperature and humidity values from sensors
 *       Arguments:
 *          -dht_sensor: DhtSensor struct. Use dht_sensor.temperature and
 *             dht.humidity to retrive read data
 */
void dht_read_and_process_data(DhtSensor **dht_sensor){
   esp_task_wdt_reset();
   float temperature=0;
   float humidity=0;
   if (dht_read_float_data((**dht_sensor).dht_type, (**dht_sensor).dht_pin, &humidity, &temperature) != ESP_OK){
      (*dht_sensor)->temperature=-99;
      (*dht_sensor)->humidity=-99;
   }
   else{
      (*dht_sensor)->temperature=temperature;
      (*dht_sensor)->humidity=humidity;
   }
}



/*
 * @function send_dht_data_with_http
 *    @brief Send dht data with http
 *
 *    @param dht_sensor: DhtSensor struct. Use dht_sensor.temperature and
 *        dht.humidity to retrive read data
 *    @param device_name: char*. Name of the device. To be part of the payload
 *    @ param http_server_configuration: http_server_configuration struct.
 *        Information of the server where to send the data to
 */
void send_dht_data_with_http(DhtSensor *dht_sensor,
         char* device_name, http_server_configuration http_server_configuration){
   esp_task_wdt_reset();
   uint8_t dht_dec_place=dht_sensor->dht_type==DHT_TYPE_DHT11?0:2;
   /******************** TEMPERATURE ***********************/
   // PROCESSING
   char chTemp[7];
   float temperature=(float)dht_sensor->temperature;
   int dec_temp;
   if(dht_dec_place==0)
      dec_temp=0;
   else
      dec_temp=(int)((dht_dec_place*abs(temperature))%dht_dec_place);

   snprintf(chTemp, sizeof chTemp, "%d.%d", (int)floor(temperature), dec_temp);
   // SENDING
   char post_data_temp[48];
   strcpy(post_data_temp, "{\"dev_name\":\"");
   strcat(post_data_temp, device_name);
   strcat(post_data_temp, "\",\"temp\":");
   strcat(post_data_temp, chTemp);
   strcat(post_data_temp, "}");
   //ESP_LOGI(DHT_TAG, "Sending temperature data: %s",post_data_temp);
   esp_task_wdt_reset();
   send_http_post_request(post_data_temp, http_server_configuration.temperature_url);
   // PROCESSING
   char chHumid[7];
   float humidity=dht_sensor->humidity;
   int dec_humid;
   if(dht_dec_place==0)
      dec_humid=0;
   else
      dec_humid=(int)((dht_dec_place*abs(humidity))%dht_dec_place);

   snprintf(chHumid, sizeof chHumid, "%d.%d", (int)floor(humidity), dec_humid);
   // SENDING
   char post_data_hum[48];
   strcpy(post_data_hum, "{\"dev_name\":\"");
   strcat(post_data_hum, device_name);
   strcat(post_data_hum, "\",\"humid\":");
   strcat(post_data_hum, chHumid);
   strcat(post_data_hum, "}");
   //ESP_LOGI(DHT_TAG, "Sending humidity data: %s",post_data_hum);
   esp_task_wdt_reset();
   send_http_post_request(post_data_hum, http_server_configuration.humidity_url);
}



/*
 * send_dht_data_with_mqtt: Send dht data with mqtt
 *       Arguments:
 *          -dht_sensor: DhtSensor struct. Use dht_sensor.temperature and
 *             dht.humidity to retrive read data
 *          -device_name: char*. Name of the device. To be part of the payload
 *          -client: esp_mqtt_client_handle_t. Mqtt client.
 *          -mqtt_configuration: esp_mqtt_client_config_t struct.
 *             Mqtt broker configuration
 *          -topic_temp: char*. Name of the topic to publish temperature data
 *          -topic_humid: char*. Name of the topic to publish humidity data
 */
void send_dht_data_with_mqtt(DhtSensor *dht_sensor,
         char* device_name,
         esp_mqtt_client_handle_t client,
         esp_mqtt_client_config_t mqtt_configuration,
         char* topic_temp, char* topic_humid){
   esp_task_wdt_reset();
   uint8_t dht_dec_place=dht_sensor->dht_type==DHT_TYPE_DHT11?0:2;
   /******************** TEMPERATURE ***********************/
   // PROCESSING
   char chTemp[7];
   float temperature=(float)dht_sensor->temperature;
   uint8_t dec_temp;
   if(dht_dec_place==0)
      dec_temp=0;
   else
      dec_temp=(int)((dht_dec_place*abs(temperature))%dht_dec_place);

   snprintf(chTemp, sizeof chTemp, "%d.%d", (int)floor(temperature), dec_temp);
   // SENDING
   char post_data_temp[48];
   strcpy(post_data_temp, "{\"dev_name\":\"");
   strcat(post_data_temp, device_name);
   strcat(post_data_temp, "\",\"temp\":");
   strcat(post_data_temp, chTemp);
   strcat(post_data_temp, "}");
   ESP_LOGI(DHT_TAG, "Sending temperature data: %s",post_data_temp);
   esp_task_wdt_reset();
   uint8_t msg_id = esp_mqtt_client_publish(client, topic_temp, post_data_temp, 0, 0, 0);
   ESP_LOGI(DHT_TAG, "temp publish successful, msg_id=%d", msg_id);

   /******************** HUMIDITY ***********************/
   // PROCESSING
   char chHumid[7];
   float humidity=dht_sensor->humidity;
   uint8_t dec_humid;
   if(dht_dec_place==0)
      dec_humid=0;
   else
      dec_humid=(int)((dht_dec_place*abs(humidity))%dht_dec_place);

   snprintf(chHumid, sizeof chHumid, "%d.%d", (int)floor(humidity), dec_humid);
   // SENDING
   char post_data_hum[48];
   strcpy(post_data_hum, "{\"dev_name\":\"");
   strcat(post_data_hum, device_name);
   strcat(post_data_hum, "\",\"humid\":");
   strcat(post_data_hum, chHumid);
   strcat(post_data_hum, "}");
   ESP_LOGI(DHT_TAG, "Sending humidity data: %s",post_data_hum);
   esp_task_wdt_reset();
   msg_id = esp_mqtt_client_publish(client, topic_humid, post_data_hum, 0, 0, 0);
   ESP_LOGI(DHT_TAG, "humid publish successful, msg_id=%d", msg_id);
}
