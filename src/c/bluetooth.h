//
//  bluetooth.h
//  watchface
//
//  Created by Alexander Linovskiy on 4/11/16.
//
//

#ifndef bluetooth_h
#define bluetooth_h

#include <pebble.h>

bool bluetooth_is_connected(void);
void bluetooth_init(void);
void bluetooth_deinit(void);
void bluetooth_load(Layer *parent_layer);
void bluetooth_unload(Window *window);

#endif /* bluetooth_h */
