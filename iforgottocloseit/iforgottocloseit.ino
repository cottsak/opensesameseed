#include <SimpleTimer.h>      // https://github.com/infomaniac50/SimpleTimer
#include <ESP8266WiFi.h>      // https://github.com/esp8266/Arduino
#include <ESP8266HTTPClient.h>

#include "keys.h"             // this file contains your usernames and passwords, etc

double openForTooLongInMins = 5;
int doorOpenedAtTimeInMills = 0;
int doorOpenDurationInSeconds = 0;
bool messageSentInThisOpening = false;
const int doorOpen = HIGH;
const int doorClosed = LOW;

const int inputPinForDoor = 2;

SimpleTimer timer;

void resetDoorOpenCounter() {
  doorOpenDurationInSeconds = 0;
  messageSentInThisOpening = false; 
}

void sendSms(String message) {
  Serial.println("making POST request to ifttt for sending sms..");

  HTTPClient http;

  http.begin(iftttMakerUrl, "A9 81 E1 35 B3 7F 81 B9 87 9D 11 DD 48 55 43 2C 8F C3 EC 87");

  http.addHeader("content-type", "application/json");
  int result = http.POST("{\"value1\":\"" + message + "\"}");

  Serial.println("status code: " + String(result));

  if(result > 0) {
    Serial.println("body:");
    Serial.println(http.getString());
  } else{
    Serial.print("FAILED. error:"); Serial.println(http.errorToString(result).c_str());
    Serial.println("body:");
    Serial.println(http.getString());
  }

  http.end();  
}

void checkOpen() {
  if( digitalRead(inputPinForDoor) == doorOpen ) {
    Serial.println("ncInputPinForDoor is HIGH");
    Serial.println("door is open");
    doorOpenDurationInSeconds += 5;
    Serial.println("doorOpenDurationInSeconds:");
    Serial.println(doorOpenDurationInSeconds);
  }
  if( digitalRead(inputPinForDoor) == doorClosed ) {
    Serial.println("ncInputPinForDoor is LOW");
    Serial.println("door is closed.");
    resetDoorOpenCounter();
    Serial.println("doorOpenDurationInSeconds:");
    Serial.println(doorOpenDurationInSeconds);   
  }  

  if( messageSentInThisOpening == false && 
    doorOpenDurationInSeconds > openForTooLongInMins * 60 ) {
    String messageToSend = (String)"WARNING: your garage door has been open for more than " + openForTooLongInMins + " mins!";
    sendSms(messageToSend);
    // todo: this does not know if it sent successfully. needs work
    Serial.println("Sent SMS: " + messageToSend);
    messageSentInThisOpening = true;
  }
}

void setup() {
  Serial.begin(115200);   // for debugging

  Serial.print("Connecting to wifi");
  WiFi.begin(wifiCreds[0], wifiCreds[1]);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\r\nWiFi connected.");
  Serial.println("access point:");
  Serial.println(WiFi.SSID());
  Serial.println("ip address:");
  Serial.println(WiFi.localIP());

  Serial.println("\r\nReady for interwebs action!\r\n");

  // using GPIO2 for input. door is normally closed.
  // can not be active LOW on reset or weird stuff happens
  // needs to be open/HIGH
  // and then closed/LOW -after- startup
  pinMode(inputPinForDoor, INPUT_PULLUP);
  
  // check to see if the door has been left open for too long every 10s
  timer.setInterval(5000, checkOpen);
}

void loop() {
  timer.run();
}



