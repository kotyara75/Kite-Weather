//
//  bluetooth.c
//  watchface
//
//  Created by Alexander Linovskiy on 4/11/16.
//
//

#include "bluetooth.h"
#include "comm.h"

bool bluetooth_is_connected(void) {
    return connection_service_peek_pebble_app_connection();
}

static BitmapLayer *s_bt_icon_layer = NULL;
static GBitmap *s_bt_icon_bitmap = NULL;

static void bluetooth_callback(bool connected) {
    // Show icon if disconnected
    if(s_bt_icon_layer)
        layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);
    
    if(connected) {
        // notify communication module to send postponed messages
        comm_bluetooth_connected();
    } else {
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

void bluetooth_load(Layer *parent_layer) {
    // Create the Bluetooth icon GBitmap
    s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);
    GRect ib = gbitmap_get_bounds(s_bt_icon_bitmap);
    
    GRect pb = layer_get_bounds(parent_layer);
    
    GRect layer_bounds = GRect(pb.size.w - ib.size.w, 0, ib.size.w, ib.size.h);
    
    // Create the BitmapLayer to display the GBitmap
    s_bt_icon_layer = bitmap_layer_create(layer_bounds);
    bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
    layer_add_child(parent_layer, bitmap_layer_get_layer(s_bt_icon_layer));
}

void bluetooth_unload(Window *window) {
    gbitmap_destroy(s_bt_icon_bitmap); s_bt_icon_bitmap = NULL;
    bitmap_layer_destroy(s_bt_icon_layer); s_bt_icon_layer = NULL;
}
