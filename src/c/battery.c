//
//  battery.c
//  watchface
//
//  Created by Alexander Linovskiy on 4/11/16.
//
//

#include "battery.h"
#include <pebble.h>

#define BATTERY_GAUGE_GAP 14

static int s_battery_level;
static Layer *s_battery_layer = NULL;

static void battery_callback(BatteryChargeState state) {
    // Record the new battery level
    s_battery_level = state.charge_percent;
    
    // Update meter
    if(s_battery_layer)
        layer_mark_dirty(s_battery_layer);
}

void battery_init(void) {
    // Register for battery level updates
    battery_state_service_subscribe(battery_callback);
    
    // Ensure battery level is displayed from the start
    battery_callback(battery_state_service_peek());
}

void battery_deinit(void) {
    battery_state_service_unsubscribe();
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    
    // Calculate the max width
    float max_width = bounds.size.w - bounds.origin.x - 1;
    
    // Find the width of the bar
    int width = (int)(float)(((float)s_battery_level / 100.0F) * max_width);
    
    // Draw the background
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    
    // Draw the bar
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}

void battery_load(Layer *parent_layer) {
    // Put battery gauge at the top of the parent layer
    GRect pb = layer_get_bounds(parent_layer);
    GRect battery_bounds = GRect(pb.origin.x + BATTERY_GAUGE_GAP, pb.origin.y, pb.size.w - BATTERY_GAUGE_GAP, 2);

    // Create battery meter Layer
    s_battery_layer = layer_create(battery_bounds);
    layer_set_update_proc(s_battery_layer, battery_update_proc);
    
    // Add to Parent
    layer_add_child(parent_layer, s_battery_layer);
}

void battery_unload(Window *window) {
    layer_destroy(s_battery_layer); s_battery_layer = NULL;
}
