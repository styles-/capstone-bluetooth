//
//  android_server.c
//  
//
//  Created by Cody Price on 3/19/16.
//
//

#include "android_server.h"

int main(int argc, char **argv) {
    
    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    char buf[1024] = { 0 };
    int sock, client, bytes_read;
    unsigned int opt = sizeof(rem_addr);
    
    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = 1;
    bind(sock, (struct sockaddr *)  &loc_addr, sizeof(loc_addr));
    
    listen(sock, 1);
    
    client = accept(sock, (struct sockaddr *) &rem_addr, &opt);
    
    ba2str( &rem_addr.rc_bdaddr, buf );
    fprintf(stderr, "accepted connection from %s\n", buf);
    
    while ( strcmp(buf, "exit") != 0) {
        memset(buf, 0, sizeof(buf));
        
        bytes_read = recv(client, buf, sizeof(buf), 0);
        if ( bytes_read > 0 )
            printf("received [%s]\n", buf);
    }
    
    close( client );
    close( sock );
}
