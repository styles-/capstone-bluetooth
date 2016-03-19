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
    int sock, client, bytes_read, status;
    unsigned int opt = sizeof(rem_addr);
    uuid_t uuid = NULL;
    if ( uuid_parse(UUID_SPP, uuid) < 0 ) {
        perror("bad uuid");
        exit(EXIT_FAILURE);
    }
    char uuid_str[64] = { 0 };
    uuid_unparse(uuid, uuid_str);
    printf("uuid: [%s]\n", uuid_str);
    
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
        printf("Enter a message: ");
//        sprintf(buf, "%s", )
        scanf("%s", buf);
        
        status = send( client, buf, sizeof(buf), 0 );
        if (status < 0)
            perror("bad status");
        puts("");
//        bytes_read = recv(client, buf, sizeof(buf), 0);
//        if ( bytes_read > 0 )
//            printf("received [%s]\n", buf);
    }
    
    close( client );
    close( sock );
    
    return EXIT_SUCCESS;
}
