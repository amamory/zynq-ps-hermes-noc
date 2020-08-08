/*
 * test_ports.c
 *
 *  Created on: 8 august 2020
 *      Author: Amory
 */

#include "xaxidma.h"
#include "xparameters.h"
#include "sleep.h"
#include "xil_cache.h"
#include "xscugic.h"

// Hermes packet size, including the flits for header and size
#define PACKET_SIZE 3

// PS is receiving from the device
#define RX_INTR_ID		XPAR_FABRIC_ZYNQ_AXI_DMA_0_S2MM_INTROUT_INTR
// PS is sending to the device
#define TX_INTR_ID		XPAR_FABRIC_ZYNQ_AXI_DMA_0_MM2S_INTROUT_INTR

u32 checkIdle(u32 baseAddress,u32 offset);
static void dmaTX_ISR(void *CallBackRef);
static void dmaRX_ISR(void *CallBackRef);

XScuGic IntcInstance;
XAxiDma myDma;

//this order will turn the ZED LEDs on from the LD7 to the LD2. LEDs 0 and 1 are not addressable in the NoC.
//                     R11 L, R11 S, R11 N,R11 E,  R01 S, R01 N, R01 L
u32 header_order[] = {0x0101,0x0100,0x0102,0x0201,0x0000,0x0002,0x0001};
#define NUM_PACKET 7

// data buffers
u32 hermes_pkg[] = {0x0001,0x0001,0x0003};
u32 hermes_pkg_in[10] = {0};

// Flags interrupt handlers use to notify the application context the events.
volatile int TxDone;
volatile int RxDone;
//volatile int Error;

