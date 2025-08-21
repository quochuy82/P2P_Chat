#include "connection.h"
#include "socket.h"

// Global variables
Connection connections[MAX_CONNECTIONS];
pthread_mutex_t connections_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t accept_thread;
extern int running;
extern int next_connection_id;

// Initialize connections array
void init_connections(void) {
    memset(connections, 0, sizeof(connections));
}

// Add new connection
int add_connection(SOCKET sock, const char* ip, int port) {
    pthread_mutex_lock(&connections_mutex);
    
    // Find empty slot
    int slot = -1;
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (!connections[i].active) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        pthread_mutex_unlock(&connections_mutex);
        printf("Maximum connections reached\n");
        return -1;
    }
    
    connections[slot].id = next_connection_id++;
    connections[slot].socket = sock;
    strcpy(connections[slot].ip, ip);
    connections[slot].port = port;
    connections[slot].active = 1;
    
    // Create thread for handling messages
    int* thread_arg = malloc(sizeof(int));
    *thread_arg = slot;
    
    if (pthread_create(&connections[slot].thread, NULL, 
                       handle_peer_messages_thread, thread_arg) != 0) {
        printf("Failed to create thread for connection\n");
        connections[slot].active = 0;
        pthread_mutex_unlock(&connections_mutex);
        free(thread_arg);
        return -1;
    }
    
    pthread_mutex_unlock(&connections_mutex);
    return connections[slot].id;
}

// Remove connection
void remove_connection(int conn_id) {
    pthread_mutex_lock(&connections_mutex);
    
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].active && connections[i].id == conn_id) {
            connections[i].active = 0;
            close(connections[i].socket);
            break;
        }
    }
    
    pthread_mutex_unlock(&connections_mutex);
}

// Close connection properly
void close_connection(int conn_id) {
    pthread_mutex_lock(&connections_mutex);
    
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].active && connections[i].id == conn_id) {
            connections[i].active = 0;
            shutdown(connections[i].socket, 2);  // SD_BOTH
            close(connections[i].socket);
            break;
        }
    }
    
    pthread_mutex_unlock(&connections_mutex);
}

// Close all connections
void close_all_connections(void) {
    pthread_mutex_lock(&connections_mutex);
    
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].active) {
            shutdown(connections[i].socket, 2);
            close(connections[i].socket);
            connections[i].active = 0;
        }
    }
    
    pthread_mutex_unlock(&connections_mutex);
}

// Find connection by IP and port
int find_connection_by_address(const char* ip, int port) {
    pthread_mutex_lock(&connections_mutex);
    
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].active && 
            strcmp(connections[i].ip, ip) == 0 && 
            connections[i].port == port) {
            pthread_mutex_unlock(&connections_mutex);
            return i;
        }
    }
    
    pthread_mutex_unlock(&connections_mutex);
    return -1;
}

// Find connection by ID
int find_connection_by_id(int conn_id) {
    pthread_mutex_lock(&connections_mutex);
    
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].active && connections[i].id == conn_id) {
            pthread_mutex_unlock(&connections_mutex);
            return i;
        }
    }
    
    pthread_mutex_unlock(&connections_mutex);
    return -1;
}

// Get connection by ID
Connection* get_connection_by_id(int conn_id) {
    pthread_mutex_lock(&connections_mutex);
    
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].active && connections[i].id == conn_id) {
            pthread_mutex_unlock(&connections_mutex);
            return &connections[i];
        }
    }
    
    pthread_mutex_unlock(&connections_mutex);
    return NULL;
}

// Get active connection count
int get_active_connection_count(void) {
    int count = 0;
    
    pthread_mutex_lock(&connections_mutex);
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].active) {
            count++;
        }
    }
    pthread_mutex_unlock(&connections_mutex);
    
    return count;
}

// Print connection list
void print_connection_list(void) {
    printf("\n=== Active Connections ===\n");
    int count = 0;
    
    pthread_mutex_lock(&connections_mutex);
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].active) {
            printf("ID: %d | IP: %s | Port: %d\n", 
                   connections[i].id, connections[i].ip, connections[i].port);
            count++;
        }
    }
    pthread_mutex_unlock(&connections_mutex);
    
    if (count == 0) {
        printf("No active connections\n");
    }
    printf("==========================\n\n");
}

// Accept connections thread
void* accept_connections_thread(void* arg) {
    struct sockaddr_in client_addr;
    SOCKET client_socket;
    
    (void)arg; // Unused parameter
    
    while (running) {
        client_socket = accept_client(&client_addr);
        
        if (client_socket == INVALID_SOCKET) {
            if (running) {
                printf("Accept failed\n");
            }
            continue;
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);
        
        int conn_id = add_connection(client_socket, client_ip, client_port);
        if (conn_id != -1) {
            printf("\n[New connection] Peer connected from %s:%d (ID: %d)\n", 
                   client_ip, client_port, conn_id);
            printf("> ");
            fflush(stdout);
        } else {
            close(client_socket);
        }
    }
    
    return NULL;
}

// Handle peer messages thread
void* handle_peer_messages_thread(void* arg) {
    int slot = *(int*)arg;
    free(arg);
    
    char buffer[BUFFER_SIZE];
    int bytes_received;
    
    while (running) {
        pthread_mutex_lock(&connections_mutex);
        if (!connections[slot].active) {
            pthread_mutex_unlock(&connections_mutex);
            break;
        }
        
        SOCKET sock = connections[slot].socket;
        char ip[INET_ADDRSTRLEN];
        strcpy(ip, connections[slot].ip);
        int port = connections[slot].port;
        int conn_id = connections[slot].id;
        pthread_mutex_unlock(&connections_mutex);
        
        bytes_received = receive_message(sock, buffer, BUFFER_SIZE);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("\n[Message from %s:%d]: %s\n", ip, port, buffer);
            printf("> ");
            fflush(stdout);
        } else if (bytes_received == 0) {
            printf("\n[Connection closed] Peer %s:%d disconnected (ID: %d)\n", 
                   ip, port, conn_id);
            printf("> ");
            fflush(stdout);
            remove_connection(conn_id);
            break;
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                printf("\n[Error] Connection with %s:%d lost (ID: %d)\n", 
                       ip, port, conn_id);
                printf("> ");
                fflush(stdout);
                remove_connection(conn_id);
                break;
            }
        }
    }
    
    return NULL;
}