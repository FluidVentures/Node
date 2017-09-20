/* NMEA RMC definition

  $GNGLL,4942.00507,N,12308.96892,W,021007.00,A,A*6C
  $GNRMC,021008.00,A,4942.00429,N,12308.96830,W,4.860,172.27,180917,,,A*6C
  $GNVTG,172.27,T,,M,4.860,N,9.001,K,A*20

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
#include <TinyGPS++.h>

#define DEBUG_MODE true //Enable/Disable serial debug output

#define RADIO_DATA_PAYLOAD_SIZE 30
#define HEADER_MSG_CHAR  0x01
#define DATA_MSG_CHAR    0x02

Enrf24 radio(PA0, PA1, PA2);
TinyGPSPlus gps;

const int SERIAL_STRING_SIZE = 128;
static int serialStringIndex = 0;

char serialString[SERIAL_STRING_SIZE];
int serialStringSize = 0;
int msgsSent = 0;

const uint8_t txaddr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01 };
static const uint32_t GPSBaud = 9600;
void dump_radio_status_to_serialport(uint8_t);
int printStatus = 0;



//********************************************** SETUP ***********************************************
//****************************************************************************************************
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

//**************************************** LOOP ******************************************************
//****************************************************************************************************
void loop() {


  if (readSerialString() == 1)
  {
    sendStringToRadio();
    //Serial.print(serialString);
    digitalWrite(33, !digitalRead(33)); // Turn the LED from off to on, or on to off
  }

  dump_radio_status_to_serialport(radio.radioState());
}


//**************************************** readSerialString ******************************************
//****************************************************************************************************
char readSerialString()
{
  char tempByte;


  while (Serial1.available() > 0) {
    tempByte = Serial1.read();

    if (tempByte == '$')
    {
      //Serial.println("Found $");
      
      serialStringIndex = 0;
      serialString[serialStringIndex++] = tempByte;
    }
    else if (tempByte == '\n')
    {
      if (serialStringIndex > 0) {
        
        //Serial.println("Found GPS line end");
        
        serialString[serialStringIndex++] = tempByte;
        serialStringSize = serialStringIndex;
        return 1;
      }
      else return -1;
    }
    else
    {
      serialString[serialStringIndex++] = tempByte;
      if (serialStringIndex >= SERIAL_STRING_SIZE) return -1;
    }
  }
}



//**************************************** sendStringToRadio *****************************************
//****************************************************************************************************
void sendStringToRadio(void)
{
  int msgPacketCount;
  int msgChecksum;
  int currentPacketIndex = 0;
  int currentStringOutputIndex = 0;
  int i;

  msgPacketCount = serialStringSize / RADIO_DATA_PAYLOAD_SIZE  + 1;
  msgChecksum = 0;  //TODO calculate this.



  msgsSent++;

  radio.print(HEADER_MSG_CHAR);   //send header packet

  radio.print(serialStringSize);

  radio.print(msgChecksum, 2);


  radio.flush();  // Force transmit (don't wait for any more data)

  if (DEBUG_MODE == true)
  {
    Serial.print("msgs sent: ");
    Serial.print(msgsSent);
    Serial.print(", ");

    Serial.print("serialStringSize: ");
    Serial.print(serialStringSize);
    Serial.print(", ");

    Serial.print("msgChecksum: ");
    Serial.print(msgChecksum);
    Serial.print(", ");

    Serial.print("HEADER_MSG_CHAR: ");
    Serial.println(HEADER_MSG_CHAR);
  }


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

//**************************************** Calc Checksum *********************************************
//****************************************************************************************************
/*
An asterisk (*) delimiter and checksum value follow the last field of data contained in an NMEA-0183 message.
The checksum is the 8-bit exclusive of all characters in the message, including the commas between fields, 
but not including the $ and asterisk delimiters. The hexadecimal result is converted to two ASCII characters 
(0–9, A–F). The most significant character appears first.
*/

//**************************************** dump_radio_status_to_serialport ***************************
//****************************************************************************************************
void dump_radio_status_to_serialport(uint8_t status)
{
  if (DEBUG_MODE == true)
  {
    if (printStatus == 25)
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
          printStatus = 0;
      }
      printStatus++;
    }
  }
}
