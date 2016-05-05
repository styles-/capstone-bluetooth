//
//  Bluetooth.hpp
//  capstone
//
//  Created by Cody Price on 5/4/16.
//  Copyright © 2016 Cody Price. All rights reserved.
//

#ifndef Bluetooth_hpp
#define Bluetooth_hpp

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <sstream>
#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sco.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "bt_info.h"

class BluetoothServer {
private:
    int mId;
    int mRfcommChan;
    int mClient;
    int mPort;
    int mSocket;
    sdp_session_t *mSdpSession;
    bool mSetupSdpSession;
    bool mSdpSessionActive;
    
    void initServer();
    int bindToFirst();
    void registerService();
    void setSocket(int socket);
    void setSetupSdpSession(bool setup_sdp_session);
    void setIsSdpSessionActive(bool is_active);
    void setSdpSession(sdp_session_t* session);
    static int getBluetoothServerId();
    
public:
    static int sId;
    
    BluetoothServer(bool setupSdpSession);
    ~BluetoothServer();
    int getClient();
    void setClient(int client);
    int getPort();
    void setPort(int port);
    int getSocket();
    sdp_session_t* getSdpSession();
    bool doSetupSdpSession();
    bool isSdpSessionActive();
    std::string readMessage();
    void writeMessage(std::string msg);
    void closeServer();
    std::string toString();
};



#endif /* Bluetooth_hpp */
