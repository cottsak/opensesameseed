#include <SimpleTimer.h>      // https://github.com/infomaniac50/SimpleTimer
#include <ESP8266WiFi.h>      // https://github.com/esp8266/Arduino
#include <WiFiClientSecure.h> // https://github.com/esp8266/Arduino
#include <Base64.h>           // https://github.com/adamvr/arduino-base64

char twilioCreds[] = "sid:token";

double openForTooLongInMins = 0.1;
int doorOpenedAtTimeInMills = 0;
int doorOpenDurationInSeconds = 0;
bool messageSentInThisOpening = false;
const int doorOpen = HIGH;
const int doorClosed = LOW;

const int inputPinForDoor = 2;

SimpleTimer timer;
WiFiClientSecure httpsClient; // putting here coz crash when it's inside a method scope

void setup() {
  Serial.begin(115200);   // for debugging

  Serial.print("Connecting to wifi");
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
    Serial.println("Sent SMS: " + messageToSend);
    messageSentInThisOpening = true;
  }
}

void resetDoorOpenCounter() {
  doorOpenDurationInSeconds = 0;
  messageSentInThisOpening = false; 
}

void sendSms(String message) {
  Serial.println("making POST request to Twilio for sending sms..");

  const char* twilioApiHost = "api.twilio.com";
  const char* twilioApiHostCertSha1 = "B2 CC A2 09 87 C2 4E EB F7 C1 F4 14 0F 49 BE C0 91 EB 50 4F";

  int inputLen = sizeof(twilioCreds);
  int encodedLen = base64_enc_len(inputLen);
  char encodedCreds[encodedLen]; 
  // note input is consumed in this step: it will be empty afterwards
  base64_encode(encodedCreds, twilioCreds, inputLen); 
  
  
  if (!httpsClient.connect(twilioApiHost, 443)) {
    Serial.println("connection failed.");
    return;
  }
  if (!httpsClient.verify(twilioApiHostCertSha1, twilioApiHost)) {
    Serial.println("certificate doesn't match. will not send message.");
    return;
  }
  
  String request = String("POST ") + "/2010-04-01/Accounts/ACbeb21044b7f420061aaf958051deed85/Messages.json" + " HTTP/1.1\r\n" +
    "Host: " + twilioApiHost + "\r\n" +
    "User-Agent: ESP8266\r\n" +
    "Authorization: Basic " + encodedCreds + " \r\n" +
    "Content-Type: application/x-www-form-urlencoded\r\n" +
    "Connection: close\r\n\r\n" +
    //escapeParameter("To=+61422601983&From=+12015524973&Body=" + message) + "\r\n";
    "To=%2B61422601983&From=%2B12015524973&Body=oh%20yeh!%20from%20esp.\r\n";
  httpsClient.print(request);
  Serial.println("request sent:");
  Serial.println(request);

  String responseString = httpsClient.readString();
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(responseString);
  Serial.println("==========");
  Serial.println("closing connection");  
}

String escapeParameter(String param) {
  param.replace("+"," ");
  param.replace("%21","!");
  param.replace("%23","#");
  param.replace("%24","$");
  param.replace("%26","&");
  param.replace("%27","'");
  param.replace("%28","(");
  param.replace("%29",")");
  param.replace("%2A","*");
  param.replace("%2B","+");
  param.replace("%2C",",");
  param.replace("%2F","/");
  param.replace("%3A",":");
  param.replace("%3B",";");
  param.replace("%3D","=");
  param.replace("%3F","?");
  param.replace("%40","@");
  param.replace("%5B","[");
  param.replace("%5D","]");
  
  return param;
}

