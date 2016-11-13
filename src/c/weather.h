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
void request_weather_update(void);
void update_wind(const char *wind_direction, const char *wind_speed);
void update_weather(const char *temperature, const char *conditions);

#endif /* weather_h */
