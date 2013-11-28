#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "constants.h"

/* SRARQ Client */

int main(int argc, char *argv[]) {

    int server_port;
    int socket_desc;
    int res;

    char buf[BUFFER_SIZE];

    // Check number of arguments, terminate when server port is not a positive number
    if(argc != 3 || (server_port = strtol(argv[2], NULL, 0)) <= 0) {
        fprintf(stderr, "Invalid parameters. Please provide server ip an port.\n");
        return 1;
    }

    socket_desc = socket(PF_INET, SOCK_DGRAM, 0);

    if(socket_desc < 0) {
        fprintf(stderr, "Can not open socket. Terminating.\n");
        return 1;
    }

    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_port = htons(server_port);
    inet_pton(AF_INET, argv[1], &(address.sin_addr));

    // Connect to specified ip and port
    res = connect(socket_desc, (struct sockaddr *)&address, sizeof(address));

    if(res < 0){
        fprintf(stderr, "Can not connect to %s port %d. Terminating.\n", argv[1], server_port);
        return 1;
    }

    char *msg = "Hello";

    send(socket_desc, msg, strlen(msg), 0);

    res = recv(socket_desc, buf, sizeof(buf), 0);

    fprintf(stderr, "Received %d bytes!\n", res);
    fprintf(stderr, "%s\n", buf);

    close(socket_desc);
    return 0;
}
