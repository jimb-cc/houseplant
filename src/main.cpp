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

SHT3X sht30(0x45);
StaticJsonDocument<500> doc;




void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
 
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
      }
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