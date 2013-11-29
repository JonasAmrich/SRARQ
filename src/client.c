#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "constants.h"
#include "srarq.h"

/* SRARQ Client */

int main(int argc, char *argv[]) {

    int server_port;
    int socket_desc;
    int res;
    int i;
    int sequence_i = 0; // next index of queued msg
    int ack_i = 0; // next index of acknowledged msg

    char buf[BUFFER_SIZE];
    char queue[QUEUE_SIZE][BUFFER_SIZE] = {{0}}; // queue of messages
    char ack[QUEUE_SIZE] = {0}; // acknowledgment states of messsages
    time_t queue_times[QUEUE_SIZE] = {0}; // times when queued messages were sent

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

    struct timeval tv;
    fd_set readfds;

    tv.tv_sec = ACK_TIMELIMIT_SEC;
    tv.tv_usec = ACK_TIMELIMIT_USEC;

    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);
    FD_SET(socket_desc, &readfds);

    while((res = select(socket_desc+1, &readfds, NULL, NULL, &tv)) >= 0){

        if(res == 0) {
            fprintf(stderr, "TIME\n");
        }else {
            for(i = 0; i <= socket_desc; i++){
                if(FD_ISSET(i, &readfds)){
                    if(i == STDIN) { // Message from user on STDIN

                        if(sequence_i - ack_i >= QUEUE_SIZE - 1) { // Queue full
                            fprintf(stderr, "Queue is full. Waiting for server to reply.\n");
                            break;
                        }

                        res = encode_msg(sequence_i, STDIN, queue[sequence_i]);
                        send(socket_desc, queue[sequence_i], res, 0);
                        sequence_i++;

                    }else if(i == socket_desc) { // Acknowledgment from server
                        res = recv(socket_desc, buf, sizeof(buf), 0);
                        fprintf(stderr, "<%s\n", buf);
                    }
                }
            }
        }

        FD_ZERO(&readfds);
        FD_SET(STDIN, &readfds);
        FD_SET(socket_desc, &readfds);

        //tv.tv_sec = min(remaining times)
    }

    close(socket_desc);
    return 0;
}
