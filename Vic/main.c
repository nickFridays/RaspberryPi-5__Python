/******************************************************************************
* Digilent Cora-Z7  PS Application
*
******************************************************************************/
/* UART TYPE  BAUD RATE
 * uartns550 9600 uartlite Configurable only in HW design
 * ps7_uart    115200 configured by bootrom/bsp - is used for this app */
 
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
// Custom headers 
#include "uart_cli.h"
#include "uut_cmd.h"

#define DEBUG true

// HW Constants 
#define BRAM0_DEVICE_ID		XPAR_BRAM_0_DEVICE_ID
#define BRAM1_DEVICE_ID		XPAR_BRAM_1_DEVICE_ID
#define INPUT_CHANNEL 		1
#define OUTPUT_CHANNEL 	    1
#define INPUT_DEVICE_ID 	XPAR_AXI_GPIO_INPUT_DEVICE_ID
#define OUTPUT_DEVICE_ID	XPAR_AXI_GPIO_OUTPUT_DEVICE_ID

#define CMD_READ  = 2; // CLI Command Type Read or Write
#define CMD_WRITE = 1;

// Vars for HW driver instances 
XBram Bram0;	// The Instance of the BRAM Driver
XBram Bram1;	     
XGpio input;
XGpio output;
XUartPs uart;
XUartPs_Config *config;

// Command blocks structure to build a BRAM command
CommandBlocks st_cmd_blk;

// Local functions 
int BramExample0(u16 DeviceId);
int BramExample1(u16 DeviceId);
static void InitializeECC(XBram_Config *ConfigPtr, u32 EffectiveAddr);
void InitUart();
long InitBram_0();
long InitBram_1();
void InitInput();
void InitOutput();
void LedBlink(int blinkNum, int delay_us);


//------ MAIN() ----------------------------------------------------------------------------------------------

int main()
{	
	unsigned int commandCPLD;
    unsigned int reset;
	u32 Data;
	uint16_t time_cntr=0; // 65,535 mSec max
	

	(void)command;
	SHnum    = 0;	// Sample Hold number
    //----------------------------------------------
	
	uint8_t cli_cmd_type = 0;
	char* cli_cmd;
    
    u8 recv_char = "";   // UART received char
	long hw_init = 0;    // HW initialization result
	//-----------------------------------------------	
	
	// Structure to pass into the function cmd_bram_write(CommandBlocks cmd) 
	CommandBlocks st_cmd_blk = {
		.CommandType = DataUpdate,     // 0x2
		.ModId       = Mod_Id,         // 0x3
		.Threshold   = Threshold_166V, // 0x3
		.DryWet      = Wet,            // 0x0
		.Synchr      = NoSync,         // 0x0
		.relayAA     = 0x00000000,
		.relayAB     = 0x00000000,
		.relayCA     = 0x00000000,
		.relayCB     = 0x00000000,
		.relayCC     = 0x00000000
	};

	//-----------------------------------------------
	// Hardware Driver Initialization
	init_platform();
    hw_init |= InitUart();
    hw_init |= InitBram_0(XPAR_BRAM_0_DEVICE_ID);
	hw_init |= InitBram_1(XPAR_BRAM_1_DEVICE_ID);
	hw_init |= InitInput();
	hw_init |= InitOutput();

	if (hw_init == 0) {
		// All functions returned 0
		xil_printf("\x1B[2J\x1B[H\r\n");
		xil_printf("Hardware Init OK\r\n");
	} else {
		// At least one function returned 1
		xil_printf("Hardware Init Failed\r\n");
	}
	
	// Blink LED to signal the start 
	LedBlink(7, 100000);

	reset = 0x00000001;

	xil_printf("UART CLI is ready. Type a command or -help:\n\r");
	// Checking user input until entered string matches a CLI command
	while (1) { 

		// Reset FPGA
		XGpio_DiscreteWrite(&output, OUTPUT_CHANNEL, reset);  

		// Poll UART to get a valid CLI Command		
		char* cli_cmd = uart_cli_poll(uart, true);
		
		// Check if command is Read or Write
		cli_cmd_type = process_uart_cmd(command, true);
		
		// Check if command is Read or Write
		u8 cli_cmd_type = process_uart_cmd(command, true);

		if (cli_cmd_type == CMD_READ) {
			if (DEBUG) {xil_printf("Executing READ operation for command: %s\n\r", command);}
			// Read from BRAM
			cmd_bram_read(cli_cmd);
			
		} else if (cli_cmd_type == CMD_WRITE) {
			if (DEBUG) {xil_printf("Executing WRITE operation for command: %s\n\r", command);}
			// Write to BRAM 
			cmd_bram_write(cli_cmd, st_cmd_blk , verbose);
		}

		usleep(10000);  // Sleep for 10 ms
	}
    //------------------------------------------------------------------------------------------------

	cleanup_platform();
	xil_printf("The End of forever loop\r\n");
	return 0;

} // main()

