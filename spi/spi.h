#ifndef __SPI_H__
#define __SPI_H__

#include <avr/io.h>
#include <avr/interrupt.h>

#define DDR_SPI		DDRB
#define PIN_SPI		PINB
#define PORT_SPI	PORTB

#define SPI_MOSI	(1 << PB3)
#define SPI_MISO	(1 << PB4)
#define SPI_SCK		(1 << PB5)
#define SPI_SS		(1 << PB2)

#define	DDR_LATCH	DDRD
#define	PORT_LATCH	PORTD

#define SPI_LATCH	(1 << PD7)

#define SPI_BUFFERSIZE	8

uint8_t spiWriteBuf[SPI_BUFFERSIZE];

void spiInitMaster (void);
void spiSend (uint8_t data);
void spiSendMultiple (uint8_t *data, uint8_t count);
void spiSendMultipleWithCallback (uint8_t *data, uint8_t count, void (*cbFunc)(void));

void spiPassiveReceiveWithCallbacks (void (*preReceiveFunc)(void), void (*postReceiveFunc)(void));

#endif // __SPI_H__

