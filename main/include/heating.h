#ifndef HEATING_H
#define HEATING_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

extern float temperature;
extern float temperature_cutoff;
extern uint8_t current_heating_mat_state;

void temp_sensor_setup_task(void *pvParameters);
esp_err_t toggle_heating(bool state);
void temperature_control_task(void *pvParameters);

#endif // HEATING_H