//---------------------------------------------------------------------------
//    Local Functions
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Initialize UART and set its baud rate
//---------------------------------------------------------------------------
void InitUart()
{
	int Status;
	config = XUartPs_LookupConfig(XPAR_XUARTPS_0_DEVICE_ID);
	XUartPs_CfgInitialize(&uart, config, config->BaseAddress);
	Status = XUartPs_SetBaudRate(&uart, 115200);  // Set to 115200 baud
	// Sleep 100 mS
	usleep(100000);
	// Clear screen and Move cursor to top-left (home)
	// xil_printf("\x1B[2J\x1B[H\r\n");
	// Report UART status
	if (Status == XST_SUCCESS) {
		   xil_printf(" UART Init was successful\r\n");
	   } else {
		   xil_printf(" UART Init failed with status code: %d\r\n", Status);
		   return XST_FAILURE
	   }
	return XST_SUCCESS
}
//---------------------------------------------------------------------------
// LedBlink(int blinkNum, delay_us)
//---------------------------------------------------------------------------
void LedBlink(int blinkNum, int delay_us)
{
	xil_printf("LED1 Blue Blink \r\n");
	uint8_t on_off = 1;
	for (int i = 0; i < blinkNum; i++) {
		XGpio_DiscreteWrite(&output, 1, on_off);  // ON/OFF
		on_off ^= 1;
		usleep(delay_us);
		//XGpio_DiscreteWrite(&output, 1, 0x00); // OFF
		//usleep(delay_us);
   }
}
//-----------------------------------------------------------------------------
//   InitInput()
//-----------------------------------------------------------------------------
long InitInput()
{
	int Xgpio_status;
	Xgpio_status = XGpio_Initialize(&input, INPUT_DEVICE_ID);
	if (Xgpio_status != XST_SUCCESS) {
		xil_printf("XGpio INPUT Init failed\r\n");
		return XST_FAILURE;
	}
	XGpio_SetDataDirection(&input, INPUT_CHANNEL , 0xF); // 0xF input
	//xil_printf("XGpio Input Init Successful\r\n");
	return XST_SUCCESS
}
//-----------------------------------------------------------------------------
//  InitOutput()
//-----------------------------------------------------------------------------
long InitOutput()
{	
	int Xgpio_status;
	Xgpio_status = XGpio_Initialize(&output, OUTPUT_DEVICE_ID);
	if (Xgpio_status != XST_SUCCESS) {
		xil_printf("XGpio OUTPUT Init failed\r\n");
		return XST_FAILURE;
	}
	// Configure as outputs
	XGpio_SetDataDirection(&output, OUTPUT_CHANNEL , 0x0);
	//xil_printf("XGpio Output Init Successful \r\n");
	return XST_SUCCESS
}
//-----------------------------------------------------------------------------
// InitBram_0()
//-----------------------------------------------------------------------------
long InitBram_0()
{
	int Status;
	Status = BramExample0(XPAR_BRAM_0_DEVICE_ID);
	if (Status != XST_SUCCESS ) {
		xil_printf("BRAM-0 Init Failed\r\n");
		return XST_FAILURE;
	}
	//xil_printf("BRAM-0 Init was Successful\r\n");
	return XST_SUCCESS;
}
//-----------------------------------------------------------------------------
// InitBram_1()
//-----------------------------------------------------------------------------
long InitBram_1()
{
	int Status;
	Status = BramExample1(XPAR_BRAM_1_DEVICE_ID);
	if (Status != XST_SUCCESS ) {
		xil_printf("BRAM-1 Init Failed\r\n");
		return XST_FAILURE;
	}
	//xil_printf("BRAM-1 Init was Successful\r\n");
	return XST_SUCCESS;
}
//------------------------------------------------------------------------------
/*   int BramExample0(u16 DeviceId)
 * Initialize the BRAM driver. If an error occurs then exit
 * Lookup configuration data in the device configuration table.
 * Use this configuration info down below when initializing this
 * driver.
/------------------------------------------------------------------------------ */
int BramExample0(u16 DeviceId)
{
	int Status;
	XBram_Config *ConfigPtr;

	ConfigPtr = XBram_LookupConfig(DeviceId);
	if (ConfigPtr == (XBram_Config *) NULL) {
		return XST_FAILURE;
	}

	Status = XBram_CfgInitialize(&Bram0, ConfigPtr, ConfigPtr->CtrlBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
        InitializeECC(ConfigPtr, ConfigPtr->CtrlBaseAddress);

	// Execute the BRAM driver self-test.
	Status = XBram_SelfTest(&Bram0, 0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}
/*----------------------------------------------------------------------------
 int BramExample1(u16 DeviceId)
 * Initialize the BRAM driver. If an error occurs then exit
 * Lookup configuration data in the device configuration table.
 * Use this configuration info for initializing this driver.
 /------------------------------------------------------------------------------*/
int BramExample1(u16 DeviceId)
{
	int Status;
	XBram_Config *ConfigPtr;

	ConfigPtr = XBram_LookupConfig(DeviceId);
	if (ConfigPtr == (XBram_Config *) NULL) {
		return XST_FAILURE;
	}

	Status = XBram_CfgInitialize(&Bram1, ConfigPtr, ConfigPtr->CtrlBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
        InitializeECC(ConfigPtr, ConfigPtr->CtrlBaseAddress);

	 // Execute the BRAM driver self-test.
	Status = XBram_SelfTest(&Bram1, 0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}
/*-------------------------------------------------------------------------------
* This function ensures that ECC in the BRAM is initialized if no hardware
* initialization is available. The ECC bits are initialized by reading and
* writing data in the memory. This code is not optimized to only read data
* in initialized sections of the BRAM.
* @param	ConfigPtr is a reference to a structure containing information
*		about a specific BRAM device.
* @param 	EffectiveAddr is the device base address in the virtual memory
*		address space.
-------------------------------------------------------------------------------------*/
void InitializeECC(XBram_Config *ConfigPtr, u32 EffectiveAddr)
{
	u32 Addr;
	volatile u32 Data;

	if (ConfigPtr->EccPresent &&
	    ConfigPtr->EccOnOffRegister &&
	    ConfigPtr->EccOnOffResetValue == 0 &&
	    ConfigPtr->WriteAccess != 0) {
		for (Addr = ConfigPtr->MemBaseAddress;
		     Addr < ConfigPtr->MemHighAddress; Addr+=4) {
			Data = XBram_In32(Addr);
			XBram_Out32(Addr, Data);
		}
		XBram_WriteReg(EffectiveAddr, XBRAM_ECC_ON_OFF_OFFSET, 1);
	}
}



