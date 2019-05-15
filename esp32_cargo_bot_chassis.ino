#include <SoftwareSerial.h>
#include <ODriveArduino.h>

template<class T> inline Print& operator <<(Print &obj,     T arg) {
  obj.print(arg);
  return obj;
}
template<>        inline Print& operator <<(Print &obj, float arg) {
  obj.print(arg, 4);
  return obj;
}

/*

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

// Serial to the ODrive
SoftwareSerial odrive0_serial(6, 5); //RX (ODrive TX), TX (ODrive RX)
SoftwareSerial odrive1_serial(8, 7); //RX (ODrive TX), TX (ODrive RX)

// ODrive object
ODriveArduino odrive0(odrive0_serial);
ODriveArduino odrive1(odrive1_serial);

//enachb
const uint64_t pipe = 0xABBDABCD71LL;              // Radio pipe addresses for the 2 nodes to communicate.

struct metricsStruct {
  int16_t leftMotor;
  int16_t rightMotor;
};

metricsStruct metrics = {0, 0};

long lastUpdate = millis();
int count = 0;

//Servo servoL;
//Servo servoR;

void setup(void)
{

  delay(2000);

  Serial.begin(115200);
  odrive0_serial.begin(115200);
  odrive1_serial.begin(115200);

  printf_begin();
  printf("\r\nCargo Bot chassis controller/\r\n");

  radio.begin();

  //enachb
  radio.setChannel(45);

  // jerome
  //radio.setChannel(70);

  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, pipe);
  radio.startListening();

  radio.printDetails();

  printf("calibrating motors\n");
  //odrive.run_state(0, ODriveArduino::AXIS_STATE_ENCODER_OFFSET_CALIBRATION, true);
  printf("done calibrating motor 0\n");
  //odrive.run_state(1, ODriveArduino::AXIS_STATE_ENCODER_OFFSET_CALIBRATION, true);
  printf("done calibrating motor 1\n");

  //odrive.run_state(0, ODriveArduino::AXIS_STATE_CLOSED_LOOP_CONTROL, true);
  //odrive.run_state(1, ODriveArduino::AXIS_STATE_CLOSED_LOOP_CONTROL, true);
  
  odrive0.SetVelocity(0, 0);
  odrive0.SetVelocity(1, 0);

  odrive1.SetVelocity(0, 0);
  odrive1.SetVelocity(1, 0);

  printf("Ready to drive.");

}

void loop(void)
{

  // if there is data ready
  if ( radio.available() )
  {
    // Get the packet from the radio
    radio.read( &metrics, sizeof(metrics) );
    lastUpdate = millis();

    int servoValL = map(metrics.leftMotor, -1000, 1000, -500, 500);
    int servoValR = map(0 - metrics.rightMotor, -1000, 1000, -500, 500);

    // Write output to motor
    odrive0.SetVelocity(0, servoValL);
    odrive0.SetVelocity(1, servoValL);

    odrive1.SetVelocity(0, servoValR);
    odrive1.SetVelocity(1, servoValR);

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

    // Kill motors
    odrive0.SetVelocity(0, 0);
    odrive0.SetVelocity(1, 0);

    odrive1.SetVelocity(0, 0);
    odrive1.SetVelocity(1, 0);

  }
}

