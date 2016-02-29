#include <SimpleTimer.h>      // https://github.com/infomaniac50/SimpleTimer
#include <ESP8266WiFi.h>      // https://github.com/esp8266/Arduino
#include <ESP8266HTTPClient.h>

#include "keys.h"             // this file contains your usernames and passwords, etc

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...) 
#endif

double openForTooLongInMins = 0.23;
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
  DEBUG_MSG("making POST request to ifttt for sending sms..\n");

  HTTPClient http;

  http.begin(iftttMakerUrl, "A9 81 E1 35 B3 7F 81 B9 87 9D 11 DD 48 55 43 2C 8F C3 EC 87");

  http.addHeader("content-type", "application/json");
  int result = http.POST("{\"value1\":\"" + message + "\"}");

  DEBUG_MSG(String("status code: " + result).c_str());

  if(result > 0) {
    DEBUG_MSG("body:\r\n");
    DEBUG_MSG((http.getString() + "\r\n").c_str());
  } else{
    DEBUG_MSG("FAILED. error:"); DEBUG_MSG((http.errorToString(result) + "\n").c_str());
    DEBUG_MSG("body:\r\n");
    DEBUG_MSG((http.getString() + "\r\n").c_str());
  }

  http.end();  
}

void checkOpen() {
  if( digitalRead(inputPinForDoor) == doorOpen ) {
    DEBUG_MSG("door is open.\r\n");
    doorOpenDurationInSeconds += 5;
    DEBUG_MSG(("doorOpenDurationInSeconds:" + (String)doorOpenDurationInSeconds + "\r\n").c_str());
  }
  if( digitalRead(inputPinForDoor) == doorClosed ) {
    DEBUG_MSG("door is closed.\r\n");
    resetDoorOpenCounter();
    DEBUG_MSG(String("doorOpenDurationInSeconds:" + (String)doorOpenDurationInSeconds + "\r\n").c_str());
  }  

  if( messageSentInThisOpening == false && 
    doorOpenDurationInSeconds > openForTooLongInMins * 60 ) {
    String messageToSend = (String)"WARNING: your garage door has been open for more than " + openForTooLongInMins + " mins!";
    sendSms(messageToSend);
    // todo: this does not know if it sent successfully. needs work
    DEBUG_MSG(String("Sent SMS: " + messageToSend + "\r\n").c_str());
    messageSentInThisOpening = true;
  }
}

void setup() {
#ifdef DEBUG_ESP_PORT  
  Serial.begin(115200);   // debug
#else   
  pinMode(1, OUTPUT);
#endif

  DEBUG_MSG("\r\n\r\n\r\nConnecting to wifi\r\n");
  WiFi.begin(wifiCreds[0], wifiCreds[1]);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_MSG(".");
  }
  DEBUG_MSG("\r\nWiFi connected.\r\n");
  DEBUG_MSG(("access point: " + WiFi.SSID() + "\r\n").c_str());
  DEBUG_MSG("ip address: "); DEBUG_MSG(WiFi.localIP().toString().c_str()); DEBUG_MSG("\r\n");

  DEBUG_MSG("\r\nReady for interwebs action!\r\n");

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



