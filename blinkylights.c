#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/delay.h>
#include <stdint.h>

typedef enum {FALSE=0, TRUE} bool_t;

#define LED0_PIN  PB0
#define BUTTON_PIN  PB3

void init(void);
void solid(void);
void fade(void);
void blink(void);
void wait_for_watchdog(uint8_t period);
void sleep_mcu(void);
void pwm_on(void);
void pwm_off(void);
bool_t check_button(void);

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

volatile bool_t button_pressed = FALSE;

int main(void)
{
  init();

  while(1)
  {
    solid();
    fade();
    blink();
  }

  return 0;
}

void init(void)
{
  uint8_t status = MCUSR;

  MCUSR = 0;

  // Determine if the reset source was a brownout
  if(bit_is_set(status, BORF))
  {
    // Don't do any more
    while(1)
    {
    }
  }

  cli();

  DDRB = 0;
  DDRB = _BV(LED0_PIN);

  // Enable pull-up resistors on input ports
  PORTB = _BV(BUTTON_PIN) |
      _BV(PB1) |
      _BV(PB2) |
      _BV(PB5);

  // Pin change interrupt
  GIMSK |= _BV(PCIE);
  PCMSK |= _BV(BUTTON_PIN);
  GIFR |= _BV(PCIF);

  sei();
}

void solid(void)
{
  while(1)
  {
    PORTB |= _BV(LED0_PIN);

    sleep_mcu();

    if(check_button())
    {
      break;
    }

  }
}

void fade(void)
{
  const register uint8_t *cstp = sine_table;
  register uint8_t cstv;

  OCR0A = 0;

  pwm_on();

  while(1)
  {
    if(check_button())
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

    wait_for_watchdog(WDTO_30MS);

    cstp++;
  }

  pwm_off();
}

void blink(void)
{
  uint8_t level = 0xFF;

  pwm_on();

  while(1)
  {
    if(check_button())
    {
      break;
    }

    OCR0A = level;

    wait_for_watchdog(WDTO_500MS);

    level ^= 0xFF;
  }

  pwm_off();
}

void pwm_on(void)
{
  TCCR0A |= ( _BV(WGM00) | _BV(WGM01) | _BV(COM0A1) );
  TCCR0B |= ( _BV(CS01) | _BV(CS00) );
}

void pwm_off(void)
{
  TCCR0A = 0;
  TCCR0B = 0;
}

void wait_for_watchdog(uint8_t period)
{
  cli();

  wdt_reset();

  WDTCR = (_BV(WDTIF) | _BV(WDTIE) | period);

  sei();

  sleep_mcu();

  // Disable watchdog interrupt
  WDTCR &= ~_BV(WDTIE);
}

void sleep_mcu(void)
{
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
  sleep_mode();

  // mcu woken up and control returned here

  sleep_disable();
}

bool_t check_button(void)
{
  if(button_pressed)
  {
    button_pressed = FALSE;
    return TRUE;
  }

  return FALSE;
}

ISR(PCINT0_vect)
{
  if(bit_is_clear(PINB, BUTTON_PIN))
  {
    _delay_ms(20);

    if(bit_is_clear(PINB, BUTTON_PIN))
    {
      button_pressed = TRUE;
    }
    else
    {
      button_pressed = FALSE;
    }
  }
}


EMPTY_INTERRUPT(WDT_vect);

