#include "adc.h"
#define EXAMPLE_ADC1_CHAN0 ADC_CHANNEL_4
#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_12

extern "C" void app_main(void) {
    ADC adc1(ADC_UNIT_1, EXAMPLE_ADC1_CHAN0, EXAMPLE_ADC_ATTEN);
    adc1.config();

    int adc_raw;
    int voltage;

    while (1) {
        adc1.read(adc_raw, voltage);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}