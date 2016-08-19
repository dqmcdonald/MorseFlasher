/* This code is intended for an ATTiny85 */
/*****************************************

   Generate a series of flashes in Morse Code
   which represent a string.

   It is assumed that an LED is attached to
   ATTiny pin 0 (chip pin 5)

   A potentiometer set up in voltage divider is leading to
   pin A3 (chip pin 2)

   Between each set of flashes the ATTiny85 will sleep
   for 15 seconds in order to maximise battery list

   June 2016
   D Q McDonald
 *****************************************/

// This is the text which will be flashed
// repeatedly. Characters only, no numbers
// are supported. Max is 30 characters, any more will be ignored.
const char* text = "SOS";


#include <ctype.h>
#include <avr/io.h>        // Adds useful constants
#include <util/delay.h>    // Adds delay_ms and delay_us functions
#include <avr/interrupt.h>
#include <avr/sleep.h>

// Variables for the Sleep/power down modes:
volatile int watchdog_counter = 0;
#define WATCHDOG_MAX 15

// Routines to set and clear bits (used in the sleep code)
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// Pin definitions
#define LED_PIN 0
#define ANALOG_PIN A3

// The following definitions are used to encode the dot dash pattern
// as bits:
#define DOT 0
#define DASH 1
#define SPACE_CODE 64
#define LOW_BIT_MASK 0b00001111

// For setting up the watchdog timer:
//0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
enum class SleepDuration  { SLEEP16MS = 0, SLEEP32MS = 1, SLEEP128MS = 3, SLEEP250MS = 4,
                            SLEEP500MS = 5, SLEEP1S = 6, SLEEP2S = 7, SLEEP4S = 8, SLEEP8S = 9
                          };


// How long the shortest flash is. Will will be set by reading the voltage from pin2
int dot_duration = 200   ;

// The maximum number of characters we can support:
#define MAX_CHARS 30

int num_characters = 0;

// A series of bytes which defines the morse
// code sequence. The first nibble (bytes 0-3)
// represents the length of the code (1-4 for
// characters),
byte morse[MAX_CHARS];

void setup() {

  convert_text_to_morse();
  
  pinMode( LED_PIN, OUTPUT );
  pinMode( ANALOG_PIN, INPUT );

  // Flash pins on startup
  digitalWrite(LED_PIN, LOW);
  delay(100);
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW );
  delay(100);
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
  delay(100);
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
  delay(50);
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(3000);

  setup_watchdog(SleepDuration::SLEEP1S); // Set the watchdog for 1s

}

void loop () {

  if ( watchdog_counter > WATCHDOG_MAX )
  { // Only proceed if we've slept enough times:
    
    watchdog_counter = 0;
    flash_morse();
    
  }

  // Shut down the ATTiny as much as possible:
  system_sleep();

}


// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(SleepDuration sleep_duration) {

  int ii = static_cast<int>(sleep_duration);
  byte bb;
  int ww;
  if (ii > 9 ) ii = 9;
  bb = ii & 7;
  if (ii > 7) bb |= (1 << 5);
  bb |= (1 << WDCE);
  ww = bb;

  MCUSR &= ~(1 << WDRF);
  // start timed sequence
  WDTCR |= (1 << WDCE) | (1 << WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
}

// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  watchdog_counter++; // increment the global counter
}


// set system into the sleep state
// system wakes up when wtchdog is timed out
void system_sleep() {

  cbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter OFF
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();
  sleep_mode();                        // System actually sleeps here
  sleep_disable();                     // System continues execution here when watchdog timed out
  sbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter ON

}


