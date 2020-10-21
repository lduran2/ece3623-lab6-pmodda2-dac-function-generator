/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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

/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xstatus.h"

/* platform and library includes */
#include "platform.h"
#include <math.h>

//DAC2Pmod from Address Editor in Vivado
#define DA2acq  0x43C00000   //DA2 acquisition    - output
#define DA2dav	0x43C00004	 //DA2 data available - input
#define DA2dat1	0x43C00008	 //DA2 channel 1 data - output
#define DA2dat2 0x43C0000C	 //DA2 channel 2 data - output

/* GPIO definitions */
#define INP_GPIO_DEVICE_ID  XPAR_AXI_GPIO_0_DEVICE_ID	/* GPIO device for input */
#define FRQ_CHANNEL 1								/* GPIO port for buttons for frequency */
#define AMP_CHANNEL 2								/* GPIO port for switches for amplitude */
/*-----------------------------------------------------------*/

#define DA2bits	12

int main()
{
	XGpio InpInst;	/* GPIO Device driver instance for the input (buttons, switches) */

	int max_p1_sin = 1 << 12;		/* max sinusoid + 1 */
	int max_amp = max_p1_sin >> 1;	/* the max amplitude and DC offset */

	xil_printf("Initializing GPIO for input . . .\n");
	/* GPIO driver initialization */
	if (XGpio_Initialize(&InpInst, INP_GPIO_DEVICE_ID) != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* set direction of both channels to output */
	XGpio_SetDataDirection(&InpInst, FRQ_CHANNEL, 0xFF);
	XGpio_SetDataDirection(&InpInst, AMP_CHANNEL, 0xFF);

	init_platform();

    xil_printf("Hello World\n\r");

    cleanup_platform();
    return 0;
}
