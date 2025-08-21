#include "command.h"
#include "socket.h"
#include "connection.h"
#include "signal.h"

// External global variables
extern int running;

// Command: help
void cmd_help(void) {
    printf("\n=== P2P Chat Application Commands ===\n");
    printf("help                     - Show this help message\n");
    printf("myip                     - Display your IP address\n");
    printf("myport                   - Display the listening port\n");
    printf("connect <ip> <port>      - Connect to a peer\n");
    printf("list                     - List all active connections\n");
    printf("terminate <id>           - Terminate a connection\n");
    printf("send <id> <message>      - Send message to a peer\n");
    printf("exit                     - Exit the application\n");
    printf("=====================================\n\n");
}

// Command: myip
void cmd_myip(void) {
    printf("Your IP address: %s\n", local_ip);
}

// Command: myport
void cmd_myport(void) {
    printf("Listening port: %d\n", listen_port);
}

// Command: connect
void cmd_connect(const char* ip, int port) {
    // Validate IP
    if (!is_valid_ip(ip)) {
        printf("Error: Invalid IP address\n");
        return;
    }
    
    // Validate port
    if (!is_valid_port(port)) {
        printf("Error: Invalid port number (must be 1-65535)\n");
        return;
    }
    
    // Check if connecting to self
    if (strcmp(ip, local_ip) == 0 && port == listen_port) {
        printf("Error: Cannot connect to yourself\n");
        return;
    }
    
    // Check if connection already exists
    if (find_connection_by_address(ip, port) != -1) {
        printf("Error: Connection already exists to %s:%d\n", ip, port);
        return;
    }
    
    // Connect to peer
    SOCKET sock;
    if (connect_to_peer(ip, port, &sock) < 0) {
        printf("Error: Failed to connect to %s:%d\n", ip, port);
        return;
    }
    
    // Add connection
    int conn_id = add_connection(sock, ip, port);
    if (conn_id != -1) {
        printf("Successfully connected to %s:%d (ID: %d)\n", ip, port, conn_id);
    } else {
        close(sock);
    }
}

// Command: list
void cmd_list(void) {
    print_connection_list();
}

// Command: terminate
void cmd_terminate(int conn_id) {
    if (find_connection_by_id(conn_id) == -1) {
        printf("Error: Connection ID %d not found\n", conn_id);
        return;
    }
    
    close_connection(conn_id);
    printf("Connection %d terminated\n", conn_id);
}

// Command: send
void cmd_send(int conn_id, const char* message) {
    if (strlen(message) > MAX_MESSAGE_LENGTH) {
        printf("Error: Message exceeds maximum length of %d characters\n", 
               MAX_MESSAGE_LENGTH);
        return;
    }
    
    Connection* conn = get_connection_by_id(conn_id);
    if (conn == NULL) {
        printf("Error: Connection ID %d not found\n", conn_id);
        return;
    }
    
    if (send_message(conn->socket, message) < 0) {
        printf("Error: Failed to send message\n");
        remove_connection(conn_id);
    } else {
        printf("Message sent to connection %d\n", conn_id);
    }
}

// Command: exit
void cmd_exit(void) {
    printf("Shutting down...\n");
    running = 0;
    
    // Close all connections
    close_all_connections();
    
    // Close listening socket
    if (listen_socket != INVALID_SOCKET) {
        close(listen_socket);
    }
    
    // Wait for threads to finish
    sleep(1);
    
    cleanup_sockets();
    printf("Goodbye!\n");
    exit(0);
}

// Process user commands
void process_command(char* command) {
    trim_string(command);
    
    if (strlen(command) == 0) {
        return;
    }
    
    char cmd[MAX_COMMAND_LENGTH];
    char arg1[MAX_COMMAND_LENGTH];
    char arg2[MAX_COMMAND_LENGTH];
    
    // Initialize buffers
    memset(cmd, 0, sizeof(cmd));
    memset(arg1, 0, sizeof(arg1));
    memset(arg2, 0, sizeof(arg2));
    
    // Parse command
    int args = sscanf(command, "%s %s %[^\n]", cmd, arg1, arg2);
    
    if (strcmp(cmd, "help") == 0) {
        cmd_help();
    } else if (strcmp(cmd, "myip") == 0) {
        cmd_myip();
    } else if (strcmp(cmd, "myport") == 0) {
        cmd_myport();
    } else if (strcmp(cmd, "connect") == 0) {
        if (args >= 3) {
            int port = atoi(arg2);
            cmd_connect(arg1, port);
        } else {
            printf("Usage: connect <ip> <port>\n");
        }
    } else if (strcmp(cmd, "list") == 0) {
        cmd_list();
    } else if (strcmp(cmd, "terminate") == 0) {
        if (args >= 2) {
            int conn_id = atoi(arg1);
            cmd_terminate(conn_id);
        } else {
            printf("Usage: terminate <connection_id>\n");
        }
    } else if (strcmp(cmd, "send") == 0) {
        if (args >= 3) {
            int conn_id = atoi(arg1);
            cmd_send(conn_id, arg2);
        } else {
            printf("Usage: send <connection_id> <message>\n");
        }
    } else if (strcmp(cmd, "exit") == 0) {
        cmd_exit();
    } else {
        printf("Unknown command: %s\n", cmd);
        printf("Type 'help' for available commands\n");
    }
}