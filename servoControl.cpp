#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <cstdio>

#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sco.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "rfcomm_server.h"
#include "bt_info.h"

#define THUMB_OPEN 170
#define THUMB_CLOSE 60
#define INDEX_OPEN 150
#define INDEX_CLOSE 60				//may need testing
#define AUX_OPEN 200
#define AUX_CLOSE 140
//#define BDADDR_ANY_INITIALIZER   {{0, 0, 0, 0, 0, 0}}
//#define BDADDR_ANY   (&(bdaddr_t) BDADDR_ANY_INITIALIZER)

using namespace std;

class Servo {
	private:
		int servoNum;
		int startPos;
		int currPos;
		static int totalServos;
	public:
		Servo (int serv, int startPos);
		Servo();
		void setServo(int pulseWidth, ofstream *file);
		~Servo(); 
};

int Servo::totalServos = 0;

Servo::Servo(int serv, int startPos){
	this -> servoNum = serv;
	this -> startPos = startPos;
	this -> currPos = startPos;
	Servo::totalServos++;
}

Servo::Servo(){
	this -> servoNum = 0;
	this -> startPos = 100;
	this -> currPos = 100;
}

void Servo::setServo(int pulseWidth, ofstream *file){
	currPos = pulseWidth;
	*file << this->servoNum << "=" << this->currPos << endl;
}
Servo::~Servo(){
	cout << "Servo " << this -> servoNum << "destroyed." << endl;
}
/*
int main(){
	
	bool running = true;
	ofstream writer ("/dev/servoblaster");
	if (!writer){
		cout << "Failure to open file." << endl;
	}
	else {
		cout << "Success opening file" << endl;
	}
	cout << "yay" << endl;
	cout << flush;
	Servo thumb(0,100);
	Servo finger(1,100);
	Servo aux(4,100);
	
//	int pos[5] = {90,100,150,200,250};
	int x = 100;
	int y=x;
	while (running){
	//			y=pos[x%5];
				if (y<250){
					y += 2;
				}
				else{ 
					y = 80;
				}
	//			thumb.setServo(y,&writer);
		//		finger.setServo(y,&writer);
		//		aux.setServo(y,&writer);
		//		usleep(10000);
				sleep(5);
		//		x++;
	}


return 0;
}*/

sdp_session_t *register_service(const uint8_t rfcomm_channel);
int init_server(const int port);
void read_server(const int client, char *response);
void write_server(const int client, const char *msg);
int getBoundSocketOnFirstPort();
int bindToFirst();

/**
 * register_service
 * args:
 *  - rfcomm_channel:
 * returns:
 *  - sdp_session_t*
 * description:
 *  - 
 */
sdp_session_t *register_service(const uint8_t rfcomm_channel) {
	bdaddr_t my_bdaddr_any = {{0, 0, 0, 0, 0, 0}};
	bdaddr_t my_bdaddr_local = {{0x0, 0x0, 0x0, 0xff, 0xff, 0xff}};
    //uint32_t svc_uuid_int[] = UUID_MEA_VAL;
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
    #ifdef DEBUG
    printf("Registering UUID %s\n", str);
    #endif
    
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
    
    return session;
}

/**
 * init_server
 * args:
 *  - port:
 * returns:
 *  - 
 * description:
 *  - 
 */
int init_server(const int port) {
    int result, sock, client;
    //struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    struct sockaddr_rc rem_addr = { 0 };
    char buffer[1024] = { 0 };
    socklen_t opt = sizeof(rem_addr);
    
    sock = bindToFirst();
    cout << "Socket = " << sock << endl;
    
    // put socket into listening mode
    result = listen(sock, 1);
  //  if (result != 0)
  //      handleError("Error: listen() unsuccessful");
    #ifdef DEBUG
    puts("listen() was successful");
    #endif
    
    // accept one connection
    #ifdef DEBUG
    puts("calling accept()..\n");
    #endif
    client = accept(sock, (struct sockaddr *) &rem_addr, &opt);
//if (client < 0)
 //       handleError("Error: unable to accept Bluetooth socket");
    #ifdef DEBUG
    printf("accept() returned %d\n", client);
    #endif
    
    ba2str(&rem_addr.rc_bdaddr, buffer);
    fprintf(stderr, "accepted connection from %s\n", buffer);
//    memset(buffer, 0, sizeof(buffer));
   
    return client;
}

int bindToFirst() {
	int sock, port, status;
	struct sockaddr_rc to_bind;
	sock = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );
	if (sock < 0) 
		cout << "Error: socket() unsuccessful" << endl;
  //      handleError("Error: socket() unsuccessful");
    #ifdef DEBUG
    printf("socket() was successful and returned %d\n", sock);
    #endif
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
		#ifdef DEBUG
		printf("bind() on channel %d was successful\n", port);
		#endif
		return sock;
	}
}

/**
 * read_server
 * args:
 *  - client:
 *  - response:
 * returns:
 *  - void
 * description:
 *  - read data from the client
 */
void read_server(const int client, char *response) {
    int bytes_read = read(client, response, sizeof(response));

    if (bytes_read > 0) {
        #ifdef DEBUG
        printf("recieved [%s]\n", response);
        #endif
    } else {
        #ifdef DEBUG
        puts("Nothing received from client!");
        #endif
    }
}

/**
 * write_server
 * args:
 *  - client:
 *  - msg:
 * returns:
 *  - void
 * description:
 *  - send data to the client
 */
