/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xbram.h"
#include "xgpio.h"
#include "xuartps.h"


#define BRAM0_DEVICE_ID		XPAR_BRAM_0_DEVICE_ID
#define BRAM1_DEVICE_ID		XPAR_BRAM_1_DEVICE_ID
#define LED_DELAY	 		50000000 // 50,000,000 x 20ns = 1000ms = 1s	processor clock = 50MHz
#define BRAM_DELAY 		 	100000000 // 12,500,000 x 20ns =  250ns=  0.25s
#define BRAM_DELAY1 		50000000 // 50,000,000 x 20ns =  1000ms=  1s
#define LED_CHANNEL			1
#define INPUT_CHANNEL 		1
#define OUTPUT_CHANNEL 		1

//#define LED 				0x01 /* Assumes bit 0 of GPIO is connected to an LED */
//#define LED_DEVICE_ID 		XPAR_AXI_GPIO_LED_DEVICE_ID
#define INPUT_DEVICE_ID 	XPAR_AXI_GPIO_INPUT_DEVICE_ID
#define OUTPUT_DEVICE_ID	XPAR_AXI_GPIO_OUTPUT_DEVICE_ID

#define POS_CommandType		14
#define ChangeRequest		0x1		//"01"
#define DataUpdate			0x2		//"10"
#define CPLDRev				0x0		//"00"

#define POS_ModId			12
#define Mod_Id				0x3 	//"11"

#define POS_Threshold		10
#define Threshold_17V		0x3		//"11"
#define Threshold_33V		0x2		//"10"
#define Threshold_84V		0x1		//"01"
#define Threshold_166V		0x0		//"00"

#define POS_DryWet			9
#define Dry					0x1		//"1"
#define Wet					0x0		//"0"

#define POS_Synch			8
#define Synch				0x1		//"1"
#define NoSynch				0x0		//"0"

#define POS_ShNum			7




int BramExample0(u16 DeviceId);
int BramExample1(u16 DeviceId);
static void InitializeECC(XBram_Config *ConfigPtr, u32 EffectiveAddr);


XBram Bram0;	/* The Instance of the BRAM Driver */
XBram Bram1;	/* The Instance of the BRAM Driver */
XGpio led;
XGpio input;
XGpio output;
XUartPs uart;
XUartPs_Config *config;

