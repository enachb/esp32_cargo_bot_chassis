/*
  Copyright (C) 2012 J. Coliz <maniacbug@ymail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  version 2 as published by the Free Software Foundation.
*/

/**
   Example Nordic FOB Receiver

   This is an example of how to use the RF24 class to receive signals from the
   Sparkfun Nordic FOB.  Thanks to Kirk Mower for providing test hardware.

   See blog post at http://maniacbug.wordpress.com/2012/01/08/nordic-fob/
*/

#include <SPI.h>
#include <RF24.h>
#include "nRF24L01.h"
#include "printf.h"

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(7, 8);
const uint64_t pipe = 0xABBDABCD71LL;              // Radio pipe addresses for the 2 nodes to communicate.

struct metricsStruct {
  int16_t leftMotor;
  int16_t rightMotor;
};

metricsStruct metrics = {0, 0};


void setup(void)
{
  Serial.begin(115200);
  printf_begin();
  printf("\r\nCargo Bot chassis controller/\r\n");

  radio.begin();
  radio.setChannel(45);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, pipe);
  radio.startListening();

  radio.printDetails();
}

void loop(void)
{

  // if there is data ready
  if ( radio.available() )
  {
    // Get the packet from the radio
    radio.read( &metrics, sizeof(metrics) );

    // Print the ID of this message.  Note that the message
    // is sent 'big-endian', so we have to flip it.
    printf("motors %d %d \n", metrics.leftMotor, metrics.rightMotor);
  }

}

