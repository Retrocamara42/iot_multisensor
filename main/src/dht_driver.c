#include "dht_driver.h"

/*
 * dht_config: Configure gpio port for dht sensor
 *       Arguments:
 *          -dht_sensor: DhtSensor struct. Contains configuration options
 *             for dht sensor.
 */
DhtSensor dht_config(DhtSensor *dht_sensor){
   gpio_config_t io_conf;
   io_conf.intr_type = GPIO_INTR_DISABLE;
   io_conf.mode = GPIO_MODE_INPUT;
   uint16_t gpio_mask = 1ULL<<*dht_sensor.dht_pin;
   io_conf.pin_bit_mask = gpio_mask;
   io_conf.pull_down_en = 1;
   io_conf.pull_up_en = 0;
   gpio_config(&io_conf);

   *dht_sensor.temperature=-1;
   *dht_sensor.humidity=-1;
   return dht_sensor;
}


/*
 * dht_read_data: Get temperature and humidity values from sensors
 *       Arguments:
 *          -dht_sensor: DhtSensor struct. Use dht_sensor.temperature and
 *             dht.humidity to retrive read data
 */
void dht_read_data(DhtSensor *dht_sensor){
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
   for(uint8_t k=0; k<MAX_DHT_READING; k++){
      // Initialize with zero
      data[0] = data[1] = data[2] = data[3] = data[4] = 0;
      // Reconfigure pin as output
      gpio_pulldown_dis(*dht_sensor.dht_pin);
      gpio_set_direction(*dht_sensor.dht_pin, GPIO_MODE_OUTPUT);
      gpio_set_level(*dht_sensor.dht_pin, GPIO_LEVEL_LOW);

      if(*dht_sensor.dht_type==DHT_11){
         // Wait 20 miliseconds if device is dht11
         vTaskDelay(20/portTICK_RATE_MS);
      }
      else{
         // Wait at least 1 milisecond if device is dht22
         vTaskDelay(1200/portTICK_RATE_US);
      }

      // Data should be sent in 40 cycle (5 bytes)
      uint32_t cycles[CYCLES_READ];

      // Reconfiguring gpio pin as input
      gpio_set_direction(*dht_sensor.dht_pin, GPIO_MODE_INPUT);
      gpio_pulldown_en(*dht_sensor.dht_pin);

      // Waiting for response start
      vTaskDelay(40/portTICK_RATE_US);

      /**** First read *****/
      while(gpio_get_level(*dht_sensor.dht_pin) == GPIO_LEVEL_LOW) {
        if(counter++ >= maxCounter*portTICK_RATE_MS) { break; }
      }

      while(gpio_get_level(*dht_sensor.dht_pin) == GPIO_LEVEL_HIGH) {
        if (counter++ >= maxCounter*portTICK_RATE_MS) { break; }
      }
      /********************/

      // Reading dht transmitted message
      for(uint8_t i = 0; i < CYCLES_READ; i+=1){
         // Restart counter
         counter = 0;
         while(gpio_get_level(*dht_sensor.dht_pin) == GPIO_LEVEL_LOW) {
           vTaskDelay(1/portTICK_RATE_US);
           if (counter++ >= maxCounter) { break; }
         }
         // Restart counter
         counter = 0;
         while(gpio_get_level(*dht_sensor.dht_pin) == GPIO_LEVEL_HIGH){
           vTaskDelay(1/portTICK_RATE_US);
           if (counter++ >= maxCounter) { break; }
         }
         cycles[i]=counter;
      }

      // Translate results into bits
      uint8_t j = 0;
      float timePassedInUs;
      for(uint8_t i =0; i < CYCLES_READ; i+=1){
         timePassedInUs = cycles[i];
         data[j/8] <<= 1;
         // If counter was above 40us, bit is 1
         if(timePassedInUs>=40)
            data[j/8] |= 1;
         j++;
      }

      // Validate data: 40 bits read + checksum
      float temperature;
      float humidity;
      if((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))){
         ESP_LOGI(DHT_TAG, "Good data found");
         humidity = (float)((data[0] << 8) + data[1]) / 10;
         if(DhtSensor.dht_pin==DHT_11 && humidity > 100){
            humidity = data[0];	// for DHT11
         }

         temperature = (float)(((data[2] & 0x7F) << 8) + data[3]) / 10;
         if(DhtSensor.dht_pin==DHT_11 && temperature > 125){
            temperature = data[2];	// for DHT11
         }
         if(data[2] & 0x80){
            temperature = -temperature;
         }

         // Validating range of dht
         if((temperature>125) || (temperature<-40) || (humidity<0) || (humidity>100)){
            temperature = -1;
            humidity = -1;
            // Waiting 2.5s before making another reading
            vTaskDelay(2500/portTICK_RATE_MS);
            continue;
         }
         break;
      }
      else{
         ESP_LOGI(DHT_TAG, "Bad data found in try: %d",k);
         humidity = -1;
         temperature = -1;
         // Waiting 2.5s before making another reading
         vTaskDelay(2500/portTICK_RATE_MS);
      }
   }

   *dht_sensor.temperature=temperature;
   *dht_sensor.humidity=humidity;
   return *dht_sensor;
}