int main()
{
    init_platform();
    u32 Data;
    volatile int Delay;
    int Status;
    unsigned int command1;
    unsigned int command2;
    unsigned int command3;
    unsigned int command4;
    unsigned int command5;
    unsigned int commandCPLD;
    unsigned int CommandType;
    unsigned int ModId;
    unsigned int Threshold;
    unsigned int DryWet;
    unsigned int Synchronization;


    unsigned int relayAA;
    unsigned int relayAB;
    unsigned int relayCA;
    unsigned int relayCB;
    unsigned int relayCC;

    unsigned int data1;
    unsigned int data2;
    unsigned int data3;
    unsigned int data4;
    unsigned int data5;
    unsigned int data6;
    unsigned int data7;
    unsigned int reset;

    uint8_t SHnum;
    int Xgpio_status;
    int out_data;
    unsigned int out_dataCPLD1;
    unsigned int out_dataCPLD2;

    int WriteEn;
    int ReadEn;
    uint16_t data16;

    int pass = 0;
    int command = 0;

    xil_printf("HELLO WORLD\r\n");
    u8 recv_char;
    config = XUartPs_LookupConfig(XPAR_XUARTPS_0_DEVICE_ID);
    XUartPs_CfgInitialize(&uart, config, config->BaseAddress);

    Status = XUartPs_SetBaudRate(&uart, 115200);  // Set to 115200 baud

    if (Status != XST_SUCCESS) {
		xil_printf("Failed to set baud rate\r\n");
	}

///////////////////////////////// LED INIT - WRITE /////////////////////////////////////////
	/*Xgpio_status = XGpio_Initialize(&led, LED_DEVICE_ID);
	if (Xgpio_status != XST_SUCCESS) {
		xil_printf("XGpio LED Initialization failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully initialized XGpio LED \r\n");

	XGpio_SetDataDirection(&led, LED_CHANNEL, 0x0); // Output

	for (Delay = 0; Delay < LED_DELAY; Delay++);
	XGpio_DiscreteWrite(&led, LED_CHANNEL, 0x1); // Turn on RED LED
	//sleep (1);
	for (Delay = 0; Delay < LED_DELAY; Delay++);
	XGpio_DiscreteWrite(&led, LED_CHANNEL, 0x2); // Turn on GREEN LED
	//sleep (1);
	for (Delay = 0; Delay < LED_DELAY; Delay++);
	XGpio_DiscreteWrite(&led, LED_CHANNEL, 0x4); // Turn on BLUE LED*/
	/////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////// BRAM INIT 1 //////////////////////////////////////////////
	//Status = BramExample(XPAR_BRAM_0_DEVICE_ID);
	Status = BramExample0(XPAR_BRAM_0_DEVICE_ID);
	if (Status != XST_SUCCESS ) {
		xil_printf("Bram0 Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Bram0 Example\r\n");
	/////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// BRAM INIT 2 //////////////////////////////////////////////
	//Status = BramExample(XPAR_BRAM_0_DEVICE_ID);

	Status = BramExample1(XPAR_BRAM_1_DEVICE_ID);
	if (Status != XST_SUCCESS ) {
		xil_printf("Bram1 Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Bram1 Example\r\n");

	/////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////// INPUT INIT ////////////////////////////////////////////////
	Xgpio_status = XGpio_Initialize(&input, INPUT_DEVICE_ID);
	if (Xgpio_status != XST_SUCCESS) {
		xil_printf("XGpio INPUT Initialization failed\r\n");
		return XST_FAILURE;
	}
		xil_printf("Successfully initialized XGpio INPUT\r\n");
	xil_printf("Configuring Input\r\n");
	XGpio_SetDataDirection(&input, INPUT_CHANNEL , 0xF); // input
	/////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////// OUTPUT INIT ////////////////////////////////////////////////
	Xgpio_status = XGpio_Initialize(&output, OUTPUT_DEVICE_ID);
	if (Xgpio_status != XST_SUCCESS) {
		xil_printf("XGpio OUTPUT Initialization failed\r\n");
		return XST_FAILURE;
	}
		xil_printf("Successfully initialized XGpio OUTPUT\r\n");
	xil_printf("Configuring Output\r\n");
	XGpio_SetDataDirection(&output, OUTPUT_CHANNEL , 0x0); // output
	/////////////////////////////////////////////////////////////////////////////////////////////

	CommandType = DataUpdate;
	ModId = Mod_Id;
	Threshold = Threshold_166V;
	DryWet = Wet;
	Synchronization = NoSynch;

	reset = 0x00000001;
	XGpio_DiscreteWrite(&output, OUTPUT_CHANNEL, reset);
	out_dataCPLD1 = 0x00000000;
	out_dataCPLD2 = 0x00000000;

    while (1) {
		xil_printf("Enter a character: ");
		while (XUartPs_IsReceiveData(uart.Config.BaseAddress) == FALSE);
		xil_printf("\r\n");

		while (!pass) {
			recv_char = XUartPs_RecvByte(uart.Config.BaseAddress);
			xil_printf("You typed: %c\r\n", recv_char);

			if (recv_char != '1') {
				xil_printf("Please type 1 to continue: \r\n");
				pass = 0;
			}
			else {
				pass = 1;
				xil_printf("\x1b[2J"); // Clear screen
				xil_printf("\x1b[H");  // Move cursor to top-left (home)
			}
		}

		WriteEn = 0;
		ReadEn = 0;
		command1 = 0x00007E09;		// HEADER = 0x7E	// POS = "000" // BOARD_ID = "1001"
		command3 = 0x00004000;		// See COMMAND Frame
		command4 = 0x0000FFFF & ~command3;	// Inversion of Command3
		command5 = 0x0000007E;		// FOOTER = 0x7E
		SHnum = 0;					// Sample and Hold number

		// INITIALIZE COMMAND //
		CommandType = DataUpdate;
		ModId = Mod_Id;
		Threshold = Threshold_166V;
		DryWet = Wet;
		Synchronization = NoSynch;

		// INITIALIZE RELAYS //
		relayAA = 0x00000000;
		relayAB = 0x00000000;
		relayCA = 0x00000000;
		relayCB = 0x00000000;
		relayCC = 0x00000000;

		while (pass) {
			Data = XGpio_DiscreteRead(&input, INPUT_CHANNEL);
			WriteEn = Data & 0x8;
			ReadEn = Data & 0x4;

			/*
			while (!WriteEn) {
				Data = XGpio_DiscreteRead(&input, INPUT_CHANNEL);
				WriteEn = Data & 0x8;
			}
			*/
			//xil_printf("WriteEn Detected:\n\r");
			xil_printf("Writing COMMAND\r\n");

			command2 = (CommandType << POS_CommandType) | (ModId << POS_ModId) | (Threshold << POS_Threshold) | (DryWet << POS_DryWet) | (Synchronization << POS_Synch);
			command2 = (command2 & 0xFFFFFF00) | SHnum;
			command3 =  (command3 & 0x0000FF00) | relayAA | relayAB | relayCA | relayCB | relayCC;
			command4 = 0x0000FFFF & ~command3;


			XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR, 0, command1);
			XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + 0x04, 0, command2);
			XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + 0x08, 0, command3);
			XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + 0x0C, 0, command4);
			XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + 0x10, 0, command5);

			xil_printf("Command1 = 0x%04X\r\n", command1);
			xil_printf("Command2 = 0x%04X\r\n", command2);
			xil_printf("Command3 = 0x%04X\r\n", command3);
			xil_printf("Command4 = 0x%04X\r\n", command4);
			xil_printf("Command5 = 0x%04X\r\n", command5);

			while (!ReadEn) {
				Data = XGpio_DiscreteRead(&input, INPUT_CHANNEL);
				ReadEn = Data & 0x4;
			}
			xil_printf("Reading DATA \r\n");

			if (CommandType == CPLDRev) {
				out_dataCPLD1 = XBram_ReadReg(XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR, 8);	//read
				out_dataCPLD2 = XBram_ReadReg(XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR, 12);	//read
			}

			for (int i = 0; i < 25; i=i+4) {
				out_data = XBram_ReadReg(XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR, i);
				if (i==0){
					data1 = out_data & 0xFFFF;
					xil_printf("Data Read Address %d (data1): 0x%04X\n\r", i, data1);
				}
				else if (i==4){
					data2 = out_data & 0xFFFF;
					xil_printf("Data Read Address %d (data2): 0x%04X\n\r", i, data2);
				}
				else if (i==8){
					data3 = out_data & 0xFFFF;
					xil_printf("Data Read Address %d (data3): 0x%04X\n\r", i, data3);
				}
				else if (i==12){
					data4 = out_data & 0xFFFF;
					xil_printf("Data Read Address %d (data4): 0x%04X\n\r", i, data4);
				}
				else if (i==16){
					data5 = out_data & 0xFFFF;
					xil_printf("Data Read Address %d (data5): 0x%04X\n\r", i, data5);
				}
				else if (i==20){
					data6 = out_data & 0xFFFF;
					xil_printf("Data Read Address %d (data6): 0x%04X\n\r", i, data6);
				}
				else if (i==24){
					data7 = out_data & 0xFFFF;
					xil_printf("Data Read Address %d (data7): 0x%04X\n\r", i, data7);
				}

			}
			xil_printf("\n\r");

			if (reset) {
				xil_printf("FIXTURE IN OPERATION\n\r");
			}
			else xil_printf("FIXTURE IN RESET\n\r");
			xil_printf("\n\r");

			if (CommandType == DataUpdate) {
				xil_printf("COMMAND = DATA UPDATE ONLY, can't operate relays\n\r");
			}
			else if (CommandType == ChangeRequest) {
				xil_printf("COMMAND = CHANGE REQUEST\n\r");
			}
			xil_printf("\n\r");

			xil_printf("CPLD_REV MM-DD = 0x%04X\n\r", out_dataCPLD1);
			xil_printf("CPLD_REV YY-RR = 0x%04X\n\r", out_dataCPLD2);
