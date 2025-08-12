#include "uut_cmd.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xbram.h"

//-------------------------------------------------------------------------------------------
// Process BRAM Write commands
//-------------------------------------------------------------------------------------------
void cmd_bram_write(const char *cli_cmd,CommandBlocks cmd_blocks, bool verbose) {

    command2 = (cmd_blocks.CommandType << POS_CommandType)  |
                (cmd_blocks.ModId       << POS_ModId)       |
                (cmd_blocks.Threshold   << POS_Threshold)   |
                (cmd_blocks.DryWet      << POS_DryWet)      |
                (cmd_blocks.Synchr      << POS_Synch);

    command2 = (command2 & 0xFFFFFF00)   | SHnum;

    command3 = (command3 & 0x0000FF00)          |
                            cmd_blocks.relayAA  |
                            cmd_blocks.relayAB  |
                            cmd_blocks.relayCA  |
                            cmd_blocks.relayCB  |
                            cmd_blocks.relayCC;

    // Write to BRAM register
    XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR,        0x00, command1);
    XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + 0x04, 0x00, command2);
    XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + 0x08, 0x00, command3);
    XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + 0x0C, 0x00, command4);
    XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + 0x10, 0x00, command5);

    // Verbose output
    if (verbose) {
        xil_printf("Building a Command Frame to send to UUT\r\n");
        xil_printf("Command1 = 0x%04X\r\n", command1);
        xil_printf("Command2 = 0x%04X\r\n", command2);
        xil_printf("Command3 = 0x%04X\r\n", command3);
        xil_printf("Command4 = 0x%04X\r\n", command4);
        xil_printf("Command5 = 0x%04X\r\n", command5);
    }
}

