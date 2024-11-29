#ifndef HEATING_H
#define HEATING_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

extern float temperature;
extern float temperature_cutoff;
extern uint8_t current_heating_mat_state;

void init_temperature_sensor();
void read_temperature();
esp_err_t toggle_heating(bool state);
void temperature_control_task(void *pvParameter);

#endif // HEATING_H