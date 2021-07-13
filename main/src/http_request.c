/*
 * http_request.c
 * @description: Implementation of functions to send an http request.
 *    Based on esp8266's documentation http_request
 * @author: @Retrocamara42
 *
 */
#include "http_request.h"

static const char *HTTP_TAG = "http_client";

/*
 * _http_event_handler: Event handler for http request
 *    Arguments:
 *       - evt: esp_http_client_event_t. Http event.
 */
esp_err_t _http_event_handler(esp_http_client_event_t *evt){
    static char *output_buffer;
    static int output_len;
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            //ESP_LOGD(HTTP_TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            //ESP_LOGD(HTTP_TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            //ESP_LOGD(HTTP_TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            //ESP_LOGD(HTTP_TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            esp_task_wdt_reset();
            ESP_LOGD(HTTP_TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                esp_task_wdt_reset();
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(HTTP_TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            esp_task_wdt_reset();
            //ESP_LOGD(HTTP_TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
                output_len = 0;
            }
            break;
        case HTTP_EVENT_DISCONNECTED:
            esp_task_wdt_reset();
            ESP_LOGI(HTTP_TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                if (output_buffer != NULL) {
                    free(output_buffer);
                    output_buffer = NULL;
                    output_len = 0;
                }
                ESP_LOGI(HTTP_TAG, "Last esp error code: 0x%x", err);
                //ESP_LOGI(HTTP_TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
    }
    return ESP_OK;
}


/*
 * send_http_post_request: Send a http request
 *    Arguments:
 *       - post_data: Char*. Body of post request
 *       - web_url: Char*. Complete url (with path) of post request
 */
void send_http_post_request(char* post_data, char* web_url){
   esp_task_wdt_reset();
   // Initialize client
   char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
   esp_http_client_config_t config = {
      .url = web_url,
      .event_handler = _http_event_handler,
      .user_data = local_response_buffer,
   };
   esp_http_client_handle_t client = esp_http_client_init(&config);

   // Request post
   esp_http_client_set_method(client, HTTP_METHOD_POST);
   esp_http_client_set_header(client, "Content-Type", "application/json");
   esp_http_client_set_post_field(client, post_data, strlen(post_data));
   esp_task_wdt_reset();
   // Perform http request
   int err = esp_http_client_perform(client);
   if(err == ESP_OK) {
      ESP_LOGI(HTTP_TAG, "HTTP POST Status = %d, content_length = %d",
         esp_http_client_get_status_code(client),
         esp_http_client_get_content_length(client));
   }
   else{
      ESP_LOGE(HTTP_TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
   }
   esp_task_wdt_reset();
   esp_http_client_cleanup(client);
}
