//
//  comm.h
//  watchface
//
//  Created by Alexander Linovskiy on 13/11/16.
//
//

#ifndef comm_h
#define comm_h

#include <pebble.h>
void comm_send_update_request(void);
void comm_bluetooth_connected(void);
void comm_init(void);
void comm_deinit(void);

#endif /* comm_h */
