//
//  time.c
//  watchface
//
//  Created by Alexander Linovskiy on 6/11/16.
//
//

#include "time.h"
#include <pebble.h>

static GFont s_time_font = NULL;
static TextLayer *s_time_layer = NULL;
static TextLayer *s_date_layer = NULL;

#define TIME_FONT_RESOURCE RESOURCE_ID_FONT_PERFECT_DOS_42
#define TIME_LAYER_SIZE 43
#define DATE_FONT FONT_KEY_GOTHIC_14
#define DATE_LAYER_SIZE 15

Layer* time_load(Layer *parent_layer) {
    // Get information about the Window
    GRect wb = layer_get_bounds(parent_layer);
    
    // Put date at the bottom of the window
    GRect date_bounds = GRect(0, wb.size.h - DATE_LAYER_SIZE, wb.size.w, DATE_LAYER_SIZE);
    
    // Put time on top of the date layer
    GRect time_bounds = GRect(0, date_bounds.origin.y - TIME_LAYER_SIZE, wb.size.w, TIME_LAYER_SIZE);
    
    // Create GFont
    s_time_font = fonts_load_custom_font(resource_get_handle(TIME_FONT_RESOURCE));
    
    // Create the TextLayer with specific bounds
    s_time_layer = text_layer_create(time_bounds);
    
    // Improve the layout to be more like a watchface
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_font(s_time_layer, s_time_font);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    
    // Add it as a child layer to the Window's root layer
    layer_add_child(parent_layer, text_layer_get_layer(s_time_layer));
    
    // Create the TextLayer to display date
    s_date_layer = text_layer_create(date_bounds);
    
    // Improve the layout to be more like a watchface
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_color(s_date_layer, GColorWhite);
    text_layer_set_font(s_date_layer, fonts_get_system_font(DATE_FONT));
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);
    
    // Add it as a child layer to the Window's root layer
    layer_add_child(parent_layer, text_layer_get_layer(s_date_layer));
    
    return text_layer_get_layer(s_date_layer);
}

void time_unload(Window *window) {
    // Destroy TextLayers
    text_layer_destroy(s_date_layer); s_date_layer = NULL;
    text_layer_destroy(s_time_layer); s_time_layer = NULL;
    
    // Unload GFont
    fonts_unload_custom_font(s_time_font); s_time_font = NULL;
}

void update_time(struct tm *tick_time) {
    // Write the current hours and minutes into a buffer
    static char s_time_buffer[8];
    strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ?
             "%H:%M" : "%I:%M", tick_time);
    
    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, s_time_buffer);
    
    static char s_date_buffer[16];
    strftime(s_date_buffer, sizeof(s_date_buffer), "%a %d %b %Y", tick_time);
    
    // Display date on the TextLayer
    text_layer_set_text(s_date_layer, s_date_buffer);
}

void time_init(void) {
    // Make sure the time is displayed from the start
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    update_time(tick_time);
}

void time_deinit(void) {
}
