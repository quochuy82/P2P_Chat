#include "common.h"
#include "socket.h"
#include "connection.h"
#include "command.h"
#include "signal.h"
#include <pthread.h>

// Global variables
int running = 1;
int next_connection_id = 1;

int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }
    
    // Parse and validate port
    int port = atoi(argv[1]);
    if (!is_valid_port(port)) {
        printf("Error: Invalid port number (must be 1-65535)\n");
        return 1;
    }
    
    // Initialize sockets
    initialize_sockets();
    
    // Initialize connections array
    init_connections();
    
    // Setup signal handlers
    setup_signal_handlers();
    
    // Get local IP address
    get_local_ip();
    
    // Setup listening socket
    if (setup_listening_socket(port) < 0) {
        printf("Failed to setup listening socket\n");
        cleanup_sockets();
        return 1;
    }
    
    // Start accept thread
    if (pthread_create(&accept_thread, NULL, accept_connections_thread, NULL) != 0) {
        printf("Failed to create accept thread\n");
        cleanup_sockets();
        return 1;
    }
    
    // Print startup information
    print_banner();
    print_startup_info();
    
    // Main command loop
    char command[MAX_COMMAND_LENGTH];
    while (running) {
        printf("> ");
        fflush(stdout);
        
        if (fgets(command, sizeof(command), stdin) != NULL) {
            // Remove newline
            command[strcspn(command, "\n")] = '\0';
            process_command(command);
        }
    }
    
    // Cleanup
    pthread_join(accept_thread, NULL);
    cleanup_sockets();
    
    return 0;
}