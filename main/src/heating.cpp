#include "heating.h"
#include "ds18x20.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "heating";

const gpio_num_t sensor_gpio = GPIO_NUM_4;
const size_t addr_count = 1;
size_t found_addr_count;
onewire_addr_t addr_list[addr_count];

float temperature = 25.0f;
float temperature_cutoff = 20.0f;
uint8_t current_heating_mat_state;

void init_temperature_sensor() {
    // Scan for sensors
    if (ds18x20_scan_devices(sensor_gpio, addr_list, addr_count, &found_addr_count) != ESP_OK) {
        ESP_LOGE(TAG, "No sensors detected!");
        return;
    } else {
        ESP_LOGI(TAG, "Sensor found at address: 0x%016llx", addr_list[0]);
    }

    // Setup GPIO pin for heating
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_INPUT_OUTPUT);
}

void read_temperature() {
    if (ds18b20_measure_and_read(sensor_gpio, addr_list[0], &temperature) == ESP_OK) {
        printf("Temperature: %.2f\n", temperature);
    } else {
        printf("Could not read temperature\n");
    }
}

esp_err_t toggle_heating(bool state) {
    return gpio_set_level(GPIO_NUM_2, state);
}

void temperature_control_task(void *pvParameter) {
    while (1) {
        read_temperature();
        current_heating_mat_state = gpio_get_level(GPIO_NUM_2);
        if (temperature < temperature_cutoff && current_heating_mat_state == 0) {
            if (toggle_heating(true) == ESP_OK) {
                ESP_LOGI(TAG, "Heating turned on");
            } else {
                ESP_LOGE(TAG, "Failed to turn on heating");
            }
        } else if (temperature >= temperature_cutoff && current_heating_mat_state == 1) {
            if (toggle_heating(false) == ESP_OK) {
                ESP_LOGI(TAG, "Heating turned off");
            } else {
                ESP_LOGE(TAG, "Failed to turn off heating");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}