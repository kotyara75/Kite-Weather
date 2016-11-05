//
//  ph_bay.c
//  watchface
//
//  Created by Alexander Linovskiy on 5/11/16.
//
//

#include "ph_bay.h"
#include <pebble.h>

static GBitmap *s_background_bitmap = NULL;
static BitmapLayer *s_background_layer = NULL;

Layer* ph_bay_load(Layer *parent_layer) {
    // Get information about the Window
    GRect wb = layer_get_bounds(parent_layer);
    
    // Create GBitmap
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PH_BAY);
    GRect ib = gbitmap_get_bounds(s_background_bitmap);
    
    GRect layer_bounds = GRect(0, 0, wb.size.w, ib.size.h);
    
    // Create BitmapLayer to display the GBitmap
    s_background_layer = bitmap_layer_create(layer_bounds);
    
    // Set the bitmap onto the layer and add to the window
    // bitmap_layer_set_background_color(s_background_layer, GColorWhite);
    bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
    
    layer_add_child(parent_layer, bitmap_layer_get_layer(s_background_layer));
    
    return bitmap_layer_get_layer(s_background_layer);
}

void ph_bay_unload(Window *window) {
    // Destroy BitmapLayer
    bitmap_layer_destroy(s_background_layer); s_background_layer = NULL;
    
    // Destroy GBitmap
    gbitmap_destroy(s_background_bitmap); s_background_bitmap = NULL;
}

void ph_bay_init(void) {
}

void ph_bay_deinit(void) {
}
