all: common client server 

client: client.c
	gcc -Wall client.c common.o -o client

server: server.c
	gcc -Wall server.c common.o -o server

common: common.c
	gcc -Wall -c common.c

clean:
	rm common.o client server

run_server_v4:
	./server v4 51511

run_client_v4:
	./client 127.0.0.1 51511

run_server_v6:
	./server v6 51511
	
run_client_v6:	
	./client ::1 51511