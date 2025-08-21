#ifndef SOCKET_H
#define SOCKET_H

#include "common.h"

// Global socket variables
extern SOCKET listen_socket;
extern int listen_port;
extern char local_ip[INET_ADDRSTRLEN];

// Socket initialization and cleanup
void initialize_sockets(void);
void cleanup_sockets(void);

// Network utilities
void get_local_ip(void);
int is_valid_ip(const char* ip);
int is_valid_port(int port);

// Socket operations
SOCKET create_socket(void);
int setup_listening_socket(int port);
SOCKET accept_client(struct sockaddr_in* client_addr);
int connect_to_peer(const char* ip, int port, SOCKET* sock);

// Data transmission
int send_message(SOCKET sock, const char* message);
int receive_message(SOCKET sock, char* buffer, int buffer_size);

#endif // SOCKET_H