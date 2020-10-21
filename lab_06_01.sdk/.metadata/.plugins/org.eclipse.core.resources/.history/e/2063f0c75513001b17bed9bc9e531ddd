
// main.c DA2PmodTest	ECE3622
// Dennis Silage   c2019

#include "xparameters.h"
#include "xil_io.h"
#include "xil_printf.h"

//DAC2Pmod from Address Editor in Vivado
#define DA2acq  0x43C00000   //DA2 acquisition    - output
#define DA2dav	0x43C00004	 //DA2 data available - input
#define DA2dat1	0x43C00008	 //DA2 channel 1 data - output
#define DA2dat2 0x43C0000C	 //DA2 channel 2 data - output

int main(void)
{
	int dacdata=0;	//DAC ramp data
	int dacdav;		//DAC data available
	int dacacq=0;	//DAC acquire

	xil_printf("\n\rStarting DA2 Pmod demo test...\n\r");
    Xil_Out32(DA2acq,0);

    while(1)
    {
    	if(dacdata==4096)
    		dacdata=0;

    	dacdav=Xil_In32(DA2dav);
    	//xil_printf("dacdav %d  dacacq %d dacdata %d\n\r",dacdav, dacacq, dacdata);
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
    			dacdata++;						//increment DAC data
    			}
    }
 }
