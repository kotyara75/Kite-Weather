#include <pebble.h>
#include "battery.h"
#include "bluetooth.h"
#include "ph_bay.h"
#include "time.h"
#include "comm.h"
#include "weather.h"

static Window *s_main_window = NULL;

#define WEATHER_UPDATE_INTERVAL 30 //minutes

static void main_window_load(Window *window) {
    // Get information about the Window
    Layer *window_layer = window_get_root_layer(window);
    
    Layer* ph_bay_layer = ph_bay_load(window_layer);
    Layer* time_layer = time_load(window_layer);

    weather_load(ph_bay_layer);
    battery_load(time_layer);
    bluetooth_load(ph_bay_layer);
}

static void main_window_unload(Window *window) {
    weather_unload(window);
    time_unload(window);
    ph_bay_unload(window);
    battery_unload(window);
    bluetooth_unload(window);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time(tick_time);
    // Get weather update every 30 minutes
    if((tick_time->tm_min % WEATHER_UPDATE_INTERVAL == 0)) {
        request_weather_update();
    }
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
    request_weather_update();
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
    comm_init();
}

static void deinit() {
    tick_timer_service_unsubscribe();
    accel_tap_service_unsubscribe();
    
    comm_deinit();
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
