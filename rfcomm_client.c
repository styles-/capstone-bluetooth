//
//  rfcomm_client.c
//  
//
//  Created by Cody Price on 3/9/16.
//
//

#include "rfcomm_client.h"

int main(int argc, char **argv) {
    
    /**
     * This part is same as simplescan, we just need to get a device
     */
  /*
    inquiry_info *devices = NULL;
    int max_rsp, num_rsp;
    int adapter_id, sock, len, flags;
    int i;
    char addr[19] = { 0 };
    char name[248] = { 0 };
    
    adapter_id = hci_get_route(NULL);
    sock = hci_open_dev( adapter_id );
    if ( adapter_id < 0 || sock < 0 ) {
        perror("opening socket..");
        exit(EXIT_FAILURE);
    }
    
    len = 8;
    max_rsp = 255;
    flags = IREQ_CACHE_FLUSH;
    devices = (inquiry_info *) malloc(max_rsp * sizeof(inquiry_info));
    
    num_rsp = hci_inquiry(adapter_id, len, max_rsp, NULL, &devices, flags);
    if ( num_rsp < 0 )
        perror("hci_inquiry");
    
    for (i = 0; i < num_rsp; ++i) {
        ba2str( &(devices + i)->bdaddr, addr );
        memset(name, 0, sizeof(name)0);
        if ( hci_read_remote_name(sock, &(devices + i)->bdaddr, sizeof(name), name, 0) != 0 )
            strcpy(name, "[unknown]");
        printf("%s %s\n", addr, name);
    }
   
    free( devices );
    
    */
    struct sockaddr_rc addr = { 0 };
    int sock, status;
    char dest[18] = "01:23:45:67:89:AB";
    
    // allocate a socket
    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    
    // set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = 1;
    str2ba( dest, &addr.rc_bdaddr );
    
    // connect to server
    status = connect( sock, (struct sockaddr *) &addr, sizeof(addr) );
    
    // send a message
    char *msg = "what up dog?";
    if ( status == 0 )
        status = send( sock, msg, sizeof(msg), 0 );
    
    if ( status < 0 )
        perror("bad status");

    close( sock );
    return EXIT_SUCCESS;
}