//-------------------------------------------------------------------------------------------
// Process Read Commands
//-------------------------------------------------------------------------------------------
void cmd_bram_read(const char *command, bool verbose) {
    unsigned int out_data = 0, out_dataCPLD1 = 0x00000000, out_dataCPLD2 = 0x00000000;
    unsigned int data1 = 0, data2 = 0, data3 = 0, data4 = 0, data5 = 0, data6 = 0, data7 = 0;

    CmdIDEnum CmdID = CMD_UNKNOWN; // Init Enum for CLI Commands

    // Identify command type
    if (strcmp(command, "cpld_rev_r") == 0) {
        CmdID = CMD_CPLD_REV;
    } else if (strcmp(command, "uut_status") == 0) {
        CmdID = CMD_UUT_STATUS;
    } else if (strcmp(command, "thresh_?") == 0) {
        CmdID = CMD_THRESH_STATUS;
    } else if (strcmp(command, "dry_wet_?") == 0) {
        CmdID = CMD_DRY_WET_STATUS;
    } else if (strcmp(command, "sync_?") == 0) {
        CmdID = CMD_SYNC_STATUS;
    } else if (strcmp(command, "relay_status_?") == 0) {
        CmdID = CMD_RELAY_STATUS;
    } else if (strcmp(command, "input_status_?") == 0) {
        CmdID = CMD_INPUT_STATUS;
    }

    // Read CPLD revision
    if (CmdID == CMD_CPLD_REV || CmdID == CMD_UUT_STATUS) {
        out_dataCPLD1 = XBram_ReadReg(XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR, 8);
        out_dataCPLD2 = XBram_ReadReg(XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR, 12);
        xil_printf("CPLD_REV MM-DD = 0x%04X, YY-RR = 0x%04X\n\r", out_dataCPLD1, out_dataCPLD2);
    }

    // Read and parse BRAM data. Print if verbose
    for (int i = 0; i <= 24; i += 4) {
        out_data = XBram_ReadReg(XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR, i);
        switch (i) {
            case 0:  data1 = out_data & 0xFFFF; if (verbose) xil_printf("Data Read Address %d (data1): 0x%04X\n\r", i, data1); break;
            case 4:  data2 = out_data & 0xFFFF; if (verbose) xil_printf("Data Read Address %d (data2): 0x%04X\n\r", i, data2); break;
            case 8:  data3 = out_data & 0xFFFF; if (verbose) xil_printf("Data Read Address %d (data3): 0x%04X\n\r", i, data3); break;
            case 12: data4 = out_data & 0xFFFF; if (verbose) xil_printf("Data Read Address %d (data4): 0x%04X\n\r", i, data4); break;
            case 16: data5 = out_data & 0xFFFF; if (verbose) xil_printf("Data Read Address %d (data5): 0x%04X\n\r", i, data5); break;
            case 20: data6 = out_data & 0xFFFF; if (verbose) xil_printf("Data Read Address %d (data6): 0x%04X\n\r", i, data6); break;
            case 24: data7 = out_data & 0xFFFF; if (verbose) xil_printf("Data Read Address %d (data7): 0x%04X\n\r", i, data7); break;
        }
    }

    // FRAME and COMMAND error checks
    if (data2 & 0x00008000) xil_printf("FRAME ERROR = 1 (BAD)\n\r");
    else xil_printf("FRAME ERROR = 0 (GOOD)\n\r");

    if (data2 & 0x00005000) xil_printf("COMMAND ERROR = 1 (BAD)\n\r");
    else xil_printf("COMMAND ERROR = 0 (GOOD)\n\r");

    // Threshold status
    if (CmdID == CMD_THRESH_STATUS || CmdID == CMD_UUT_STATUS) {
        if ((data2 & 0x00000C00) == 0x00000C00) xil_printf("THRESHOLD = 17V\n\r");
        else if ((data2 & 0x00000400) == 0x00000400) xil_printf("THRESHOLD = 33V\n\r");
        else if ((data2 & 0x00000800) == 0x00000800) xil_printf("THRESHOLD = 84V\n\r");
        else xil_printf("THRESHOLD = 166V\n\r");
    }

    // Dry/Wet status
    if (CmdID == CMD_DRY_WET_STATUS || CmdID == CMD_UUT_STATUS) {
        if (data2 & 0x00000200) xil_printf("DRY/WET = 1 (DRY Used)\n\r");
        else xil_printf("DRY/WET = 0 (WET Used)\n\r");
    }

    // Sync status
    if (CmdID == CMD_SYNC_STATUS || CmdID == CMD_UUT_STATUS) {
        if (data2 & 0x00000100) xil_printf("SYNCHRONIZE = 1\n\r");
        else xil_printf("SYNCHRONIZE = 0\n\r");
    }

    // Relay status
    if (CmdID == CMD_RELAY_STATUS || CmdID == CMD_UUT_STATUS) {
        if (data2 & 0x00002000) xil_printf("12V_RLY COMMAND = 1 (OFF)\n\r");
        else xil_printf("12V_RLY COMMAND = 0 (ON)\n\r");

        if (data2 & 0x00001000) xil_printf("COIL ERROR = 1 (BAD)\n\r");
        else xil_printf("COIL ERROR = 0 (GOOD)\n\r");

        xil_printf("RelayAA = %s\n\r", (data3 & 0x00000001) ? "ON" : "OFF");
        xil_printf("RelayAB = %s\n\r", (data3 & 0x00000002) ? "ON" : "OFF");
        xil_printf("RelayCA = %s\n\r", (data3 & 0x00000004) ? "ON" : "OFF");
        xil_printf("RelayCB = %s\n\r", (data3 & 0x00000008) ? "ON" : "OFF");
        xil_printf("RelayCC = %s\n\r", (data3 & 0x00000010) ? "ON" : "OFF");
    }

    // Input status
    if (CmdID == CMD_INPUT_STATUS || CmdID == CMD_UUT_STATUS) {
        xil_printf("Input_1 = %s\n\r", (data4 & 0x00000001) ? "ON" : "OFF");
        xil_printf("Input_2 = %s\n\r", (data4 & 0x00000002) ? "ON" : "OFF");
        xil_printf("Input_3 = %s\n\r", (data4 & 0x00000004) ? "ON" : "OFF");
        xil_printf("Input_4 = %s\n\r", (data4 & 0x00000008) ? "ON" : "OFF");
        xil_printf("Input_5 = %s\n\r", (data4 & 0x00000010) ? "ON" : "OFF");
        xil_printf("Input_6 = %s\n\r", (data4 & 0x00000020) ? "ON" : "OFF");
        xil_printf("Input_7 = %s\n\r", (data4 & 0x00000040) ? "ON" : "OFF");
    }

    //xil_printf("\n\r");
}






