/*
 * LUFA_CDC.c
 *
 * Created: 14/03/2014 01:52:15
 *  Author: Stanley Lio
 */

/************************************************************************
Polling works so well that I'm not even sure if I should move to the IRQ
method. Especially considering how much trouble I had with my previous
attempt...
************************************************************************/

#include "ArduinoProMicro.h"
#include "VirtualSerial.h"
#include "NRF24L01.h"
#include <avr/io.h>
#include <util/delay.h>

USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
{
	.Config =
	{
		.ControlInterfaceNumber   = 0,
		.DataINEndpoint           =
		{
			.Address          = CDC_TX_EPADDR,
			.Size             = CDC_TXRX_EPSIZE,
			.Banks            = 1,
		},
		.DataOUTEndpoint =
		{
			.Address          = CDC_RX_EPADDR,
			.Size             = CDC_TXRX_EPSIZE,
			.Banks            = 1,
		},
		.NotificationEndpoint =
		{
			.Address          = CDC_NOTIFICATION_EPADDR,
			.Size             = CDC_NOTIFICATION_EPSIZE,
			.Banks            = 1,
		},
	},
};

static inline void beep(void);
static inline void beep()
{
	set(D9PORT,D9);
	_delay_us(25);		// over 20us
	clear(D9PORT,D9);
}

int main(void)
{
	SetupHardware();
	//GlobalInterruptEnable();
	
	nRF24_as_transmitter();
	nRF24_put_in_FIFO('a');
	
	while (1)
	{
		loop_until_bit_is_clear(D3PIN,D3);
		LED_amber_ON;

		beep();
		
		// send a byte via RF. the char should already be in the tx buffer now
		set(PORTF,CE);		// toggle CE to send
		_delay_us(20);		// at least 10uS
		clear(PORTF,CE);
		
		nRF24_put_in_FIFO('a');

		/*while (!nRF24_TX_FIFO_EMPTY())
		{
			set(PORTF,CE);		// toggle CE to send
			_delay_us(15);		// at least 10uS
			clear(PORTF,CE);
			//_delay_us(2);		// whatever
		}*/
		
		_delay_ms(100);
		loop_until_bit_is_set(D3PIN,D3);
		_delay_ms(200);

		LED_amber_OFF;
	}

/*	while (1)
	{
		while (CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface))
		{
			int16_t b = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
			if ('a' == b)
			{
				set(D9PORT,D9);
				_delay_us(22);		// over 20us
				clear(D9PORT,D9);
				
				nRF24_put_in_FIFO('a');
				
				//if (!nRF24_TX_FIFO_EMPTY())
				{
					LED_amber_ON;
					nRF24_as_transmitter();
			
					// keep sending as long as:
					// 1. there's still stuff to send in TX FIFO, or
					// 2. max # of retry reached
					while (!nRF24_TX_FIFO_EMPTY())
					{
						set(PORTF,CE);		// toggle CE to send
						_delay_us(12);		// at least 10uS
						clear(PORTF,CE);
						//_delay_us(2);		// whatever
					}
					LED_amber_OFF;
				}
			}
		}
		
		// - - - - -
		// LUFA CDC overhead
		// - - - - -
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);		// MUST CALL THESE PERIODICALLY and FREQUENTLY! todo: how frequently?
		USB_USBTask();
	}*/
}


void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	LED_amber_INIT;
	LED_amber_OFF;
	LED_green_INIT;
	LED_green_OFF;
	
	init_nRF24L01();
	
	// ultrasonic sensor TX EN
	set(D9DDR,D9);
	clear(D9PORT,D9);
	
	set(D2DDR,D2);
	clear_D2;
	clear(D3DDR,D3);
	set_D3;
	
	//USB_Init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	//
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	//
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

