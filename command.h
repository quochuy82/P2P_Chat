#ifndef COMMAND_H
#define COMMAND_H

#include "common.h"

// Command processing
void process_command(char* command);

// Individual command handlers
void cmd_help(void);
void cmd_myip(void);
void cmd_myport(void);
void cmd_connect(const char* ip, int port);
void cmd_list(void);
void cmd_terminate(int conn_id);
void cmd_send(int conn_id, const char* message);
void cmd_exit(void);

#endif // COMMAND_H