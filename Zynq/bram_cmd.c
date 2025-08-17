
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xbram.h"
#include "xgpio.h"
#include "xuartps.h"
#include "sleep.h"
#include <string.h>
#include <stdbool.h>

#include "bram_cmd.h"
#include "uart_cli.h"

u32 cmd_frame2        = 0;
u32 cmd_frame3        = 0x00004000;

//-------------------------------------------------------------------------------------------
// void cmd_bram_write(const char *cli_cmd, CommandBlocks st_frame_bits, bool verbose) 
// Process BRAM Write commands   
// @param const char *cli_cmd         - actual CLI command
// @param  CommandBlocks *st_frame_bits - command frame blocks ctructure is passed as temp. copy by 
// reference to preserve the original instace 
//-------------------------------------------------------------------------------------------
void cmd_bram_write(const char *cli_cmd, CommandBlocks st_frame_bits, s8 index, bool verbose) { 

    // Frame 1, 4 and 5 are constants def in setup.h
    static   u8 SHnum = 0;
    cmd_frame2        = 0;
    cmd_frame3        = 0x00004000;
    
    // Get updated copy of st_frame_bits processed in cli command handler. 
    CommandBlocks st_tmp_frame_bits = cli_cmd_handler(cli_cmd, &st_frame_bits, index, verbose);

    // Build 5 command frames.  Shift each frame bits to specific positions
    cmd_frame2 = (st_tmp_frame_bits.CommandType  << Bit_Pos_CmdType)    |
								 (st_tmp_frame_bits.ModId         << Bit_Pos_ModId)         |
								 (st_tmp_frame_bits.Threshold    << Bit_Pos_Threshold)   |
								 (st_tmp_frame_bits.DryWet        << Bit_Pos_DryWet)       |
								 (st_tmp_frame_bits.Synchr         << Bit_Pos_Sync);

    cmd_frame2 = (cmd_frame2 & 0xFFFFFF00)  | SHnum;

    cmd_frame3 = (cmd_frame3 & 0x0000FF00)       |
									st_tmp_frame_bits.relayAA  |
									st_tmp_frame_bits.relayAB  |
									st_tmp_frame_bits.relayCA  |
									st_tmp_frame_bits.relayCB  |
									st_tmp_frame_bits.relayCC;

    // Write each command frame to BRAM register
    XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR,        0x00, cmd_frame1);
    XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + 0x04, 0x00, cmd_frame2);
    XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + 0x08, 0x00, cmd_frame3);
    XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + 0x0C, 0x00, cmd_frame4);
    XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + 0x10, 0x00, cmd_frame5);

    // Verbose output
    if (verbose) {
        xil_printf("DBG: Command frames sent to Cora-Z7 -> UUT:\r\n");
        xil_printf("Frame1 = 0x%04X\r\n", cmd_frame1);
        xil_printf("Frame2 = 0x%04X\r\n", cmd_frame2);
        xil_printf("Frame3 = 0x%04X\r\n", cmd_frame3);
        xil_printf("Frame4 = 0x%04X\r\n", cmd_frame4);
        xil_printf("Frame5 = 0x%04X\r\n", cmd_frame5);
    }

    // Update Sample Hold 
    if (SHnum == 128 || st_tmp_frame_bits.Synchr != NoSync)
        SHnum = 0;
    else
        SHnum++;

}

