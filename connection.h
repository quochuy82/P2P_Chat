#ifndef CONNECTION_H
#define CONNECTION_H

#include "common.h"
#include <pthread.h>

// Connection structure
typedef struct {
    int id;
    SOCKET socket;
    char ip[INET_ADDRSTRLEN];
    int port;
    pthread_t thread;
    int active;
} Connection;

// Global connections array and mutex
extern Connection connections[MAX_CONNECTIONS];
extern pthread_mutex_t connections_mutex;
extern pthread_t accept_thread;

// Connection management functions
void init_connections(void);
int add_connection(SOCKET sock, const char* ip, int port);
void remove_connection(int conn_id);
void close_connection(int conn_id);
void close_all_connections(void);

// Connection search functions
int find_connection_by_address(const char* ip, int port);
int find_connection_by_id(int conn_id);
Connection* get_connection_by_id(int conn_id);

// Connection info functions
int get_active_connection_count(void);
void print_connection_list(void);

// Thread functions
void* accept_connections_thread(void* arg);
void* handle_peer_messages_thread(void* arg);

#endif // CONNECTION_H