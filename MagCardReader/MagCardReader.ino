/*
   MagCardRead - Read data from a magnetic stripe card.

   Copyright (c) 2020 Richard Teel <richteel@teelsys.com>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.


   Modified from Carlos Rodrigues <cefrodrigues@gmail.com> original MagStripe library
   Based on this: http://cal.freeshell.org/2009/11/magnetic-stripe-reader/
     ...and this: http://www.abacus21.com/Magnetic-Strip-Encoding-1586.html
*/

#include "MagCardRead.h"


#define pinCardDetect1 4
#define pinStrobe1 2
#define pinData1 5

#define pinCardDetect2 6
#define pinStrobe2 3
#define pinData2 7

// Visual feedback when the card is being read...
static const byte READ_LED = 8;  //13
static const byte ERROR_LED = 9; //12

//MagCardRead card;
//MagCardRead card(pinCardDetect);
MagCardRead card(pinCardDetect1, pinStrobe1, pinData1);
MagCardRead card2(pinCardDetect2, pinStrobe2, pinData2);

/*
   Track 3 is the one that can contain the most characters (107).
   We add one more to accomodate the final '\0', as the data is a C string...
*/
static const byte DATA_BUFFER_LEN = 108;
static char data[DATA_BUFFER_LEN];


void readCard(MagCardRead crd) {
  // Don't do anything if there isn't a card present...
  if (!crd.available()) {
    return;
  }

  // Show that a card is being read...
  digitalWrite(READ_LED, HIGH);
  Serial.println("-- Read Completed --");

  // Read the card into the buffer "data" (as a null-terminated string)...
  short chars = crd.read(data, DATA_BUFFER_LEN);

  // Show that the card has finished reading...
  digitalWrite(READ_LED, LOW);

  // If there was an error reading the card, blink the error LED...
  if (chars < 0) {
    digitalWrite(ERROR_LED, HIGH);
    delay(250);
    digitalWrite(ERROR_LED, LOW);
    Serial.println("Read Error...");
    Serial.println(chars);

    return;
  }

  Serial.println("Good Read...");
  Serial.print("Read Direction: ");
  //Serial.println(crd.read_direction());
  Serial.println(crd.read_direction() == 1 ? "Forward" : crd.read_direction() == 2 ? "Backward" : "Unknown");


  // Send the data to the computer...
  Serial.println(data);
}

void setup()
{
  pinMode(READ_LED, OUTPUT);
  pinMode(ERROR_LED, OUTPUT);

  // The card data will be sent over serial...
  Serial.begin(9600);

  // Initialize the library for reading track 2...
  card.begin(1);
  card2.begin(2);

  // Start with the feedback LEDs off...
  digitalWrite(READ_LED, LOW);
  digitalWrite(ERROR_LED, LOW);
}


void loop()
{
  readCard(card);
  readCard(card2);
}


/* EOF - MagCardRead.ino */
