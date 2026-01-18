#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

/* Maximum command line length */
#define SHELL_MAX_CMD_LEN 256

/* Maximum number of arguments */
#define SHELL_MAX_ARGS 16

/* Command handler function pointer type */
typedef void (*command_handler_t)(int argc, char *argv[]);

/* Command structure */
typedef struct {
    const char *name;
    const char *description;
    command_handler_t handler;
} shell_command_t;

/* Initialize the shell */
void shell_init(void);

/* Run the main shell loop */
void shell_run(void);

#endif /* SHELL_H */
