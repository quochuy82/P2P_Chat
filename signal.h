#ifndef SIGNAL_H
#define SIGNAL_H

#include "common.h"

// Signal handling functions
void signal_handler(int sig);
void setup_signal_handlers(void);

// String manipulation functions
void trim_string(char* str);
void str_to_lower(char* str);
void str_to_upper(char* str);
int is_empty_string(const char* str);

// Display and UI functions
void print_banner(void);
void print_startup_info(void);
void print_prompt(void);
void clear_screen(void);
void print_error(const char* message);
void print_success(const char* message);
void print_warning(const char* message);
void print_info(const char* message);

// Time utilities
void get_current_time_str(char* buffer, size_t size);
void print_timestamp(void);

// System utilities
void set_terminal_title(const char* title);
int kbhit(void);

#endif // SIGNAL_H