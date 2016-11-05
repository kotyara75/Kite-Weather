//
//  ph_bay.h
//  watchface
//
//  Created by Alexander Linovskiy on 5/11/16.
//
//

#ifndef ph_bay_h
#define ph_bay_h

#include <pebble.h>

Layer*  ph_bay_load(Layer *parent_layer);
void ph_bay_unload(Window *window);
void ph_bay_init(void);
void ph_bay_deinit(void);

#endif /* ph_bay_h */
