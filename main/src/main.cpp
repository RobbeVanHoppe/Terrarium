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
#include "ds18x20.h"

extern "C" void app_main(void) {
        ds18x20_addr_t sensor_addr = DS18X20_ANY;
        float temperature;
        esp_err_t res = ds18x20_measure_and_read(GPIO_NUM_4, sensor_addr, &temperature);

        while (1) {
            if (res == ESP_OK) {
                printf("Temperature: %.2f\n", temperature);
            } else {
                printf("Could not read temperature\n");
            }
        vTaskDelay(pdMS_TO_TICKS(1000));

        res = ds18x20_measure_and_read(GPIO_NUM_4, sensor_addr, &temperature);
    }
}