////////////////////////////////////////////// STATUS CHECK	//////////////////////////////////////////////
			if (data2 & 0x00008000) {
				xil_printf("FRAME ERROR = 1 (BAD)\n\r");
			}
			else xil_printf("FRAME ERROR = 0 (GOOD)\n\r");

			if (data2 & 0x00005000) {
				xil_printf("COMMAND ERROR = 1 (BAD)\n\r");
			}
			else xil_printf("COMMAND ERROR = 0 (GOOD)\n\r");

			if (data2 & 0x00002000) {
				xil_printf("12V_RLY COMMAND = 1 (OFF)\n\r");
			}
			else xil_printf("12V_RLY COMMAND = 0 (ON)\n\r");

			if (data2 & 0x00001000) {
				xil_printf("COIL ERROR = 1 (BAD)\n\r");
			}
			else xil_printf("COIL ERROR = 0 (GOOD)\n\r");

			if ((data2 & 0x00000C00) == 0x00000C00) {
				xil_printf("THRESHOLD = 17V\n\r");
			}
			else if ((data2 & 0x00000400) == 0x00000400) {
				xil_printf("THRESHOLD = 33V\n\r");
			}
			else if ((data2 & 0x00000800) == 0x00000800) {
				xil_printf("THRESHOLD = 84V\n\r");
			}
			else if ((data2 & 0x00000000) == 0x00000000) {
				xil_printf("THRESHOLD = 166V\n\r");
			}

			if (data2 & 0x00000200) {
				xil_printf("DRY/WET = 1 (DRY Used)\n\r");
			}
			else xil_printf("DRY/WET = 0 (WET Used)\n\r");

			if (data2 & 0x00000100) {
				xil_printf("SYNCHRONIZE = 1\n\r");
			}
			else xil_printf("SYNCHRONIZE = 0\n\r");

			xil_printf("\n\r");
