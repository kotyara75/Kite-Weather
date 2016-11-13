//
//  weather.c
//  watchface
//
//  Created by Alexander Linovskiy on 13/11/16.
//
//

#include "weather.h"
#include <pebble.h>
#include "comm.h"


#define WEATHER_LAYER_Y 1
#define WEATHER_FONT_RESOURCE_ID RESOURCE_ID_FONT_PERFECT_DOS_20
#define WEATHER_LAYER_SIZE 21
#define WEATER_LAYER_SHIFT 5

#define WIND_FONT_RESOURCE_ID RESOURCE_ID_FONT_PERFECT_DOS_20
#define WIND_LAYER_Y WEATHER_LAYER_Y + WEATHER_LAYER_SIZE
#define WIND_LAYER_SIZE 20
#define WIND_LAYER_SHIFT 45

static GFont s_weather_font = NULL;
static GFont s_wind_font = NULL;

static TextLayer *s_weather_layer = NULL;
static TextLayer *s_wind_layer = NULL;

void update_wind(const char *wind_direction, const char *wind_speed) {
    static char wind_layer_buffer[7] = "";
    static time_t last_wind_update_time = 0;
    
    last_wind_update_time = time(NULL);
    
    // Assemble full string and display
    snprintf(wind_layer_buffer, sizeof(wind_layer_buffer), "%s%s", wind_direction, wind_speed);
    text_layer_set_text(s_wind_layer, wind_layer_buffer);
    
    //TODO: display update times
    (void)last_wind_update_time;
}

void update_weather(const char *temperature, const char *conditions) {
    static char weather_layer_buffer[32] = "";
    static time_t last_weather_update_time = 0;
    
    last_weather_update_time = time(NULL);
    
    // Assemble full weather string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s %s", temperature, conditions);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
    
    //TODO: display update times
    (void)last_weather_update_time;
}

void request_weather_update(void) {
    text_layer_set_text(s_wind_layer, "...");

    comm_send_update_request();
}

Layer* weather_load(Layer *parent_layer) {
    
    GRect ph_bounds = layer_get_bounds(parent_layer);
    GPoint center = grect_center_point(&ph_bounds);
    GRect weather_bounds = GRect(WEATER_LAYER_SHIFT, WEATHER_LAYER_Y, ph_bounds.size.w-WEATER_LAYER_SHIFT, WEATHER_LAYER_SIZE);
    GRect wind_bounds = GRect(WIND_LAYER_SHIFT, center.y - WIND_LAYER_SIZE / 2, ph_bounds.size.w-WIND_LAYER_SHIFT, WIND_LAYER_SIZE);
    
    // Create temperature Layer
    s_weather_layer = text_layer_create(weather_bounds);
    // Style the text
    text_layer_set_background_color(s_weather_layer, GColorClear);
    text_layer_set_text_color(s_weather_layer, GColorBlack);
    text_layer_set_text_alignment(s_weather_layer, GTextAlignmentLeft);
    text_layer_set_text(s_weather_layer, "Loading...");
    // Create second custom font, apply it and add to Window
    s_weather_font = fonts_load_custom_font(resource_get_handle(WEATHER_FONT_RESOURCE_ID));
    text_layer_set_font(s_weather_layer, s_weather_font);
    layer_add_child(parent_layer, text_layer_get_layer(s_weather_layer));
    
    // Create Wind Layer
    s_wind_layer = text_layer_create(wind_bounds);
    // Style the text
    text_layer_set_background_color(s_wind_layer, GColorClear);
    text_layer_set_text_color(s_wind_layer, GColorBlack);
    text_layer_set_text_alignment(s_wind_layer, GTextAlignmentCenter);
    text_layer_set_text(s_wind_layer, "Loading...");
    // Create second custom font, apply it and add to Window
    s_wind_font = fonts_load_custom_font(resource_get_handle(WIND_FONT_RESOURCE_ID));
    text_layer_set_font(s_wind_layer, s_wind_font);
    
    Layer * layer = text_layer_get_layer(s_wind_layer);
    layer_add_child(parent_layer, layer);
    return layer;
}

void weather_unload(Window *window) {
    // Destroy TextLayers
    text_layer_destroy(s_wind_layer); s_wind_layer = NULL;
    text_layer_destroy(s_weather_layer); s_weather_layer = NULL;
    
    // Unload GFont
    fonts_unload_custom_font(s_wind_font); s_wind_font = NULL;
    fonts_unload_custom_font(s_weather_font); s_weather_font = NULL;
}
