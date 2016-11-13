//
//  comm.c
//  watchface
//
//  Created by Alexander Linovskiy on 13/11/16.
//
//

#include "comm.h"
#include "weather.h"
#include "bluetooth.h"
#include <pebble.h>

// Schedule the timeout timer
const int timeout_interval_ms = 1000;
const int retry_interval_ms = 2000;

static bool s_js_ready = false;
static AppTimer *s_update_timeout_timer = NULL;
static bool s_comm_retry_when_connected = false;

static bool comm_is_js_ready(void) {
    return s_js_ready;
}

static void comm_postpone_request(void) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Bluetooth is not connected, posponing sending the request.");
    
    s_comm_retry_when_connected = true;
}

static void weather_update_timout_handler(void *context) {
    s_update_timeout_timer = NULL;
    // Retry the update request
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Message send timeout, retrying...");
    comm_send_update_request();
}

void comm_bluetooth_connected(void) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Connected to the phone.");
    if (s_comm_retry_when_connected) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Request has been posponed before, trying again...");

        s_comm_retry_when_connected = false;
        comm_send_update_request();
    }
}

void comm_send_update_request(void) {
    if (!bluetooth_is_connected()) {
        comm_postpone_request();
        return;
    }
        
    if (!comm_is_js_ready()) {
        // do nothing, timeout timer below will do the job
        APP_LOG(APP_LOG_LEVEL_DEBUG, "JS part is not ready to recieve messages, request sending is postponed");
    } else {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Sending update request ...");
        // Send message to the phone
        DictionaryIterator *iter = 0;
        AppMessageResult result = app_message_outbox_begin(&iter);
        if(result == APP_MSG_OK) {
            // send the message requesting weather update
            dict_write_uint8(iter, 0, 0);
            result = app_message_outbox_send();
            if(result != APP_MSG_OK)
                APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d, will retry after timeout...", (int)result);
        } else
            // The outbox cannot be used right now, do nothing, timeour timer will do the job
            APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d, will retry after timeout...", (int)result);
    }
    
    s_update_timeout_timer = app_timer_register(timeout_interval_ms, weather_update_timout_handler, NULL);

}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    // Store incoming information
    static char temperature_buffer[8] = "...";
    static char conditions_buffer[24] = "...";
    static char wind_speed_buffer[4] = "...";
    static char wind_direction_buffer[4] = "...";
        
    Tuple *ready_tuple = dict_find(iterator, MESSAGE_KEY_JSReady);
    if(ready_tuple) {
        // PebbleKit JS is ready! Now it's safe to send messages
        s_js_ready = true;
    }
    
    // Read tuples for data
    
    // Temperature
    Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
    if(temp_tuple) {
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)temp_tuple->value->int32);
    }
    
    // Conditions
    Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);
    if(conditions_tuple) {
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
    }
    
    if (temp_tuple || conditions_tuple) {
        update_weather(temperature_buffer, conditions_buffer);
    }

    // Wind speed string
    Tuple *wind_speed_tuple = dict_find(iterator, MESSAGE_KEY_WIND_SPEED);
    if(wind_speed_tuple) {
        snprintf(wind_speed_buffer, sizeof(wind_speed_buffer), "%d", (int)wind_speed_tuple->value->int32);
    }
    
    // Wind direction string
    Tuple *wind_direction_tuple = dict_find(iterator, MESSAGE_KEY_WIND_DIRECTION);
    if(wind_direction_tuple) {
        snprintf(wind_direction_buffer, sizeof(wind_direction_buffer), "%s", wind_direction_tuple->value->cstring);
    }
    
    if (wind_speed_tuple || wind_direction_tuple) {
        update_wind(wind_direction_buffer, wind_speed_buffer);
    }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Incomning message dropped. Reason: %d", (int)reason);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Message send failed. Reason: %d, will retry after timeout.", (int)reason);
    
    // If message send failed before timer elapsed, clean up the timer
    if(s_update_timeout_timer) {
        app_timer_cancel(s_update_timeout_timer);
        s_update_timeout_timer = NULL;
    }
    
    if (!bluetooth_is_connected()) {
        // Watch is not connected to the phone, postpone update attempts until it's connected back
        comm_postpone_request();
        return;
    }

    // Retry after some interval
    // Use the timeout handler to perform the same action - resend the message
    s_update_timeout_timer = app_timer_register(retry_interval_ms, weather_update_timout_handler, NULL);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Outbox sent successfully, cancel timer");
    // Successful message, the timeout is not needed anymore for this message
    app_timer_cancel(s_update_timeout_timer);
    s_update_timeout_timer = NULL;
}

void comm_init(void) {
    
    // Register callbacks
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
    
    
    // Open AppMessage
    app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
}

void comm_deinit(void) {
    app_message_deregister_callbacks();
}
