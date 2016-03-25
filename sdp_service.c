//
//  sdp_service.c
//  
//
//  Created by Cody Price on 3/19/16.
//
//

#include "sdp_service.h"
#include "bt_info.h"

//#define UUID_STR "9159BA6D-F5BA-43C6-8C9B-0E318CBD1809"

sdp_session_t* register_service() {
    
}

int main() {
    uint32_t uuid_int[] = { 0x9159BA6D, 0xF5BA43C6, 0x8C9B0E31, 0x8CBD1809 };
    
    uuid_t uu1, uu2;
    if ( uuid_parse(UUID_SPP, u1) < 0 ) {
        perror("bad uuid");
        exit(EXIT_FAILURE);
    }
    
    sdp_uuid128_create( &uu2, &uuid_int );
    
    int test = uuid_compare(uu1, uu2);
    if (test == 0)
        printf
    
    char uuid_str[64] = { 0 };
    uuid_unparse(uuid, uuid_str);
    printf("uuid: [%s]\n", uuid_str);
}