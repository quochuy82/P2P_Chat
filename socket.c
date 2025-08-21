#include "socket.h"

// Global socket variables
SOCKET listen_socket = INVALID_SOCKET;
int listen_port = 0;
char local_ip[INET_ADDRSTRLEN];

// Initialize socket library (Windows specific)
void initialize_sockets(void) {
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        exit(1);
    }
    #endif
}

// Cleanup socket library (Windows specific)
void cleanup_sockets(void) {
    #ifdef _WIN32
    WSACleanup();
    #endif
}

// Get local IP address
void get_local_ip(void) {
    #ifdef _WIN32
    char hostname[256];
    struct hostent* host;
    struct in_addr addr;
    
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        host = gethostbyname(hostname);
        if (host != NULL) {
            memcpy(&addr, host->h_addr_list[0], sizeof(struct in_addr));
            strcpy(local_ip, inet_ntoa(addr));
            return;
        }
    }
    strcpy(local_ip, "127.0.0.1");
    #else
    struct ifaddrs *ifaddr, *ifa;
    int family;
    
    if (getifaddrs(&ifaddr) == -1) {
        strcpy(local_ip, "127.0.0.1");
        return;
    }
    
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        
        family = ifa->ifa_addr->sa_family;
        
        if (family == AF_INET) {
            char host[INET_ADDRSTRLEN];
            getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                       host, INET_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);
            
            if (strcmp(ifa->ifa_name, "lo") != 0) {
                strcpy(local_ip, host);
                break;
            }
        }
    }
    
    freeifaddrs(ifaddr);
    #endif
}

// Check if IP address is valid
int is_valid_ip(const char* ip) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip, &(sa.sin_addr));
    return result == 1;
}

// Check if port is valid
int is_valid_port(int port) {
    return port > 0 && port <= 65535;
}

// Create a new socket
SOCKET create_socket(void) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Failed to create socket\n");
        return INVALID_SOCKET;
    }
    
    // Allow socket reuse
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, 
                   (char*)&opt, sizeof(opt)) < 0) {
        printf("setsockopt failed\n");
        close(sock);
        return INVALID_SOCKET;
    }
    
    return sock;
}

// Setup listening socket
int setup_listening_socket(int port) {
    struct sockaddr_in server_addr;
    
    listen_socket = create_socket();
    if (listen_socket == INVALID_SOCKET) {
        return -1;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(listen_socket, (struct sockaddr*)&server_addr, 
             sizeof(server_addr)) < 0) {
        printf("Bind failed on port %d\n", port);
        close(listen_socket);
        return -1;
    }
    
    if (listen(listen_socket, BACKLOG) < 0) {
        printf("Listen failed\n");
        close(listen_socket);
        return -1;
    }
    
    listen_port = port;
    printf("Listening on port %d\n", port);
    return 0;
}

// Accept client connection
SOCKET accept_client(struct sockaddr_in* client_addr) {
    socklen_t addr_len = sizeof(*client_addr);
    return accept(listen_socket, (struct sockaddr*)client_addr, &addr_len);
}

// Connect to a peer
int connect_to_peer(const char* ip, int port, SOCKET* sock) {
    *sock = create_socket();
    if (*sock == INVALID_SOCKET) {
        return -1;
    }
    
    struct sockaddr_in peer_addr;
    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &peer_addr.sin_addr);
    
    if (connect(*sock, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0) {
        close(*sock);
        return -1;
    }
    
    return 0;
}

// Send message through socket
int send_message(SOCKET sock, const char* message) {
    return send(sock, message, strlen(message), 0);
}

// Receive message from socket
int receive_message(SOCKET sock, char* buffer, int buffer_size) {
    return recv(sock, buffer, buffer_size - 1, 0);
}