//
//  app_sync.c
//  watchface
//
//  Created by Alexander Linovskiy on 8/11/16.
//
//

// !!!!!!!!!!!!!!
// not optimal - updates all keys on every message from the mobile ...
// removed from main() for a while...
// !!!!!!!!!!!!!!

#include "app_sync.h"

static AppSync s_app_sync;

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
    
    // Store incoming information
    static char temperature_buffer[8] = "...";
    static char conditions_buffer[24] = "...";
    static char weather_layer_buffer[32] = "";
    
    static char wind_speed_buffer[4] = "...";
    static char wind_direction_buffer[4] = "...";
    static char wind_layer_buffer[7] = "";
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App key changed %d", key);
    
    if(key == MESSAGE_KEY_JSReady && new_tuple->value->int32) {
        // PebbleKit JS is ready! Now it's safe to send messages
        // TODO: s_js_ready = true;
        return;
    }
    
    if (key == MESSAGE_KEY_TEMPERATURE) {
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)new_tuple->value->int32);
    } else if (key == MESSAGE_KEY_CONDITIONS) {
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", new_tuple->value->cstring);
    } else if (key == MESSAGE_KEY_WIND_SPEED) {
        snprintf(wind_speed_buffer, sizeof(wind_speed_buffer), "%d", (int)new_tuple->value->int32);
    } else if (key == MESSAGE_KEY_WIND_DIRECTION) {
        snprintf(wind_direction_buffer, sizeof(wind_direction_buffer), "%s", new_tuple->value->cstring);
    }
    
    if (key == MESSAGE_KEY_TEMPERATURE || MESSAGE_KEY_CONDITIONS) {
        // Assemble full weather string and display
        snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s %s", temperature_buffer, conditions_buffer);
        // TODO: text_layer_set_text(s_weather_layer, weather_layer_buffer);
    }
    
    if (key == MESSAGE_KEY_WIND_SPEED || MESSAGE_KEY_WIND_DIRECTION) {
        // Assemble full string and display
        snprintf(wind_layer_buffer, sizeof(wind_layer_buffer), "%s%s", wind_direction_buffer, wind_speed_buffer);
        // TODO: text_layer_set_text(s_wind_layer, wind_layer_buffer);
    }
}

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "App Message Sync Error: %d", app_message_error);
}

void app_sync_load(void) {
    static uint8_t s_sync_buffer[SYNC_BUFFER_SIZE];
    
    // TODO: https://developer.pebble.com/docs/c/Foundation/Storage/
    
    Tuplet initial_values[] = {
        TupletInteger(MESSAGE_KEY_TEMPERATURE, (int32_t) -1),
        TupletCString(MESSAGE_KEY_CONDITIONS, "N/A"),
        TupletInteger(MESSAGE_KEY_WIND_SPEED, (int32_t) -1),
        TupletCString(MESSAGE_KEY_WIND_DIRECTION, "N/A"),
        TupletInteger(MESSAGE_KEY_JSReady, (int32_t) 0)
    };
    
    app_sync_init(&s_app_sync, s_sync_buffer, sizeof(s_sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
                  sync_tuple_changed_callback, sync_error_callback, NULL);
}

void app_sync_unload(void) {
    app_sync_deinit(&s_app_sync);
}
