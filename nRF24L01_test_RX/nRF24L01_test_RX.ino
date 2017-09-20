/* NMEA RMC definition

  $GPRMC,001850.325,V,3754.921,N,08002.494,W,45.8,1.77,180917,,E*49

  .      1         2 3       4  5       6 7   8   9   10  11 12
       |         | |       |  |       | |   |   |    |   | |
  $--RMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,xxxx,x.x,a*hh
  1) Time (UTC)
  2) Status, V = Navigation receiver warning
  3) Latitude
  4) N or S
  5) Longitude
  6) E or W
  7) Speed over ground, knots
  8) Track made good, degrees true
  9) Date, ddmmyy
  10) Magnetic Variation, degrees
  11) E or W
  12) Checksum

*/

#include <SPI.h>
#include <Enrf24.h>
#include "PString.h"

#define DEBUG_MODE true //Enable/Disable serial debug output

#define RADIO_DATA_PAYLOAD_SIZE 30
#define HEADER_MSG_CHAR  0x01
#define DATA_MSG_CHAR    0x02

Enrf24 radio(PA0, PA1, PA2); // Set up nRF24L01 radio on SPI bus pins

const uint8_t rxaddr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01 };
void dump_radio_status_to_serialport(uint8_t);

//********************************************** SETUP ***********************************************
//****************************************************************************************************
void setup() {
  Serial.begin(9600);

  SPI.begin(); //Initialize nRF24L01 radio
  SPI.setDataMode(SPI_MODE0); //nRF24L01 radio data mode
  SPI.setBitOrder(MSBFIRST);

  radio.begin();  // Defaults 1Mbps, channel 0, max TX power
  radio.setRXaddress((void*)rxaddr);

  pinMode(33, OUTPUT); //TX/RX LED
  digitalWrite(33, LOW); //TX/RX LED

  radio.enableRX();  // Start listening
  delay(2500); // Delay 2.5sec to open serial monitor for viewing
  dump_radio_status_to_serialport(radio.radioState());
}

//**************************************** LOOP ******************************************************
//****************************************************************************************************
void loop()
{
  char tempByte[128];

  while (!radio.available(true));

  if (radio.read(tempByte))
  {
    Serial.print(tempByte);
    digitalWrite(33, !digitalRead(33)); // Turn the LED from off to on, or on to off
  }
}

//**************************** dump radio status*********************************************
void dump_radio_status_to_serialport(uint8_t status)
{
  Serial.print("Enrf24 radio transceiver status: ");
  switch (status) {
    case ENRF24_STATE_NOTPRESENT:
      Serial.println("NO TRANSCEIVER PRESENT");
      break;

    case ENRF24_STATE_DEEPSLEEP:
      Serial.println("DEEP SLEEP <1uA power consumption");
      break;

    case ENRF24_STATE_IDLE:
      Serial.println("IDLE module powered up w/ oscillators running");
      break;

    case ENRF24_STATE_PTX:
      Serial.println("Actively Transmitting");
      break;

    case ENRF24_STATE_PRX:
      Serial.println("Receive Mode");
      break;

    default:
      Serial.println("UNKNOWN STATUS CODE");
  }
}
