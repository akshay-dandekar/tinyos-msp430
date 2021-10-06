
#ifndef UART1_H
#define	UART1_H

#include <msp430.h>
#include <stdint.h>
#define CTRL_Z 26

enum uart_baudrate {
	  UART_BAUDRATE_2400
	, UART_BAUDRATE_4800
	, UART_BAUDRATE_9600
	, UART_BAUDRATE_19200
	, UART_BAUDRATE_38400
	, UART_BAUDRATE_57600
	, UART_BAUDRATE_115200
	, UART_BAUDRATE_END
};

enum uart_format {
	  UART_FORMAT_8N1
	, UART_FORMAT_8N2
	, UART_FORMAT_8E1
	, UART_FORMAT_8E2
	, UART_FORMAT_8O1
	, UART_FORMAT_8O2
	, UART_FORMAT_END
};

void uart1_init(enum uart_baudrate br, enum uart_format cfg);
void uart1_trans( uint8_t data );
void uart1_send_int( int32_t num ); 
void uart1_send_str( uint8_t *data, int len ); 


#endif
