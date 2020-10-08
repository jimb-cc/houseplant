#include <Arduino.h>
#include <WEMOS_SHT3X.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

const int SAMPLES= 10;
const int LDR_READ_PIN= 36;
const int MOIST_POWER_PIN= 26;
const int MOIST_READ_PIN= 39;

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR char name[15] = CLIENT;

SHT3X sht30(0x45);
StaticJsonDocument<500> doc;




void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.print(" with password ");
  Serial.println(password);

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_BUILTIN, LOW);
}


void POSTData()
{
      
      if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");

      String json;
      serializeJson(doc, json);

      Serial.println(json);
      int httpResponseCode = http.POST(json);
      Serial.println(httpResponseCode);
      }
}


void getDevice()
{

    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();

    uint64_t chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
    Serial.printf("***ESP32 Chip ID = %04X%08X\n",(uint16_t)(chipid>>32),(uint32_t)chipid);//print High 2 bytes
    char buffer[200];
    sprintf(buffer, "%04X%08X",(uint16_t)(chipid>>32),(uint32_t)chipid);
    //sprintf(buffer, "esp32%" PRIu64, ESP.getEfuseMac());

    // int vbatt_raw = 0;
    // for (int i=0;i<SAMPLES;i++)
    // {
    //    vbatt_raw += analogRead(PIN_POWER);
    //    delay(100);
    // }
    // vbatt_raw = vbatt_raw/SAMPLES;
    //float vbatt = map(vbatt_raw, 0, 4096, 0, 4200);

    doc["device"]["IP"] = WiFi.localIP().toString();
    doc["device"]["RSSI"] = String(WiFi.RSSI());
    doc["device"]["type"] = TYPE;
    doc["device"]["name"] = name;
    doc["device"]["chipid"] = buffer;
    doc["device"]["bootCount"] = bootCount;
    doc["device"]["wakeup_reason"] = String(wakeup_reason);
    //doc["device"]["vbatt_raw"] = vbatt_raw;
    //doc["device"]["vbatt"] = map(vbatt_raw, 0, 4096, 0, 4200);

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
  ++bootCount; // move this to setup()
  // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_BUILTIN, HIGH);
  getDevice();
  senseLight(LDR_READ_PIN);
  senseMoisture(MOIST_POWER_PIN, MOIST_READ_PIN);
  senseTemp();
  senseHumid();
  digitalWrite(LED_BUILTIN, LOW);
  delay(10000);
  Serial.println("Posting...");
  POSTData();
  serializeJsonPretty(doc, Serial);
  Serial.println("\nDone.");
}



/*

MongoDB Atlas Realm Function
----------------------------



exports = function(payload){
    var atlas = context.services.get("mongodb-atlas");
    var coll = atlas.db("iot").collection("readings");
    try {
      if (payload.body) 
      {
        body = EJSON.parse(payload.body.text());
      }
      coll.insertOne(body);
      console.log(body);

    } catch (e) {
      console.log("Error inserting sensor reading doc: " + e);
      return e.message();
    }
};



----------------
*/