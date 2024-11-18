#include "adc.h"

const static char *TAG = "ADC_CLASS";

ADC::ADC(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten)
    : unit(unit), channel(channel), atten(atten), handle(nullptr), cali_handle(nullptr), do_calibration(false) {}

ADC::~ADC() {
    if (handle) {
        adc_oneshot_del_unit(handle);
    }
    if (do_calibration) {
        calibration_deinit();
    }
}

esp_err_t ADC::config() {
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = unit,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(handle, channel, &config));

    do_calibration = calibration_init();
    return ESP_OK;
}

esp_err_t ADC::read(int &raw, int &voltage) {
    ESP_ERROR_CHECK(adc_oneshot_read(handle, channel, &raw));
    ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", unit + 1, channel, raw);
    if (do_calibration) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, raw, &voltage));
        ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", unit + 1, channel, voltage);
    }
    return ESP_OK;
}

bool ADC::calibration_init() {
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
esp_err_t ret = ESP_FAIL;

    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    return calibrated;
}

void ADC::calibration_deinit() {
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(cali_handle));
}