void write_server(const int client, const char *msg) {
    if (strlen(msg) == 0) {
        #ifdef DEBUG
        puts("Nothing to send to the client!");
        #endif
        return;
    }

    char msgArray[1024] = { 0 };
    int bytes_sent;
    sprintf(msgArray, msg);
    bytes_sent = write(client, msgArray, sizeof(msgArray));
    if (bytes_sent > 0) {
        #ifdef DEBUG
        printf("sent [%s]\n", msgArray);
        #endif
    } //else {
       // handleError("Error: couldn't send data to client");
    //}
}

/**
 * handleError
 * args:
 *  - msg:
 * returns:
 *  - void
 * description:
 *  - print custom error message and errno string and then exit()
 */
//inline void handleError(string msg) {
//    fprintf(stderr, "%s -> %s(%d)\n", msg, strerror(errno), errno);
//    exit(EXIT_FAILURE);
//}

//inline void printError(string msg) {
 //   fprintf(stderr, "%s -> %s(%d)\n", msg, strerror(errno), errno);
//}

int getBoundSocketOnFirstPort() {
	bdaddr_t my_bdaddr_any = {{0, 0, 0, 0, 0, 0}};
	int sock, status;
	struct sockaddr_rc to_bind;
	sock = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );
	to_bind.rc_family = AF_BLUETOOTH;
	to_bind.rc_bdaddr = my_bdaddr_any;
	to_bind.rc_channel = 0;
	
	status = bind( sock, (struct sockaddr *) &to_bind, sizeof(to_bind) );
	if ( status == 0 ) {
		#ifdef DEBUG
		struct sockaddr_in sa;
		socklen_t sa_len = sizeof(sa);
		if ( getsockname(sock, (struct sockaddr *) &sa, &sa_len) == -1 ) {
			//printError("Error: getsockname() on Bluetooth socket failed");
			cout << "Error: getsockname() on Bluetooth socket failed" << endl;
		} else {
			fprintf(stderr, "Bluetooth socket bound to port %d\n", (int) ntohs(sa.sin_port));
		}
		#endif
		
		return sock;
	} else {
		return -1;
	}
}

int main(int argc, char **argv) {
	
    ofstream writer ("/dev/servoblaster");
	if (!writer){
		cout << "Failure to open file." << endl;
	}
	else {
		cout << "Success opening file" << endl;
	}
	
	Servo thumb(4, THUMB_OPEN);
	Servo index(3, INDEX_OPEN);
	Servo aux(1, AUX_OPEN);
	cout << "before anything" << endl;
	fflush(stdout);
    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
	char buf[1024] = { 0 };
	char buffer1;
    int sock, client, bytes_read;
    unsigned int opt = sizeof(rem_addr);
    puts("Socket init done\n");
    /*
    uuid_t uuid = NULL;
    if ( uuid_parse( UUID_STR, uuid) != 0 ) {
        fprintf(stderr, "uuid parse failed");
        exit(EXIT_FAILURE);
    }
    char uuid_str[64] = { 0 };
    uuid_unparse(uuid, uuid_str);
    printf("uuid: [%s]\n", uuid_str); */
    
    int port = 1;
    sdp_session_t *session = register_service(port); 
    client = init_server(port);

 /*   
    //allocate socket
    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    puts("Socket created\n");
    // bind socket to port 1 of the first available bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
	bdaddr_t my_bdaddr_any = {{0, 0, 0, 0, 0, 0}};
    loc_addr.rc_bdaddr = my_bdaddr_any;
//	loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = 1;
    bind(sock, (struct sockaddr *) &loc_addr, sizeof(loc_addr));
 
    // put socket into listening mode
    listen(sock, 1);
    
    // accept one connection
    client = accept(sock, (struct sockaddr *) &rem_addr, &opt);
    puts("right before badass stream");
    ba2str( &rem_addr.rc_bdaddr, buf );
    fprintf(stderr, "accepted connection from %s\n", buf);*/
  //  	Servo thumb;
//	Servo index;
//	Servo aux;
    while (1) {
   //     memset(buf, 0, sizeof(buf));
       // buffer1 = 0;
        
        // read data from the client
        bytes_read = recv(client, &buffer1, sizeof(buffer1), 0);
        if ( bytes_read > 0 ){
            printf("received [%d]\n", buffer1);
            switch(buffer1){
				case '1':
						thumb.setServo(THUMB_OPEN,&writer);
					//	usleep(250);
					
						index.setServo(INDEX_OPEN,&writer);
					//	usleep(250);
						aux.setServo(AUX_OPEN,&writer);
						break;
				case '2':
						thumb.setServo(70,&writer);
						//usleep(25000);
						usleep(500000);
						index.setServo(INDEX_CLOSE,&writer);
						//usleep(250);
						aux.setServo(AUX_CLOSE,&writer);	
						break;
				case '3': 
						thumb.setServo(THUMB_OPEN,&writer);
						//usleep(250);
						index.setServo(INDEX_OPEN,&writer);
					//	usleep(250);
						aux.setServo(AUX_CLOSE,&writer);
						break;
				case '4':
						
						//usleep(250);
						thumb.setServo(THUMB_CLOSE,&writer);
						usleep(500000);
						index.setServo(INDEX_CLOSE,&writer);
						//usleep(250);
						aux.setServo(AUX_OPEN,&writer);
						
						break;
				default:
						break;
			}
		}
    }
    
    //close connection
    close( client );
    close( sock );
    
    return EXIT_SUCCESS;
}

