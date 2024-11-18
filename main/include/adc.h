#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

class ADC {
public:
    ADC(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten);
    ~ADC();
    esp_err_t config();
    esp_err_t read(int &raw, int &voltage);

private:
    adc_unit_t unit;
    adc_channel_t channel;
    adc_atten_t atten;
    adc_oneshot_unit_handle_t handle;
    adc_cali_handle_t cali_handle;
    bool do_calibration;

    bool calibration_init();
    void calibration_deinit();
};