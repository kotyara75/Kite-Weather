//
//  bluetooth.c
//  watchface
//
//  Created by Alexander Linovskiy on 4/11/16.
//
//

#include "bluetooth.h"

static BitmapLayer *s_background_layer, *s_bt_icon_layer = NULL;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap = NULL;

static void bluetooth_callback(bool connected) {
    // Show icon if disconnected
    if(s_bt_icon_layer)
        layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);
    
    if(!connected) {
        // Issue a vibrating alert
        // vibes_double_pulse();
    }
}

void bluetooth_init(void) {
    // Register for Bluetooth connection updates
    connection_service_subscribe((ConnectionHandlers) {
        .pebble_app_connection_handler = bluetooth_callback
    });
    
    // Show the correct state of the BT connection from the start
    bluetooth_callback(connection_service_peek_pebble_app_connection());
}

void bluetooth_deinit(void) {
    connection_service_unsubscribe();
}

void bluetooth_load(Window *window) {
    // Create the Bluetooth icon GBitmap
    s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);
    
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    // Create the BitmapLayer to display the GBitmap
    s_bt_icon_layer = bitmap_layer_create(GRect(bounds.origin.x+bounds.size.w-30, 12, 30, 30));
    bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
}

void bluetooth_unload(Window *window) {
    gbitmap_destroy(s_bt_icon_bitmap); s_bt_icon_bitmap = NULL;
    bitmap_layer_destroy(s_bt_icon_layer); s_bt_icon_layer = NULL;
}
