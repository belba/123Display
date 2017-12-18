/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define OLED_RESET 4 // not used / nicht genutzt bei diesem Display

Adafruit_SSD1306 display(OLED_RESET);
SoftwareSerial BMSserial(8,9);
extern HardwareSerial Serial;

int ledPin = 7;

const int msgBuffSize = 58;      // max. Zeichen im "Telegramm"
int msgBuffer[msgBuffSize];
int msgBufferIndex = 0;
float Batteryvoltage = 0;
char Current3signHEX = 0;
float Current3 = 0;
int SOC = 0;

void checkForNewData() {
  digitalWrite(ledPin, HIGH);
  delay(10);
  digitalWrite(ledPin, LOW);
  BMSserial.listen();
  msgBufferIndex = 0;
  while (msgBufferIndex <= msgBuffSize-1){
    if (BMSserial.available()){
      msgBuffer[msgBufferIndex] = BMSserial.read();
      msgBufferIndex++;
    }
  }
  BMSserial.stopListening();
}

boolean checkChecksum(){
  // create own checksum
  int checksum = 0;
  for (int count=0; count <= msgBuffSize-2; count++){
    checksum = checksum + msgBuffer[count];
  }
  checksum = checksum & 0xFF;
  if ( checksum == msgBuffer[57]) {
    Serial.println ("Checksum is OK");
    return true;
  }else {
    Serial.println ("Checksum failed");
    return false;
  }
}

void decBatteryvoltage(){
  int BatteryvoltageHEX = 0;
  byte byte1 = msgBuffer[0];
  byte byte2 = msgBuffer[1];
  byte byte3 = msgBuffer[2];
  BatteryvoltageHEX = (byte1<<16) | (byte2<<8)| byte3;
  Batteryvoltage = BatteryvoltageHEX * 0.005;
}

void decCurrentsensor3(){
  byte byte9 =msgBuffer[9];
  byte byte10 =msgBuffer[10];
  byte byte11 =msgBuffer[11];
  int Current3HEX = (byte10<<8)| byte11;
  Current3signHEX = byte9;
  Current3 = Current3HEX * 0.125;
}

void decSOC(){
  SOC = msgBuffer[40];
}

void print2LCDstatus(){

    display.clearDisplay();

    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("Voltage: ");
    display.setCursor(54,0);
    display.print(Batteryvoltage);
    display.print(" V");

    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(0,9);
    display.println("SOC: ");
    display.setCursor(60,9);
    display.print(SOC);
    display.print(" %");

    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,25);
    display.println("Current: ");
    display.setCursor(54,25);
    display.print(Current3signHEX);
    display.print(Current3);
    display.print(" A");

    display.display();
}

void print2LCDChkErr(){

    display.clearDisplay();

    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(0,0);
    display.println("Checksum");
    display.println("Error...");

    display.display();
}


void setup()   {                
  // initialize with the I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  BMSserial.begin(9600);
  pinMode(ledPin, OUTPUT);
}

void loop() {
    checkForNewData();
    if (checkChecksum()){
       decBatteryvoltage();
       decCurrentsensor3();
       decSOC();
       print2LCDstatus();
    }else{
       print2LCDChkErr();
    }
}

