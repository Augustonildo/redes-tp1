all: client server

client: client.c
	gcc client.c -o client

server: server.c
	gcc server.c -o server

clean:
	rm client server

valgrind_client:
	valgrind --leak-check=yes --log-file=valgrind.rpt client

valgrind_server:
	valgrind --leak-check=yes --log-file=valgrind.rpt server

run_server_v4:
	./server v4 51511

run_client_v4:
	./client 127.0.0.1 51511

run_server_v6:
	./server v6 51511
	
run_client_v6:	
	./client ::1 51511