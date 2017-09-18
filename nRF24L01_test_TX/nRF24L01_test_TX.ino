//$GNGLL,4942.00507,N,12308.96892,W,021007.00,A,A*6C
//$GNRMC,021008.00,A,4942.00429,N,12308.96830,W,4.860,172.27,180917,,,A*6C
//$GNVTG,172.27,T,,M,4.860,N,9.001,K,A*20

#include <SPI.h>
#include <Enrf24.h>
#include <TinyGPS++.h>
#include <nRF24L01.h>
#include <string.h>

Enrf24 radio(PA0, PA1, PA2);
TinyGPSPlus gps;

const uint8_t txaddr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01 };
static const uint32_t GPSBaud = 9600;
void dump_radio_status_to_serialport(uint8_t);

void setup() {
  Serial.begin(GPSBaud);
  Serial1.begin(GPSBaud);

  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  radio.begin();  // Defaults 1Mbps, channel 0, max TX power
  radio.setTXaddress((void*)txaddr);

  delay(2500);
  dump_radio_status_to_serialport(radio.radioState());
}

void loop() {


  radio.print(gps.location.lat(), 6);
  radio.print(",");
  radio.print(gps.location.lng(), 6);
  
  radio.flush();  // Force transmit (don't wait for any more data)


  Serial.print(gps.location.rawLat().deg), 4;
  Serial.print(",");
  Serial.print(gps.location.lng(), 6);
  Serial.println();


  digitalWrite(33, HIGH);
  smartDelay(10);
  digitalWrite(33, LOW);

  //dump_radio_status_to_serialport(radio.radioState());  // Should report IDLE
  smartDelay(1000);

}


// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (Serial1.available())
      gps.encode(Serial1.read());
  } while (millis() - start < ms);
}

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
