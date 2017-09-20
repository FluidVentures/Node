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

const int SERIAL_STRING_SIZE = 128;

char serialString[SERIAL_STRING_SIZE];
int serialStringSize = 0;

const uint8_t txaddr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01 };
static const uint32_t GPSBaud = 9600;
void dump_radio_status_to_serialport(uint8_t);
int printStatus = 0;

void setup() {
  Serial.begin(GPSBaud);
  Serial1.begin(GPSBaud);

  pinMode(33, OUTPUT);

  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  radio.begin();  // Defaults 1Mbps, channel 0, max TX power
  radio.setTXaddress((void*)txaddr);

  delay(2500);
  dump_radio_status_to_serialport(radio.radioState());
}

void loop() {

  if (readSerialString() == 1)
  {
    sendStringToRadio();
    //Serial.print(serialString);
    digitalWrite(33, !digitalRead(33)); // Turn the LED from off to on, or on to off
  }
  
  delay(100);
  digitalWrite(33, !digitalRead(33)); // Turn the LED from off to on, or on to off


  /*
    if (printStatus == 100)
    {
      dump_radio_status_to_serialport(radio.radioState());  // Should report IDLE
      printStatus = 0;
    }
    printStatus++;
  */

}



//**************************************** readSerialString ******************************************

#define RADIO_DATA_PAYLOAD_SIZE 30
#define HEADER_MSG_CHAR  0x01
#define DATA_MSG_CHAR    0x02

char readSerialString()
{
  char tempByte;
  int index = 0;

  while (Serial1.available() > 0) {
    tempByte = Serial1.read();

    if (tempByte == '$')
    {
      //Serial.println("Found $");
      index = 0;
      serialString[index++] = tempByte;
    }
    else if (tempByte == '\n')
    {
      if (index > 0) {
        //Serial.println("Found GPS line end");
        serialString[index++] = tempByte;
        serialStringSize = index;
        return 1;
      }
      else return -1;
    }
    else
    {
      serialString[index++] = tempByte;
      if (index >= SERIAL_STRING_SIZE) return -1;
    }
  }
}



//**************************************** sendStringToRadio ******************************************
void sendStringToRadio(void)
{
  int msgPacketCount;
  int msgChecksum;
  int currentPacketIndex = 0;
  int currentStringOutputIndex = 0;
  int i;

  msgPacketCount = serialStringSize / RADIO_DATA_PAYLOAD_SIZE  + 1;
  msgChecksum = 0;  //TODO calculate this.

  //send header packet
  radio.print(HEADER_MSG_CHAR);
  //Serial.print(HEADER_MSG_CHAR);

  radio.print(serialStringSize);
  //Serial.print(serialStringSize);

  radio.print(msgChecksum, 2);
  //Serial.print(msgChecksum);

  Serial.println();
  radio.flush();  // Force transmit (don't wait for any more data)


  for (currentPacketIndex = 0; currentPacketIndex < msgPacketCount; currentPacketIndex++)
  {
    radio.print(DATA_MSG_CHAR);
    radio.print(currentPacketIndex);

    for (i = 0; i < RADIO_DATA_PAYLOAD_SIZE; i++)
    {
      radio.print(serialString[currentStringOutputIndex++]);
      if (currentStringOutputIndex >= serialStringSize) break;
    }
    radio.flush();
  }
}




//**************************************** dump_radio_status_to_serialport ******************************************
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
