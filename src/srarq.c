#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "constants.h"

#define debug_hex(_s, _len, _c) {int _i; fprintf(stderr, _c); for(_i = 0; _i < _len; _i++){fprintf(stderr, "%x|", _s[_i]);} fprintf(stderr, "\n");}

/* SRARQ functions */

int xor(char data[], unsigned char len){
    int i, x = 0;
    for(i = 0; i < len; i++){
        x ^= (int) data[i];
    }
    return x;
}

void send_packet(int socket_desc, char buf[], size_t len, const struct sockaddr *dest_addr, socklen_t addrlen){
    if(DROP_PACKETS && !(rand() % 5)){ // drop 1/5 packets
        if(DEBUG) fprintf(stderr, "> Dropped packet. I'm so sorry\n");
        return;
    }
    if(DEBUG>1) debug_hex(buf, len, ">> ")
    sendto(socket_desc, buf, len, 0, dest_addr, addrlen);
}

// Reads message from file descriptor and writes packet to destination
// Format: |sequence_high|sequence_low|length|data (length bytes)|checksum|
// Returns length of packet
// TODO: read from buffer instead of fd?
int encode_msg(int sequence, int fd, char dest[]){
    int len;

    dest[0] = sequence >> 8;
    dest[1] = sequence;
    len = read(fd, &dest[3], MSG_LEN);
    dest[2] = len;
    dest[len+3] = xor(&dest[3], len);
    dest[len+4] = 0;

    return len+META_LEN;
}

// Returns -1 for invalid or zero size packet, sequence number otherwise
int decode_msg(char msg[], int *len){
    if(len != NULL){
        *len = (unsigned char) msg[2];
    }

    if(xor(&msg[3], (unsigned char) msg[2]) != msg[(unsigned char) msg[2]+3]){
        fprintf(stderr, "Invalid packet received!\n");
        return -1;
    }

    return (unsigned char) msg[2] == 0 ? -1 : 0 | (unsigned char) msg[0] << 8 | (unsigned char) msg[1];
}

// Prints message to stdout
void print_msg(char msg[]){
    printf("%.*s", (unsigned char) msg[2], &msg[3]);
}
