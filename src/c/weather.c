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

const time_t WEATHER_UPDATE_INTERVAL_SEC = 30 * 60;
// const time_t WEATHER_UPDATE_INTERVAL_SEC = 2 * 60;

// Persitent storage model
const uint32_t current_storage_version = 0;

const uint32_t weather_data_version_key = 0;
const uint32_t weather_text_key = 1;
const uint32_t weather_last_update_key = 2;
const uint32_t wind_text_key = 3;
const uint32_t wind_last_update_key = 4;


// Globals
static GFont s_weather_font = NULL;
static GFont s_wind_font = NULL;

static TextLayer *s_weather_layer = NULL;
static TextLayer *s_wind_layer = NULL;

static char wind_layer_buffer[7] = "...";
static time_t last_wind_update_time = 0;

static char weather_layer_buffer[32] = "...";
static time_t last_weather_update_time = 0;

void update_wind(const char *wind_direction, const char *wind_speed) {

    last_wind_update_time = time(NULL);
    
    // Assemble full string
    snprintf(wind_layer_buffer, sizeof(wind_layer_buffer), "%s%s", wind_direction, wind_speed);
    
    // Cache fresh data to persistent storage
    persist_write_int(wind_last_update_key, last_wind_update_time);
    persist_write_string(wind_text_key, wind_layer_buffer);

    // Display updated wind
    text_layer_set_text(s_wind_layer, wind_layer_buffer);
    
    //TODO: display update time
}

void update_weather(const char *temperature, const char *conditions) {
    
    last_weather_update_time = time(NULL);
    
    // Assemble full weather string
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s %s", temperature, conditions);
    
    // Cache fresh data to persistent storage
    persist_write_int(weather_last_update_key, last_wind_update_time);
    persist_write_string(weather_text_key, weather_layer_buffer);
    
    // Display updated weather
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
    
    //TODO: display update time
}

void weather_request_update(void) {
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
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
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
    text_layer_set_text(s_wind_layer, wind_layer_buffer);
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

uint32_t weather_update_interval_m(void) {
    return WEATHER_UPDATE_INTERVAL_SEC / 60;
}

void weather_init(void) {
    // Check the last storage scheme version the app used
    uint32_t last_storage_version = persist_read_int(weather_data_version_key);
    
    if (last_storage_version != current_storage_version) {
        // intialize persistent storage
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Cache is not initialised, or data version is unsupported");
        persist_write_int(weather_data_version_key, current_storage_version);
    } else {
        // read last update time
        if (persist_exists(weather_last_update_key))
            last_weather_update_time = persist_read_int(weather_last_update_key);
        
        if (persist_exists(wind_last_update_key))
            last_wind_update_time = persist_read_int(weather_last_update_key);
        
        time_t current_time = time(NULL);
        if (current_time - last_weather_update_time < WEATHER_UPDATE_INTERVAL_SEC &&
            current_time - last_wind_update_time < WEATHER_UPDATE_INTERVAL_SEC) {
            // read recently saved data
            persist_read_string(weather_text_key, weather_layer_buffer, sizeof(weather_layer_buffer));
            persist_read_string(wind_text_key, wind_layer_buffer, sizeof(weather_layer_buffer));
            return;
        }
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Cached data too old: wind: %d, weather: %d, time: %d", last_wind_update_time, last_weather_update_time, current_time);
    }
    
    // no freash data found, refresh
    weather_request_update();
}
