#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "constants.h"
#include "srarq.h"

/* SRARQ Server */

int main(int argc, char *argv[]) {

    int server_port;
    int socket_desc;
    int res = 0;
    int print_i = 0; // index of message that should be printed next

    char buf[BUFFER_SIZE];
    char queue[QUEUE_SIZE][BUFFER_SIZE] = {{0}};

    // Check number of arguments, terminate when server port is not a positive number
    if(argc != 2 || (server_port = strtol(argv[1], NULL, 0)) <= 0) {
        fprintf(stderr, "Invalid parameters. Please provide server port as a first parameter.\n");
        return 1;
    }

    socket_desc = socket(PF_INET, SOCK_DGRAM, 0);

    if(socket_desc < 0) {
        perror("Can not open socket");
        return 1;
    }

    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(server_port);

    // Bind to specified port
    res = bind(socket_desc, (struct sockaddr *)&address, sizeof(address));

    if(res < 0){
        perror("Can not bind to port");
        return 1;
    }

    fprintf(stderr, "Running server on port %d!\n", server_port);

    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    int len, sequence = -1;
    while(recvfrom(socket_desc, buf, sizeof(buf), 0, (struct sockaddr *)&client_address, &client_address_len) >= 0){

        sequence = decode_msg(buf, &len);

        if(sequence >= 0){
            // save message
            memcpy(queue[sequence % QUEUE_SIZE], buf, len+META_LEN);

            // send acknowledgement
            send_packet(socket_desc, buf, len+META_LEN, (struct sockaddr *)&client_address, client_address_len);

            // print all messages that can be printed
            while(decode_msg(queue[print_i % QUEUE_SIZE], NULL) == print_i) {
                print_msg(queue[print_i % QUEUE_SIZE]);
                print_i++;
            }
        }
    }

    close(socket_desc);
    return 0;
}
