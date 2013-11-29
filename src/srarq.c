#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "constants.h"

#define debug_hex(_s, _len, _c) int _i; fprintf(stderr, _c); for(_i = 0; _i < _len; _i++){fprintf(stderr, "%x|", _s[_i]);} fprintf(stderr, "\n");

/* SRARQ functions */

int xor(char data[], int len){
    int i, x = 0;
    for(i = 0; i < len; i++){
        x ^= (int) data[i];
    }
    return x;
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

    debug_hex(dest, len+META_LEN, ">")
    return len+5;
}

// Returns -1 for invalid or zero size packet, sequence number otherwise
int decode_msg(char msg[], int *len){
    if(len != NULL){
        *len = msg[2];
    }

    if(xor(&msg[3], msg[2]) != msg[msg[2]+3]){
        fprintf(stderr, "Invalid packet received!\n");
        return -1;
    }

    return msg[2] == 0 ? -1 : 0 | msg[0] << 8 | msg[1];
}

// Prints message to stdout
void print_msg(char msg[]){
    printf("%.*s", msg[2], &msg[3]);
}
