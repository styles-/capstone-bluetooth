//
//  rfcomm_client_loop.c
//  
//
//  Created by Cody Price on 3/14/16.
//
//

#include "rfcomm_client_loop.h"
#include "bt_info.h"
#include <time.h>
#include <sys/time.h>

void randomize(char *buf) {
    int i;
    for (i = 0; i < 1007; ++i)
        buf[i] = (random() % 52) + 0x40;
}

int main(int argc, char **argv) {
    
    struct sockaddr_rc addr = { 0 };
    int sock, status;
    char dest[18] = ADDR_BT_PLUG_2;
    
    // allocate a socket
    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    
    // set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = 1;
    str2ba( dest, &addr.rc_bdaddr );
    
    // connect to server
    status = connect( sock, (struct sockaddr *) &addr, sizeof(addr) );
    
    int i = 0;
    // send a message
    char msg[1008] = { 0 };
    if ( status == 0 ) {
        for (i = 0; i < 100; ++i) {
            randomize(msg);
            puts(msg);
            status = write( sock, msg, sizeof(msg) );
            if ( status < 0 )
                perror("bad status");
            usleep(10);
        }
    }
    
    close( sock );
    return EXIT_SUCCESS;
}
