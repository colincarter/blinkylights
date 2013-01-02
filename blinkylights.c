#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdint.h>

typedef enum {FALSE=0, TRUE} bool_t;

#define LED0_PIN	PB0
#define LED1_PIN	PB1
#define BUTTON_PIN	PB2

bool_t button_pressed(void);
void solid(void);
void fade(void);

// Sine table generated from http://www.daycounter.com/Calculators/Sine-Generator-Calculator2.phtml
const uint8_t PROGMEM sine_table[] = {
	1,1,2,3,5,6,8,10,12,15,17,20,23,26,29,33,36,40,44,
	48,53,57,61,66,71,76,81,86,91,96,101,106,112,117,
	122,128,133,138,143,149,154,159,164,169,174,179,184,
	189,194,198,202,207,211,215,219,222,226,229,232,235,
	238,240,243,245,247,249,250,252,253,254,254,255,255,
	255,255,254,254,253,252,250,249,247,245,243,240,238,
	235,232,229,226,222,219,215,211,207,202,198,194,189,
	184,179,174,169,164,159,154,149,143,138,133,128,122,
	117,112,106,101,96,91,86,81,76,71,66,61,57,53,48,44,
	40,36,33,29,26,23,20,17,15,12,10,8,6,5,3,2,1,1,0
};

int main(void)
{
	// Everything output
	DDRB = 0xFF;

//	DDRB &= ~_BV(BUTTON_PIN);

	// Pull up resistor on button pin
//	PORTB |= _BV(BUTTON_PIN);

	// Set Fast PWM mode
	TCCR0A |= ( _BV(WGM00) | _BV(WGM01) );
	TCCR0B |= ( _BV(CS01) | _BV(CS00) );

	// Clear OC0A on Compare Match, set OC0A at TOP
	// Clear OC0B on Compare Match, set OC0B at TOP
	TCCR0A |= ( _BV(COM0A1) | _BV(COM0B1) );

	OCR0A = 0;
	OCR0B = 0;

	while(1)
	{
		solid();
		fade();
	}

	return 0;
}

void solid(void)
{
	PORTB |= _BV(LED0_PIN);

	while(!button_pressed())
		;
}

void fade(void)
{
	const register uint8_t *cstp = sine_table;
	register uint8_t cstv;

	while(1)
	{
		if(button_pressed())
		{
			break;
		}

		cstv = pgm_read_byte_near(cstp);

		if(cstv == 0)
		{
			cstp = sine_table;
			continue;
		}

		OCR0A = cstv;
		OCR0B = cstv;

				// Need to sleep here
		_delay_ms(50);

		cstp++;
	}
}

bool_t button_pressed(void)
{
	if(bit_is_clear(PORTB, BUTTON_PIN))
	{
		_delay_ms(2);
		if(bit_is_clear(PORTB, BUTTON_PIN))
			return TRUE;
	}

	return FALSE;
}

EMPTY_INTERRUPT(WDT_vect);
