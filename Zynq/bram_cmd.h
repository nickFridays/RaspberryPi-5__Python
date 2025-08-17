#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xbram.h"
#include "xgpio.h"
#include "xuartps.h"
#include "sleep.h"
#include <string.h>

#include "uart_cli.h"

#ifndef UUT_DATA_H
#define UUT_DATA_H

// HW Constants 
#define BRAM0_DEVICE_ID		XPAR_BRAM_0_DEVICE_ID
#define BRAM1_DEVICE_ID		XPAR_BRAM_1_DEVICE_ID
#define INPUT_CHANNEL 		1
#define OUTPUT_CHANNEL 	    1
#define INPUT_DEVICE_ID 	XPAR_AXI_GPIO_INPUT_DEVICE_ID
#define OUTPUT_DEVICE_ID	XPAR_AXI_GPIO_OUTPUT_DEVICE_ID

// Constants for CommandBlocks initialization
#define Bit_Pos_CmdType	     14
#define Bit_Pos_ModId		         12
#define Bit_Pos_Threshold        10
#define Bit_Pos_DryWet             9
#define Bit_Pos_Sync                  8
#define WriteRequest	        0x1		// b01      ChangeRequest
#define ReadRequest	        0x2		// b10  DataUpdate
#define CPLDRev			        0x0		// b00
#define Mod_Id			        0x3 	// b11
#define Threshold_17V		0x3		// b11
#define Threshold_33V		0x2		// b10
#define Threshold_84V		0x1		// b01
#define Threshold_166V		0x0		// b00
#define Dry					        0x1		// b1
#define Wet				            0x0		// b0
#define Sync				        0x1		// b1
#define NoSync				    0x0		// b0


// Vars for HW driver instances 
extern XBram Bram0;
extern XBram Bram1;
extern XGpio input;
extern XGpio output;
extern XUartPs uart;
extern XUartPs_Config *config;

// BRAM Command frame variables
extern u32 cmd_frame2;  // = 0;
extern u32 cmd_frame3;  // = 0x00004000;  // See COMMAND Frame
// BRAM Command Frame 1, 4 and 5 are constants 
static const u32 cmd_frame1 = 0x00007E09;	            // HEADER = 0x7E,  POS = "000",  BOARD_ID = "1001"
static const u32 cmd_frame4 = 0x0000FFFF & ~  0x00004000;  // cmd_frame3; Inversion of Command3
static const u32 cmd_frame5 = 0x0000007E;				// FOOTER = 0x7E


// Structure with holders for Command frame Blocks
typedef struct {
    u32 CommandType;
    u32 ModId;
    u32 Threshold;
    u32 DryWet;
    u32 Synchr;
    u32 relayAA;
    u32 relayAB;
    u32 relayCA;
    u32 relayCB;
    u32 relayCC;
} CommandBlocks;
extern CommandBlocks st_frame_bits;

// Function prototype
void cmd_bram_write(const char *cli_cmd, CommandBlocks st_frame_bits, s8 index, bool verbose);
void cmd_bram_read(const char  *cli_cmd, CommandBlocks st_frame_bits, s8 index, bool verbose);
CommandBlocks cli_cmd_handler(const char *cli_cmd, CommandBlocks *st_frame_bits, u8 index, bool verbose);


#endif // UUT_DATA_H
