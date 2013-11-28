all: server client

server:
	gcc -Wall -o bin/server src/server.c

client:
	gcc -Wall -o bin/client src/client.c

clean:
	rm bin/*
