#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "string.h"

/* Port I/O functions */
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/* Command buffer */
static char cmd_buffer[SHELL_MAX_CMD_LEN];
static int cmd_pos = 0;

/* Forward declarations for command handlers */
static void cmd_help(int argc, char *argv[]);
static void cmd_clear(int argc, char *argv[]);
static void cmd_echo(int argc, char *argv[]);
static void cmd_version(int argc, char *argv[]);
static void cmd_reboot(int argc, char *argv[]);
static void cmd_halt(int argc, char *argv[]);

/* Built-in commands table */
static const shell_command_t commands[] = {
    {"aj",      "AJOS command prefix (use: aj <command>)", NULL},
    {NULL, NULL, NULL}  /* Sentinel */
};

/* Subcommands for 'aj' */
static const shell_command_t aj_commands[] = {
    {"help",    "List available commands",  cmd_help},
    {"clear",   "Clear the screen",         cmd_clear},
    {"echo",    "Print text back",          cmd_echo},
    {"version", "Print OS version",         cmd_version},
    {"reboot",  "Reboot the system",        cmd_reboot},
    {"halt",    "Halt the CPU",             cmd_halt},
    {NULL, NULL, NULL}  /* Sentinel */
};

/*
 * Print the shell prompt
 */
static void print_prompt(void) {
    vga_print("AJOS> ");
}

/*
 * Parse command line into arguments
 * Returns: number of arguments
 */
static int parse_command(char *cmdline, char *argv[]) {
    int argc = 0;
    int in_arg = 0;

    while (*cmdline != '\0' && argc < SHELL_MAX_ARGS) {
        if (*cmdline == ' ' || *cmdline == '\t') {
            if (in_arg) {
                *cmdline = '\0';  /* Terminate current argument */
                in_arg = 0;
            }
        } else {
            if (!in_arg) {
                argv[argc++] = cmdline;  /* Start new argument */
                in_arg = 1;
            }
        }
        cmdline++;
    }

    return argc;
}

/*
 * Execute a command
 */
static void execute_command(int argc, char *argv[]) {
    if (argc == 0) {
        return;  /* Empty command */
    }

    /* Check if command starts with "aj" */
    if (strcmp(argv[0], "aj") == 0) {
        if (argc < 2) {
            vga_print("Usage: aj <command>\n");
            vga_print("Type 'aj help' for a list of commands.\n");
            return;
        }

        /* Search for the subcommand */
        for (int i = 0; aj_commands[i].name != NULL; i++) {
            if (strcmp(argv[1], aj_commands[i].name) == 0) {
                /* Pass argc-1 and argv+1 to shift past "aj" */
                aj_commands[i].handler(argc - 1, argv + 1);
                return;
            }
        }

        /* Subcommand not found */
        vga_print("Unknown command: aj ");
        vga_print(argv[1]);
        vga_print("\nType 'aj help' for a list of commands.\n");
        return;
    }

    /* Command not found - suggest using aj prefix */
    vga_print("Unknown command: ");
    vga_print(argv[0]);
    vga_print("\nCommands use 'aj' prefix. Type 'aj help' for help.\n");
}

/*
 * Process the current command in the buffer
 */
static void process_command(void) {
    char *argv[SHELL_MAX_ARGS];
    int argc;

    /* Null-terminate the command */
    cmd_buffer[cmd_pos] = '\0';

    /* Parse and execute */
    argc = parse_command(cmd_buffer, argv);
    execute_command(argc, argv);

    /* Reset buffer */
    cmd_pos = 0;
    memset(cmd_buffer, 0, SHELL_MAX_CMD_LEN);
}

/*
 * Handle a single character of input
 */
static void handle_input(char c) {
    if (c == '\n') {
        /* Enter pressed - process command */
        vga_putchar('\n');
        process_command();
        print_prompt();
    } else if (c == '\b') {
        /* Backspace pressed */
        if (cmd_pos > 0) {
            cmd_pos--;
            cmd_buffer[cmd_pos] = '\0';
            /* Move cursor back, print space, move back again */
            vga_putchar('\b');
            vga_putchar(' ');
            vga_putchar('\b');
        }
    } else if (c >= 32 && c < 127) {
        /* Printable character */
        if (cmd_pos < SHELL_MAX_CMD_LEN - 1) {
            cmd_buffer[cmd_pos++] = c;
            vga_putchar(c);  /* Echo character */
        }
    }
}

/*
 * Command handlers
 */

static void cmd_help(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    vga_print("Available commands:\n");
    for (int i = 0; aj_commands[i].name != NULL; i++) {
        vga_print("  aj ");
        vga_print(aj_commands[i].name);
        vga_print(" - ");
        vga_print(aj_commands[i].description);
        vga_putchar('\n');
    }
}

static void cmd_clear(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    vga_clear();
}

static void cmd_echo(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (i > 1) {
            vga_putchar(' ');
        }
        vga_print(argv[i]);
    }
    vga_putchar('\n');
}

static void cmd_version(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    vga_print("AJOS v1.0.0\n");
}

static void cmd_reboot(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    vga_print("Rebooting...\n");

    /* Send reset command to keyboard controller */
    /* Wait for keyboard controller to be ready */
    while (inb(0x64) & 0x02) {
        /* Busy wait */
    }

    /* Send reset command (0xFE) to port 0x64 */
    outb(0x64, 0xFE);

    /* If that didn't work, halt */
    __asm__ volatile ("cli; hlt");
}

static void cmd_halt(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    vga_print("System halted.\n");

    /* Disable interrupts and halt */
    __asm__ volatile ("cli; hlt");
}

/*
 * Initialize the shell
 */
void shell_init(void) {
    cmd_pos = 0;
    memset(cmd_buffer, 0, SHELL_MAX_CMD_LEN);
}

/*
 * Main shell loop
 */
void shell_run(void) {
    char c;

    /* Initialize shell */
    shell_init();

    /* Print initial prompt */
    print_prompt();

    /* Main loop */
    while (1) {
        /* Get character from keyboard */
        c = keyboard_getchar();

        /* Handle the input */
        if (c != 0) {
            handle_input(c);
        }
    }
}
