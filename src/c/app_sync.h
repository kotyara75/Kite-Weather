//
//  app_sync.h
//  watchface
//
//  Created by Alexander Linovskiy on 8/11/16.
//
//

#ifndef app_sync_h
#define app_sync_h

#include <pebble.h>

#define SYNC_BUFFER_SIZE APP_MESSAGE_INBOX_SIZE_MINIMUM

void app_sync_load(void);
void app_sync_unload(void);


#endif /* app_sync_h */
