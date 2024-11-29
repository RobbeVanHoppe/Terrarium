#include "web_server.h"
#include "esp_http_server.h"
#include "esp_log.h"

static const char *TAG = "webserver";
static httpd_handle_t server = NULL;
extern float temperature;

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
        ESP_LOGE(TAG, "Failed to open file for reading");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

        char buf[1024];
    size_t read_bytes = fread(buf, 1, sizeof(buf), f);
    fclose(f);

    httpd_resp_send(req, buf, read_bytes);
    return ESP_OK;
}

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

void start_webserver()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &temperature_uri);
        esp_err_t res = httpd_register_uri_handler(server, &index_uri);
        if (res != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to register URI handler, error=%s", esp_err_to_name(res));
        }
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