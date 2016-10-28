#include <pebble.h>

static Window *s_main_window;

static GBitmap *s_background_bitmap;
static BitmapLayer *s_background_layer;

static GFont s_time_font;
static GFont s_weather_font;

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_weather_layer;

#define TIME_LAYER_SIZE 50
#define DATE_FONT FONT_KEY_GOTHIC_14
#define DATE_LAYER_SIZE 15
#define TIME_LAYER_Y PBL_IF_ROUND_ELSE(58, 52)
#define WEATHER_LAYER_Y PBL_IF_ROUND_ELSE(125, 120)
#define WEATHER_LAYER_SIZE 25

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
    s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));
    text_layer_set_font(s_weather_layer, s_weather_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));


}

static void main_window_unload(Window *window) {
    // Destroy TextLayers
    text_layer_destroy(s_weather_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_time_layer);
    
    // Unload GFont
    fonts_unload_custom_font(s_weather_font);
    fonts_unload_custom_font(s_time_font);
    
    // Destroy BitmapLayer
    bitmap_layer_destroy(s_background_layer);
    
    // Destroy GBitmap
    gbitmap_destroy(s_background_bitmap);
}

static void update_time() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    
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
    update_time();
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
    update_time();
    
    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
    // Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
