/*
 * dht_driver.c
 * Description: Implementation of functions to read data from dht 11
 *    and dht 22 sensors.
 * Author: Juan Manuel Neyra
 *
 */
#include "dht_driver.h"

static char *DHT_TAG = "dht";

/*
 * dht_config: Configure gpio port for dht sensor
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

   (*dht_sensor)->temperature=-1;
   (*dht_sensor)->humidity=-1;
}


/*
 * dht_read_data: Get temperature and humidity values from sensors
 *       Arguments:
 *          -dht_sensor: DhtSensor struct. Use dht_sensor.temperature and
 *             dht.humidity to retrive read data
 */
void dht_read_data(DhtSensor **dht_sensor){
   esp_task_wdt_reset();
   // Store dht data
   int data[5] = {0, 0, 0, 0, 0};
   // Counts how many microseconds the pin is in high or in low state
   uint32_t counter = 0;
   // If counter is higher than this value in seconds, there was an error
   uint32_t maxCounter = 1000;

   /*
    * Detect change and read data. Repeated MAX_DHT_READING times or until
    * correct value is obtained
    */
   float temperature;
   float humidity;
   for(uint8_t k=0; k<MAX_DHT_READING; k++){
      // Initialize with zero
      data[0] = data[1] = data[2] = data[3] = data[4] = 0;
      // Reconfigure pin as output
      gpio_pulldown_dis((*dht_sensor)->dht_pin);
      gpio_set_direction((*dht_sensor)->dht_pin, GPIO_MODE_OUTPUT);
      gpio_set_level((*dht_sensor)->dht_pin, GPIO_LEVEL_LOW);

      if((*dht_sensor)->dht_type==DHT_11){
         // Wait 20 miliseconds if device is dht11
         vTaskDelay(20/portTICK_RATE_MS);
      }
      else{
         // Wait at least 1 milisecond if device is dht22
         ets_delay_us(1200);
      }

      // Data should be sent in 40 cycle (5 bytes)
      uint32_t cycles[CYCLES_READ];

      // Reconfiguring gpio pin as input
      gpio_set_direction((*dht_sensor)->dht_pin, GPIO_MODE_INPUT);
      gpio_pulldown_en((*dht_sensor)->dht_pin);

      // Waiting for response start
      ets_delay_us(40);

      /**** First read *****/
      while(gpio_get_level((*dht_sensor)->dht_pin) == GPIO_LEVEL_LOW) {
        ets_delay_us(1);
        if(counter++ >= maxCounter) { break; }
      }

      while(gpio_get_level((*dht_sensor)->dht_pin) == GPIO_LEVEL_HIGH) {
        ets_delay_us(1);
        if (counter++ >= maxCounter) { break; }
      }
      /********************/

      // Reading dht transmitted message
      for(uint8_t i = 0; i < CYCLES_READ; i+=1){
         // Restart counter
         counter = 0;
         while(gpio_get_level((*dht_sensor)->dht_pin) == GPIO_LEVEL_LOW) {
           ets_delay_us(1);
           if (counter++ >= maxCounter) { break; }
         }
         // Restart counter
         counter = 0;
         while(gpio_get_level((*dht_sensor)->dht_pin) == GPIO_LEVEL_HIGH){
           ets_delay_us(1);
           if (counter++ >= maxCounter) { break; }
         }
         cycles[i]=counter;
      }

      // Translate results into bits
      uint8_t j = 0;
      float timePassedInUs;
      for(uint8_t i =0; i < CYCLES_READ; i+=1){
         //ESP_LOGI(DHT_TAG, "Cycle %d: %d",i,cycles[i]);
         timePassedInUs = cycles[i];
         data[j/8] <<= 1;
         // If counter was above 20us, bit is 1
         if(timePassedInUs>=20)
            data[j/8] |= 1;
         j++;
      }

      // Validate data: 40 bits read + checksum
      if((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))){
         //ESP_LOGI(DHT_TAG, "Checksum validated");
         humidity = (float)((data[0] << 8) + data[1]) / 10;
         if(((*dht_sensor)->dht_type==DHT_11) && humidity > 100){
            humidity = data[0];	// for DHT11
         }

         temperature = (((data[2] & 0x7F) << 8) + data[3]) / 10;
         if((*dht_sensor)->dht_type==DHT_11 && temperature > 125){
            temperature = data[2];	// for DHT11
         }
         if(data[2] & 0x80){
            temperature = -temperature;
         }

         // Validating range of dht
         if((temperature>125) || (temperature<-40) || (humidity<0) || (humidity>100)){
            //ESP_LOGI(DHT_TAG, "Invalid range, retrying...");
            temperature = -1;
            humidity = -1;
            // Waiting 2.5s before making another reading
            vTaskDelay(2500/portTICK_RATE_MS);
            continue;
         }
         ESP_LOGI(DHT_TAG, "Good data found");
         break;
      }
      else{
         //ESP_LOGI(DHT_TAG, "Bad data found in try: %d",k);
         humidity = -1;
         temperature = -1;
         // Waiting 2.5s before making another reading
         vTaskDelay(2500/portTICK_RATE_MS);
      }
   }
   (*dht_sensor)->temperature=temperature;
   (*dht_sensor)->humidity=humidity;
}



/*
 * send_dht_data_with_http: Send dht data with http
 *       Arguments:
 *          -dht_sensor: DhtSensor struct. Use dht_sensor.temperature and
 *             dht.humidity to retrive read data
 *          -http_server_configuration: http_server_configuration struct.
 *             Information of the server where to send the data to
 */
void send_dht_data_with_http(DhtSensor *dht_sensor,
         http_server_configuration http_server_configuration){
   esp_task_wdt_reset();
   uint8_t dht_dec_place=dht_sensor->decimal_place;
   /******************** TEMPERATURE ***********************/
   // PROCESSING
   char chTemp[7];
   float temperature=(float)dht_sensor->temperature;
   int dec_temp=(int)((dht_dec_place*abs(temperature))%dht_dec_place);
   snprintf(chTemp, sizeof chTemp, "%d.%d", (int)floor(temperature), dec_temp);
   // SENDING
   char post_data_temp[24];
   strcpy(post_data_temp, "{\"temperature\":");
   strcat(post_data_temp, chTemp);
   strcat(post_data_temp, "}");
   //ESP_LOGI(DHT_TAG, "Sending temperature data: %s",post_data_temp);
   esp_task_wdt_reset();
   send_http_post_request(post_data_temp, http_server_configuration.temperature_url);
   // PROCESSING
   char chHumid[7];
   float humidity=dht_sensor->humidity;
   int dec_humid=(int)((dht_dec_place*abs(humidity))%dht_dec_place);
   snprintf(chHumid, sizeof chHumid, "%d.%d", (int)floor(humidity), dec_humid);
   // SENDING
   char post_data_hum[24];
   strcpy(post_data_hum, "{\"humidity\":");
   strcat(post_data_hum, chHumid);
   strcat(post_data_hum, "}");
   //ESP_LOGI(DHT_TAG, "Sending humidity data: %s",post_data_hum);
   esp_task_wdt_reset();
   send_http_post_request(post_data_hum, http_server_configuration.humidity_url);
}



/*
 * send_dht_data_with_mqtt: Send dht data with http
 *       Arguments:
 *          -dht_sensor: DhtSensor struct. Use dht_sensor.temperature and
 *             dht.humidity to retrive read data
 *          -http_server_configuration: http_server_configuration struct.
 *             Information of the server where to send the data to
 */
void send_dht_data_with_http(DhtSensor *dht_sensor,
         http_server_configuration mqtt_configuration){
