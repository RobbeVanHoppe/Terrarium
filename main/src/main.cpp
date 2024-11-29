// #include "adc.h"

// #include "ds18x20.h"

// // #define EXAMPLE_ADC1_CHAN0 ADC_CHANNEL_4
// // #define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_12

// extern "C" void app_main(void) {
//     ADC adc1(ADC_UNIT_1, EXAMPLE_ADC1_CHAN0, EXAMPLE_ADC_ATTEN);
//     adc1.config();

//     int adc_raw;
//     int voltage;

//     while (1) {
//         adc1.read(adc_raw, voltage);
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ds18x20.h"
#include "esp_log.h"

const gpio_num_t sensor_gpio = GPIO_NUM_4;
const size_t addr_count = 1;
size_t found_addr_count;
onewire_addr_t addr_list[addr_count];

float temperature;

esp_err_t setup_temperature()
{
    // Initialize the GPIO
    if (ds18x20_scan_devices(sensor_gpio, addr_list, addr_count, &found_addr_count) != ESP_OK)
    {
        ESP_LOGE("DS18B20", "No sensors detected!");
        return ESP_FAIL;
    }
    // Print the address of the sensor found on the bus
    ESP_LOGI("DS18B20", "Sensor found at address: 0x%016llx", addr_list[0]);
    return ESP_OK;
}

void read_temperature()
{
    if (ds18x20_measure_and_read(sensor_gpio, addr_list[0], &temperature) != ESP_OK)
    {
        ESP_LOGE("DS18B20", "Could not read temperature!");
        return;
    }
    else
    {
        ESP_LOGI("DS18B20", "Temperature: %.2f", temperature);
    }
}

extern "C" void app_main(void)
{
    // scan and setup ds18b20 temp sensors
    setup_temperature();

    while (1)
    {
        read_temperature();
    }
}
