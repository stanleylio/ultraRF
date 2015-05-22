/*
 * LUFA_CDC.c
 *
 * Created: 14/03/2014 01:52:15
 *  Author: Stanley Lio
 */

#include "ArduinoProMicro.h"
#include "VirtualSerial.h"
#include "NRF24L01.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#define SEQ_LENGTH	6000

// use ADC free-running mode
// buffer and group before CDC send

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

volatile uint16_t seq_idx = 0;
volatile uint16_t old_adc = UINT16_MAX;

//volatile uint8_t S[SEQ_LENGTH];

ISR(ADC_vect)
{
	if (seq_idx < SEQ_LENGTH)
	{
		char sbuf[20];
		uint16_t val;
		uint16_t got;
		
		LED_amber_ON;
		val = ADC;
		seq_idx++;
		
		//got = snprintf(sbuf,sizeof(sbuf),"%04d,%05hu,%05hu,%05hu\n",seq_idx,cnt,val,old_adc);
		//CDC_Device_SendData(&VirtualSerial_CDC_Interface,sbuf,got);
		//_delay_us(200);

		//S[seq_idx] = ADC >> 2;
		
		if ((val > old_adc) && ((val - old_adc) > 10))
		{
			//CDC_Device_SendString(&VirtualSerial_CDC_Interface,"HIT!\n");
			//got = snprintf(sbuf,sizeof(sbuf),"%04d,%05hu,%05hu\n",seq_idx,val,old_adc);
			//CDC_Device_SendData(&VirtualSerial_CDC_Interface,sbuf,got);
			
			//got = snprintf(sbuf,sizeof(sbuf),"%04hu\n",seq_idx);
			//got = snprintf(sbuf,sizeof(sbuf),"%.3f\n",(tmp - 1 - 397)/56.0);		// "not implemented" that's lame...
			
			dtostrf(((double)seq_idx - 397)/56.0,2,2,sbuf);
			sbuf[4] = 'm';
			sbuf[5] = '\n';
			got = 6;
			CDC_Device_SendData(&VirtualSerial_CDC_Interface,sbuf,got);

			// stop ADC, reset seq_idx, old_adc and timer
			clear(ADCSRA,ADEN);
			set(ADCSRA,ADIF);
			seq_idx = 0;
			old_adc = UINT16_MAX;
		}
		
		old_adc = val;
		
		LED_amber_OFF;
	}
	else
	{
		// async code is tricky business...
		clear(ADCSRA,ADEN);
		set(ADCSRA,ADIF);
		seq_idx = 0;
		old_adc = UINT16_MAX;

		/*uint16_t i = 0;
		char sbuf[30];
		uint16_t got;
		for (i = 0; i < SEQ_LENGTH; i++)
		{
			got = snprintf(sbuf,sizeof(sbuf),"%04d,%03hu\n",i,S[i]);
			CDC_Device_SendData(&VirtualSerial_CDC_Interface,sbuf,got);
		}*/
	}
}

int main(void)
{
	SetupHardware();
	GlobalInterruptEnable();

	nRF24_as_receiver();
	
	clear(A0DDR,A0);
	clear_A0;
	DIDR0 = 0b10000000;
	ADMUX = 0b01000111;			// AVCC as reference, MUX=ADC7
	clear(ADCSRA,ADEN);
	set(ADCSRA,ADIE);
	set(ADCSRA,ADATE);
	ADCSRA |= 0b00000110;		// XTAL/64
	//ADCSRA |= 0b00000111;		// XTAL/128
	//ADCSRA += 2;				// XTAL/4
	set(ADCSRB,ADHSM);
	sei();
	
	while (1)
	{
		if (!nRF24_RX_FIFO_EMPTY())
		{
			int b = nRF24_get_from_FIFO();

			if (('a' == b) && (seq_idx <= 0))
			{
				LED_green_ON;
				
				// start ADC (free-running)
				old_adc = 65535;
				set(ADCSRA,ADEN);
				set(ADCSRA,ADSC);
				//_delay_ms(1);
				LED_green_OFF;
			}
		}
		
		// - - - - -
		// LUFA CDC overhead
		// - - - - -
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
	}
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
	
	set(D9DDR,D9);
	clear(D9PORT,D9);
	
	USB_Init();
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

