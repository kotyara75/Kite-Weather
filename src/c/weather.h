//
//  weather.h
//  watchface
//
//  Created by Alexander Linovskiy on 13/11/16.
//
//

#ifndef weather_h
#define weather_h

#include <pebble.h>

Layer* weather_load(Layer *parent_layer);
void weather_unload(Window *window);
void weather_request_update(void);
void update_wind(const char *wind_direction, const char *wind_speed);
void update_weather(const char *temperature, const char *conditions);
uint32_t weather_update_interval_m(void);
void weather_init(void);

#endif /* weather_h */
