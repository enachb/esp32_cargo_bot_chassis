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
#include <Servo.h>

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(A0, 10);
const uint64_t pipe = 0xABBDABCD71LL;              // Radio pipe addresses for the 2 nodes to communicate.

struct metricsStruct {
  int16_t leftMotor;
  int16_t rightMotor;
};

metricsStruct metrics = {0, 0};

long lastUpdate = millis();
int count = 0;

Servo servoL;
Servo servoR;

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

  servoL.attach(5);
  servoR.attach(4);

}

void loop(void)
{

  // if there is data ready
  if ( radio.available() )
  {
    // Get the packet from the radio
    radio.read( &metrics, sizeof(metrics) );
    lastUpdate = millis();

    int servoValL = map(metrics.leftMotor, -1000, 1000, 0, 180);
    int servoValR = map(0 - metrics.rightMotor, -1000, 1000, 0, 180);

    servoL.write(servoValL);
    servoR.write(servoValR);

    // Print the ID of this message.  Note that the message
    // is sent 'big-endian', so we have to flip it.
    if (count % 10 == 0) {
      printf("motors %d %d servo %d %d\n", metrics.leftMotor, metrics.rightMotor, servoValL, servoValR);
      count = 1;
    } else {
      count++;
    }


  }

  //Deadman switch - turn motors off if we haven't heard from them from 500ms
  if (lastUpdate + 500 < millis()) {
    printf("**************** ACTIVATING DEAD MAN SWITCH ************************** \n");
    servoL.write(90);
    servoR.write(90);
  }
}

