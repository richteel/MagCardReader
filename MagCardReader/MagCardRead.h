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

#ifndef MAGCARDREAD_H
#define MAGCARDREAD_H

#include <Arduino.h>

// Default pins used by init
// The data and strobe pins depend on the board model...
#if defined(__AVR_ATmega32U4__)
// Arduino Leonardo and Arduino Micro:
#  define MAGSTRIPE_RDT 3  /* data pin (yellow) */
#  define MAGSTRIPE_RCL 2  /* strobe pin (orange) */
#else
// Arduino Uno and Arduino Mega:
#  define MAGSTRIPE_RDT 2  /* data pin (yellow) */
#  define MAGSTRIPE_RCL 3  /* strobe pin (orange) */
#endif

#  define MAGSTRIPE_CLS 4  /* card present pin (green) */

// Cards can be read in one of these possible ways...
enum ReadDirection { READ_UNKNOWN, READ_FORWARD, READ_BACKWARD };

class MagCardRead {
  public:
    MagCardRead(uint8_t cardDetectPin = MAGSTRIPE_CLS, uint8_t strobePin = MAGSTRIPE_RCL, uint8_t dataPin = MAGSTRIPE_RDT);

    // Initialize the library (attach interrupts)...
    void begin(uint8_t track);

    // Deinitialize the library (detach interrupts)...
    void stop();

    // Check if there is a card present for reading...
    bool available();

    // Read the data from the card as ASCII...
    short read(char *data, uint8_t size);

    // The direction of the last card read...
    enum ReadDirection read_direction();

  private:
    uint8_t _cardDetect_pin;
    uint8_t _strobe_pin;
    uint8_t _data_pin;

    uint8_t _track;
    enum ReadDirection direction;

    void reverse_bits();
    bool verify_parity(uint8_t c);
    bool verify_lrc(short start, short length);
    short find_sentinel(uint8_t pattern);
    short decode_bits(char *data, uint8_t size);
};


#endif  /* MAGCARDREAD_H */


/* vim: set expandtab ts=4 sw=4: */
