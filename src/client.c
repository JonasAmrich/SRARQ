#include <limits.h>
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

int main(int argc, char *argv[]){

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

    if(DROP_PACKETS) srand(time(NULL));

    // Check number of arguments, terminate when server port is not a positive number
    if(argc != 3 || (server_port = strtol(argv[2], NULL, 0)) <= 0){
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

    tv.tv_sec = INF_TIME;
    tv.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);
    FD_SET(socket_desc, &readfds);

    int len, sequence, set_stdin, current_time, eof = 0;

    while((res = select(socket_desc+1, &readfds, NULL, NULL, &tv)) >= 0){

        current_time = time(NULL);
        set_stdin = !eof;

        if(res == 0) { // Time limit expired without acknowledgment, send packets again

            for(i = 0; i < QUEUE_SIZE; i++){
                if(queue_times[i] > 0 && queue_times[i] + ACK_TIMELIMIT <= current_time){
                    queue_times[i] = current_time;
                    decode_msg(queue[i], &len);
                    send_packet(socket_desc, queue[i], len+META_LEN, NULL, 0);
                    if(DEBUG){
                        fprintf(stderr, "> Time limit expired. %d was sent again.\n", decode_msg(queue[i], NULL));
                        fflush(stderr);
                    }
                }
            }

        }else {
            for(i = 0; i <= socket_desc; i++){
                if(FD_ISSET(i, &readfds)){
                    if(i == STDIN){ // Message from user on STDIN

                        if(sequence_i - ack_i >= QUEUE_SIZE - 1){ // Queue full
                            fprintf(stderr, "> Queue is full. Waiting for server to reply.\n");
                            fflush(stderr);
                            set_stdin = 0;
                            FD_CLR(STDIN, &readfds);
                            break;
                        }

                        len = encode_msg(sequence_i, STDIN, queue[sequence_i % QUEUE_SIZE]);

                        if(len == META_LEN){
                            FD_CLR(STDIN, &readfds);
                            eof = 1;
                            set_stdin = 0;
                            break;
                        }

                        send_packet(socket_desc, queue[sequence_i % QUEUE_SIZE], len, NULL, 0);
                        ack[sequence_i % QUEUE_SIZE] = 0;
                        queue_times[sequence_i % QUEUE_SIZE] = time(NULL);
                        sequence_i++;

                    }else if(i == socket_desc){ // Acknowledgment from server
                        recv(socket_desc, buf, sizeof(buf), 0);
                        sequence = decode_msg(buf, &len);
                        if(sequence >= 0){

                            if(memcmp(queue[sequence % QUEUE_SIZE], buf, len) == 0){
                                ack[sequence % QUEUE_SIZE] = 1;
                                queue_times[sequence % QUEUE_SIZE] = 0;

                                // Move queue over all acknowledged messages
                                while(ack[ack_i % QUEUE_SIZE]){
                                    ack[ack_i % QUEUE_SIZE] = 0;
                                    ack_i++;
                                }

                                if(DEBUG){
                                    fprintf(stderr, "> Got acknowledgment for %d\n", sequence);
                                    fflush(stderr);
                                }
                            }
                        }
                    }
                }
            }
        }

        if(eof && ack_i == sequence_i){
            break;
        }

        FD_ZERO(&readfds);
        FD_SET(socket_desc, &readfds);
        if(set_stdin) FD_SET(STDIN, &readfds);

        // Set time limit to next expiring packet
        //current_time = time(NULL);
        tv.tv_sec = INF_TIME;
        for(i = 0; i < QUEUE_SIZE; i++){
            if(queue_times[i] > 0 && current_time + ACK_TIMELIMIT - queue_times[i] < tv.tv_sec){ // this is not cool
                tv.tv_sec = current_time + ACK_TIMELIMIT - queue_times[i];
            }
        }
    }

    close(socket_desc);
    return 0;
}