////////////////////////////////////////////// CONTACT OUTPUT CHECK	//////////////////////////////////////////////
			if (data3 & 0x00000001){
				xil_printf("RelayAA = ON \n\r");
			}
			else xil_printf("RelayAA = OFF \n\r");

			if (data3 & 0x00000002){
				xil_printf("RelayAB = ON \n\r");
			}
			else xil_printf("RelayAB = OFF \n\r");

			if (data3 & 0x00000004){
				xil_printf("RelayCA = ON \n\r");
			}
			else xil_printf("RelayCA = OFF \n\r");

			if (data3 & 0x00000008){
				xil_printf("RelayCB = ON \n\r");
			}
			else xil_printf("RelayCB = OFF \n\r");

			if (data3 & 0x00000010){
				xil_printf("RelayCC = ON \n\r");
			}
			else xil_printf("RelayCC = OFF \n\r");

			xil_printf("\n\r");
////////////////////////////////////////////// CONTACT INPUT CHECK	//////////////////////////////////////////////

			if (data4 & 0x00000001){
				xil_printf("Input_1 = ON \n\r");
			}
			else xil_printf("Input_1 = OFF \n\r");

			if (data4 & 0x00000002){
				xil_printf("Input_2 = ON \n\r");
			}
			else xil_printf("Input_2 = OFF \n\r");

			if (data4 & 0x00000004){
				xil_printf("Input_3 = ON \n\r");
			}
			else xil_printf("Input_3 = OFF \n\r");

			if (data4 & 0x00000008){
				xil_printf("Input_4 = ON \n\r");
			}
			else xil_printf("Input_4 = OFF \n\r");

			if (data4 & 0x00000010){
				xil_printf("Input_5 = ON \n\r");
			}
			else xil_printf("Input_5 = OFF \n\r");

			if (data4 & 0x00000020){
				xil_printf("Input_6 = ON \n\r");
			}
			else xil_printf("Input_6 = OFF \n\r");

			if (data4 & 0x00000040){
				xil_printf("Input_7 = ON \n\r");
			}
			else xil_printf("Input_7 = OFF \n\r");
