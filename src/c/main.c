#include <pebble.h>
#include "battery.h"
#include "bluetooth.h"
#include "ph_bay.h"
#include "time.h"


bool comm_is_js_ready(void);
void weather_update(void);

static Window *s_main_window = NULL;

static GFont s_weather_font = NULL;
static GFont s_wind_font = NULL;

static TextLayer *s_weather_layer = NULL;
static TextLayer *s_wind_layer = NULL;

#define WEATHER_UPDATE_INTERVAL 30 //minutes

#define WEATHER_LAYER_Y 1
#define WEATHER_FONT_RESOURCE_ID RESOURCE_ID_FONT_PERFECT_DOS_20
#define WEATHER_LAYER_SIZE 21
#define WEATER_LAYER_SHIFT 5

#define WIND_FONT_RESOURCE_ID RESOURCE_ID_FONT_PERFECT_DOS_20
#define WIND_LAYER_Y WEATHER_LAYER_Y + WEATHER_LAYER_SIZE
#define WIND_LAYER_SIZE 20
#define WIND_LAYER_SHIFT 45


static void main_window_load(Window *window) {
    // Get information about the Window
    Layer *window_layer = window_get_root_layer(window);
    
    Layer* ph_bay_layer = ph_bay_load(window_layer);
    Layer* time_layer = time_load(window_layer);
    
    GRect ph_bounds = layer_get_bounds(ph_bay_layer);
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
    layer_add_child(ph_bay_layer, text_layer_get_layer(s_weather_layer));

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
    layer_add_child(ph_bay_layer, text_layer_get_layer(s_wind_layer));
    
    battery_load(time_layer);
    bluetooth_load(ph_bay_layer);
}

static void main_window_unload(Window *window) {
    // Destroy TextLayers
    text_layer_destroy(s_wind_layer); s_wind_layer = NULL;
    text_layer_destroy(s_weather_layer); s_weather_layer = NULL;

    // Unload GFont
    fonts_unload_custom_font(s_wind_font); s_wind_font = NULL;
    fonts_unload_custom_font(s_weather_font); s_weather_font = NULL;
    
    time_unload(window);
    ph_bay_unload(window);
    battery_unload(window);
    bluetooth_unload(window);
}

static void weather_update_timout_handler(void *context) {
    // Retry the update request
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Message send timeout, retrying...");
    weather_update();
}

static AppTimer *s_update_timeout_timer = 0;

void weather_update(void) {
    text_layer_set_text(s_wind_layer, "...");
    
    if (!comm_is_js_ready()) {
        // do nothing, timeout timer below will do the job
        APP_LOG(APP_LOG_LEVEL_DEBUG, "JS part is not ready to recieve messages, weather update is postponed");
    } else {
        // Request weather update by seding message to phone
        DictionaryIterator *iter = 0;
        AppMessageResult result = app_message_outbox_begin(&iter);
        if(result == APP_MSG_OK) {
            // send the message requesting weather update
            dict_write_uint8(iter, 0, 0);
            result = app_message_outbox_send();
            if(result != APP_MSG_OK)
                APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
        } else
            // The outbox cannot be used right now, do nothing, timeour timer will do the job
            APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
    }
    
    // Schedule the timeout timer
    const int interval_ms = 1000;
    s_update_timeout_timer = app_timer_register(interval_ms, weather_update_timout_handler, NULL);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time(tick_time);
    // Get weather update every 30 minutes
    if((tick_time->tm_min % WEATHER_UPDATE_INTERVAL == 0)) {
        weather_update();
    }
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
    weather_update();
}

static bool s_js_ready = false;
bool comm_is_js_ready() {
    return s_js_ready;
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    // Store incoming information
    static char temperature_buffer[8] = "...";
    static char conditions_buffer[24] = "...";
    static char weather_layer_buffer[32] = "";

    static char wind_speed_buffer[4] = "...";
    static char wind_direction_buffer[4] = "...";
    static char wind_layer_buffer[7] = "";
    
    static time_t last_weather_update_time = 0;
    static time_t last_wind_update_time = 0;
    
    Tuple *ready_tuple = dict_find(iterator, MESSAGE_KEY_JSReady);
    if(ready_tuple) {
        // PebbleKit JS is ready! Now it's safe to send messages
        s_js_ready = true;
    }
    
    // Read tuples for data
    Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
    Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);
    Tuple *wind_speed_tuple = dict_find(iterator, MESSAGE_KEY_WIND_SPEED);
    Tuple *wind_direction_tuple = dict_find(iterator, MESSAGE_KEY_WIND_DIRECTION);
    
    // Display WEATHER
    
    // Temperature
    if(temp_tuple) {
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)temp_tuple->value->int32);
        last_weather_update_time = time(NULL);
    }
    
    // Conditions
    if(conditions_tuple) {
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
        last_weather_update_time = time(NULL);
    }
    
    // Assemble full weather string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
    
    
    // Display WIND
    
    // Wind speed string
    if(wind_speed_tuple) {
        snprintf(wind_speed_buffer, sizeof(wind_speed_buffer), "%d", (int)wind_speed_tuple->value->int32);
        last_wind_update_time = time(NULL);
    }
   
    // Wind direction string
    if(wind_direction_tuple) {
        snprintf(wind_direction_buffer, sizeof(wind_direction_buffer), "%s", wind_direction_tuple->value->cstring);
        last_wind_update_time = time(NULL);
    }
    
    // Assemble full string and display
    snprintf(wind_layer_buffer, sizeof(wind_layer_buffer), "%s%s", wind_direction_buffer, wind_speed_buffer);
    text_layer_set_text(s_wind_layer, wind_layer_buffer);
    
    //TODO: display update times
    (void)last_wind_update_time;
    (void)last_weather_update_time;
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped. Reason: %d", (int)reason);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Message send failed. Reason: %d, retrying", (int)reason);
    
    // Message failed before timer elapsed, reschedule for later
    if(s_update_timeout_timer) {
        app_timer_cancel(s_update_timeout_timer);
        s_update_timeout_timer = NULL;
    }
    
    // Use the timeout handler to perform the same action - resend the message
    const int retry_interval_ms = 500;
    s_update_timeout_timer = app_timer_register(retry_interval_ms, weather_update_timout_handler, NULL);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Outbox sent successfully, cancel timer");
    // Successful message, the timeout is not needed anymore for this message
    app_timer_cancel(s_update_timeout_timer);
}

static void init() {
    // Create main Window element and assign to pointer
    s_main_window = window_create();
    
    // Set handlers to manage the elements inside the Window
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });
    
    // Set background color
    window_set_background_color(s_main_window, GColorBlack);
    
    // Show the Window on the watch, with animated=true
    window_stack_push(s_main_window, true);
    
    time_init();
    
    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    
    // Register accelerator tap
    accel_tap_service_subscribe(accel_tap_handler);
    
    ph_bay_init();
    battery_init();
    bluetooth_init();
    
    // Register callbacks
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
 

    // Open AppMessage
    app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
}

static void deinit() {
    app_message_deregister_callbacks();
    tick_timer_service_unsubscribe();
    accel_tap_service_unsubscribe();
    
    time_deinit();
    ph_bay_deinit();
    battery_deinit();
    bluetooth_deinit();
    
    // Destroy Window
    window_destroy(s_main_window); s_main_window = NULL;
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