//-------------------------------------------------------------------------------------------
// Process Read/Status Commands
//-------------------------------------------------------------------------------------------
void cmd_bram_read(const char *cli_cmd, CommandBlocks st_frame_bits, s8 index, bool verbose) {
    unsigned int out_data = 0, out_dataCPLD1 = 0x00000000, out_dataCPLD2 = 0x00000000;
    unsigned int data1 = 0, data2 = 0, data3 = 0, data4 = 0, data5 = 0, data6 = 0, data7 = 0;

    // cmd_indx is the CLI command index in the enum.  arg index is a global var updated in main.
    en_cli_cmd_indx cmd_indx = (en_cli_cmd_indx)index;

    // Read CPLD revision
    if (cmd_indx == CMD_CPLD_REV_R) {
        out_dataCPLD1 = XBram_ReadReg(XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR, 8);
        out_dataCPLD2 = XBram_ReadReg(XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR, 12);
        xil_printf("CPLD_REV MM-DD = 0x%04X, YY-RR = 0x%04X\n\r", out_dataCPLD1, out_dataCPLD2);
    }

    // Read and parse BRAM data. Print if verbose = DEBUG = true
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
    if (data2 & 0x00008000) xil_printf("FRAME ERROR = 1 ERROR\n\r");
    else if (verbose) xil_printf("FRAME ERROR = 0 OK\n\r");

    if (data2 & 0x00005000) xil_printf("COMMAND ERROR = 1 ERROR\n\r");
    else if (verbose) xil_printf("COMMAND ERROR = 0 OK\n\r");

    // Threshold status
    if (cmd_indx == CMD_THRESH_Q || cmd_indx == CMD_UUT_STATUS_Q) {
        if ((data2 & 0x00000C00) == 0x00000C00) xil_printf("THRESHOLD = 17V\n\r");
        else if ((data2 & 0x00000400) == 0x00000400) xil_printf("THRESHOLD = 33V\n\r");
        else if ((data2 & 0x00000800) == 0x00000800) xil_printf("THRESHOLD = 84V\n\r");
        else xil_printf("THRESHOLD = 166V\n\r");
    }

    // Dry/Wet status
    if (cmd_indx == CMD_DRY_WET_Q || cmd_indx == CMD_UUT_STATUS_Q) {
        if (data2 & 0x00000200) xil_printf("DRY/WET = 1 - DRY\n\r");
        else xil_printf("DRY/WET = 0 -WET\n\r");
    }

    // Sync status
    if (cmd_indx == CMD_SYNC_Q || cmd_indx == CMD_UUT_STATUS_Q) {
        if (data2 & 0x00000100) xil_printf("SYNCHRONIZE = 1\n\r");
        else xil_printf("SYNCHRONIZE = 0\n\r");
    }

    // Relay status and Relay 12V rail. 
    if (cmd_indx == CMD_12V_RL_RAIL_Q  || cmd_indx == CMD_UUT_STATUS_Q) {
        if (data2 & 0x00002000) xil_printf("12V_Relay_Rail Status = 1 OFF\n\r");
        else xil_printf("12V_Relay Rail Status = 0 ON\n\r");
        if (data2 & 0x00001000) xil_printf("COIL ERROR = 1 Error\n\r");
        else if (verbose) xil_printf("COIL ERROR = 0 GOOD\n\r");
    }

    if (cmd_indx == CMD_RELAY_STATUS_Q || cmd_indx == CMD_UUT_STATUS_Q) {
        xil_printf("RelayAA = %s\n\r", (data3 & 0x00000001) ? "ON" : "OFF");
        xil_printf("RelayAB = %s\n\r", (data3 & 0x00000002) ? "ON" : "OFF");
        xil_printf("RelayCA = %s\n\r", (data3 & 0x00000004) ? "ON" : "OFF");
        xil_printf("RelayCB = %s\n\r", (data3 & 0x00000008) ? "ON" : "OFF");
        xil_printf("RelayCC = %s\n\r", (data3 & 0x00000010) ? "ON" : "OFF");
    }

    // Input status
    if (cmd_indx == CMD_INPUT_STATUS_Q || cmd_indx == CMD_UUT_STATUS_Q) {
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

//-------------------------------------------------------------------------------------------
// For each cli command update and return the temp copy of the structure passed by reference 
// 
//-------------------------------------------------------------------------------------------
CommandBlocks cli_cmd_handler(const char *cli_cmd, CommandBlocks *st_frame_bits, u8 index, bool verbose) {

    // Get a temp copy from the parent function cmd_bram_writ() to modify command frame fields
    CommandBlocks st_cp_frm_bits = *st_frame_bits;  

    en_cli_cmd_indx cmd_indx = (en_cli_cmd_indx)index;

   // Init BRAM command frame bits based on user CLI command 
    switch (cmd_indx) {
        case CMD_CPLD_REV_R:
            XGpio_DiscreteWrite(&output, OUTPUT_CHANNEL, 0x00000000); 	// assert 
			usleep(20000);			                                    // 20 mSec. delay ?
			XGpio_DiscreteWrite(&output, OUTPUT_CHANNEL, 0x00000001); 	// de-assert 
            st_cp_frm_bits.CommandType = CPLDRev;
            if (verbose) xil_printf("Cmd: cpld_rev_? - CmdType = 0x%04X\n\r", st_cp_frm_bits.CommandType);
            break;
        case CMD_CPLD_REV_W:
            if (verbose) xil_printf("Cmd: cpld_rev_w - Not Implemented  Write CPLD revision\n\r");
            break;
        case CMD_THRESH_17:
            st_cp_frm_bits.CommandType = WriteRequest;
			st_cp_frm_bits.Threshold = Threshold_17V;
            if (verbose) xil_printf("Cmd: thresh_17 = 0x%04X  cmdType: 0x%04X\n\r", st_cp_frm_bits.Threshold , st_cp_frm_bits.CommandType);
            break;
        case CMD_THRESH_33:
            st_cp_frm_bits.CommandType = WriteRequest;
			st_cp_frm_bits.Threshold = Threshold_33V;
            if (verbose) xil_printf("Cmd: thresh_33 = 0x%04X  cmdType: 0x%04X\n\r", st_cp_frm_bits.Threshold , st_cp_frm_bits.CommandType);
            break;
        case CMD_THRESH_84:
            st_cp_frm_bits.CommandType = WriteRequest;
			st_cp_frm_bits.Threshold = Threshold_84V;
            if (verbose) xil_printf("Cmd: thresh_84 = 0x%04X  cmdType: 0x%04X\n\r", st_cp_frm_bits.Threshold , st_cp_frm_bits.CommandType);
            break;
        case CMD_THRESH_166:
            st_cp_frm_bits.CommandType = WriteRequest;
			st_cp_frm_bits.Threshold = Threshold_166V;
            if (verbose) xil_printf("Cmd: thresh_166 = 0x%04X  cmdType: 0x%04X\n\r", st_cp_frm_bits.Threshold , st_cp_frm_bits.CommandType);
            break;
        case CMD_THRESH_Q:
            st_cp_frm_bits.CommandType = ReadRequest;
            if (verbose) xil_printf("Cmd: thresh_?  cmdType: 0x%04X\n\r", st_cp_frm_bits.CommandType);
            break;
        case CMD_DRY_WET_D:
            st_cp_frm_bits.CommandType = WriteRequest;
			st_cp_frm_bits.DryWet = Dry;
            if (verbose) xil_printf("Cmd: Set Dry= 0x%04X  cmdType: 0x%04X\n\r", st_cp_frm_bits.DryWet, st_cp_frm_bits.CommandType);
            break;
        case CMD_DRY_WET_W:
            st_cp_frm_bits.CommandType = WriteRequest;
			st_cp_frm_bits.DryWet = Wet;
            if (verbose) xil_printf("Cmd: Set Wet = 0x%04X  cmdType: 0x%04X\n\r", st_cp_frm_bits.DryWet, st_cp_frm_bits.CommandType);
            break;
        case CMD_DRY_WET_Q:
            st_cp_frm_bits.CommandType = ReadRequest;
            if (verbose) xil_printf("Cmd: dry_wet_?  cmdType: 0x%04X\n\r", st_cp_frm_bits.CommandType);
            break;
        case CMD_SYNC_YES:
            st_cp_frm_bits.CommandType = WriteRequest;
            st_cp_frm_bits.Synchr = Sync;
            if (verbose) xil_printf("Cmd: sync_yes  = 0x%04X  cmdType: 0x%04X\n\r", st_cp_frm_bits.Synchr, st_cp_frm_bits.CommandType);
            break;
        case CMD_SYNC_NO:
            if (verbose) xil_printf("Cmd: sync_no - Disable sync\n");
            st_cp_frm_bits.CommandType = WriteRequest;
            st_cp_frm_bits.Synchr = NoSync;
            if (verbose) xil_printf("Cmd: sync_no  = 0x%04X  cmdType: 0x%04X\n\r", st_cp_frm_bits.Synchr, st_cp_frm_bits.CommandType);
            break;
        case CMD_SYNC_Q:
            st_cp_frm_bits.CommandType = ReadRequest;
            if (verbose) xil_printf("Cmd: sync_?   cmdType: 0x%04X\n\r", st_cp_frm_bits.CommandType);
            break;
        case CMD_UUT_STATUS_Q:
            if (verbose) xil_printf("Cmd: uut_status_?   cmdType: 0x%04X\n\r", st_cp_frm_bits.CommandType);
            break;
        case CMD_RESET:
            //reset = 0x00000001;
            if (verbose) xil_printf("Cmd: reset =  \n\r");
            XGpio_DiscreteWrite(&output, OUTPUT_CHANNEL, 0x00000001);
            break;
        case CMD_RELAY_AA:
            st_cp_frm_bits.CommandType = WriteRequest;
			st_cp_frm_bits.relayAA = st_cp_frm_bits.relayAA ^ 1;
            if (verbose) xil_printf("Cmd: relay_aa = %d\n\r", st_cp_frm_bits.relayAA );
            break;
        case CMD_RELAY_AB:
            st_cp_frm_bits.CommandType = WriteRequest;
			st_cp_frm_bits.relayAB = st_cp_frm_bits.relayAB ^ (1<<1);
            if (verbose) xil_printf("Cmd: relay_ab = %d\n\r", st_cp_frm_bits.relayAB );
            break;
        case CMD_RELAY_CA:
            st_cp_frm_bits.CommandType = WriteRequest;
            st_cp_frm_bits.relayCA = st_cp_frm_bits.relayCA ^ (1<<2);
            if (verbose) xil_printf("Cmd: relay_ca = %d\n\r", st_cp_frm_bits.relayCA );
            break;
        case CMD_RELAY_CB:
            st_cp_frm_bits.CommandType = WriteRequest;
            st_cp_frm_bits.relayCB = st_cp_frm_bits.relayCB ^ (1<<3);
            if (verbose) xil_printf("Cmd: relay_cb = %d\n\r", st_cp_frm_bits.relayCB );
            break;
        case CMD_RELAY_CC:
            st_cp_frm_bits.CommandType = WriteRequest;
            st_cp_frm_bits.relayCC = st_cp_frm_bits.relayCC ^ (1<<4);
            if (verbose) xil_printf("Cmd: relay_cc = %d\n\r", st_cp_frm_bits.relayCC );
            break;
        case CMD_12V_RL_RAIL:
            st_cp_frm_bits.CommandType = WriteRequest;
            if (verbose) xil_printf("Cmd: relay_12v_rl -  Not Implemented - Toggle 12V relay rail ON\n\r");
            break;
        case CMD_12V_RL_RAIL_Q:
            st_cp_frm_bits.CommandType = ReadRequest;
            if (verbose) xil_printf("Cmd: relay_12v_? - 12V Relay rail status\n\r");
            break;
        case CMD_RELAY_STATUS_Q:
            st_cp_frm_bits.CommandType = ReadRequest;
            if (verbose) xil_printf("Cmd: relays_? -  Read status for all relays\n\r");
            break;
        case CMD_INPUT_STATUS_Q:
            st_cp_frm_bits.CommandType = ReadRequest;
            if (verbose) xil_printf("Cmd: input_status_? - Read status for all inputs\n\r");
            break;
        case CMD_RELAYS_ON:
            st_cp_frm_bits.CommandType = WriteRequest;
            st_cp_frm_bits.relayAA = 0x00000001;
            st_cp_frm_bits.relayAB = 0x00000002;
            st_cp_frm_bits.relayCA = 0x00000004;
            st_cp_frm_bits.relayCB = 0x00000008;
            st_cp_frm_bits.relayCC = 0x00000010;
            if (verbose) xil_printf("Cmd: relays_on - Turn all relays ON\n\r");      
            break;
        case CMD_RELAYS_OFF:
            st_cp_frm_bits.CommandType = WriteRequest;
            st_cp_frm_bits.relayAA = 0x00000000;
            st_cp_frm_bits.relayAB = 0x00000000;
            st_cp_frm_bits.relayCA = 0x00000000;
            st_cp_frm_bits.relayCB = 0x00000000;
            st_cp_frm_bits.relayCC = 0x00000000;
            if (verbose) xil_printf("Cmd: relays_off - Turn all relays OFF\n");            
            break;
        case CMD_HELP:
            if (verbose) xil_printf("Cmd: -help - Show help\n");
            print_help_cmd();
            break;
        case CMD_UNKNOWN:
        default:
            xil_printf("Unknown command: %s\n", cli_cmd);
            break;
    }

    return st_cp_frm_bits;

}


