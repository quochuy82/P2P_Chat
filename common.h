#ifndef COMMON_H
#define COMMON_H

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>

// Platform-specific includes
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
    #define close closesocket
    #define sleep(x) Sleep((x) * 1000)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <ifaddrs.h>
    #include <sys/types.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    typedef int SOCKET;
#endif

// Constants
#define BUFFER_SIZE 1024
#define MAX_MESSAGE_LENGTH 100
#define MAX_CONNECTIONS 50
#define BACKLOG 10
#define MAX_COMMAND_LENGTH 256
#define INET_ADDRSTRLEN 16

// Global state
extern int running;
extern int next_connection_id;

#endif // COMMON_H