int main(){
	//init_platform();
    u32 status;
    XScuGic_Config *IntcConfig;
	XAxiDma_Config *myDmaConfig;

	// =======================================
	// DMA initialization
	// =======================================
	myDmaConfig = XAxiDma_LookupConfigBaseAddr(XPAR_AXIDMA_0_BASEADDR);
	status = XAxiDma_CfgInitialize(&myDma, myDmaConfig);
	if(status != XST_SUCCESS){
		print("DMA initialization failed\n");
		return XST_FAILURE;
	}
	print("DMA initialization success...\n");

	// =======================================
	// Interrupt controller initialization
	// =======================================
	IntcConfig = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
	status = XScuGic_CfgInitialize(&IntcInstance, IntcConfig, IntcConfig->CpuBaseAddress);
	if(status != XST_SUCCESS){
		print("Interrupt Controller initialization failed\n");
		return XST_FAILURE;
	}

	// setting up tx interrupt
	XScuGic_SetPriorityTriggerType(&IntcInstance, TX_INTR_ID, 0xA0, 3);
	status = XScuGic_Connect(&IntcInstance, TX_INTR_ID, (Xil_InterruptHandler)dmaTX_ISR,&myDma);
	//status = XScuGic_Connect(&IntcInstance, TX_INTR_ID, (Xil_InterruptHandler)dmaISR,&myDma);
	if (status != XST_SUCCESS) {
		xil_printf("Failed tx connect intc\n");
		return XST_FAILURE;
	}
	XScuGic_Enable(&IntcInstance, TX_INTR_ID);

	// setting up rx interrupt
	XScuGic_SetPriorityTriggerType(&IntcInstance, RX_INTR_ID, 0xA1, 3);
	status = XScuGic_Connect(&IntcInstance, RX_INTR_ID, (Xil_InterruptHandler)dmaRX_ISR,&myDma);
	if (status != XST_SUCCESS) {
		xil_printf("Failed rx connect intc\n");
		return XST_FAILURE;
	}
	XScuGic_Enable(&IntcInstance, RX_INTR_ID);

	/* Enable interrupts from the hardware */
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,(void *)&IntcInstance);
	Xil_ExceptionEnable();
	print("Interrupt controller initialization success...\n");

	// =======================================
	// Enable DMA interrupts
	// =======================================
	// enable only interrupt completion
	XAxiDma_IntrEnable(&myDma, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
	/* Disable all interrupts before setup */
	XAxiDma_IntrDisable(&myDma, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);
	XAxiDma_IntrDisable(&myDma, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
	/* Enable all interrupts */
	XAxiDma_IntrEnable(&myDma, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);
	XAxiDma_IntrEnable(&myDma, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);


	// =======================================
	// Send data
	// =======================================
	xil_printf("Sending data transfer ...\n");
	// Flush the buffers before the DMA transfer, in case the Data Cache is enabled
	Xil_DCacheFlushRange((UINTPTR)hermes_pkg, PACKET_SIZE*sizeof(u32));
	Xil_DCacheFlushRange((UINTPTR)hermes_pkg_in, PACKET_SIZE*sizeof(u32));

	// Initialize flags before start transfer test
	TxDone = 0;
	RxDone = 0;

	// the last packet sent, to addr 0x0001, is attached to the DMA  input and will be received by Zynq
	// PS is receiving the packet from the noc
	status = XAxiDma_SimpleTransfer(&myDma, (u32)hermes_pkg_in, PACKET_SIZE*sizeof(u32),XAXIDMA_DEVICE_TO_DMA);
	if(status != XST_SUCCESS){
		print("DMA rx failed\n");
		return XST_FAILURE;
	}

	for (int i=0;i<NUM_PACKET;i++){
		// set the header
		hermes_pkg[0] = header_order[i];

		// Initialize flags before start transfer test
		TxDone = 0;
		// PS is sending the packet to the noc
		status = XAxiDma_SimpleTransfer(&myDma, (u32)hermes_pkg, PACKET_SIZE*sizeof(u32),XAXIDMA_DMA_TO_DEVICE);
		if(status != XST_SUCCESS){
			print("DMA tx failed\n");
			return XST_FAILURE;
		}

		// wait tx end or the timeout
		int cont = 2000;
		while(!(TxDone || (cont <= 0 ))){
			xil_printf("I am working!\n");
			cont --;
		}
		if (!cont){
			xil_printf("DMA tx timeout!\n");
			return XST_FAILURE;
		}

		// wait for 5 secs before sending a new packet
		sleep(5);
	}

	// wait the interrupt from the loopback or the timeout
	int cont = 2000;
	while(!((RxDone && TxDone) || (cont <= 0 ))){
		xil_printf("I am working!\n");
		cont --;
	}
	if (!cont){
		xil_printf("DMA timeout!\n");
		return XST_FAILURE;
	}


	xil_printf("Checking data ... ");
	Xil_DCacheInvalidateRange((UINTPTR)hermes_pkg_in, PACKET_SIZE*sizeof(int));
	if (memcmp(hermes_pkg,hermes_pkg_in,PACKET_SIZE*sizeof(int))==0){
		xil_printf("packets matched !!!\n");
	}else{
		xil_printf("packets do not matched !!!\n");
		for (int i =0; i<PACKET_SIZE; i++){
			xil_printf("sent [%x] and received [%x]\n", hermes_pkg[i],hermes_pkg_in[i]);
		}
	}
	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;

}

// if the data is static allocated, check for idle
// if the data is dynamically  allocated, check for halted
u32 checkIdle(u32 baseAddress,u32 offset){
	u32 status;
	status = (XAxiDma_ReadReg(baseAddress,offset))&XAXIDMA_IDLE_MASK;
	return status;
}

// interrupt handler for 'dma to device' data transfers
static void dmaTX_ISR(void *CallBackRef){
	u32 status;
	XAxiDma *AxiDmaInst = (XAxiDma *)CallBackRef;
	// disable the corresponding interrupt
	//XScuGic_Disable(&IntcInstance, TX_INTR_ID);
	XAxiDma_IntrDisable(AxiDmaInst, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);
	XAxiDma_IntrAckIrq(AxiDmaInst,  XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);
	xil_printf("int tx activated!\n");

	// avoid overwriting any previous unfinished dma transfer
	status = checkIdle(XPAR_AXIDMA_0_BASEADDR,0x4);
	while(status == 0){
	    	status = checkIdle(XPAR_AXIDMA_0_BASEADDR,0x4);
	}
	xil_printf("PS is sending to the NoC!\n");
	// enable the interrupt
	//XScuGic_Enable(&IntcInstance, TX_INTR_ID);
	XAxiDma_IntrEnable(AxiDmaInst,  XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);
	TxDone = 1;
}


// interrupt handler for 'device to dma' data transfers
static void dmaRX_ISR(void *CallBackRef){
	XAxiDma *AxiDmaInst = (XAxiDma *)CallBackRef;
	// disable the corresponding interrupt
	XAxiDma_IntrDisable(AxiDmaInst, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrAckIrq(AxiDmaInst,  XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
	xil_printf("int rx activated!\n");

	// do stuff - compare to the sent packet
	xil_printf("PS is receiving from the NoC!\n");

	// enable the interrupt
	XAxiDma_IntrEnable(AxiDmaInst,  XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
	RxDone = 1;
}

