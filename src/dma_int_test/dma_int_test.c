/*
 * hermes_test.c
 *
 *  Created on: 7 de jun de 2020
 *      Author: lsa
 */

#include "xaxidma.h"
#include "xparameters.h"
#include "sleep.h"
//#include "xil_cache.h"
#include "xscugic.h"

//#include "platform.h"

/*
 *
#ifdef XPAR_INTC_0_DEVICE_ID
#define RX_INTR_ID		XPAR_INTC_0_AXIDMA_0_S2MM_INTROUT_VEC_ID
#define TX_INTR_ID		XPAR_INTC_0_AXIDMA_0_MM2S_INTROUT_VEC_ID
#else
#define RX_INTR_ID		XPAR_FABRIC_AXIDMA_0_S2MM_INTROUT_VEC_ID
#define TX_INTR_ID		XPAR_FABRIC_AXIDMA_0_MM2S_INTROUT_VEC_ID
#endif
 *
 */

#define RX_INTR_ID		0
#define TX_INTR_ID		1

static void dmaISR();

XScuGic IntcInstance;
XAxiDma myDma;

int main(){
	//init_platform();
	u32 hermes_pkg[] = {0x0001,0x0001,0x0003};
	u32 hermes_pkg_in[10] = {0};
    u32 status;
    XScuGic_Config *IntcConfig;

	XAxiDma_Config *myDmaConfig;

	IntcConfig = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);

	status = XScuGic_CfgInitialize(&IntcInstance, IntcConfig, IntcConfig->CpuBaseAddress);

	if(status != XST_SUCCESS){
		print("Interrupt Controller initialization failed\n");
		return -1;
	}

	//void XScuGic_SetPriorityTriggerType(&IntcInstance, u32 Int_Id, u8 Priority, u8 Trigger);
	XScuGic_SetPriorityTriggerType(&IntcInstance, TX_INTR_ID, 0xA0, 3);


	status = XScuGic_Connect(&IntcInstance, TX_INTR_ID, (Xil_InterruptHandler)dmaISR,&myDma);
	//status = XScuGic_Connect(&IntcInstance, TX_INTR_ID, (Xil_InterruptHandler)dmaISR,0);
	if (status != XST_SUCCESS) {
		return status;
	}

	XScuGic_Enable(&IntcInstance, TX_INTR_ID);


	/* Enable interrupts from the hardware */

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,(void *)&IntcInstance);
	Xil_ExceptionEnable();


	// send data
	xil_printf("Status before data transfer %0x\n",status);
	Xil_DCacheFlushRange((u32)hermes_pkg,3*sizeof(u32));

	status = XAxiDma_SimpleTransfer(&myDma, (u32)hermes_pkg_in, 3*sizeof(u32),XAXIDMA_DEVICE_TO_DMA);
	if(status != XST_SUCCESS){
		print("DMA initialization failed\n");
		return -1;
	}

	// wait the interrupt from the loopback
	while(1){
		xil_printf("I am working!\n");
	}


}


static void dmaISR(){
	// disable the corresponding interrupt
	XScuGic_Disable(&IntcInstance, TX_INTR_ID);
	//Xil_Out32(addr,0x0)
	// read stuff
	//xil_printf("%d\n",Xil_In32(addr));
	// enable the interrupt
	//Xil_Out32(addr,0x1)
	XScuGic_Enable(&IntcInstance, TX_INTR_ID);

}
