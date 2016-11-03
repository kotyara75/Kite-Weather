//
//  battery.h
//  watchface
//
//  Created by Alexander Linovskiy on 4/11/16.
//
//

#ifndef battery_h
#define battery_h

#include <pebble.h>

void battery_init(void);
void battery_deinit(void);
void battery_load(Window *window);
void battery_unload(Window *window);

#endif /* battery_h */
