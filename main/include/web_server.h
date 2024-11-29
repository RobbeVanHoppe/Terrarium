#include <esp_err.h>
#ifndef WEBSERVER_H
#define WEBSERVER_H

void start_webserver();
void stop_webserver();

esp_err_t update_temperature_cutoff(float new_temperature);

#endif // WEBSERVER_H