//
//  time.h
//  watchface
//
//  Created by Alexander Linovskiy on 6/11/16.
//
//

#ifndef time_h
#define time_h

#include <pebble.h>

void update_time(struct tm *tick_time);
Layer* time_load(Layer *parent_layer);
void time_unload(Window *window);
void time_init(void);
void time_deinit(void);

#endif /* time_h */
