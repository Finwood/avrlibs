#include "spi.h"

uint8_t spiWriteBuf[SPI_BUFFERSIZE] = { 0 };
volatile uint8_t spiIdxPending = 0, spiIdxCurrent = 0;
uint8_t _spiSendByte (void);
void _spiAppendBuffer (uint8_t data);
void (*_spiCallbackFunc)(void) = 0;

#define	_SPI_RECEIVE_IDLE		0
#define	_SPI_RECEIVE_PENDING	1
#define	_SPI_RECEIVE_WORKING	2

uint8_t _spiReceiveStatus = _SPI_RECEIVE_IDLE;
void (*_spiPreReceiveFunc)(void) = 0;
void (*_spiPostReceiveFunc)(void) = 0;

void spiInitMaster (void) {
	DDR_SPI |= SPI_MOSI | SPI_SCK | SPI_SS;
	DDR_SPI &= ~SPI_MISO;

	#ifdef SPI_LATCH
		DDR_LATCH |= SPI_LATCH;
	#endif

	// enable SPI, as Master, MSB first, set clock rate fck/16
	SPCR |= (1 << SPE) | (1 << MSTR) | (0 << DORD) | (0 << SPR1) | (1 << SPR0);

	// enable SPI Interrupt
	SPCR |= (1 << SPIE);

	// deselect input slave
	PORT_SPI |= SPI_SS;
}

void spiSend (uint8_t data) {
	spiSendMultiple(&data, 1);
}

void spiSendMultiple (uint8_t *data, uint8_t count) {
	spiSendMultipleWithCallback(data, count, 0);
}

void spiSendMultipleWithCallback (uint8_t *data, uint8_t count, void (*cbFunc)(void)) {
	for (uint8_t i = 0; i < count; i++)
		_spiAppendBuffer(data[i]);
	_spiCallbackFunc = cbFunc;
	_spiSendByte();
}

void _spiAppendBuffer (uint8_t data) {
	spiWriteBuf[spiIdxPending] = data;
	spiIdxPending = (spiIdxPending + 1) % SPI_BUFFERSIZE;
}

uint8_t _spiSendByte (void) {
	if (spiIdxPending != spiIdxCurrent) {
		if (_spiReceiveStatus == _SPI_RECEIVE_PENDING) {
			_spiReceiveStatus = _SPI_RECEIVE_WORKING;
			if (_spiPreReceiveFunc) _spiPreReceiveFunc();
			_spiPreReceiveFunc = 0;
			PORT_SPI &= ~SPI_SS;
		}
		SPDR = spiWriteBuf[spiIdxCurrent];
		spiIdxCurrent = (spiIdxCurrent + 1) % SPI_BUFFERSIZE;
		return 1;
	} else {
		return 0;
	}
}

void spiPassiveReceiveWithCallbacks (void (*preReceiveFunc)(void), void (*postReceiveFunc)(void)) {
	_spiPreReceiveFunc = preReceiveFunc;
	_spiPostReceiveFunc = postReceiveFunc;
	_spiReceiveStatus = _SPI_RECEIVE_PENDING;
}

// SPI Serial Transfer Complete
ISR (SPI_STC_vect) {
	if (_spiReceiveStatus == _SPI_RECEIVE_WORKING) {
		_spiReceiveStatus = _SPI_RECEIVE_IDLE;
		PORT_SPI |= SPI_SS;
		if (_spiPostReceiveFunc) _spiPostReceiveFunc();
		_spiPostReceiveFunc = 0;
	}
	if (!_spiSendByte()) {
		#ifdef SPI_LATCH
			// transfer shift to storage
			PORT_LATCH |= SPI_LATCH;
			PORT_LATCH &= ~SPI_LATCH;
		#endif
		if (_spiCallbackFunc) _spiCallbackFunc();
	}
}


