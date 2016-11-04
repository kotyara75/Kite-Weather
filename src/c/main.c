#include <pebble.h>
#include "battery.h"
#include "bluetooth.h"

static Window *s_main_window = NULL;

static GBitmap *s_background_bitmap = NULL;
static BitmapLayer *s_background_layer = NULL;

static GFont s_time_font = NULL;
static GFont s_weather_font = NULL;
static GFont s_wind_font = NULL;

static TextLayer *s_time_layer = NULL;
static TextLayer *s_date_layer = NULL;
static TextLayer *s_weather_layer = NULL;
static TextLayer *s_wind_layer = NULL;

#define WEATHER_UPDATE_INTERVAL 30 //minutes

#define TIME_LAYER_SIZE 50
#define DATE_FONT FONT_KEY_GOTHIC_14
#define DATE_LAYER_SIZE 15
#define TIME_LAYER_Y PBL_IF_ROUND_ELSE(58, 52)

#define WEATHER_LAYER_Y PBL_IF_ROUND_ELSE(125, 120)
#define WEATHER_FONT_RESOURCE_ID RESOURCE_ID_FONT_PERFECT_DOS_20
#define WEATHER_LAYER_SIZE 25

#define WIND_FONT_RESOURCE_ID RESOURCE_ID_FONT_PERFECT_DOS_20
#define WIND_LAYER_Y PBL_IF_ROUND_ELSE(26, 20)
#define WIND_LAYER_SIZE 25

static void main_window_load(Window *window) {
    // Get information about the Window
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    // Create GBitmap
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
    
    // Create BitmapLayer to display the GBitmap
    s_background_layer = bitmap_layer_create(bounds);
    
    // Set the bitmap onto the layer and add to the window
    bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
    
    // Create GFont
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));
    
    // Create the TextLayer with specific bounds
    s_time_layer = text_layer_create(
                                     GRect(0, TIME_LAYER_Y, bounds.size.w, TIME_LAYER_SIZE));
    
    // Improve the layout to be more like a watchface
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorBlack);
    text_layer_set_font(s_time_layer, s_time_font);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    
    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
    
    // Create the TextLayer to display date
    s_date_layer = text_layer_create(
                                     GRect(0, bounds.size.h - DATE_LAYER_SIZE, bounds.size.w, DATE_LAYER_SIZE));
    // Improve the layout to be more like a watchface
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_color(s_date_layer, GColorWhite);
    text_layer_set_font(s_date_layer, fonts_get_system_font(DATE_FONT));
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);
    
    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
    
    // Create temperature Layer
    s_weather_layer = text_layer_create(
                                        GRect(0, WEATHER_LAYER_Y, bounds.size.w, WEATHER_LAYER_SIZE));
    // Style the text
    text_layer_set_background_color(s_weather_layer, GColorClear);
    text_layer_set_text_color(s_weather_layer, GColorWhite);
    text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
    text_layer_set_text(s_weather_layer, "Loading...");
    // Create second custom font, apply it and add to Window
    s_weather_font = fonts_load_custom_font(resource_get_handle(WEATHER_FONT_RESOURCE_ID));
    text_layer_set_font(s_weather_layer, s_weather_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));

    // Create Wind Layer
    s_wind_layer = text_layer_create(
                                        GRect(0, WIND_LAYER_Y, bounds.size.w-30, WIND_LAYER_SIZE));
    // Style the text
    text_layer_set_background_color(s_wind_layer, GColorClear);
    text_layer_set_text_color(s_wind_layer, GColorWhite);
    text_layer_set_text_alignment(s_wind_layer, GTextAlignmentCenter);
    text_layer_set_text(s_wind_layer, "Loading...");
    // Create second custom font, apply it and add to Window
    s_wind_font = fonts_load_custom_font(resource_get_handle(WIND_FONT_RESOURCE_ID));
    text_layer_set_font(s_wind_layer, s_wind_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_wind_layer));
    
    battery_load(window);
    bluetooth_load(window);

}

static void main_window_unload(Window *window) {
    // Destroy TextLayers
    text_layer_destroy(s_wind_layer); s_wind_layer = NULL;
    text_layer_destroy(s_weather_layer); s_weather_layer = NULL;
    text_layer_destroy(s_date_layer); s_date_layer = NULL;
    text_layer_destroy(s_time_layer); s_time_layer = NULL;
    
    // Unload GFont
    fonts_unload_custom_font(s_wind_font); s_wind_font = NULL;
    fonts_unload_custom_font(s_weather_font); s_weather_font = NULL;
    fonts_unload_custom_font(s_time_font); s_time_font = NULL;
    
    // Destroy BitmapLayer
    bitmap_layer_destroy(s_background_layer); s_background_layer = NULL;
    
    // Destroy GBitmap
    gbitmap_destroy(s_background_bitmap); s_background_bitmap = NULL;
    
    battery_unload(window);
    bluetooth_unload(window);
}

static void update_weather(void) {
    text_layer_set_text(s_wind_layer, "...");
    // Request weather update by seding message to phone
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, 0, 0);
    AppMessageResult r = app_message_outbox_send();
    if(r != APP_MSG_OK)
        APP_LOG(APP_LOG_LEVEL_ERROR, "Can't send message, error code %d", (int)r);
}

static void update_time(struct tm *tick_time) {
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

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time(tick_time);
    // Get weather update every 30 minutes
    if(tick_time->tm_min % WEATHER_UPDATE_INTERVAL == 0) {
        update_weather();
    }
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
    update_weather();
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
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
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
    snprintf(wind_layer_buffer, sizeof(wind_layer_buffer), "%s %s", wind_direction_buffer, wind_speed_buffer);
    text_layer_set_text(s_wind_layer, wind_layer_buffer);
    
    //TODO: display update times
    (void)last_wind_update_time;
    (void)last_weather_update_time;
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox sent successfully.");
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
    
    // Make sure the time is displayed from the start
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    update_time(tick_time);
    
    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    
    // Register accelerator tap
    accel_tap_service_subscribe(accel_tap_handler);
    
    // Init battery gauge
    battery_init();
    
    // Init bluetooth icon
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