///////////////////////////////////////////////////////////////////////////////////////////////////


			if (SHnum == 128) {
				SHnum = 0;
			}
			else {
				if (Synchronization == NoSynch) SHnum += 1;
				else SHnum = 0;
			}
			for (Delay = 0; Delay < BRAM_DELAY1; Delay++);

			if (XUartPs_IsReceiveData(uart.Config.BaseAddress)) {
				recv_char = XUartPs_RecvByte(uart.Config.BaseAddress);
				xil_printf("recv_char: %c \r\n", recv_char);
				switch (recv_char) {
				case '0':
					xil_printf("Please type 1 to continue: \r\n");
					pass = 0;
					out_dataCPLD1 = 0x00000000; // reset to all zero
					out_dataCPLD2 = 0x00000000; // reset to all zero
					break;

				case '2':
					command = 2;
					CommandType = ChangeRequest;
					if (Threshold == Threshold_17V) {
						Threshold = Threshold_33V;
					}
					else if (Threshold == Threshold_33V) {
						Threshold = Threshold_84V;
					}
					else if (Threshold == Threshold_84V) {
						Threshold = Threshold_166V;
					}
					else if (Threshold == Threshold_166V) {
						Threshold = Threshold_17V;
					}
					break;

				case '3':
					command = 3;
					CommandType = ChangeRequest;
					if (DryWet == Wet) DryWet = Dry;
					else DryWet = Wet;
					break;

				case '4':
					command = 4;
					CommandType = ChangeRequest;
					if (Synchronization == NoSynch) Synchronization = Synch;
					else Synchronization = NoSynch;
					break;

				case '5':
					command = 5;
					CommandType = DataUpdate;
					break;

				case '6':
					command = 6;
					CommandType = ChangeRequest;
					break;

				case '7':
					if (reset == 0x00000001) reset = 0x00000000;
					else if(reset == 0x00000000) reset = 0x00000001;
					XGpio_DiscreteWrite(&output, OUTPUT_CHANNEL, reset);
					//pass = 0;
					break;

				case '8':
					XGpio_DiscreteWrite(&output, OUTPUT_CHANNEL, 0x00000000); 	//assert reset
					for (Delay = 0; Delay < BRAM_DELAY1; Delay++);				//delay
					XGpio_DiscreteWrite(&output, OUTPUT_CHANNEL, 0x00000001); 	//de-assert reset
					CommandType = CPLDRev;
					commandCPLD = (CommandType << POS_CommandType) | (ModId << POS_ModId) | (Threshold << POS_Threshold) | (DryWet << POS_DryWet) | (Synchronization << POS_Synch);
					XBram_WriteReg(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR, 0, commandCPLD);
					break;

				case 'a':
					CommandType = ChangeRequest;
					relayAA = relayAA ^ 1;
					break;

				case 's':
					CommandType = ChangeRequest;
					relayAB = relayAB ^ (1<<1);
					break;

				case 'd':
					CommandType = ChangeRequest;
					relayCA = relayCA ^ (1<<2);
					break;

				case 'f':
					CommandType = ChangeRequest;
					relayCB = relayCB ^ (1<<3);
					break;

				case 'g':
					CommandType = ChangeRequest;
					relayCC = relayCC ^ (1<<4);
					break;

				case 'z':
					//CommandType = ChangeRequest;
					relayAA = 0x00000000;
					relayAB = 0x00000000;
					relayCA = 0x00000000;
					relayCB = 0x00000000;
					relayCC = 0x00000000;
					break;

				case 'x':
					//CommandType = ChangeRequest;
					relayAA = 0x00000001;
					relayAB = 0x00000002;
					relayCA = 0x00000004;
					relayCB = 0x00000008;
					relayCC = 0x00000010;
					break;

				}


			}

			xil_printf("\033[2J");   // Clear screen
			xil_printf("\033[H");    // Move cursor to home position

		}
    }

    cleanup_platform();
    xil_printf("Bram Test done\r\n");
    return 0;
}


int BramExample0(u16 DeviceId)
{
	int Status;
	XBram_Config *ConfigPtr;

	/*
	 * Initialize the BRAM driver. If an error occurs then exit
	 */

	/*
	 * Lookup configuration data in the device configuration table.
	 * Use this configuration info down below when initializing this
	 * driver.
	 */

	ConfigPtr = XBram_LookupConfig(DeviceId);
	if (ConfigPtr == (XBram_Config *) NULL) {
		return XST_FAILURE;
	}

	Status = XBram_CfgInitialize(&Bram0, ConfigPtr,
				     ConfigPtr->CtrlBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


        InitializeECC(ConfigPtr, ConfigPtr->CtrlBaseAddress);


	/*
	 * Execute the BRAM driver selftest.
	 */
	Status = XBram_SelfTest(&Bram0, 0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

int BramExample1(u16 DeviceId)
{
	int Status;
	XBram_Config *ConfigPtr;

	/*
	 * Initialize the BRAM driver. If an error occurs then exit
	 */

	/*
	 * Lookup configuration data in the device configuration table.
	 * Use this configuration info down below when initializing this
	 * driver.
	 */

	ConfigPtr = XBram_LookupConfig(DeviceId);
	if (ConfigPtr == (XBram_Config *) NULL) {
		return XST_FAILURE;
	}

	Status = XBram_CfgInitialize(&Bram1, ConfigPtr,
				     ConfigPtr->CtrlBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


        InitializeECC(ConfigPtr, ConfigPtr->CtrlBaseAddress);


	/*
	 * Execute the BRAM driver selftest.
	 */
	Status = XBram_SelfTest(&Bram1, 0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function ensures that ECC in the BRAM is initialized if no hardware
* initialization is available. The ECC bits are initialized by reading and
* writing data in the memory. This code is not optimized to only read data
* in initialized sections of the BRAM.
*
* @param	ConfigPtr is a reference to a structure containing information
*		about a specific BRAM device.
* @param 	EffectiveAddr is the device base address in the virtual memory
*		address space.
*
* @return
*		None
*
* @note		None.
*
*****************************************************************************/
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
