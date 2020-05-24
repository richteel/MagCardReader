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

#include <Arduino.h>

// Enough bits to read any of the three tracks...
#define BIT_BUFFER_LEN 800


// Variables used by the interrupt handlers...
static volatile bool next_bit = 0;                       // next bit to read
static volatile unsigned char bits[BIT_BUFFER_LEN / 8];  // buffer for bits being read
static volatile short num_bits = 0;                      // number of bits already read
static volatile uint8_t vdata_pin = 0;                   // data pin to be read by interrupt


// Manipulate the bit buffer...
static void bits_set(short index, bool bit);
static bool bits_get(short index);

// The interrupt handlers...
static void handle_strobe(void);

MagCardRead::MagCardRead(uint8_t cardDetectPin, uint8_t strobePin, uint8_t dataPin)
{
  _cardDetect_pin = cardDetectPin;
  _strobe_pin = strobePin;
  _data_pin = dataPin;

  direction = READ_UNKNOWN;
}

void MagCardRead::begin(uint8_t track)
{
  _track = track;

  pinMode(_cardDetect_pin, INPUT_PULLUP);
  pinMode(_strobe_pin, INPUT_PULLUP);
  pinMode(_data_pin, INPUT_PULLUP);

  // Reading is more reliable when using interrupts...
  attachInterrupt(digitalPinToInterrupt(_strobe_pin), handle_strobe, FALLING);  // strobe pin
}


void MagCardRead::stop()
{
  detachInterrupt(digitalPinToInterrupt(_strobe_pin));
}


bool MagCardRead::available()
{
  vdata_pin = _data_pin;
  return digitalRead(_cardDetect_pin) == LOW;
}


short MagCardRead::read(char *data, unsigned char size)
{
  // Fail if no card present...
  if (!available()) {
    return -1;
  }

  // Empty the bit buffer...
  num_bits = 0;
  next_bit = 0;

  // Wait while the data is being read by the interrupt routines...
  while (available()) {}

  // Decode the raw bits...
  short chars = decode_bits(data, size);
  direction = READ_FORWARD;

  // If the data looks bad, reverse and try again...
  if (chars < 0) {
    reverse_bits();
    chars = decode_bits(data, size);
    direction = READ_BACKWARD;
  }

  // The card could not be read successfully...
  if (chars < 0) {
    direction = READ_UNKNOWN;
  }

  return chars;
}


enum ReadDirection MagCardRead::read_direction()
{
  return direction;
}


void MagCardRead::reverse_bits()
{
  for (short i = 0; i < num_bits / 2; i++) {
    bool b = bits_get(i);

    bits_set(i, bits_get(num_bits - i - 1));
    bits_set(num_bits - i - 1, b);
  }
}


bool MagCardRead::verify_parity(unsigned char c)
{
  unsigned char parity = 0;

  for (unsigned char i = 0; i < 8; i++) {
    parity += (c >> i) & 1;
  }

  // The parity must be odd...
  return parity % 2 != 0;
}


bool MagCardRead::verify_lrc(short start, short length)
{
  unsigned char parity_bit = (_track == 1 ? 7 : 5);

  // Count the number of ones per column (ignoring parity bits)...
  for (short i = 0; i < (parity_bit - 1); i++) {
    short parity = 0;

    for (short j = i; j < length; j += parity_bit) {
      parity += bits_get(start + j);
    }

    // Even parity is what we want...
    if (parity % 2 != 0) {
      return false;
    }
  }

  return true;
}


short MagCardRead::find_sentinel(unsigned char pattern)
{
  unsigned char bit_accum = 0;
  unsigned char bit_length = (_track == 1 ? 7 : 5);

  for (short i = 0; i < num_bits; i++) {
    bit_accum >>= 1;                               // rotate the bits to the right...
    bit_accum |= bits_get(i) << (bit_length - 1);  // ...and add the current bit

    // Stop when the start sentinel pattern is found...
    if (bit_accum == pattern) {
      return i - (bit_length - 1);
    }
  }

  // No start sentinel was found...
  return -1;
}


short MagCardRead::decode_bits(char *data, unsigned char size) {
  short bit_count = 0;
  unsigned char chars = 0;
  unsigned char bit_accum = 0;
  unsigned char bit_length = (_track == 1 ? 7 : 5);

  short bit_start = find_sentinel(_track == 1 ? 0x45 : 0x0b);
  if (bit_start < 0) {  // error, start sentinel not found
    return -1;
  }

  for (short i = bit_start; i < num_bits; i++) {
    bit_accum >>= 1;                                 // rotate the bits to the right...
    bit_accum |= (bits_get(i) << (bit_length - 1));  // ...and add the current bit

    bit_count++;

    if (bit_count % bit_length == 0) {
      if (chars >= size) {  // error, the buffer is too small
        return -1;
      }

      // A null means we reached the end of the data...
      if (bit_accum == 0) {
        break;
      }

      // The parity must be odd...
      if (!verify_parity(bit_accum)) {
        return -1;
      }

      // Remove the parity bit...
      bit_accum &= ~(1 << (bit_length - 1));

      // Convert the character to ASCII...
      data[chars] = bit_accum + (_track == 1 ? 0x20 : 0x30);
      chars++;

      // Reset...
      bit_accum = 0;
    }
  }

  // Turn the data into a null-terminated string...
  data[chars] = '\0';

  if (data[chars - 2] != '?') {  // error, the end sentinel is not in the right place
    return -1;
  }

  // Verify the LRC (even parity across columns)...
  if (!verify_lrc(bit_start, chars * bit_length)) {
    return -1;
  }

  return chars;
}


static void bits_set(short index, bool bit)
{
  volatile unsigned char *b = &bits[index / 8];
  unsigned char m = 1 << (index % 8);

  *b = bit ? (*b | m) : (*b & ~m);
}


static bool bits_get(short index)
{
  return bits[index / 8] & (1 << (index % 8));
}


static void handle_strobe()
{
  // Avoid a crash in case there are too many bits (garbage)...
  if (num_bits >= BIT_BUFFER_LEN) {
    return;
  }

  next_bit = !digitalRead(vdata_pin);

  bits_set(num_bits, next_bit);
  num_bits++;
}


/* vim: set expandtab ts=4 sw=4: */
