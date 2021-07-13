/*
 * mqtt_ssl.c
 * @description: Implementation of functions to connect to mqtt ssl.
 *    Based on esp8266's documentation mqtt_ssl
 * @author: @Retrocamara42
 *
 */
#include "mqtt_ssl.h"

static const char *MQTT_TAG = "MQTT_SSL";
static uint8_t mqttStatusConnection=0;

void default_mqtt_on_event_data_cb(uint8_t topic_len, char* topic, uint8_t data_len, char* data) { }

static mqtt_on_event_data_cb custom_mqtt_on_event_data_cb = &default_mqtt_on_event_data_cb;


/*
 * mqtt_on_event_data_cb: Function that runs when the event MQTT_EVENT_DATA is active
 *    Arguments:
 *       - on_event_data_cb: void*. Custom function to run when data is received
 */
void set_mqtt_on_event_data_cb(void (*on_event_data_cb)(uint8_t topic_len, char* topic, uint8_t data_len, char* data)){
   custom_mqtt_on_event_data_cb=(*on_event_data_cb);
}


/*
 * mqtt_event_handler_cb: Logic for event handler for mqtt
 *    Arguments:
 *       - event: esp_mqtt_event_handle_t. Mqtt event.
 */
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event){
    switch(event->event_id){
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_CONNECTED");
            mqttStatusConnection=1;
            break;
        case MQTT_EVENT_DISCONNECTED:
            mqttStatusConnection=0;
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            break;
        case MQTT_EVENT_PUBLISHED:
            break;
        case MQTT_EVENT_DATA:
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            custom_mqtt_on_event_data_cb(event->topic_len, event->topic, event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_ESP_TLS) {
                ESP_LOGI(MQTT_TAG, "Error esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
                ESP_LOGI(MQTT_TAG, "Tls stack error: 0x%x", event->error_handle->esp_tls_stack_err);
            } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                ESP_LOGI(MQTT_TAG, "Connection refused: 0x%x", event->error_handle->connect_return_code);
            } else {
                ESP_LOGW(MQTT_TAG, "Unknown error: 0x%x", event->error_handle->error_type);
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}


/*
 * mqtt_event_handler: Event handler for mqtt
 *    Arguments:
 *       - handler_args: void*. Generic args.
 *       - base: esp_event_base_t. Event base.
 *       - event_id: int32_t. Event id.
 *       - event_data: void*. Event data.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    mqtt_event_handler_cb(event_data);
}


/*
 * mqtt_app_start: Configure and start mqtt
 *    Arguments:
 *       - mqtt_cfg: esp_mqtt_client_config_t. Mqtt configuration struct.
 *    Returns:
 *       - client: esp_mqtt_client_handle_t. Mqtt client.
 */
esp_mqtt_client_handle_t mqtt_app_start(const esp_mqtt_client_config_t* mqtt_cfg){
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
    return client;
}


/*
 * mqtt_subscribe: Subscribe to mqtt topic
 *    Arguments:
 *       - client: esp_mqtt_client_handle_t. Mqtt client.
 *       - topic: char*. Topic to subscribe to.
 *       - qos: uint8_t. Quality of service.
 */
void mqtt_subscribe(esp_mqtt_client_handle_t client, char* topic, uint8_t qos){
   uint8_t msg_id = esp_mqtt_client_subscribe(client, topic, qos);
   ESP_LOGI(MQTT_TAG, "sent subscribe successful, msg_id=%d", msg_id);
}



/*
 * mqtt_unsubscribe: Unsubscribe to mqtt topic
 *    Arguments:
 *       - client: esp_mqtt_client_handle_t. Mqtt client.
 *       - topic: char*. Topic to unsubscribe to.
 *       - qos: uint8_t. Quality of service.
 */
void mqtt_unsubscribe(esp_mqtt_client_handle_t client, char* topic){
   uint8_t msg_id = esp_mqtt_client_unsubscribe(client, topic);
   ESP_LOGI(MQTT_TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
}
