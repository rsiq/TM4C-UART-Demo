#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
//#include "switch_counter_interrupt_TivaWare.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/uart.h"

//Some Code Copied and/or Modified from Dr. Xiaorong Zhang for use in SFSU ENGR 478
//All other code written by Rene Siqueiros

//*****************************************************************************
//
//!
//! Design a counter. The counter is incremented by 1 when SW1 (PF4) or SW2 (PF0) 
//! is pressed.
//
//*****************************************************************************

// global variable visible in Watch window of debugger
// increments at least once per button press
volatile unsigned long count = 0;
char str[3];
	
void
PortFunctionInit(void)
{
    //
    // Enable Peripheral Clocks 
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    //
    // Enable pin PF4 for GPIOInput
    //
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);

    //
    // Enable pin PF0 for GPIOInput
    //

    //
    //First open the lock and select the bits we want to modify in the GPIO commit register.
    //
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0x1;

    //
    //Now modify the configuration of the pins that we unlocked.
    //
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);
	
		//Enable pull-up on PF4 and PF0
		GPIO_PORTF_PUR_R |= 0x11; 

}


void
Interrupt_Init(void)
{
  IntEnable(INT_GPIOF);  							// enable interrupt 30 in NVIC (GPIOF)
	IntPrioritySet(INT_GPIOF, 0x00); 		// configure GPIOF interrupt priority as 0
	GPIO_PORTF_IM_R |= 0x11;   		// arm interrupt on PF0 and PF4
	GPIO_PORTF_IS_R &= ~0x11;     // PF0 and PF4 are edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;   	// PF0 and PF4 not both edges trigger 
  GPIO_PORTF_IEV_R |= 0x11;  	// PF0 and PF4 falling edge event
	
}

void
uart_Init(void) {
	
		SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
		//UARTFIFOEnable(UART1_BASE);

    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 115200,
        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}


//interrupt handler
void GPIOPortF_Handler(void)
{

			//SW1 is pressed
	if(GPIO_PORTF_RIS_R&0x10)
	{
		// acknowledge flag for PF4
		GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4); 
	}
	
	//SW2 is pressed
  if(GPIO_PORTF_RIS_R&0x01)
	{
		// acknowledge flag for PF0
		GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);
	}
	count++;
	SysCtlDelay(500000);
}

int main(void)
{
	int temp = 0;
	
		//initialize the GPIO ports	
		PortFunctionInit();
	
		//initialize UART0
		uart_Init();
		
		//configure the GPIOF interrupt
		Interrupt_Init();
	
		IntMasterEnable();       		// globally enable interrupt
		count = 996;
    //
    // Loop forever.
    //
    while(1)
    {
		if (temp != count)
		{
		sprintf(str, "%d", count);
		UARTCharPut(UART1_BASE, str[0]); 
		UARTCharPut(UART1_BASE, str[1]); 
		UARTCharPut(UART1_BASE, str[2]);
		UARTCharPut(UART1_BASE, ' ');
		UARTCharPut(UART1_BASE, '\n'); 
		UARTCharPut(UART1_BASE, '\r');	
	  temp = count;
		}
		else
		{
		
		}
		
    }
}