// Convert the string in "text" into the
// series of codes representing the flash
// sequence in "morse".
void convert_text_to_morse(void) {

  char c;
  int i;

  num_characters = strlen(text);

  for ( i = 0; i < num_characters; i++ ) {
    c = toupper(text[i]);
    // Initialize each byte
    morse[i] = 0;

    switch ( c ) {

      case 'A':
        // Dot - Dash
        morse[i] = 2;
        morse[i] += (DOT << 4) | (DASH << 5);
        break;

      case 'B':
        // Dash Dot Dot Dot
        morse[i] = 4;
        morse[i] += (DASH << 4) | (DOT << 5) | (DOT << 6) | (DOT << 7);
        break;

      case 'C':
        //Dash Dot Dash Dot
        morse[i] = 4;
        morse[i] += (DASH << 4) | (DOT << 5) | (DASH << 6) | (DOT << 7 );
        break;

      case 'D':
        // Dash Dot Dot
        morse[i] = 3;
        morse[i] += (DASH << 4) | (DOT << 5) | (DOT << 6);
        break;

      case 'E':
        // Dot
        morse[i] = 1;
        morse[i] += (DOT << 4);
        break;

      case 'F':
        // Dot Dot Dash Dot
        morse[i] = 4;
        morse[i] += (DOT << 4) | (DOT << 5) | (DASH << 6) | (DOT << 7 );
        break;

      case 'G':
        // Dash Dash Dot
        morse[i] = 3;
        morse[i] += (DASH << 4 ) | (DASH << 5) | (DOT << 6 );
        break;

      case 'I':
        // Dot Dot
        morse[i] = 2;
        morse[i] += (DOT << 4 ) | (DOT << 5 );
        break;

      case 'J':
        // Dot Dash Dash Dash
        morse[i] = 4;
        morse[i] += (DOT << 4 ) | (DASH << 5 ) | (DASH << 6 ) | (DASH << 7 );
        break;

      case 'K':
        // Dash Dot Dash
        morse[i] = 3;
        morse[i] += (DASH << 4 ) | (DOT << 5 ) | (DASH << 6 );
        break;

      case 'L':
        // Dot Dash Dot Dot
        morse[i] = 4;
        morse[i] += (DOT << 4 ) | (DASH << 5 ) | (DOT << 6) | (DOT << 7 );
        break;

      case 'M':
        // Dash Dash
        morse[i] = 2;
        morse[i] += (DASH << 4 ) | (DASH << 5 );
        break;

      case 'N':
        // Dash Dot
        morse[i] = 2;
        morse[i] += (DASH << 4 ) | (DOT << 5 );
        break;

      case 'O':
        // Dash Dash Dash
        morse[i] = 3;
        morse[i] += (DASH << 4 ) | (DASH << 5 ) | (DASH << 6 );
        break;

      case 'P':
        // Dot Dash Dash Dot
        morse[i] = 4;
        morse[i] += (DOT << 4 ) | (DASH << 5 ) | (DASH << 6 ) | (DOT << 7 );
        break;

      case 'Q':
        // Dash Dash Dot Dash
        morse[i] = 4;
        morse[i] += (DASH << 4 ) | (DASH << 5 ) | (DOT << 6 ) | (DASH << 7 );
        break;

      case 'R':
        // Dot Dash Dot
        morse[i] = 3;
        morse[i] += (DOT << 4 ) | (DASH << 5 ) | (DOT << 6 );
        break;

      case 'S':
        // Dot Dot Dot
        morse[i] = 3;
        morse[i] += (DOT << 4 ) | (DOT << 5 ) | (DOT << 7 );
        break;

      case 'T':
        // Dash
        morse[i] = 1;
        morse[i] += (DASH << 4 );
        break;

      case 'U':
        // Dot Dot Dash
        morse[i] = 3;
        morse[i] += (DOT << 4 ) | (DOT << 5 ) | (DASH << 6 );
        break;

      case 'V':
        // Dot Dot Dot Dash
        morse[i] = 4;
        morse[i] += (DOT << 4 ) | (DOT << 5 ) | (DOT << 6 ) | (DASH << 7);
        break;

      case 'W':
        // Dot Dash Dash
        morse[i] = 3;
        morse[i] += (DOT << 4 ) | (DASH << 5 ) | (DASH << 6 ) ;
        break;

      case 'X':
        // Dash Dot Dot Dash
        morse[i] = 4;
        morse[i] += (DASH << 4 ) | (DOT << 5 ) | (DOT << 6 ) | (DASH << 7 );
        break;

      case 'Y':
        // Dash Dot Dash Dash
        morse[i] = 4;
        morse[i] += (DASH << 4 ) | (DOT << 5 ) | (DASH << 6 ) | (DASH << 7 );
        break;

      case 'Z':
        // Dash Dash Dot Dot
        morse[i] = 4;
        morse[i] += (DASH << 4 ) | (DASH << 5 ) | (DOT << 6 ) | (DOT << 7 );
        break;

      case ' ':
        //
        morse[i] = SPACE_CODE;

        break;

      default: // Just ignore any non-characters
        break;

    }

  }
}


// The code which actually flashes the morse based on the input characters:

void flash_morse() {

  int i, j;
  int num_flashes;
  int delay_time;
  int bit_to_read;

  for ( i = 0; i < num_characters; i++ ) {
    num_flashes = morse[i]  & LOW_BIT_MASK;
    if ( num_flashes == SPACE_CODE ) {
      // Space
      delay_time = dot_duration * 3;
      delay( delay_time );
    }
    else {
      bit_to_read = 4;
      for ( j = 0; j < num_flashes; j++ ) {

        dot_duration = analogRead(ANALOG_PIN );
        if ( dot_duration < 50)
          dot_duration = 50;

        if ( bitRead( morse[i], bit_to_read) == DOT ) {
          delay_time = dot_duration;
        }
        else {
          // Three times a dot for a dash
          delay_time = 3 * dot_duration;
        }
        digitalWrite(LED_PIN, HIGH);
        delay( delay_time );
        digitalWrite( LED_PIN, LOW );
        if ( j != num_flashes - 1 ) {
          delay( dot_duration ); // One unit delay between flashes
        }
        bit_to_read += 1;
      }
    }
    // A delay of 3 units between letters
    if ( i != num_characters - 1 ) {
      delay( dot_duration * 3 );
    }
  }
}











