//
//  Bluetooth.cpp
//  capstone
//
//  Created by Cody Price on 5/4/16.
//  Copyright © 2016 Cody Price. All rights reserved.
//

#include "Bluetooth.hpp"

BluetoothServer::BluetoothServer(bool setupSdpSession ) {
    mId = getBluetoothServerId();
    initServer();
    if ( setupSdpSession ) {
        registerService();
    }
}

BluetoothServer::~BluetoothServer() {
    sdp_close( mSdpSession );
    close( mSocket );
    close( mClient );
}

void BluetoothServer::closeServer() {
    std::cout << "Closing bluetooth server " << mId << std::endl;
    delete(this);
}

int BluetoothServer::getBluetoothServerId() {
    return sId++;
}

void BluetoothServer::initServer() {
    int result, sock, client;
    struct sockaddr_rc rem_addr = { 0 };
    char buffer[1024] = { 0 };
    socklen_t opt = sizeof(rem_addr);
    
    sock = bindToFirst();
    std::cout << "Socket = " << sock << std::endl;
    setSocket(sock);
    
    // put socket into listening mode
    std::cout << "Calling listen() on socket " << sock << std::endl;
    result = listen(sock, 1);
    
    std::cout << "Calling accept() on socket " << sock << std::endl;
    client = accept(sock, (struct sockaddr *) &rem_addr, &opt);
    std::cout << "accept() returned " << client << std::endl;
    setClient(client);
    
    ba2str(&rem_addr.rc_bdaddr, buffer);
    std::cout << "Accepted connection from " << buffer << std::endl;
}

int BluetoothServer::bindToFirst() {
    int sock, port, status;
    struct sockaddr_rc to_bind;
    sock = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );
    if (sock < 0) {
        std::cout << "Error: socket() unsuccessful" << std::endl;
        return -1;
    }
    //      handleError("Error: socket() unsuccessful");
/*
    printf("socket() was successful and returned %d\n", sock);
*/
    bdaddr_t my_bdaddr_any = {{0, 0, 0, 0, 0, 0}};
    to_bind.rc_family = AF_BLUETOOTH;
    to_bind.rc_bdaddr = my_bdaddr_any;
    for ( port = 1; port <= 30; ++port ) {
        to_bind.rc_channel = port;
        status = bind(sock, (struct sockaddr *) &to_bind, sizeof(to_bind));
        if ( status == 0 )
            break;
    }
    if ( port > 30 )
        return -1;
    else {
/*
        printf("bind() on channel %d was successful\n", port);
*/
        return sock;
    }
}

void BluetoothServer::registerService() {
    bdaddr_t my_bdaddr_any = {{0, 0, 0, 0, 0, 0}};
    bdaddr_t my_bdaddr_local = {{0x0, 0x0, 0x0, 0xff, 0xff, 0xff}};

    uint32_t svc_uuid_int[] = UUID_RFCOMM_VAL;
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
    std::cout << "Registering UUID " << str << std::endl;
    
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
    
    int rfcomm_channel = (rand() % 30) + 1;
    channel = sdp_data_alloc(SDP_UINT8, &rfcomm_channel);
    for (int i = 0; i < 4; ++i) {
        if ( channel == NULL ) {
            rfcomm_channel = (rand() % 30) + 1;
            channel = sdp_data_alloc(SDP_UINT8, &rfcomm_channel);
        } else {
            break;
        }
    }
    if ( channel == NULL ) {
        setIsSdpSessionActive(false);
        return;
    }
    
    rfcomm_list = sdp_list_append(0, &rfcomm_uuid);
    sdp_list_append(rfcomm_list, channel);
    sdp_list_append(proto_list, rfcomm_list);
    
    access_proto_list = sdp_list_append(0, proto_list);
    sdp_set_access_protos(&record, access_proto_list);
    
    // set the name, provider, and description
    sdp_set_info_attr(&record, service_name, service_prov, svc_dsc);
    
    // connect to the local SDP server, register the service record, and disconnect
    session = sdp_connect(&my_bdaddr_any, &my_bdaddr_local, SDP_RETRY_IF_BUSY);
    if (session == NULL)
        exit(EXIT_FAILURE);
    //handleError("Error: (sdp_session_t *) session == null");
    else
        sdp_record_register(session, &record, 0);
    
    // cleanup
    sdp_data_free(channel);
    sdp_list_free(l2cap_list, 0);
    sdp_list_free(rfcomm_list, 0);
    sdp_list_free(root_list, 0);
    sdp_list_free(access_proto_list, 0);
    sdp_list_free(svc_class_list, 0);
    sdp_list_free(profile_list, 0);
    
    setSdpSession( session );
    setIsSdpSessionActive( true );
}

void BluetoothServer::setSocket(int socket) {
    this->mSocket = socket;
}

void BluetoothServer::setSetupSdpSession(bool setup_sdp_session) {
    this->mSetupSdpSession = setup_sdp_session;
}

void BluetoothServer::setIsSdpSessionActive(bool is_active) {
    this->mSdpSessionActive = is_active;
}

void BluetoothServer::setSdpSession(sdp_session_t* session) {
    this->mSdpSession = session;
}

int BluetoothServer::getClient() {
    return this->mClient;
}

void BluetoothServer::setClient(int client) {
    this->mClient = client;
}

int BluetoothServer::getPort() {
    return this->mPort;
}

void BluetoothServer::setPort(int port) {
    this->mPort = port;
}

int BluetoothServer::getSocket() {
    return this->mSocket;
}

sdp_session_t* BluetoothServer::getSdpSession() {
    return this->mSdpSession;
}

bool BluetoothServer::doSetupSdpSession() {
    return this->mSetupSdpSession;
}

bool BluetoothServer::isSdpSessionActive() {
    return this->mSdpSessionActive;
}

std::string BluetoothServer::readMessage() {
    char response[1024];
    int bytes_read = read(client, response, sizeof(response));
    
    if (bytes_read > 0) {
/*
        printf("recieved [%s]\n", response);
 */
        return string(response);
    } else {
        std::cout << "Nothing received from client!" << std::endl;
        return "";
    }
}

void BluetoothServer::writeMessage(std::string msg) {
    if ( msg == nullptr || msg.length() == 0 ) {
        std::cout << "Nothing to send to the client!";
        return;
    }
    
    char msgArray[1024] = { 0 };
    int bytes_sent;
    //std::string s
    memcpy(msgArray, s.c_str(), s.size());
    msgArray[1023] = 0;
    
    bytes_sent = write( client, )
}

std::string BluetoothServer::toString() {
    stringstream ss;
    
    return ss.str();
}
