#include <TinyGPS++.h>
#include <SPI.h>

static const uint32_t GPSBaud = 9600;
const int buttonPin = 32;     // the number of the pushbutton pin
const int mainLED =  33;
int buttonState = 0;         // variable for reading the pushbutton status
int legendCount = 0;         // variable for reading the pushbutton status

TinyGPSPlus gps;

void setup()
{
  Serial.begin(GPSBaud);
  Serial1.begin(GPSBaud);

  pinMode(mainLED, OUTPUT);     // initialize the LED pin as an output:
  pinMode(buttonPin, INPUT);    // initialize the pushbutton pin as an input:

}

void loop()
{

  // buttonState = digitalRead(buttonPin);   // read the state of the pushbutton value:

  printGPSdata();
  smartDelay(500);
  fixLED();
}


// Print GPS data
static void printGPSdata()
{
  if (legendCount == 10)
  {
    Serial.println(F("Sats HDOP Latitude   Longitude   Fix  Date       Time     Date Alt    Chars Sentences Checksum"));
    Serial.println(F("          (deg)      (deg)       Age                      Age  (m)    RX    RX        Fail"));
    legendCount = 0;
  }
  legendCount++;

  printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
  printInt(gps.hdop.value(), gps.hdop.isValid(), 5);
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  printInt(gps.location.age(), gps.location.isValid(), 5);
  printDateTime(gps.date, gps.time);
  printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);

  printInt(gps.charsProcessed(), true, 6);
  printInt(gps.sentencesWithFix(), true, 10);
  printInt(gps.failedChecksum(), true, 9);
  Serial.println();
}

// Onboard LED Fix status
static void fixLED()
{
  if (gps.location.isUpdated())
    digitalWrite(mainLED, HIGH);
  else
    digitalWrite(33, !digitalRead(33)); // Turn the LED from off to on, or on to off
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

static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i = flen; i < len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}

static void printDateTime(TinyGPSDate & d, TinyGPSTime & t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }

  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i = 0; i < len; ++i)
    Serial.print(i < slen ? str[i] : ' ');
  smartDelay(0);
}
