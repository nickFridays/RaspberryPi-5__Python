#ifndef UART_CLI_H
#define UART_CLI_H

#include "xuartps.h"
#include <stdbool.h>

#define CMD_BUFFER_SIZE 18   // for the longest 14 char command 
#define MAX_COMMANDS    30  // 29 valid commands + 1 sentinel

// Structure for commands and description
typedef struct {
    const char *name;
    const char *description;
} st_cli_cmd;

// List of supported commands
static const st_cli_cmd cli_cmd_list[MAX_COMMANDS] = {
    {"cpld_rev_?",      "Read CPLD revision"},         //L21
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
    {"reset",           "Reset system"},
    {"relay_aa",        "Toggle relay AA"},
    {"relay_ab",        "Toggle relay AB"},
    {"relay_ca",        "Toggle relay CA"},
    {"relay_cb",        "Toggle relay CB"},
    {"relay_cc",        "Toggle relay CC"},
    {"relay_12v_rl",    "Toggle relay 12V rail"},
    {"relay_12v_?",     "Read RLY 12V rail status"},
    {"relays_?",        "All Relays status"},
    {"input_status_?",  "Read status for all inputs"},
    {"relays_on",       "Toggle relays ON"},
    {"relays_off",      "Toggle relays OFF"},
    {"-help",           "Show help"},
    {"exit",            "Exit CLI and close app."},
    {"debug",            "Toggle verbose-silent mode."},
    {"", ""}            // Sentinel to mark end of list  //L49
};

// Enum for command indexes (must match cli_cmd_list order)
typedef enum {
    CMD_CPLD_REV_R = 0,           //L54
    CMD_CPLD_REV_W,
    CMD_THRESH_17,
    CMD_THRESH_33,
    CMD_THRESH_84,
    CMD_THRESH_166,
    CMD_THRESH_Q,
    CMD_DRY_WET_D,
    CMD_DRY_WET_W,
    CMD_DRY_WET_Q,
    CMD_SYNC_YES,
    CMD_SYNC_NO,
    CMD_SYNC_Q,
    CMD_UUT_STATUS_Q,
    CMD_RESET,
    CMD_RELAY_AA,
    CMD_RELAY_AB,
    CMD_RELAY_CA,
    CMD_RELAY_CB,
    CMD_RELAY_CC,
    CMD_12V_RL_RAIL,
    CMD_12V_RL_RAIL_Q,
    CMD_RELAY_STATUS_Q,
    CMD_INPUT_STATUS_Q,
    CMD_RELAYS_ON,
    CMD_RELAYS_OFF,
    CMD_HELP,
    CMD_EXIT,
	CMD_DEBUG,
    CMD_UNKNOWN // Use as sentinel for unknown  L82
} en_cli_cmd_indx;

// Function declarations
char* uart_cli_poll(XUartPs uart, bool verbose);
s8    process_uart_cmd(const char *cli_cmd, bool verbose);
void  print_help_cmd(void);

#endif // UART_CLI_H
