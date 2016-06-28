/* This code is intended for an ATTiny85 */
/*****************************************
 * 
 * Generate a series of flashes in Morse Code
 * which represent a string.
 * 
 * It is assumed that an LED is attached to
 * ATTiny pin 0
 * 
 * June 2016
 * D Q McDonald
 *****************************************/

#include <ctype.h>

#define LED_PIN 0

// The following definitions are used to encode the dot dash pattern
// as bits:
#define DOT 0
#define DASH 1

// How long the shortest flash is:
int dot_duration = 100;

// This is the text which will be flashed
// repeatedly. Characters only, no numbers
// are supported
const char* text = 
"Quentin McDonald";
// Max is: ############################## 

#define MAX_CHARS 30

int num_characters = 0;

// A series of bytes which defines the morse
// code sequence. The first nibble (bytes 0-3)
// represents the length of the code (1-4 for
// characters), 
byte morse[MAX_CHARS];

// Convert the string in "text" into the
// series of codes representing the flash 
// sequence in "morse".
void convert_text_to_morse(void) {

  char c;
  int i;

  num_characters = strlen(text);

  for( i=0; i<num_characters; i++ ) {
    c = toupper(text[i]);
    // Initialize each byte
    morse[i] = 0;

    switch( c ) {

    case 'A':
      // Dot - Dash
      morse[i] = 2;
      morse[i] += (DOT<<4) | (DASH<<5);
      break;

    case 'B':
      // Dash Dot Dot Dot
      morse[i]=4;
      morse[i] += (DASH<<4) | (DOT<<5) | (DOT<<6) | (DOT<<7);
      break;

    case 'C':
      //Dash Dot Dash Dot
      morse[i]= 4;
      morse[i] += (DASH<<4) | (DOT << 5) | (DASH<<6) | (DOT<<7 );
      break;

    case 'D':
      // Dash Dot Dot
      morse[i] = 3;
      morse[i] += (DASH<<4) | (DOT<<5) | (DOT<<6);
      break;

    case 'E':
      // Dot
      morse[i]=1;
      morse[i] += (DOT<<4);
      break;

    default: // Just ignore any non-characters
      break;

    }

  }


};


void setup() {

  convert_text_to_morse();
  pinMode( LED_PIN, OUTPUT );

  digitalWrite(LED_PIN,LOW);
  delay(1000);
}

void loop () {

  int i,j;
  int num_flashes;
  int delay_time;
  int bit_to_read;


  digitalWrite(LED_PIN,LOW);
  for( i=0; i< num_characters; i++ ) {
    num_flashes = morse[i]  & 0b00001111;
    bit_to_read = 4;
    for ( j=0; j < num_flashes; j++ ) {
      
      if( bitRead( morse[i], bit_to_read) == DOT ) {
        delay_time = dot_duration;
      } 
      else {
        // Three times a dot for a dash
        delay_time = 3*dot_duration;
      }
      digitalWrite(LED_PIN,HIGH);
      delay( delay_time );
      digitalWrite( LED_PIN, LOW );
      delay( dot_duration ); // One unit delay between flashes
      bit_to_read += 1;
    }

    // A delay of 3 units between letters
    delay( dot_duration * 3 );

  }

  // A delay of 7 units between words
  delay( dot_duration * 7 );
}










