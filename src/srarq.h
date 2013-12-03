
void send_packet(int socket_desc, const void *buf, size_t len, const struct sockaddr *dest_addr, socklen_t addrlen);
int encode_msg(int sequence, int fd, char dest[]);
int decode_msg(char msg[], int *len);
void print_msg(char msg[]);
