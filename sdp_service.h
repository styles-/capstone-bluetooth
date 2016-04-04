//
//  sdp_service.h
//  
//
//  Created by Cody Price on 3/19/16.
//
//

#ifndef sdp_service_h
#define sdp_service_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

sdp_session_t *register_service(const uint8_t rfcomm_channel);
int init_server(const int port);
void read_server(const int client, char *response);
void write_server(const int client, const char *msg);
inline void handleError(char *msg);

#endif /* sdp_service_h */
