#ifndef UART_CLI_H
#define UART_CLI_H

#include "xuartps.h"
#include <stdbool.h>

#define CMD_BUFFER_SIZE 18   // for the longest 14 char command 
#define MAX_COMMANDS 28  // 27 valid commands + 1 sentinel

static const uint8_t CMD_READ  = 2; // CLI Command Type Read or Write
static const uint8_t CMD_WRITE = 1;

// Structure for commands and description
typedef struct {
    const char *name;
    const char *description;
} st_cli_cmd;

// List of supported commands
static const st_cli_cmd cli_cmd_list[MAX_COMMANDS] = {
    {"cpld_rev_r",      "Read CPLD revision"},
    {"cpld_rev_w",      "Write CPLD revision"},
    {"thresh_17",       "Set threshold to 17_V"},
    {"thresh_33",       "Set threshold to 33_V"},
    {"thresh_84",       "Set threshold to 84_V"},
    {"thresh_166",      "Set threshold to 166_V"},
    {"thresh_?",        "Read threshold status"},
    {"dry_wet_d",       "Set Dry mode"},
    {"dry_wet_w",       "Set Wet mode"},
    {"dry_wet_?",       "Read Dry/Wet status"},
    {"sync_yes",        "Enable sync"},
    {"sync_no",         "Disable sync"},
    {"sync_?",          "Read sync status"},
    {"uut_status_?",    "Read/print full status"},
    {"chg_req",         "Request cmd change"},
    {"reset",           "Reset system"},
    {"relay_aa",        "Toggle relay AA"},
    {"relay_ab",        "Toggle relay AB"},
    {"relay_ca",        "Toggle relay CA"},
    {"relay_cb",        "Toggle relay CB"},
    {"relay_cc",        "Toggle relay CC"},
    {"relay_all_on",    "Turn all relays ON"},
    {"relay_all_off",   "Turn all relays OFF"},
    {"relay_status_?",  "Read status for all relays"},
    {"input_status_?",  "Read status for all inputs"},
    {"-help",           "Show help"},
    {"", ""}  // Sentinel to mark end of list
};

// Function declarations
char*   uart_cli_poll(XUartPs uart, bool verbose);
uint8_t process_uart_cmd(const char *cli_cmd, bool verbose);
void    print_help_cmd(void);

#endif // UART_CLI_H
