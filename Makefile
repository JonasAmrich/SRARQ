all: lib server client

server:
	gcc -Wall -o bin/server src/server.c -lsrarq -Lbin

client:
	gcc -Wall -o bin/client src/client.c -lsrarq -Lbin

lib:
	gcc -c -Wall -Werror -fpic src/srarq.c -o bin/srarq.o
	gcc -shared -o bin/libsrarq.so bin/srarq.o
	rm bin/srarq.o

clean:
	rm bin/*
