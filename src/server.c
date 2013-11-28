#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "constants.h"

#define MAX_CONNECTIONS 1

/* SRARQ Server */

int main(int argc, char *argv[]) {

    int server_port;
    int socket_desc;
    int res = 0;

    char buf[260];
    char queue[QUEUE_SIZE][BUFFER_SIZE];

    // Check number of arguments, terminate when server port is not a positive number
    if(argc != 2 || (server_port = strtol(argv[1], NULL, 0)) <= 0) {
        fprintf(stderr, "Invalid parameters. Please provide server port as a first parameter.\n");
        return 1;
    }

    socket_desc = socket(PF_INET, SOCK_DGRAM, 0);

    if(socket_desc < 0) {
        fprintf(stderr, "Can not open socket. Terminating.\n");
        return 1;
    }

    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(server_port);

    // Bind to specified port
    res = bind(socket_desc, (struct sockaddr *)&address, sizeof(address));

    if(res < 0){
        fprintf(stderr, "Can not bind to port %d. Terminating.\n", server_port);
        return 1;
    }

    fprintf(stderr, "Running server on port %d!\n", server_port);

    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    while((res = recvfrom(socket_desc, buf, sizeof(buf), 0, (struct sockaddr *)&client_address, &client_address_len)) >= 0){

        fprintf(stderr, "Received %d bytes!\n", res);
        fprintf(stderr, "%s\n", buf);

        char *msg = "Hi there! I'm server, who are you?";

        sendto(socket_desc, msg, strlen(msg), 0, (struct sockaddr *)&client_address, client_address_len);
    }

    close(socket_desc);
    return 0;
}
