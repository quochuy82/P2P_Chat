#include "signal.h"
#include "socket.h"
#include <time.h>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <termios.h>
    #include <fcntl.h>
    #include <sys/ioctl.h>
#endif

// ANSI color codes for terminal output
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_WHITE   "\x1b[37m"
#define COLOR_RESET   "\x1b[0m"
#define COLOR_BOLD    "\x1b[1m"

// External variables
extern int running;

// Signal handler for graceful shutdown
void signal_handler(int sig) {
    switch(sig) {
        case SIGINT:
            printf("\n%s[!] Received interrupt signal (Ctrl+C)%s\n", 
                   COLOR_YELLOW, COLOR_RESET);
            printf("%sUse 'exit' command to quit properly.%s\n", 
                   COLOR_CYAN, COLOR_RESET);
            print_prompt();
            break;
            
        #ifndef _WIN32
        case SIGTERM:
            printf("\n%s[!] Received termination signal%s\n", 
                   COLOR_RED, COLOR_RESET);
            running = 0;
            break;
            
        case SIGHUP:
            printf("\n%s[!] Terminal disconnected%s\n", 
                   COLOR_YELLOW, COLOR_RESET);
            break;
            
        case SIGPIPE:
            // Ignore broken pipe signal
            break;
        #endif
            
        default:
            printf("\n%s[!] Received signal %d%s\n", 
                   COLOR_YELLOW, sig, COLOR_RESET);
            break;
    }
}

// Setup signal handlers
void setup_signal_handlers(void) {
    // Handle Ctrl+C
    signal(SIGINT, signal_handler);
    
    #ifndef _WIN32
    // Unix/Linux specific signals
    signal(SIGTERM, signal_handler);
    signal(SIGHUP, signal_handler);
    signal(SIGPIPE, SIG_IGN);  // Ignore broken pipe
    #endif
}

// Trim whitespace from string
void trim_string(char* str) {
    if (str == NULL) return;
    
    // Trim trailing whitespace
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[--len] = '\0';
    }
    
    // Trim leading whitespace
    char* start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    // Move string if needed
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

// Convert string to lowercase
void str_to_lower(char* str) {
    if (str == NULL) return;
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

// Convert string to uppercase
void str_to_upper(char* str) {
    if (str == NULL) return;
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

// Check if string is empty or only whitespace
int is_empty_string(const char* str) {
    if (str == NULL) return 1;
    while (*str) {
        if (!isspace((unsigned char)*str)) return 0;
        str++;
    }
    return 1;
}

// Print application banner
void print_banner(void) {
    printf("\n");
    printf("%s╔════════════════════════════════════╗%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s║     %sP2P Chat Application v1.0%s      %s║%s\n", 
           COLOR_CYAN, COLOR_BOLD, COLOR_RESET, COLOR_CYAN, COLOR_RESET);
    printf("%s║      %sPeer-to-Peer Messaging%s        %s║%s\n", 
           COLOR_CYAN, COLOR_WHITE, COLOR_RESET, COLOR_CYAN, COLOR_RESET);
    printf("%s╚════════════════════════════════════╝%s\n", COLOR_CYAN, COLOR_RESET);
    printf("\n");
}

// Print startup information
void print_startup_info(void) {
    printf("%s=== P2P Chat Application Started ===%s\n", COLOR_GREEN, COLOR_RESET);
    printf("Local IP: %s%s%s\n", COLOR_YELLOW, local_ip, COLOR_RESET);
    printf("Listening on port: %s%d%s\n", COLOR_YELLOW, listen_port, COLOR_RESET);
    printf("Type '%shelp%s' for available commands\n", COLOR_CYAN, COLOR_RESET);
    printf("%s====================================%s\n\n", COLOR_GREEN, COLOR_RESET);
}

// Print command prompt
void print_prompt(void) {
    printf("%s> %s", COLOR_BOLD, COLOR_RESET);
    fflush(stdout);
}

// Clear terminal screen
void clear_screen(void) {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Print error message
void print_error(const char* message) {
    printf("%s[ERROR] %s%s\n", COLOR_RED, message, COLOR_RESET);
}

// Print success message
void print_success(const char* message) {
    printf("%s[SUCCESS] %s%s\n", COLOR_GREEN, message, COLOR_RESET);
}

// Print warning message
void print_warning(const char* message) {
    printf("%s[WARNING] %s%s\n", COLOR_YELLOW, message, COLOR_RESET);
}

// Print info message
void print_info(const char* message) {
    printf("%s[INFO] %s%s\n", COLOR_BLUE, message, COLOR_RESET);
}

// Get current time as string
void get_current_time_str(char* buffer, size_t size) {
    time_t rawtime;
    struct tm* timeinfo;
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, size, "%H:%M:%S", timeinfo);
}

// Print current timestamp
void print_timestamp(void) {
    char time_str[32];
    get_current_time_str(time_str, sizeof(time_str));
    printf("[%s] ", time_str);
}

// Set terminal window title
void set_terminal_title(const char* title) {
    #ifdef _WIN32
        SetConsoleTitle(title);
    #else
        printf("\033]0;%s\007", title);
        fflush(stdout);
    #endif
}

// Check if keyboard key is pressed (non-blocking)
int kbhit(void) {
    #ifdef _WIN32
        return _kbhit();
    #else
        struct termios oldt, newt;
        int ch;
        int oldf;
        
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        
        ch = getchar();
        
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        
        if(ch != EOF) {
            ungetc(ch, stdin);
            return 1;
        }
        
        return 0;
    #endif
}