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