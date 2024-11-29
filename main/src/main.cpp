#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "ds18x20.h"
#include "esp_log.h"
#include "web_server.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include <esp_spiffs.h>

char buf[16];
int buflen = 16;

static const char *TAG = "main";
static EventGroupHandle_t s_wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

float temperature;
float temperature_cutoff = 20.0f;

/// @brief Wifi setup

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying to connect to the AP");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: %s", esp_ip4addr_ntoa(&event->ip_info.ip, buf, buflen));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta() {
    s_wifi_event_group = xEventGroupCreate();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "stud-2024",
            .password = "Stud3nt5",
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}

void init_spiffs() {
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    // Initialize SPIFFS
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "\rSPIFFS initialized");

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}


/// @brief Main program loop

extern "C" void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Init spiffs for serving static files
    init_spiffs();

    // Initialize Wi-Fi
    wifi_init_sta();

    // Wait for Wi-Fi connection
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

    const gpio_num_t sensor_gpio = GPIO_NUM_4;
    size_t addr_count = 1;
    size_t found_addr_count;
    onewire_addr_t addr_list[addr_count];

    // Scan for sensors
    if (ds18x20_scan_devices(sensor_gpio, addr_list, addr_count, &found_addr_count) != ESP_OK) {
        ESP_LOGE("DS18B20", "No sensors detected!");
        return;
    } else {
        ESP_LOGI("DS18B20", "Sensor found at address: 0x%016llx", addr_list[0]);
    }

    // Start the web server
    start_webserver();

    while (1) {
        esp_err_t res = ds18x20_measure_and_read(sensor_gpio, addr_list[0], &temperature);
        if (res == ESP_OK) {
            printf("Temperatuure: %.2f\n", temperature);
        } else {
            printf("Could not read temperature\n");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}