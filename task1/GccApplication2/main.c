#define F_CPU 8000000UL
#include <avr/io.h>
#include <stdint.h> // needed for uint8_t
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>


#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD -1
volatile char ReceivedChar;
volatile uint8_t ADCvalue;



int USART0SendByte(char u8Data, FILE *stream)
{

	while(!(UCSR0A&(1<<UDRE0))){};
	// Transmit data
	UDR0 = u8Data;
	return 0;
}

ISR(TIMER1_OVF_vect)
{
	printf("temperature %u F\r\n ", ((ADCvalue*9)/10)+32 );
	TCNT1 = 57724;	
}

ISR (USART_RX_vect)
{
	ReceivedChar = UDR0; // Read data from the RX buffer
	UDR0 = ReceivedChar; // Write the data to the TX buffer
}
ISR(ADC_vect)
{
	ADCvalue = ADCH; // only need to read the high value for 8 bit
}
//set stream pointer
FILE usart0_str = FDEV_SETUP_STREAM(USART0SendByte,NULL,  _FDEV_SETUP_RW);

int main( void )
{
	stdin = stdout = &usart0_str;
	/*Set baud rate */
	UBRR0H = (MYUBRR >> 8);
	UBRR0L = MYUBRR;
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0)|(1<<RXCIE0); // Enable receiver and transmitter
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ00); // Set frame: 8data, 1 stp
	
	ADMUX = 0; // use ADC0
	ADMUX |= (1 << REFS0); // use AVcc as the reference
	ADMUX |= (1 << ADLAR); // Right adjust for 8 bit resolution
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 128 prescale for 16Mhz
	ADCSRA |= (1 << ADATE); // Set ADC Auto Trigger Enable
	ADCSRB = 0; // 0 for free running mode
	ADCSRA |= (1 << ADEN); // Enable the ADC
	ADCSRA |= (1 << ADIE); // Enable Interrupts
	ADCSRA |= (1 << ADSC); // Start the ADC conversion
	
	TIMSK1 = (1<<TOIE1);
	TCNT1 = 57724; // 65535 - 7811
	TCCR1B = (1<<CS12)|(1<<CS10);	
	sei();
	while(1)
	{
		
	}
	return 1;
}