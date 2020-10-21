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
#include <math.h>

//DAC2Pmod from Address Editor in Vivado
#define DA2acq  0x43C00000   //DA2 acquisition    - output
#define DA2dav	0x43C00004	 //DA2 data available - input
#define DA2dat1	0x43C00008	 //DA2 channel 1 data - output
#define DA2dat2 0x43C0000C	 //DA2 channel 2 data - output

/* GPIO definitions */
#define INP_GPIO_DEVICE_ID  XPAR_AXI_GPIO_0_DEVICE_ID	/* GPIO device for input */
#define FRQ_CHANNEL 1			/* GPIO port for buttons for frequency */
#define AMP_CHANNEL 2			/* GPIO port for switches for amplitude */
/*-----------------------------------------------------------*/

#define DA2_BITS	12		/* # bits in DA2 bus */
#define N_FS		5		/* number of frequencies */
#define	FS			500000	/* sampling frequency */
#define	CHECK_INC	100000	/* print at every CHECK_INC samples */

int main()
{
	XGpio InpInst;	/* GPIO Device driver instance for the input (buttons, switches) */
	int amp_data;	/* the switch data for amplitude */

	/* frequency is treated by finding the bit-index of the first bit
	 * set until it is reset */
	int frq_data = 0b0000;	/* the button data for frequency */
	_Bool enable_frq = TRUE;				/* enable detect frequency */
	_Bool should_print_amp_frq = FALSE;		/* print the amplitude and frequency*/

	int max_p1_sin = (1 << 12);		/* max sinusoid + 1 */
	int dc_off = (max_p1_sin >> 1);	/* the max amplitude + 1 and DC offset */
	double amp_scale				/* the amplitude scale*/
		= (((double)(dc_off - 1))/0b1111);

	/* ideally, we would use a lookup table for the cosines, but it
	 * does not look like this works, possibly too little memory. */
	// double coss[N_FS][FS];							/* the array of cosines */

	/* for sinusoidal values */
	double amp;										/* the amplitude */
	double W = 0;									/* discrete time frequency */
	double cosWn;									/* the result of cosine */
	double Wn;										/* the input of the cosine */
	double f[N_FS] = { 0.0, 0.5, 1.0, 2.0, 5.0 };	/* frequencies, 1-indexed */
	int i_f;										/* index of frequency */
	int n;											/* sample number */
	int check;										/* next sample to check */

	int dacdata=0;	//DAC ramp data
	int dacdav;		//DAC data available
	int dacacq=0;	//DAC acquire

	/* reset sample number */
	n = 0;

	xil_printf("Initializing GPIO for input . . .\n");
	/* GPIO driver initialization */
	if (XGpio_Initialize(&InpInst, INP_GPIO_DEVICE_ID) != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* set direction of both channels to output */
	XGpio_SetDataDirection(&InpInst, FRQ_CHANNEL, 0xFF);
	XGpio_SetDataDirection(&InpInst, AMP_CHANNEL, 0xFF);

	xil_printf("\n\rStarting DA2 Pmod sinusoidal waves...\r\n");
	Xil_Out32(DA2acq,0);

    while(1)
    {
    	/* reset the sample number */
    	if(n==FS)
    		n=0;

    	/* find the frequency amplitude */
    	amp_data = XGpio_DiscreteRead(	/* read amplitude data, */
    			&InpInst, AMP_CHANNEL);
    	amp = amp_scale * amp_data;		/* and scale it */

    	/* find the frequency */
    	frq_data = XGpio_DiscreteRead(	/* read the raw button data, */
    			&InpInst, FRQ_CHANNEL);
    	/* if detect frequency is enabled */
    	if (enable_frq) {
    		/* look through the frequencies */
    		for (i_f = N_FS; i_f--; ) {
    			/* check for a bit at i_f - 1 */
    			if (frq_data & (1 << (i_f - 1))) {
    				/* if there is, i_f is found */
    				xil_printf("Frequency bit %d is  on: 0x%x.\n",
    						(i_f - 1), frq_data);
    				enable_frq = FALSE;			/* stop looking */
    				W = ((2*M_PI*f[i_f])/FS);	/* calculate discrete time freq */
    				should_print_amp_frq = TRUE;
    				break;
    			}
    		} /* end for i_f */
    	} /* end if (enable_frq) */
    	/* otherwise */
    	else {
    		/* look again when the bit is off */
    		if (!(frq_data & (1 << (i_f - 1)))) {
    			xil_printf("Frequency bit %d is off: 0x%x.\n", (i_f - 1), frq_data);
        		i_f = 0;	/* reset frequency to 0 */
    			enable_frq = TRUE;
        		W = 0;
				should_print_amp_frq = TRUE;
    		}
    	}

    	/* print if should */
    	if (should_print_amp_frq) {
    		xil_printf("\tAmplitude: %4d.\n", (int)amp);
    		xil_printf("\tFrequency: %6d/%d.\n", (int)(W*FS), FS);
    		should_print_amp_frq = FALSE;
    	}

    	/* send to the DA2 */

    	dacdav=Xil_In32(DA2dav);

    	if(dacdav==0 && dacacq==0)			//DAC not in use?
			{
    			Xil_Out32(DA2dat1, dacdata);	//output DAC data
    			Xil_Out32(DA2dat2, dacdata);	//output DAC data
    			Xil_Out32(DA2acq,1);			//DAC acquire
    			dacacq=1;
			}
		if(dacdav==1 && dacacq==1)			//DAC data available
			{
    			Xil_Out32(DA2acq,0);			//stop DAC acquire
    			dacacq=0;
    	    	/* calculate the sinusoidal value */
    			Wn = W*n;
    	    	cosWn = cos(Wn);	/* the weight */
    	    	dacdata = (((int)floor(amp * cosWn)) + dc_off);
    	    	if (n == check) {
    	    		xil_printf("Out at sample %d => %d => %d.\n", n, (int)Wn, dacdata);
    	    		check += CHECK_INC;
    	    		/* reset if >= sample frequency */
    	    		if (check >= FS) {
    	    			check = 0;
    	    		}
    	    	}
    	    	n++;	/* next sample */
			}
    }

	/* no errors */
    return XST_SUCCESS;
}
