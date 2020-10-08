#include <Arduino.h>
#include <WEMOS_SHT3X.h>
#include <ArduinoJson.h>

const int SAMPLES= 10;
const int LDR_READ_PIN= 36;
const int MOIST_POWER_PIN= 26;
const int MOIST_READ_PIN= 39;

SHT3X sht30(0x45);
StaticJsonDocument<500> doc;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  
}

int senseMoisture(int powerPin, int readPin)
{
  int reading = 0;
  delay(100);
  for (int i = 0; i < SAMPLES; i++)
  {
    reading += analogRead(readPin);
    delay(50);
   }
   reading = reading/10;
  doc["sensors"]["moisture"] = reading;
  return reading;
}

float senseTemp()
{
  float reading = 0;
  if(sht30.get()==0)
  {
    reading = (float)sht30.cTemp;
  }
  else
  {
    Serial.println("Error!");
  }
  doc["sensors"]["temprature"] = reading;
  return reading;
}

float senseHumid()
{
  float reading = 0;
  if(sht30.get()==0)
  {
    reading = (float)sht30.humidity;
  }
  else
  {
    Serial.println("Error!");
  }
  doc["sensors"]["humidity"] = reading;
  return reading;
}


int senseLight(int readPin)
{
  int reading = 0;
  for (int i = 0; i < SAMPLES; i++)
  {
    reading += analogRead(readPin);
    delay(50);
  }
  reading = reading/10;
  doc["sensors"]["light"] = reading;
  return reading;
}

void loop()
{
  // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_BUILTIN, HIGH);
  senseLight(LDR_READ_PIN);
  senseMoisture(MOIST_POWER_PIN, MOIST_READ_PIN);
  senseTemp();
  senseHumid();
  digitalWrite(LED_BUILTIN, LOW);
  delay(5000);
  Serial.println();
  serializeJsonPretty(doc, Serial);
  Serial.println();
}



