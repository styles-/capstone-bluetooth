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

sdp_session_t *register_service(const uint8_t rfcomm_channel) {
    int32_t svc_uuid_int[] = { 0x9159BA6D, 0xF5BA43C6, 0x8C9B0E31, 0x8CBD1809 };
    //uint32_t svc_uuid_int[] = { 0x00001101, 0x00001000, 0x80000080, 0x5f9b34fb };
    const char *service_name = "My EMG Arm Bluetooth Server";
    const char *svc_dsc = "A server that interfaces with the My EMG Arm Android app";
    const char *service_prov = "MyEmgArm";
    
    uuid_t root_uuid, l2cap_uuid, rfcomm_uuid, svc_uuid, svc_class_uuid;
    sdp_list_t  *l2cap_list = 0,
                *rfcomm_list = 0,
                *root_list = 0,
                *proto_list = 0,
                *access_proto_list = 0,
                *svc_class_list = 0,
                *profile_list = 0;
    sdp_data_t *channel = 0;
    sdp_profile_desc_t profile;
    sdp_record_t record = { 0 };
    sdp_session_t *session = 0;
    
    // set the general service ID
    sdp_uuid128_create(&svc_uuid, &svc_uuid_int);
    sdp_set_service_id(&record, svc_uuid);
    
    char str[256] = "";
    sdp_uuid2strn(&svc_uuid, str, 256);
    printf("Registering UUID %s\n", str);
    
    // set the service class
    sdp_uuid16_create(&svc_class_uuid, SERIAL_PORT_SVCLASS_ID);
    svc_class_list = sdp_list_append(0, &svc_class_uuid);
    sdp_set_service_classes(&record, svc_class_list);
    
    // set the Bluetooth profile information
    sdp_uuid16_create(&profile.uuid, SERIAL_PORT_PROFILE_ID);
    profile.version = 0x1000;
    profile_list = sdp_list_append(0, &profile);
    sdp_set_profile_descs(&record, profile_list);
    
    // make the service record publicly browsable
    sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
    root_list = sdp_list_append(0, &root_uuid);
    sdp_set_browse_groups(&record, root_list);
    
    // set l2cap information
    sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
    l2cap_list = sdp_list_append(0, &l2cap_uuid);
    proto_list = sdp_list_append(0, l2cap_list);
    
    // register the RFCOMM channel for RFCOMM sockets
    sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
    channel = sdp_data_alloc(SDP_UINT8, &rfcomm_channel);
    rfcomm_list = sdp_list_append(0, &rfcomm_uuid);
    sdp_list_append(rfcomm_list, channel);
    sdp_list_append(proto_list, rfcomm_list);
    
    access_proto_list = sdp_list_append(0, proto_list);
    sdp_set_access_protos(&record, access_proto_list);
    
    // set the name, provider, and description
    sdp_set_info_attr(&record, service_name, service_prov, svc_dsc);
    
    // connect to the local SDP server, register the service record, and disconnect
    session = sdp_connect(BDADDR_ANY, BDADDR_LOCAL, SDP_RETRY_IF_BUSY);
    if (session == NULL) {
        fprintf(stderr, "Error: (sdp_session_t *) session == null -> %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    } else {
        sdp_record_register(session, &record, 0);
    }
    
    // cleanup
    sdp_data_free(channel);
    sdp_list_free(l2cap_list, 0);
    sdp_list_free(rfcomm_list, 0);
    sdp_list_free(root_list, 0);
    sdp_list_free(access_proto_list, 0);
    sdp_list_free(svc_class_list, 0);
    sdp_list_free(profile_list, 0);
    
    return session;
}

int init_server(const int port) {
    int result, sock, client;
//    int result, sock, client, bytes_read, bytes_sent;
    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    char buffer[1024] = { 0 };
    socklen_t opt = sizeof(rem_addr);
    
    // local bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t) port;
    
    // register service
    sdp_session_t *session = register_service(port);
    
    // allocate socket
    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    printf("socket() returned %d\n", sock);
    
    // bind socket to port 3 of the first available
    result = bind(sock, (struct sockaddr *) &loc_addr, sizeof(loc_addr));
    printf("bind() on channel %d return %d\n", port, result);
    
    // put socket into listening mode
    result = listen(sock, 1);
    printf("listen() returned %d\n", result);
    
    //sdpRegisterL2cap(port)
    
    // accept one connection
    printf("calling accept()\n");
    client = accept(sock, (struct sockaddr *) &rem_addr, &opt);
    printf("accept() returned %d\n", client);
    
    ba2str(&rem_addr.rc_bdaddr, buffer);
    fprintf(stderr, "accepted connection from %s\n", buffer);
//    memset(buffer, 0, sizeof(buffer));
    
    return client;
}

void read_server(const int client, char *response) {
    // read data from the client
//    char input[1024] = { 0 };
//    int bytes_read = read(client, input, sizeof(input));
    int bytes_read = read(client, response, sizeof(response));
    if (bytes_read > 0) {
        printf("recieved [%s]\n", response);
//        printf("recieved [%s]\n", input);
//        return input;
//    } else {
//        return "";
    }
}

void write_server(const int client, const char *msg) {
    // send data to the client
    char msgArray[1024] = { 0 };
    int bytes_sent;
    sprintf(msgArray, msg);
    bytes_sent = write(client, msgArray, sizeof(msgArray));
    if (bytes_sent > 0)
        printf("sent [%s]\n", msgArray);
}

int main() {
    
#ifdef DEBUG
    puts("Debugging enabled..");
#endif
    
    int client = init_server(14);
    sleep(5);
    getchar();
//    sdp_close( session );
    close(client);
    return EXIT_SUCCESS;
//    if ( uuid_parse(UUID_SPP, uu1) < 0 ) {
//        perror("bad uuid");
//        exit(EXIT_FAILURE);
//    }
    
//    sdp_uuid128_create( &uu2, &uuid_int );
    
//    int test = uuid_compare(uu1, uu2);
//    if (test == 0)
//        printf("same uuid\n");
//    else
//        printf("different uuid\n");
    
//    char uuid_str[64] = { 0 };
//    uuid_unparse(uuid, uuid_str);
//    printf("uuid: [%s]\n", uuid_str);
}
