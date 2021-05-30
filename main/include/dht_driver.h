#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"

// Amount of cycles to be transmitted
#define CYCLES_READ 40

// Number of retrys when reading dht data
#define MAX_DHT_READING 5

// Gpio level enum
typedef enum{
    GPIO_LEVEL_LOW = 0,
    GPIO_LEVEL_HIGH = 1,
}Gpio_Level;

// Dht type enum
typedef enum{
    DHT_11 = 0,
    DHT_22 = 1,
}Dht_Type;


// Contains configuration and values read from dht sensor
typedef struct DhtSensor {
   uint8_t  dht_pin;
   Dht_Type dht_type;
   float temperature;
   float humidity;
} DhtSensor;

static const char *DHT_TAG = "dht";
