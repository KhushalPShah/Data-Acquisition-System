#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LPC17xx.h"
#include "comm.h"
#include "monitor.h"
#include "lpc17xx_gpio.h"
#include "uart.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/dhcp.h"
#include "lwip/tcp_impl.h"
#include "lwip/opt.h"
#include "netif/ethernetif.h"
#include "netif/etharp.h"
#include "app/system_init.h"
#include "delay.h"
#include "uart.h"
#include "cypher.h"
#include "rtc_internal.h"
#include "diskio.h"
#include "can_driver.h"

/*******************************************************************************************/
/*				Global variables
********************************************************************************************/

//Make a Buffer for the UDP Message.
struct pbuf *udp_buf;

/****************************************************************************************
 * Function to initialise the controller
 
 ***************************************************************************************/
void config_controller()
{
	SysTick_Config(100000000 / TIMER_TICK_HZ);	// Controller clock is @ 100MHz

	LPC_GPIO0->FIODIR |= (1 << 10); 			// Pin P0.10 connected to onboard LED
												// direction set as output pin.
	
	//Initialize the communications.
	comm_init();
	
	//Intialize the UART/Serial communication. Not needed for now. 
	uart_init();
	
	//Intialize the LWIP Library. It is the library we use to implement the Networking part.
	lwip_init();
	
	//Intialize the On Board Real time clock to maintain a track of time.
	rtc_init();

}


/****************************************************************************************
 * Function to initialise the LWIP Library
 
 ***************************************************************************************/
static void lwip_init(void)
{
	// Intialize the memory to be used.
	mem_init();
	memp_init();

	// Initialize the Buffers to be used.
	pbuf_init();

	// Initialize the Ethernet Layer functions.
	etharp_init();

	// Intialize the IP Layer functions.
	ip_init();

	// Intialize the UDP Layer functions.
	udp_init();

	// Following is the details that the LPC1768 will use for itself.
	// Thus, its IP Address will be 192.168.1.100.
	{
		IP4_ADDR(&my_ipaddr_data,  192, 168, 1, 100);
		IP4_ADDR(&my_netmask_data, 255, 255, 255, 0);
		IP4_ADDR(&my_gw_data, 192, 168, 1, 30);
	}

	// Intializing the Ethernet Interface
	netif_add(netif_eth0, &my_ipaddr_data, &my_netmask_data, &my_gw_data, NULL, ethernetif_init, ethernet_input);

	netif_set_default(netif_eth0);

	
	// Making the Interface Active.
	netif_set_up(netif_eth0);
}

int main (int argc, char **argv)
{
	
	// All the struct based Variables are user defined Structure. 
	// PCB is a Protocol control Block. It stores the information pertaining to a Message:
	// For eg: NUmber of bytes in the Message, the Source and the Destination IP etc.
	
	struct udp_pcb *udpPcb;
	struct udp_pcb *udpPcb_recv;
	
	// Put the IP Address for the Destination(Server) in this variable.
	struct ip_addr ip_computer;
	
	// We put the IP Address as a Broadcast address. So, irrespective of the IP Address of the Computer(Server), it will
	// receive the UDP Message.
	ip_computer.addr = htonl(IPADDR_BROADCAST);
	
	
	config_controller();

	udpPcb = udp_new();										// Create a new UDP PCB

/*******************************************************************************************/
/***		Application follows
****************************************************************************************** */

	// Format in which information is transmitted :
	// TotalValue will be a 32 Bit Value
	// Out of Which, the Significant 16 bits will be used to store the ADC Value, i.e. adcVal 
	// The Lower 16 bits will be used to store the Digital Status,i.e. digitalStatus 
	// (0 or 1, although only 1 bit is needed, I used 16 to maintain the simplicity in the program).
	// Once the totalValue is computed using the above two variables, the totalValue is converted from an 
	// integer to an array of characters, using the function itoa.
	// itoa works something like:
	// 1357 (which will be the integer value of variable) -> ["1","3","5","7"]
	
	// We then send this array of character as a UDP Message to the Server.
	
	
	char* data = (char *)malloc(sizeof(char) * 10);
	// Initial Values
	uint16_t adcVal = 256;
	uint16_t digitalStatus = 1;
	uint32_t totalValue = 0;
	
	// This code does not read the real ADC values. It simulates the working by incrementing the 
	// ADC values on its own.
	// Same for Digital Status.
	
	while(1)
	{
		adcVal += 256;			// Increment ADC Value by 256.
		if(adcVal > 4096)		//Max value of ADC can be 4096, so if it crosses that, set it back to initial value
			adcVal = 256;
		if(digitalStatus)		//Toggle the Digital Status in each cycle.
			digitalStatus = 0;
		else
			digitalStatus = 1;

		totalValue = (adcVal << 16) | digitalStatus;	//Compute the totalValue
		itoa(totalValue,data,10);						// Convert into Array of characters
		udp_buf = pbuf_alloc(PBUF_TRANSPORT, 10, PBUF_RAM);	//Make a buffer to store the data.
		memcpy(udp_buf->payload, data, 10);					//Copy the data into the buffer.
		udp_sendto(udpPcb, udp_buf, &ip_computer, UDP_PORT);//Send the UDP Message.
		pbuf_free(udp_buf);								//Free memory allocated.
		delay_ms(2000);									//delay for 2 sec.
	}
	return 0;
}