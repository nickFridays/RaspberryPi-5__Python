#ifndef UUT_DATA_H
#define UUT_DATA_H

// Constants for CommandBlocks initialization
#define POS_CommandType	     14
#define POS_ModId		     12
#define POS_Threshold		 10
#define POS_DryWet			  9
#define POS_Synch			  8
#define ChangeRequest	    0x1		// b01
#define DataUpdate		    0x2		// b10
#define CPLDRev			    0x0		// b00
#define Mod_Id			    0x3 	// b11
#define Threshold_17V		0x3		// b11
#define Threshold_33V		0x2		// b10
#define Threshold_84V		0x1		// b01
#define Threshold_166V		0x0		// b00
#define Dry					0x1		// b1
#define Wet				    0x0		// b0
#define Sync				0x1		// b1
#define NoSync				0x0		// b0

uint8_t      SHnum;
unsigned int command = 0;
unsigned int command1 = 0x00007E09;	// HEADER = 0x7E,  POS = "000",  BOARD_ID = "1001"
unsigned int command2;
unsigned int command3 = 0x00004000;	            // See COMMAND Frame
unsigned int command4 = 0x0000FFFF & ~command3;	// Inversion of Command3
unsigned int command5 = 0x0000007E;				// FOOTER = 0x7E

// Structure definition for CommandBlocks
typedef struct {
    unsigned int CommandType;
    unsigned int ModId;
    unsigned int Threshold;
    unsigned int DryWet;
    unsigned int Synchr;
    unsigned int relayAA;
    unsigned int relayAB;
    unsigned int relayCA;
    unsigned int relayCB;
    unsigned int relayCC;
} CommandBlocks;
extern CommandBlocks st_cmd_blk;

// Define CLI command for parsing
typedef enum {
    CMD_UNKNOWN,
    CMD_CPLD_REV,
    CMD_UUT_STATUS,
    CMD_THRESH_STATUS,
    CMD_DRY_WET_STATUS,
    CMD_SYNC_STATUS,
    CMD_RELAY_STATUS,
    CMD_INPUT_STATUS
} CmdIDEnum;


// Function prototype
void cmd_bram_write(const char *cli_cmd, CommandBlocks cmd, bool verbose);
void cmd_bram_read(const char *command, bool verbose);

#endif // UUT_DATA_H