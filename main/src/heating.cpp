#include "heating.h"
#include "ds18b20.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <onewire_bus_impl_rmt.h>

#define EXAMPLE_ONEWIRE_BUS_GPIO 4
#define EXAMPLE_ONEWIRE_MAX_DS18B20 2
#define HEATING_MAT_GPIO GPIO_NUM_2

static const char *TAG = "heating";

int ds18b20_device_num = 0;
ds18b20_device_handle_t ds18b20s[EXAMPLE_ONEWIRE_MAX_DS18B20];
onewire_device_iter_handle_t iter = NULL;
onewire_device_t next_onewire_device;
esp_err_t search_result = ESP_OK;

float temperature = 25.0f;
float temperature_cutoff = 20.0f;
uint8_t current_heating_mat_state = 0;

void temp_sensor_setup_task(void *pvParameters)
{
    // install 1-wire bus
    onewire_bus_handle_t bus = NULL;
    onewire_bus_config_t bus_config = {
        .bus_gpio_num = EXAMPLE_ONEWIRE_BUS_GPIO,
    };
    onewire_bus_rmt_config_t rmt_config = {
        .max_rx_bytes = 10, // 1byte ROM command + 8byte ROM number + 1byte device command
    };
    ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));

    // create 1-wire device iterator, which is used for device search
    ESP_ERROR_CHECK(onewire_new_device_iter(bus, &iter));
    ESP_LOGI(TAG, "Device iterator created, start searching...");
    do
    {
        search_result = onewire_device_iter_get_next(iter, &next_onewire_device);
        if (search_result == ESP_OK)
        { // found a new device, let's check if we can upgrade it to a DS18B20
            ds18b20_config_t ds_cfg = {};
            // check if the device is a DS18B20, if so, return the ds18b20 handle
            if (ds18b20_new_device(&next_onewire_device, &ds_cfg, &ds18b20s[ds18b20_device_num]) == ESP_OK)
            {
                ESP_LOGI(TAG, "Found a DS18B20[%d], address: %016llX", ds18b20_device_num, next_onewire_device.address);
                ds18b20_device_num++;
            }
            else
            {
                ESP_LOGI(TAG, "Found an unknown device, address: %016llX", next_onewire_device.address);
            }
        }
    } while (search_result != ESP_ERR_NOT_FOUND);
    ESP_ERROR_CHECK(onewire_del_device_iter(iter));
    ESP_LOGI(TAG, "Searching done, %d DS18B20 device(s) found", ds18b20_device_num);

    // setup GPIO pin for heating mat
    gpio_set_direction(HEATING_MAT_GPIO, GPIO_MODE_INPUT_OUTPUT);

    vTaskDelete(NULL);
}

void read_temperature()
{
    for (int i = 0; i < ds18b20_device_num; i++)
    {
        ESP_ERROR_CHECK(ds18b20_trigger_temperature_conversion(ds18b20s[i]));
        ESP_ERROR_CHECK(ds18b20_get_temperature(ds18b20s[i], &temperature));
        ESP_LOGI(TAG, "temperature read from DS18B20[%d]: %.2fC", i, temperature);
    }
}

esp_err_t toggle_heating(bool state)
{
    return gpio_set_level(HEATING_MAT_GPIO, state);
}

void temperature_control_task(void *pvParameters)
{
    while (1)
    {
        read_temperature();
        current_heating_mat_state = gpio_get_level(HEATING_MAT_GPIO);
        if (temperature < temperature_cutoff && current_heating_mat_state == 0)
        {
            if (toggle_heating(true) == ESP_OK)
            {
                ESP_LOGI(TAG, "Heating turned on");
            }
            else
            {
                ESP_LOGE(TAG, "Failed to turn on heating");
            }
        }
        else if (temperature >= temperature_cutoff && current_heating_mat_state == 1)
        {
            if (toggle_heating(false) == ESP_OK)
            {
                ESP_LOGI(TAG, "Heating turned off");
            }
            else
            {
                ESP_LOGE(TAG, "Failed to turn off heating");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}