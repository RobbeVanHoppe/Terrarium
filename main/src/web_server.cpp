#include "web_server.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "cJSON.h"
#include <sys/param.h>

static const char *TAG = "webserver";
static httpd_handle_t server = NULL;
extern float temperature;
extern float temperature_cutoff;

/// @brief Handler for getting the temperature, index and updating the temperature cutoff

esp_err_t temperature_get_handler(httpd_req_t *req)
{
    char resp_str[64];
    snprintf(resp_str, sizeof(resp_str), "%.2f\n", temperature);
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t index_get_handler(httpd_req_t *req)
{
    FILE* f = fopen("/spiffs/index.html", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open index file for reading");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    // Allocate buffer for file content
    char *buf = (char *)malloc(4098); // Adjust size as needed
    if (buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for file content");
        fclose(f);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    size_t read_bytes = fread(buf, 1, 4098, f);
    fclose(f);

    if (read_bytes == 0) {
        ESP_LOGE(TAG, "Failed to read file content");
        free(buf);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_send(req, buf, read_bytes);
    free(buf);
    return ESP_OK;
}

esp_err_t update_temp_cutoff_handler(httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            return ESP_FAIL;
        }
        remaining -= ret;
    }

    cJSON *json = cJSON_Parse(buf);
    if (json == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    cJSON *temp_cutoff = cJSON_GetObjectItem(json, "temperature_cutoff");
    if (!cJSON_IsNumber(temp_cutoff)) {
        cJSON_Delete(json);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    float new_temp_cutoff = temp_cutoff->valuedouble;
    esp_err_t res = update_temperature_cutoff(new_temp_cutoff);
    cJSON_Delete(json);

    if (res == ESP_OK) {
        httpd_resp_sendstr(req, "Temperature cutoff updated successfully");
    } else {
        httpd_resp_sendstr(req, "Invalid temperature cutoff value");
    }

    return ESP_OK;
}

/// @brief Register URI handlers

httpd_uri_t temperature_uri = {
    .uri = "/temperature",
    .method = HTTP_GET,
    .handler = temperature_get_handler,
    .user_ctx = NULL};

httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_get_handler,
    .user_ctx = NULL};

httpd_uri_t update_temp_cutoff_uri = {
    .uri = "/update_temp_cutoff",
    .method = HTTP_POST,
    .handler = update_temp_cutoff_handler,
    .user_ctx = NULL
};

/// @brief Start and stop the webserver

void start_webserver()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK)
    {
        // TODO: put in some error handling
        httpd_register_uri_handler(server, &temperature_uri);
        httpd_register_uri_handler(server, &update_temp_cutoff_uri);
        httpd_register_uri_handler(server, &index_uri);
        ESP_LOGI(TAG, "Webserver started");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to start webserver");
    }
}

void stop_webserver()
{
    if (server)
    {
        httpd_stop(server);
        ESP_LOGI(TAG, "Webserver stopped");
    }
}

/// @brief Update the temperature cutoff

esp_err_t update_temperature_cutoff(float new_temperature) {
    if (new_temperature < 20.0f || new_temperature > 40.0f) {
        return ESP_ERR_INVALID_ARG;
    }
    temperature_cutoff = new_temperature;
    ESP_LOGI(TAG, "Temperature cutoff updated to %.2f", temperature_cutoff);
    return ESP_OK;
}