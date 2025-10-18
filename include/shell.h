#ifndef SHELL_H
#define SHELL_H

// Simple shell interface for Aether OS
void shell_init(void);
void shell_run(void);
void shell_process_command(const char* command);

// Built-in commands
void cmd_help(void);
void cmd_clear(void);
void cmd_meminfo(void);
void cmd_sysinfo(void);
void cmd_uptime(void);
void cmd_echo(const char* args);
void cmd_test(const char* args);

#endif // SHELL_H