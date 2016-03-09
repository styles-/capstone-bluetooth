CC = gcc
CFLAGS = -lbluetooth

all: simplescan rfcomm_server rfcomm_client

clean: 
	rm simplescan 
	rm rfcomm_server 
	rm rfcomm_client

simplescan:
	$(CC) $(CFLAGS) simplescan.c -o simplescan

rfcomm_server:
	$(CC) $(CFLAGS) rfcomm_server.c -o rfcomm_server

rfcomm_client:
	$(CC) $(CFLAGS) rfcomm_client.c -o rfcomm_client
