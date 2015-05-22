/*
 * NRF24L01.h
 *
 * Created: 15/08/2014 12:52:38
 *  Author: Stanley Lio
 */ 


#ifndef NRF24L01_H_
#define NRF24L01_H_

#define CE	PF6
#define CSN	PF5
#define IRQ	PB6

#define R_REGISTER			0
#define W_REGISTER			0b00100000
#define R_RX_PAYLOAD		0b01100001
#define W_TX_PAYLOAD		0b10100000
#define FLUSH_TX			0b11100001
#define FLUSH_RX			0b11100010
#define REUSE_TX_PL			0b11100011
#define R_RX_PL_WID			0b01100000
#define	W_ACK_PAYLOAD		0b10101000
#define W_TX_PAYLOAD_NOACK	0b10110000
#define NOP					0xFF

#define CONFIG				0x00
	#define PRIM_RX			0
	#define PWR_UP			1
	#define CRCO			2
	#define EN_CRC			3
	#define MASK_MAX_RT		4
	#define MASK_TX_DS		5
	#define MASK_RX_DR		6
#define EN_AA				0x01
#define EN_RXADDR			0x02
#define	SETUP_AW			0x03
#define SETUP_RETR			0x04
#define RF_CH				0x05
#define RF_SETUP			0x06
#define STATUS				0x07
	#define MAX_RT			4
	#define TX_DS			5
	#define RX_DR			6
#define OBSERVE_TX			0x08
#define RPD					0x09
#define	RX_ADDR_P0			0x0A
#define	RX_ADDR_P1			0x0B
#define	RX_ADDR_P2			0x0C
#define	RX_ADDR_P3			0x0D
#define	RX_ADDR_P4			0x0E
#define	RX_ADDR_P5			0x0F
#define TX_ADDR				0x10
#define RX_PW_P0			0x11
#define RX_PW_P1			0x12
#define RX_PW_P2			0x13
#define RX_PW_P3			0x14
#define RX_PW_P4			0x15
#define RX_PW_P5			0x16
#define FIFO_STATUS			0x17
	#define TX_REUSE		6
	#define TX_FULL			5
	#define TX_EMPTY		4
	#define RX_FULL			1
	#define RX_EMPTY		0
#define DYNPD				0x1C
#define FEATURE				0x1D
	#define EN_DYN_ACK		0


#include "LUFA/Drivers/Peripheral/SPI.h"
#include "util/delay.h"

static inline void nRF24_put_in_FIFO(uint8_t val)
{
	clear(PORTF,CSN);
	SPI_TransferByte(W_TX_PAYLOAD);
	//SPI_TransferByte(W_TX_PAYLOAD_NOACK);
	SPI_TransferByte(val);
	set(PORTF,CSN);
}

static inline uint8_t nRF24_get_from_FIFO(void)
{
	uint8_t tmp = 0;
	clear(PORTF,CSN);
	SPI_TransferByte(R_RX_PAYLOAD);
	tmp = SPI_TransferByte(NOP);
	set(PORTF,CSN);
	return tmp;
}

static inline void write_register(uint8_t reg, uint8_t val)
{
	clear(PORTF,CSN);
	SPI_TransferByte(W_REGISTER | reg);
	SPI_TransferByte(val);
	set(PORTF,CSN);
}

static inline uint8_t read_register(uint8_t reg)
{
	uint8_t tmp = 0;
	clear(PORTF,CSN);
	SPI_TransferByte(R_REGISTER | reg);
	tmp = SPI_TransferByte(NOP);
	set(PORTF,CSN);
	return tmp;
}

static inline void flush_rx(void)
{
	clear(PORTF,CSN);
	SPI_TransferByte(FLUSH_RX);
	set(PORTF,CSN);
}

static inline void flush_tx(void)
{
	clear(PORTF,CSN);
	SPI_TransferByte(FLUSH_TX);
	set(PORTF,CSN);
}

static void init_nRF24L01(void)
{
	// nRF24L01+ control pins
	set(DDRF,CE);
	set(DDRF,CSN);
	clear(DDRB,IRQ);							// now on PB6
	clear(PORTF,CE);
	set(PORTF,CSN);

	// SPI	might not need these
	DDRB |= 0b00000110;							// MOSI and SCK as output, PB6 as IRQ input
	SPCR = _BV(SPE) | _BV(MSTR);				// Enable SPI as Master
	set(SPSR,SPI2X);							// awesome speed (fclk/2)

	write_register(CONFIG,0b00111110);			// POWER UP, 2-byte CRC, as PTX, mask TX_DS and MAX_RT
	_delay_ms(5);								// Tpd2stby, 4.5mS minimum for crystal
	write_register(EN_AA,0);					// disable auto retransmission
	write_register(SETUP_AW,0b00000001);		// 3-byte address field
	write_register(SETUP_RETR,0);				// 0 retry
	write_register(RF_CH,77);					// channel = 2400MHz + val MHz
	//write_register(RF_SETUP,0b00100110);		// air rate=250kbps, power=0dBm
	//write_register(RF_SETUP,0b00100000);		// air rate=250kbps, power=-18dBm
	//write_register(RF_SETUP,0b00000110);		// air rate=1Mbps, power=0dBm
	write_register(RF_SETUP,0b00001110);		// air rate=2Mbps, power=0dBm
	write_register(RX_PW_P0,1);					// pipe0 payload length=1byte
	write_register(FEATURE,EN_DYN_ACK);			// enable no ack
	flush_rx();
	flush_tx();
	write_register(STATUS,0b01110000);			// clear all interrupt flags
}

static inline uint8_t nRF24_TX_FIFO_EMPTY(void)
{
	uint8_t tmp = read_register(FIFO_STATUS);
	return bit_is_set(tmp,TX_EMPTY);
}

static inline uint8_t nRF24_TX_FIFO_FULL(void)
{
	uint8_t tmp = read_register(FIFO_STATUS);
	return bit_is_set(tmp,TX_FULL);
}

static inline uint8_t nRF24_RX_FIFO_EMPTY(void)
{
	uint8_t tmp = read_register(FIFO_STATUS);
	return bit_is_set(tmp,RX_EMPTY);
}

static inline uint8_t nRF24_RX_FIFO_FULL(void)
{
	uint8_t tmp = read_register(FIFO_STATUS);
	return bit_is_set(tmp,RX_FULL);
}

static inline uint8_t nRF24_MAX_RETRY_REACHED(void)
{
	uint8_t tmp = read_register(STATUS);
	return bit_is_set(tmp,MAX_RT);
}

static inline void nRF24_as_transmitter(void)
{
	uint8_t config = read_register(CONFIG);
	clear(config,PRIM_RX);
	write_register(CONFIG,config);
	_delay_us(130);
	clear(PORTF,CE);
	_delay_us(130);
}

static inline void nRF24_as_receiver(void)
{
	uint8_t config = read_register(CONFIG);
	set(config,PRIM_RX);
	write_register(CONFIG,config);
	_delay_us(130);
	set(PORTF,CE);
	_delay_us(130);
}

#endif /* NRF24L01_H_ */