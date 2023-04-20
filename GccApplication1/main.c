/*
 * GccApplication1.c
 *
 * Created: 17/04/2023 14:02:19
 * Author : USER
 */ 

#define F_CPU 8000000UL					/* Define CPU Frequency e.g. here its 8MHz */
#include <avr/io.h>						/* Include AVR std. library file */
#include <util/delay.h>					/* Include inbuilt defined Delay header file */
#include <string.h>
#include <string.h>
#include "socket.h"
#include "w5100.h"

#define LCD_Command_Dir DDRC		/* Define LCD command port direction register */
#define LCD_Command_Port PORTC		/* Define LCD command port */
#define LCD_Data_Dir DDRE			/* Define LCD data port direction register */
#define LCD_Data_Port PORTE			/* Define LCD data port */
#define RS PD4							/* Define Register Select (data reg./command reg.) signal pin */
#define RW PD5							/* Define Read/Write signal pin */
#define EN PD3							/* Define Enable signal pin */

#define BUFFER_SIZE 128

static uint8_t mac_address[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
static uint8_t ip_address[4] = {192, 168, 1, 100};
static uint16_t port = 80;


void LCD_Command(unsigned char cmnd)
{
	LCD_Data_Port= cmnd;
	LCD_Command_Port &= ~(1<<RS);		/* RS=0 command reg. */
	LCD_Command_Port &= ~(1<<RW);		/* RW=0 Write operation */
	LCD_Command_Port |= (1<<EN);		/* Enable pulse */
	_delay_us(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(3);
}

void LCD_Char (unsigned char char_data)	/* LCD data write function */
{
	LCD_Data_Port= char_data;
	LCD_Command_Port |= (1<<RS);		/* RS=1 Data reg. */
	LCD_Command_Port &= ~(1<<RW);		/* RW=0 write operation */
	LCD_Command_Port |= (1<<EN);		/* Enable Pulse */
	_delay_us(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(1);
}

void LCD_Init (void)					/* LCD Initialize function */
{
	LCD_Command_Dir = 0xFF;				/* Make LCD command port direction as o/p */
	LCD_Data_Dir = 0xFF;				/* Make LCD data port direction as o/p */
	_delay_ms(20);						/* LCD Power ON delay always >15ms */
	
	LCD_Command (0x38);					/* Initialization of 16X2 LCD in 8bit mode */
	LCD_Command (0x0C);					/* Display ON Cursor OFF */
	LCD_Command (0x06);					/* Auto Increment cursor */
	LCD_Command (0x01);					/* clear display */
	LCD_Command (0x80);					/* cursor at home position */
}

void LCD_String (char *str)				/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)				/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_String_xy (char row, char pos, char *str)	/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);		/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);		/* Command of first row and required position<16 */
	LCD_String(str);					/* Call LCD string function */
}

void LCD_Clear()
{
	LCD_Command (0x01);					/* clear display */
	LCD_Command (0x80);					/* cursor at home position */
}

int main()
{

	LCD_Init();							/* Initialize LCD */

	LCD_String("ElectronicWINGS");		/* write string on 1st line of LCD*/
	LCD_Command(0xc0);					/* Go to 2nd line*/
	LCD_String("Hello World");			/* Write string on 2nd line*/
	
	uint8_t buffer[BUFFER_SIZE];
	uint16_t data_size = 0;
	uint8_t socket_number = 0;

	// Initialize the W5100
	w5100_init();

	// Set the MAC address
	w5100_setMACAddress(mac_address);

	// Set the IP address
	w5100_setIPAddress(ip_address);

	// Open a TCP socket on the specified port
	socket_number = socketTCPOpen(port);

	while (1) {
		// Check if data is available
		if (socketRecvAvailable(socket_number) > 0) {
			// Read the data
			data_size = socketRecv(socket_number, buffer, BUFFER_SIZE);

			// Do something with the data
			// For example, print it to the serial monitor
			for (int i = 0; i < data_size; i++) {
				printf("%c", buffer[i]);
			}

			// Send a response
			socketSend(socket_number, buffer, data_size);
		}
	}

	return 0;
}
