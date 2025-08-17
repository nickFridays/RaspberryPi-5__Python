
#include "xil_printf.h"
#include <string.h>

#include "uart_cli.h"
#include "bram_cmd.h"

//-------------------------------------------------------------------------------------------
// Process a single command string.   *index in enum command 
//-------------------------------------------------------------------------------------------
s8 process_uart_cmd(const char *cli_cmd, bool verbose) {
	// Cmd help
    if (strcmp(cli_cmd, "-help") == 0) {
    	// Clear screen.
    	//xil_printf("\x1B[2J\x1B[H\r\n");
        print_help_cmd();
        return CMD_HELP;
    }
    // Go through the structure cli_cmd_list until 0 dummy record {"", ""}
    for (s8 i = 0; cli_cmd_list[i].name[0] != '\0'; i++) {
        if (strcmp(cli_cmd, cli_cmd_list[i].name) == 0) {
            if (verbose) {
                xil_printf("Command recognized: %s\n\r", cli_cmd_list[i].name);
                xil_printf("Description: %s\n\r", cli_cmd_list[i].description);
            }
            // return index for cli command processing 
            return i ;                    //*index = i;

            /* Check if a command ends with '?' meaning Read otherwise Write
            if (strchr(cli_cmd, '?') != NULL) {
                // Read command
                return CMD_READ;
            } else {
                // Write command
                return CMD_WRITE;
            }
            return; */
        }
    }
    xil_printf("Unknown command: %s\n\r", cli_cmd);
    return -1;
}
//-------------------------------------------------------------------------------------------
// Print all available commands in response to help command
//-------------------------------------------------------------------------------------------
void print_help_cmd(void) {
    xil_printf("Available commands:\r\n |    name      |        description        |\r\n"
               "  -------------------------------------------\r\n");

    //Go through the structure cli_cmd_list until 0 dummy record {"", ""}
    for (int i = 0; cli_cmd_list[i].name[0] != '\0'; i++) {
        xil_printf("  %s    -    %s\r\n", cli_cmd_list[i].name, cli_cmd_list[i].description);
    }
}
//-------------------------------------------------------------------------------------------
// Poll UART for input and process commands
//-------------------------------------------------------------------------------------------
char* uart_cli_poll(XUartPs uart, bool verbose) {
    static char cmd_buffer[CMD_BUFFER_SIZE];
    static int buffer_index = 0;
    u8 recv_char;

    while (1) {
        if (XUartPs_IsReceiveData(uart.Config.BaseAddress)) {
            recv_char = XUartPs_RecvByte(uart.Config.BaseAddress);

            if (recv_char == '\r' || recv_char == '\n') {
                if (buffer_index > 0) {
                    cmd_buffer[buffer_index] = '\0';
                    if (verbose) {
                        xil_printf("Command received: %s\n\r", cmd_buffer);
                    }
                    buffer_index = 0;
                    return cmd_buffer;
                }
                // Ignore empty lines
            } else if (buffer_index < CMD_BUFFER_SIZE - 1) {
                cmd_buffer[buffer_index++] = recv_char;
            }
        }
    }
}
//-------------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------------

