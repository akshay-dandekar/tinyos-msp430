/* UART */
#include <msp430.h>
#include <stdint.h>

#include "uart1.h"

const uint8_t uart_mctl_lookup[] = {
	  0x92		// BAUDRATE 2400
	, 0x6D		// BAUDRATE 4800
	, 0x92		// BAUDRATE 9600
	, 0x6D		// BAUDRATE 19200
	, 0x92		// BAUDRATE 38400
	, 0xef		// BAUDRATE 57600
	, 0xAA		// BAUDRATE 115200
};

const uint16_t uart_ubrr_lookup[] = {
	  0x0D05		// BAUDRATE 2400
	, 0x0682		// BAUDRATE 4800
	, 0x0341		// BAUDRATE 9600
	, 0x01a0		// BAUDRATE 19200
	, 0x00D0		// BAUDRATE 38400
	, 0x008A		// BAUDRATE 57600
	, 0x0045		// BAUDRATE 115200
};


void uart1_init(enum uart_baudrate br, enum uart_format fmt) {
		/* UART logic held in RESET */
		U1CTL |= (1 << 0);

		/* Set stop bits format */
		switch(fmt % 2) {
			/* 1 stop bits */
			case 0:
				U1CTL &= ~(1 << 5);
				break;
			/* 2 stop bits */
			case 1:
				U1CTL |= (1 << 5);
				break;
		}

		/* Set parity */
		switch(fmt / 2) {
			/* None Parity */
			case 0:
				U1CTL &= ~(1 << 7);
				break;
			/* Even Parity */
			case 1:
				U1CTL &= ~(1 << 7);
				U1CTL |= (1 << 6);
				break;
			/* Odd Parity */
			case 2:
				U1CTL &= ~(1 << 7);
				U1CTL &= ~(1 << 6);
				break;
		}

		/* Set data format to 8Bits */
		U1CTL |= (1 << 4);

		/* Set UART Clock to SMCLK 8MHz 
		 * first clear the bit and then select SMCLK */
		U1TCTL &= ~( 3 << 4 );
		U1TCTL |=  ( 2 << 4 );

		/* Set Baud rate to 9600 for 833.3KHZ */
		U1BR0  = (uart_ubrr_lookup[br] & 0x00FF);                      
		U1BR1  = (uart_ubrr_lookup[br] & 0xFF00) >> 8;

		/* Modulation factor for baud correction */
		U1MCTL = uart_mctl_lookup[br];

		/* Transmit enable */                      
		ME2 |= (1 << 5);

		/* Receive enable */
		ME2 |= (1 << 4);

		/* USART reset released for operation */                     
		U1CTL  &= ~(1 << 0);

		// /* Receive Interrupt enable */
		// IE2 |= (1 << 4);
}

void uart1_trans( uint8_t data ) {
	uint8_t temp;
	U1TXBUF = data;

	/* wait till transmission is complete */	
	do {
			temp = (IFG2 & 0x20);
	} while (temp == 0); 
}

void uart1_send_str(uint8_t *data, int len) {
	int i;

	if(len < 0) {
		return;
	}

	if(len == 0) {
		while(*data != 0x00) {
			uart1_trans(*data);
			data++;
		}
	} else {
		i = 0;
		while(i < len) {
			uart1_trans(data[i]);
			i++;
		}
	}

}

void uart1_send_int( int32_t num ) {
	int32_t j, res;
	int flag;
	char arr[9];

	/* Set every place in array to '0' */
	for(j = 0; j < sizeof(arr); j++) {
		arr[j] = '0';
	}

	/* If number is negative set the flag */
	if (num < 0) {
		flag = 1;
	}

	j = sizeof(arr) - 1;
	while(num != 0) {
		res = num % 10;
		num = num / 10;

		/* Convert to ascii */
		if(res < 0) {
			/* In case of negative res convert to +ve */
			res *= -1;
		}
		arr[j] = (char) (res + '0');

		j--;
	}

	if (flag == 1) {
		arr[0] = '-';
	}

	for(j = 0; j < sizeof(arr); j++) {
		uart1_trans(arr[j]);
	}
}
